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
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>

#include "auto-config.h"
#include "log.h"

char *
format_date(char *buf)
{
    struct tm *l;
    time_t t = time(NULL);

    l = localtime(&t);
    sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d",
         l->tm_mon+1, l->tm_mday, l->tm_year+1900, l->tm_hour, l->tm_min,
         l->tm_sec);
    return buf;
}

void LOGERR (const char *err, ... )
{
    char buf[1024];
    va_list args;

    va_start (args, err);
    vsnprintf (buf, sizeof(buf), err, args);
    va_end (args);

    LOG(buf);
    OUTPUT(buf);
}

void LOG (const char *text, ... )
{
    char date[80];
    char buf[1024];
    va_list args;

    va_start (args, text);
    vsnprintf (buf, sizeof(buf), text, args);
    va_end (args);

    fprintf(stderr, "%d: [%s] %s\n", getpid(), format_date(date), buf);
}

void OUTPUT (const char *text, ... )
{
    char buf[1024];
    va_list args;

    va_start (args, text);
    vsnprintf (buf, sizeof(buf), text, args);
    va_end (args);

    printf("Content-Type: text/plain\n\n%s\n", buf);
}

