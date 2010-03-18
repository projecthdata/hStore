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

#include "auto-config.h"
#include "util.h"
#include "log.h"
#include "config.h"

hstore_config_t hstore_config_create(void) {
    hstore_config_t config = calloc(1, sizeof(struct hstore_config));
    return config;
}

int hstore_config_read(hstore_config_t config, const char *filename) {
    hstore_option_t last_option = NULL;
    hstore_plugin_t last_plugin = NULL;
    char buf[128];
    FILE *file;
    char *fn;

    if (!config)
        return EINVAL;

    if (!filename) 
        fn = "hstore.conf";
    else
        fn = (char *) filename;

    file = fopen(fn, "r");
    if (!file) 
        return errno;

    while((fgets(buf, sizeof(buf), file))!=NULL) {
        char *property, *value;
        if (buf[0] == '#' || buf[0] == ';' || buf[0] == '\n')
            continue;
        hstore_chomp(buf);

        property = strtok(buf, " \t");
        if (!property)
            continue;

        value = strtok(NULL, " \t");
        if (!value) 
            continue;

        /* Process plugins */
        if (!strcasecmp(property, "Plugin")) {
            hstore_plugin_t plugin = calloc(1, sizeof(struct hstore_plugin));
            char *so_path, *symbol_name;
            
            so_path = strtok(NULL, " \t");
            if (!so_path) {
                free(plugin);
                continue;
            }

            symbol_name = strtok(NULL, " \t");
            if (!symbol_name) {
                free(plugin);
                continue;
            }

            plugin->request_method = strdup(value);
            plugin->so_path = strdup(so_path);
            plugin->symbol_name = strdup(symbol_name);

            LOG("%s adding plugin %s/%s/%s", __func__, plugin->request_method,
                plugin->so_path, plugin->symbol_name);

            if (last_plugin == NULL) {
                config->plugins = plugin;
                last_plugin = plugin;
            } else {
                last_plugin->next = plugin;
                last_plugin = plugin;
            }
        } else {
            hstore_option_t option = calloc(1, sizeof(struct hstore_option));
            if (!option)
                return errno;
            option->property = strdup(property);
            option->value = strdup(value);

            LOG("%s setting property %s => %s", __func__, option->property,
                option->value);
           
            if (last_option == NULL) {
                config->options = option;
                last_option = option;
            } else {
                last_option->next = option;
                last_option = option;
            }
        }
    }
    fclose(file);

    return 0;    
}

void hstore_config_destroy(hstore_config_t config) {
    hstore_option_t option;

    if (!config)
        return;

    option = config->options;
    while(option) {
        hstore_option_t next = option->next;

        if (!strcasecmp(option->property, "Plugin") && option->value) {
            hstore_plugin_t plugin = option->value;
            free(plugin->request_method);
            free(plugin->so_path);
            free(plugin->symbol_name);
        } 

        free(option->property);
        free(option->value);
        option = next;
    }

    free(config);
    return;
}

/* Find the hstore_option_t associated with the given property */

hstore_option_t hstore_option_find(
    hstore_config_t config,
    const char *property)
{
    hstore_option_t option;

    if (!config || !property)
        return NULL;
    option = config->options;
    while(option && strcasecmp(option->property, property)) {
        option = option->next;
    }
    return option;
}   

/* Return the value for the given property as text */

char *hstore_option_findtext(
    hstore_config_t config, 
    const char *property)
{
    hstore_option_t option = hstore_option_find(config, property);
    if (option)
        return option->value;
    return NULL;
}

