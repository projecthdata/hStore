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
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "auto-config.h"
#include "hstore.h"
#include "config.h"
#include "log.h"
#include "util.h"

int hStoreLogger(hstore_context_t ctx);
int hStoreDocDelete(hstore_context_t ctx);
int hStoreDocReader(hstore_context_t ctx);
int hStoreDocWriter(hstore_context_t ctx);

int hStoreLogger(hstore_context_t ctx) {
    char *logfile = hstore_option_findtext(ctx->config, "LogFile");
    FILE *file;
    char date[80];

    if (logfile == NULL) {
        LOGERR("%s logfile %s does not exist", __func__, logfile); 
        return EINVAL;
    }

    file = fopen(logfile, "a");
    if (!file) {
        LOGERR("%s could not open logfile %s for writing: %s", __func__, 
            logfile, strerror(errno));
        return errno;
    }

    fprintf(file, "%d: [%s] %s: %s %s\n", getpid(), hstore_format_date(date), 
        ctx->remote_addr, ctx->request_method, ctx->request_path);
    fclose(file);

    return 0;
}

int hStoreDocDelete(hstore_context_t ctx) {
    char *document_root = hstore_option_findtext(ctx->config, "DocumentRoot");
    char *document_path;

    if (document_root)
        asprintf(&document_path, "%s/%s", document_root, ctx->request_path);
    else
        document_path = strdup(ctx->request_path);

    unlink(document_path);

    OUTPUT("OK %s", ctx->request_path); 
    free(document_path);
    return 0;
}

int hStoreDocReader(hstore_context_t ctx) {
    char *document_root = hstore_option_findtext(ctx->config, "DocumentRoot");
    char *document_path;
    FILE *file;
    char buf[128];
    struct stat s_doc;

    if (document_root) {
        asprintf(&document_path, "%s/%s", document_root, ctx->request_path);
    } else {
        document_path = strdup(ctx->request_path);
    }
    stat(document_path, &s_doc);

    /* Dynamically write root document */
    if (!strcmp(ctx->request_path + strlen(ctx->request_path) - 9,
        "/root.xml") && hstore_path_depth(ctx->request_path) == 2)
    {
        struct stat s;
        char *dir = strdup(ctx->request_path);
        char *dirpath, *j;
        DIR *dirp;
        struct dirent *dirent;
        char date[80];

        hstore_format_date(date);
        j = strchr(date, ' ');
        if (j) 
            j[0] = 0; 

        dir[strlen(dir)-8] = 0;

        if (document_root)
            asprintf(&dirpath, "%s/%s", document_root, dir);
        else
            dirpath = strdup(dir);

        /* Begin output */
        printf("Content-Type: application/xml\n\n");
        printf(
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<root xmlns=\"http://projecthdata.org/hdata/schemas/2009/06/core\""
" xmlns:xsi=\"http://www.w2.org/2001/XMLSchema-instance\">\n"
"    <documentId>rootid0</documentId>\n"
"    <version>%1.1f</version>\n"
"    <created>%s</created>\n"
"    <lastModified>%s</lastModified>\n"
"    <sections>\n",
    HRF_VERSION, date, date);

        LOG("%s scanning %s for section directories", __func__, dirpath);

        dirp = opendir(dirpath);
        while( ( dirent = readdir(dirp) ) != NULL) {
            char *directory_path;
            if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
                continue;
            asprintf(&directory_path, "%s/%s", dirpath, dirent->d_name);
            if (stat(directory_path, &s)==0) {
                if (s.st_mode & S_IFDIR) {
                    LOG("%s found directory %s at %s", __func__, dirent->d_name,
                        directory_path);
                    printf(
                    "        <section typeId=\"\" path=\"%s\" name=\"\"/>\n", 
                        dirent->d_name);
                }
            } else {
                LOG("%s stat failed for %s; ignoring.", __func__, 
                    directory_path);
            }

            free(directory_path);
        }

        free(dirpath);
        printf("    </sections>\n");
        printf("</root>\n");

    /* Generate ATOM feed of directories */
    } else if (s_doc.st_mode & S_IFDIR) {
        DIR *dirp;
        struct dirent *dirent;
        char date[80], last_updated[80];
        struct tm *l;
        l = localtime(&s_doc.st_ctime);
        
        hstore_format_date(date);
        snprintf(last_updated, sizeof(last_updated), "%04d-%02d-%02d %02d:%02d",
            l->tm_year+1900, l->tm_mon+1, l->tm_mday, l->tm_hour, l->tm_min);

        printf("Content-Type: application/xml\n\n");
        printf(
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
"<feed xmlns=\"http://www.w3.org/2005/Atom\">\n"
"<title>%s</title>\n"
"<link href=\"%s\" rel=\"self\" />\n"
"<updated>%s</updated>\n"
, ctx->request_path, ctx->request_path, last_updated);

        LOG("%s generating ATOM feed for %s", __func__, document_path);
        dirp = opendir(document_path);
        while( ( dirent = readdir(dirp) ) != NULL) {
            struct stat s;
            char file_updated[80];
            char *fn;

            asprintf(&fn, "%s/%s", document_path, dirent->d_name);
            stat(fn, &s);
            struct tm *lf;
            lf = localtime(&s.st_mtime);
            snprintf(file_updated, sizeof(last_updated), 
                "%04d-%02d-%02d %02d:%02d", l->tm_year+1900, l->tm_mon+1, 
                l->tm_mday, l->tm_hour, l->tm_min);

            free(fn);

            if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
                continue;
            printf(
"<entry>\n"
"    <title>%s</title>\n"
"    <link href=\"%s/%s\" />\n"
"    <updated>%s</updated>\n"
"</entry>\n"
, dirent->d_name, ctx->request_path, dirent->d_name, file_updated);

        }

        printf("</feed>\n");

    /* Fetch all oher files */
    } else {
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
    }

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
