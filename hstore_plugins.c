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
#include "hstore.h"
#include "config.h"
#include "log.h"

int hStoreDocDelete(hstore_context_t ctx) {
    char *document_root = hstore_option_findtext(ctx->config, "DocumentRoot");
    char *document_path;

    if (document_root)
        asprintf(&document_path, "%s/%s", document_root, ctx->request_path);
    else
        document_path = strdup(ctx->request_path);

    unlink(document_path);

    LOG("OK %s", ctx->request_path); 
    free(document_path);
    return 0;
}

int hStoreDocReader(hstore_context_t ctx) {
    char *document_root = hstore_option_findtext(ctx->config, "DocumentRoot");
    char *document_path;
    FILE *file;
    char buf[128];

    if (document_root)
        asprintf(&document_path, "%s/%s", document_root, ctx->request_path);
    else
        document_path = strdup(ctx->request_path);

    file = fopen(document_path, "r");
    if (!file) {
        LOGERR("%s could not open file for reading: %s", __func__, 
            strerror(errno));
        return errno;
    }

    printf("Content-Type: application/xml\n\n");
    while((fgets(buf, sizeof(buf), file))!=NULL) {
        fprintf(stdout, "%s", buf);
    }
    fclose(file);

    free(document_path);
    return 0;
}

int hStoreDocWriter(hstore_context_t ctx) {
    char *document_root = hstore_option_findtext(ctx->config, "DocumentRoot");
    char *content_length = getenv("CONTENT_LENGTH");
    char *document_path;
    FILE *file;
    char *buf;
    size_t len;

    if (!content_length) {
        LOGERR("%s no content length specified", __func__);
        return EINVAL;
    }

    len = atoi(content_length);
    buf = calloc(1, len+1);
    if (!buf) {
        LOGERR("%s could not alloc memory for content: %s", __func__, 
            strerror(errno));
        return errno;
    }

    if (!(fread(buf, 1, len, stdin))) {
        LOGERR("%s couldn't read from stdin: %s", __func__, strerror(errno));
        return errno;
    }

    if (document_root)
        asprintf(&document_path, "%s/%s", document_root, ctx->request_path);
    else
        document_path = strdup(ctx->request_path);

    /* Create underlying directory structure, if necessary */
    {
        char *path = strdup(document_path);
        char *dir = strtok(path, "/");
        char *next = strtok(NULL, "/");
        char p[strlen(document_path)+1];

        p[0] = 0;
        while(next) {
            struct stat s;

            strlcat(p, "/", sizeof(p));
            strlcat(p, dir, sizeof(p));
            if ((stat(p, &s))!=0) {
                mkdir(p, 0777);
            }
            dir = next;
            next = strtok(NULL, "/");
        }
   
        free(path);
    }

    /* Don't allow overwrites using POST */
    if (!strcasecmp(ctx->request_method, "POST")) {
        struct stat s;
        if ((stat(document_path, &s))==0) {
            LOGERR("%s document exists", __func__);
            free(document_path);
            return EEXIST;
        } 
    }

    file = fopen(document_path, "w");
    if (!file) {
        LOGERR("%s could not open file for writing: %s", __func__, 
            strerror(errno));
        free(document_path);
        return errno;
    }

    if (!(fwrite(buf, len, 1, file))) {
        fclose(file);
        LOGERR("%s write failed: %s", __func__, strerror(errno));
        free(document_path);
        if (errno)
            return errno;
        return EIO;
    }
    fclose(file);

    OUTPUT("OK %s", ctx->request_path);
    free(document_path);
    return 0;
}
