/*
 HSTORE SERVER
 COPYRIGHT (C) 2010 BY MITRE CORPORATION
 ALL RIGHTS RESERVED

 USE OR DUPLICATION OF THIS INTELLECTUAL PROPERTY HEREIN WITHOUT WRITTEN
 AUTHORIZATION IS STRICTLY PROHIBITED. UNAUTHORIZED USE OR DUPLICATION MAY
 RESULT IN CRIMINAL PROSECUTION TO THE FULLEST EXTENT OF THE LAW.
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct hstore_option {
    char *property;
    void *value;
    struct hstore_option *next;
} *hstore_option_t;

typedef struct hstore_plugin {
    char *request_method;
    char *so_path;
    char *symbol_name;
    struct hstore_plugin *next;
} *hstore_plugin_t;

typedef struct hstore_config {
    hstore_option_t options;
    hstore_plugin_t plugins;
} *hstore_config_t;

hstore_config_t hstore_config_create(void);
int hstore_config_read(hstore_config_t config, const char *filename);
void hstore_config_destroy(hstore_config_t config);

char *hstore_option_findtext(hstore_config_t config, const char *property);

#endif /* _CONFIG_H */
