/* 
 HSTORE SERVER
 COPYRIGHT (C) 2010 BY MITRE CORPORATION
 ALL RIGHTS RESERVED

 USE OR DUPLICATION OF THIS INTELLECTUAL PROPERTY HEREIN WITHOUT WRITTEN
 AUTHORIZATION IS STRICTLY PROHIBITED. UNAUTHORIZED USE OR DUPLICATION MAY
 RESULT IN CRIMINAL PROSECUTION TO THE FULLEST EXTENT OF THE LAW.
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>

#include "auto-config.h"
#include "util.h"

void hstore_chomp(char *buf) {
  size_t len = strlen(buf);
  if (buf[len-1] == 10) {
    buf[len-1] = 0;
    len--;
  }
  if (buf[len-1] == 13) {
    buf[len-1] = 0;
  }
  return;
}

#ifndef HAVE_STRL
/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat(dst, src, siz)
        char *dst;
        const char *src;
        size_t siz;
{
        register char *d = dst;
        register const char *s = src;
        register size_t n = siz;
        size_t dlen;

        /* Find the end of dst and adjust bytes left but don't go past end */
        while (n-- != 0 && *d != '\0')
                d++;
        dlen = d - dst;
        n = siz - dlen;

        if (n == 0)
                return(dlen + strlen(s));
        while (*s != '\0') {
                if (n != 1) {
                        *d++ = *s;
                        n--;
                }
                s++;
        }
        *d = '\0';

        return(dlen + (s - src));       /* count does not include NUL */
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(dst, src, siz)
        char *dst;
        const char *src;
        size_t siz;
{
        register char *d = dst;
        register const char *s = src;
        register size_t n = siz;

        /* Copy as many bytes as will fit */
        if (n != 0 && --n != 0) {
                do {
                        if ((*d++ = *s++) == 0)
                                break;
                } while (--n != 0);
        }

        /* Not enough room in dst, add NUL and traverse rest of src */
        if (n == 0) {
                if (siz != 0)
                        *d = '\0';              /* NUL-terminate dst */
                while (*s++)
                        ;
        }

        return(s - src - 1);    /* count does not include NUL */
}
#endif

char *
hstore_format_date(char *buf)
{
    struct tm *l;
    time_t t = time(NULL);

    l = localtime(&t);
    sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d",
         l->tm_mon+1, l->tm_mday, l->tm_year+1900, l->tm_hour, l->tm_min,
         l->tm_sec);
    return buf;
}

int
hstore_path_depth(char *buf)
{
    int depth = 0;
    char *p, *str;

    if (buf == NULL)
        return 0;

    str = strdup(buf);
    p = strtok(str, "/");
    while(p) {
        if (!strcmp(p, ".."))
            depth--;
        else if (strcmp(p, ".")) 
            depth++;
        p = strtok(NULL, "/");
    }
            
    free(str);
    return depth;
}
