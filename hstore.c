/*
 HSTORE SERVER
 COPYRIGHT (C) 2010 BY MITRE CORPORATION
 ALL RIGHTS RESERVED

 USE OR DUPLICATION OF THIS INTELLECTUAL PROPERTY HEREIN WITHOUT WRITTEN
 AUTHORIZATION IS STRICTLY PROHIBITED. UNAUTHORIZED USE OR DUPLICATION MAY
 RESULT IN CRIMINAL PROSECUTION TO THE FULLEST EXTENT OF THE LAW.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "auto-config.h"
#include "hstore.h"
#include "util.h"

hstore_context_t hstore_context_create(void);
int hstore_context_init(hstore_context_t ctx, int argc, char *argv[]);
void hstore_context_destroy(hstore_context_t ctx);

int process_request(hstore_context_t ctx);

int main(int argc, char *argv[]) {
    hstore_context_t ctx = hstore_context_create();
    hstore_config_t config = hstore_config_create();
    int r;
   
    if (!ctx) {
        LOGERR("Unable to create hStore context: %s", strerror(errno)); 
        return errno;
    }

    if ( (r = hstore_context_init(ctx, argc, argv)) ) {
        LOGERR("Unable to initialize hStore context: %d/%s", r, 
            strerror(errno));
        hstore_context_destroy(ctx);
        return r;
    }

    if (!config) {
        LOGERR("Unable to create hStore config: %s", strerror(errno));
        hstore_context_destroy(ctx);
        return errno;
    }

    if ( (r = hstore_config_read(config, NULL)) ) {
        LOGERR("Unable to read hStore configuration: %d/%s", r,
            strerror(errno));
        hstore_context_destroy(ctx);
        hstore_config_destroy(config);
        return r;
    }

    ctx->config = config;
    process_request(ctx);

    hstore_context_destroy(ctx);
    hstore_config_destroy(config);
    return EXIT_SUCCESS;
}

/* hStore context functions */

hstore_context_t hstore_context_create(void) {
    hstore_context_t ctx = calloc(1, sizeof(struct hstore_context));
    return ctx;
}

void hstore_context_destroy(hstore_context_t ctx) {
    if (!ctx)
        return;

    free(ctx->request_path);
    free(ctx->request_method);
    free(ctx->remote_addr);
    free(ctx);
}

int hstore_context_init(hstore_context_t ctx, int argc, char *argv[]) {
    char *env;

    if (!ctx) {
        LOG("%s no hstore context", __func__);
        return EINVAL;
    }

    env = getenv("REQUEST_URI");
    if (env) {
        char *scriptname = getenv("SCRIPT_NAME");
        if (scriptname) {
            size_t len = strlen(scriptname);

            if (!strncmp(env, scriptname, len)) {
                size_t len = strlen(scriptname);
                env += len;
            } 
        }
        ctx->request_path = strdup(env);
    }

    env = getenv("REMOTE_ADDR");
    if (env)
        ctx->remote_addr = strdup(env);

    env = getenv("REQUEST_METHOD");
    if (env)
        ctx->request_method = strdup(env);
    else {
        LOG("%s no REQUEST_METHOD set", __func__);
        return EINVAL;
    }

    return 0;
}

/* Main plugin architecture loop */

int process_request(hstore_context_t ctx) {
    hstore_plugin_t plugin = ctx->config->plugins;
    int r = 0;

    while(plugin) {
        /* Fire all of the plugins using the given request method */
        if (plugin->request_method[0] == '*' 
            || !strcasecmp(plugin->request_method, ctx->request_method)) 
        {
            void *handle;
            int (*ptr)(hstore_context_t);

            LOG("%s firing plugin %s/%s/%s", __func__, plugin->request_method,
                plugin->so_path, plugin->symbol_name);
            handle = dlopen(plugin->so_path, RTLD_NOW);
            if (!handle) {
                LOG("%s failed to dlopen(%s): %s", __func__, plugin->so_path, 
                    strerror(errno));
                plugin = plugin->next;
                continue;
            }

            ptr = (int (*)(hstore_context_t))dlsym(handle, plugin->symbol_name); 
            if (!ptr) {
                LOG("%s failed to dlsym(%s/%s): %s", __func__, plugin->so_path,
                    plugin->symbol_name, strerror(errno));
            } else {
                r = (*ptr)(ctx);
                if (r) {
                    LOG("%s plugin(%s/%s) failed with error %d", __func__,
                        plugin->so_path, plugin->symbol_name, r);
                }
            }
            dlclose(handle);

            if (r) 
                break;
        }
        plugin = plugin->next;
    }
    return r;
}

