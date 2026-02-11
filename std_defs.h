/*
static char sccs_id[] = "@(#)std_defs.h	5.5  9/1/88";
*/

/*
 * Copyright 1988 by Siemens Research and Technology Laboratories, Princeton, NJ
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Siemens Research and Technology
 * Laboratories not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * SIEMENS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * SIEMENS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*  Base macro definitions for the Tiling Window Manager.
 *
 *  ONLY general macros go here - general type definitions go in basetype.h!
 */

#ifndef STD_DEFS

#define STD_DEFS


#ifdef bool
#undef bool
#endif

typedef int	bool;

typedef char	*pointer;
typedef void	(*VoidFunc)();

/*	initial pointer value
 */
#ifndef  NULL
#define  NULL		0
#endif

/*	values for "bool" type
 */
#ifndef  TRUE
#define  TRUE		1
#endif

#ifndef  FALSE
#define  FALSE		0
#endif

#define  allocate(type, quan)	( (type *) malloc(sizeof(type) * (quan)) )

#define  FOREVER		for (;;)


/*  string comparison macros
 */
#define streql(str1,str2)  \
    ( ( ( (str1) == NULL ) || ( str1[0] == '\0' ) ) ?  \
        ( ( (str2) == NULL ) || ( str2[0] == '\0' ) ) :  \
        ( ( (str2) != NULL ) && ( strcmp(str1,str2) == 0 ) ) )

#define str_copy(str1,str2)  \
{  \
    if ( (str2) == NULL )  \
        (str1) = NULL;  \
    else  \
    {  \
        (str1) = allocate( char, 1 + strlen(str2) );  \
        strcpy( str1, str2 );  \
    }  \
}

#define str_replace(str1,str2)  \
{  \
    if ( (str1) != NULL )  \
        free( str1 );  \
    str_copy( str1, str2 )  \
}

#define str_length(str) ((str)? strlen(str):0)

#define str_free(str) \
{ \
      if (str) \
	  free((char *) str); \
}

/*	minimal error status return values
 */
#define  SUCCESS	 0
#define  FAILURE	-1


/*  value comparison macros
 */
#define ABS(val)  \
    ( ( (val) < 0 ) ? -(val) : (val) ) 

#define MIN(val1,val2)  \
    ( ( (val1) < (val2) ) ? (val1) : (val2) )

#define MAX(val1,val2)  \
    ( ( (val1) > (val2) ) ? (val1) : (val2) )

#define Is_Even_Number(number)    ( ((number % 2) == 0) ? TRUE : FALSE )
#define Is_Odd_Number(number)     ( ((number % 2) == 0) ? FALSE : TRUE )

#include <X11/Xos.h>

#endif /* STD_DEFS */
