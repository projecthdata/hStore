/*
 HSTORE SERVER
 COPYRIGHT (C) 2010 BY MITRE CORPORATION
 ALL RIGHTS RESERVED

 USE OR DUPLICATION OF THIS INTELLECTUAL PROPERTY HEREIN WITHOUT WRITTEN
 AUTHORIZATION IS STRICTLY PROHIBITED. UNAUTHORIZED USE OR DUPLICATION MAY
 RESULT IN CRIMINAL PROSECUTION TO THE FULLEST EXTENT OF THE LAW.
*/

#include "config.h"

#ifndef _HSTORE_H_
#define _HSTORE_H_

typedef struct hstore_context {
    hstore_config_t config;
    char *request_path;
    char *request_method;
    char *remote_addr;
} *hstore_context_t;

void LOGERR(const char *err, ... );
void LOG (const char *text, ... );
void OUTPUT (const char *text, ... );

#endif /* _HSTORE_H */
