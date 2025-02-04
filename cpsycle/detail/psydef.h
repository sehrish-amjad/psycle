/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(psy_DEF_H)
#define psy_DEF_H

#include "psyconf.h"
#include "stdint.h"
#include <stddef.h>
#include "psyversion.h"
#include "os.h"

#ifndef __cplusplus
#ifdef DIVERSALIS__OS__POSIX
#include <stdbool.h>
#else
typedef uint8_t bool;
#endif
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef psy_min
#define psy_min(a, b) (((a < b) ? a : b))
#endif

#ifndef psy_max
#define psy_max(a, b)  (((a) > (b)) ? (a) : (b)) 
#endif

#ifndef psy_sgn
#define psy_sgn(x) ((x > 0) ? 1 : ((x < 0) ? -1 : 0))
#endif

/* psycle status */

#define PSY_OK 0
#define PSY_ERRFILE 1
#define PSY_ERRRUN	2
#define PSY_ERRFILEFORMAT 3
#define PSY_WARN 4

/*
** Invalid index. Used to indicate index is disabled, or an out of range index.
** BEWARE!!! UINTPTR_T, not minus one.
*/
#define psy_INDEX_INVALID UINTPTR_MAX

#ifndef INLINE
# if __GNUC__ && !__GNUC_STDC_INLINE__
#  define INLINE static __inline__
# elif __GNUC__
#  define INLINE static __inline__
# elif _MSC_VER
#  define INLINE __inline
# endif
#endif /* INLINE */

typedef void (*psy_fp_disposefunc)(void*);

typedef char char_dyn_t;

typedef struct psy_ui_PropertyMode {	
	bool set;
} psy_ui_PropertyMode;


typedef struct psy_RealPair {
	double first;
	double second;
} psy_RealPair;

INLINE psy_RealPair psy_realpair_make(double first, double second)
{
	psy_RealPair rv;

	rv.first = first;
	rv.second = second;
	return rv;
}

typedef enum psy_Encoding {
	PSY_ENCODING_NONE,
	PSY_ENCODING_UTF8,
	PSY_ENCODING_LATIN
} psy_Encoding;

#if _MSC_VER
#define psy_SLASHSTR "\\"
#define psy_SLASH '\\'
#define psy_MAX_PATH 260
#else
#define psy_SLASHSTR  "/"
#define psy_SLASH '/'
#define psy_MAX_PATH 4096
#endif

#endif
