/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PORTABLE_H)
#define PORTABLE_H

#include "psydef.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/*
** Windows stuff
*/
#if defined(_WIN32) 	/* { */

/* #define _CRTDBG_MAP_ALLOC
   #include <crtdbg.h> */

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#if !defined(_CRT_NONSTDC_NO_WARNINGS)
#define _CRT_NONSTDC_NO_WARNINGS
#endif

#endif			/* } */

INLINE char* psy_strdup(const char* str)
{
	if (str) {
		return strdup(str);
	}
	return NULL;
}

INLINE uintptr_t psy_strlen(const char* str)
{	
	return (str)
		? strlen(str)
		: 0;
}

INLINE char* psy_strreset(char** dest, const char* text)
{
	assert(dest);

	if (*dest == NULL) {
		*dest = psy_strdup(text);
	} else if (text != *dest) {		
		char* temp;

		temp = psy_strdup(text);
		free(*dest);
		*dest = temp;
	}
	return *dest;
}

INLINE char* psy_strcat_realloc(char* str1, const char* str2)
{
	if (!str2) {
		return str1;
	} else if (!str1) {
		return psy_strdup(str2);
	} else {	
		char* rv;
		char* resize;
		uintptr_t str1_len;
		uintptr_t str2_len;

		str1_len = strlen(str1);
		str2_len = strlen(str2);
		resize = (char*)realloc(str1, str1_len + str2_len + 1);
		if (!resize) {
			return NULL;
		}
		rv = resize;
		memcpy(rv + str1_len, str2, str2_len + 1);
		return rv;
	}
}

INLINE void psy_snprintf(char* buf, size_t buflen, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4996 )
	if (buflen > 0) {
		_vsnprintf(buf, buflen, fmt, ap);
		buf[buflen - 1] = '\0';
	}
#pragma warning( pop )
#else
	vsnprintf(buf, buflen, fmt, ap);	
#endif	
	va_end(ap);
}

INLINE char* psy_replacechar(char* str, char c, char r)
{
	char* p;
		
	for (p = strchr(str, c); p != 0; p = strchr(p + 1, c)) *p = r;
	return p;
}

/*-------------------------------------------------------------------------
   psy_strlwr is included from https://sourceforge.net/projects/sdcc/
   strlwr.c - part of string library functions

   Written by Vangelis Rokas, 2004 <vrokas AT otenet.gr>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/
INLINE char* psy_strlwr(char* str)
{
    char* ret = str;

    while (*str) {
        *str = tolower(*str);
        str++;
    }

    return (ret);
}


/*
** converts dos ascii to utf8
** NULL allcoates memory or
** out needs allocated memory of strlen(in) * 2 + 1
*/

INLINE char* psy_dos_to_utf8(char *in, char *out)
{
	char* p;
	char* q;
		
	if (!in) {
		return NULL;
	}
	if (out == NULL) {		
		out = (char*)malloc(sizeof(char) * psy_strlen(in) * 2 + 1);
	}
	if (!out) {
		return NULL;
	}
	q = out;	
    for (p = in; (*p) != '\0'; p++) {
        uint8_t ch = *p;
        
        if (ch < 0x80) {
			*q = ch;
            ++q;
        } else {
			*q = (0xc0 | (ch >> 6));
			q++;
            *q = (0x80 | (ch & 0x3f));
            q++;
        }
    }
    *q = '\0';
    return out;
}


#endif /* PORTABLE_H */
