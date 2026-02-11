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
#include "copyright.h"

#include <stdio.h>

#include "basetype.h"

#include "draw.h"

#include "twinD.h"
#include "twin.h"
#include "uwinD.h"

extern FILE* Open_Read();
extern FILE* Open_Write();

typedef pointer Layoutp;
#define LAYOUT_NULL (Layout *)NULL

/* ------------------------------------------------------------ */

#define strnullcpy( str1, str2 )  \
{  \
    str2[strlen(str2)-1] = '\0';  \
    strcpy( str1, str2 );  \
}

/* ------------------------------------------------------------ */

int unmatched_layouts = 0;

static char hoststr[128];
static char preference_str[128];
static char *preference_file;
static FILE *layout_stream;

/* ------------------------------------------------------------ */

typedef struct layout Layout;

struct layout
{
    Layout *next;
    bool found;

    char res_class[128];
    char res_name[128];
    char hostname[128];
    char identity[128];
    char command[128];

    bool open;
    Size siz;
    TWPoint center;
    TWPoint icon_center;

    bool zoomed;
    Size regular_size;
    Size regular_min;
    Size zoom_size;
    Size zoom_min;
    Size max_size;
    
    TWRectangle rezoom_rect;
    
    bool positioned;

    bool explicit_fixed_completely;
    bool explicit_fixed_height;
    bool explicit_fixed_width;
    bool explicit_require_manual_close;
    bool explicit_listener_tied_to_zoom;
    bool explicit_zoom_tied_to_desire;

    bool fixed_completely;
    bool fixed_height;
    bool fixed_width;
    bool require_manual_close;
    bool listener_tied_to_zoom;
    bool zoom_tied_to_desire;
};

static Layout *layout_head, *layout_tail;
                                                                                        
/* ------------------------------------------------------------ */

static void Layout_Scan_Values( layoutp, str )
    char *str;
    Layout *layoutp;
{
    sscanf( str,
        "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
        &(layoutp->open),
        &(layoutp->siz.width),
        &(layoutp->siz.height),
        &(layoutp->center.x),
        &(layoutp->center.y),
        &(layoutp->icon_center.x),
        &(layoutp->icon_center.y),
        &(layoutp->zoomed),
        &(layoutp->regular_size.width),
        &(layoutp->regular_size.height),
        &(layoutp->regular_min.width),
        &(layoutp->regular_min.height),
        &(layoutp->zoom_size.width),
        &(layoutp->zoom_size.height),
        &(layoutp->zoom_min.width),
        &(layoutp->zoom_min.height),
        &(layoutp->max_size.width),
        &(layoutp->max_size.height),
        &(layoutp->rezoom_rect.left_x),
        &(layoutp->rezoom_rect.right_x),
        &(layoutp->rezoom_rect.top_y),
        &(layoutp->rezoom_rect.bottom_y),
        &(layoutp->positioned) );
}

/* ------------------------------------------------------------ */

static void Layout_Scan_Options( layoutp, str )
    char *str;
    Layout *layoutp;
{
    int scanned;

    scanned = sscanf( str,
        "%d %d %d %d %d %d %d %d %d %d %d %d\n",
        &(layoutp->explicit_require_manual_close),
        &(layoutp->require_manual_close),
        &(layoutp->explicit_fixed_width),
        &(layoutp->fixed_width),
        &(layoutp->explicit_fixed_height),
        &(layoutp->fixed_height),
        &(layoutp->explicit_fixed_completely),
        &(layoutp->fixed_completely),
        &(layoutp->explicit_listener_tied_to_zoom),
        &(layoutp->listener_tied_to_zoom),
        &(layoutp->explicit_zoom_tied_to_desire),
        &(layoutp->zoom_tied_to_desire) );

    if ( scanned < 12 )  /* old format */
    {
        layoutp->explicit_zoom_tied_to_desire = FALSE;
        sscanf( str,
            "%d %d %d %d %d %d %d %d %d %d\n",
            &(layoutp->explicit_listener_tied_to_zoom),
            &(layoutp->explicit_fixed_completely),
            &(layoutp->explicit_fixed_height),
            &(layoutp->explicit_fixed_width),
            &(layoutp->explicit_require_manual_close),
            &(layoutp->listener_tied_to_zoom),
            &(layoutp->fixed_completely),
            &(layoutp->fixed_height),
            &(layoutp->fixed_width),
            &(layoutp->require_manual_close) );
    }        
}

/* ------------------------------------------------------------ */

void Layout_Update( filnam )
    char *filnam;
{
    if ( filnam == NULL )
        preference_file = NULL;
    else
    {
        (void) strcpy( preference_str, filnam );
        preference_file = preference_str;
    }
}

/* ------------------------------------------------------------ */

char *Layout_Init()
{
    char str[256];

    gethostname( hoststr, 128 );

    preference_file = NULL;

    layout_tail = layout_head = LAYOUT_NULL;

    layout_stream = Open_Read( ".rtl.layout" );
    if ( layout_stream == NULL )
        return NULL;

    fgets( str, 128, layout_stream );

    fgets( str, 128, layout_stream );
    sscanf( str, "%s", preference_str );
    if ( strlen(preference_str) != 0 )
        preference_file = preference_str;

    return preference_file;
}

/* ------------------------------------------------------------ */

void Layout_Read()
{
    char str[256];
    Layout *layoutp;
    int wid, ht;

    if ( layout_stream == NULL )
        return;

    fgets( str, 128, layout_stream );
    if ( sscanf( str, "%d %d", &wid, &ht ) == 2 )
    {
        UserGlobals_Set_Value( RTL_WIDTH, wid );
        UserGlobals_Set_Value( RTL_HEIGHT, ht );
    }

    while ( fgets( str, 256, layout_stream ) != NULL )
    {
        unmatched_layouts++;
        layoutp = allocate( Layout, 1 );
        layoutp->found = FALSE;

        fgets( str, 256, layout_stream );
        strnullcpy( layoutp->res_class, str );

        fgets( str, 256, layout_stream );
        strnullcpy( layoutp->res_name, str );

        fgets( str, 256, layout_stream );
        strnullcpy( layoutp->hostname, str );

        fgets( str, 256, layout_stream );
        strnullcpy( layoutp->identity, str );

        fgets( str, 256, layout_stream );
        strnullcpy( layoutp->command, str );

        fgets( str, 256, layout_stream );
        Layout_Scan_Values( layoutp, str );
        fgets( str, 256, layout_stream );
        Layout_Scan_Options( layoutp, str );

        if ( layout_tail == LAYOUT_NULL )
            layout_head = layoutp;
        else
            layout_tail->next = layoutp;
        layout_tail = layoutp;
        layoutp->next = LAYOUT_NULL;
    }
    fclose ( layout_stream );
}
    
/* ------------------------------------------------------------ */

Layoutp Layout_Find( uwinp )
    Userwin *uwinp;
{
    Layout *layoutp;
    bool same;
    
    if ( unmatched_layouts <= 0 )
        return (Layoutp)LAYOUT_NULL;
    
    if ( (uwinp->res_class == NULL) && (uwinp->res_name == NULL) )
        return (Layoutp)LAYOUT_NULL;

        /* Match host, res, identity, command */

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
	/* for ULTRIX */
	same = streql( uwinp->hostname, layoutp->hostname ) &&
	    streql( uwinp->res_class, layoutp->res_class ) &&
		streql( uwinp->res_name, layoutp->res_name );
	same = same && streql( uwinp->identity, layoutp->identity ) &&
	    streql( uwinp->command, layoutp->command );

        if ( (! layoutp->found) && same)
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
	/* for ULTRIX */
	same = streql( "*", layoutp->hostname ) &&
	    streql( uwinp->res_class, layoutp->res_class ) &&
		streql( uwinp->res_name, layoutp->res_name );
	same = same && streql( uwinp->identity, layoutp->identity ) &&
	    streql( uwinp->command, layoutp->command );
	
        if ( (! layoutp->found) && same)
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

        /* Match host, res, identity */
    
    if ( (uwinp->res_class == NULL) && (uwinp->res_name == NULL) )
        return (Layoutp)LAYOUT_NULL;

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
	/* for ULTRIX */
	same = streql( uwinp->hostname, layoutp->hostname ) &&
	    streql( uwinp->res_class, layoutp->res_class );
	same = same && streql( uwinp->res_name, layoutp->res_name ) &&
	    streql( uwinp->identity, layoutp->identity );

        if ( (! layoutp->found) && same)
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
	/* for ULTRIX */

	same = streql( "*", layoutp->hostname ) &&
	    streql( uwinp->res_class, layoutp->res_class );
	same = same && streql( uwinp->res_name, layoutp->res_name ) &&
	    streql( uwinp->identity, layoutp->identity );

        if ( (! layoutp->found) && same)
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

        /* Match host, res, command */
    
    if ( (uwinp->res_class == NULL) && (uwinp->res_name == NULL) )
        return (Layoutp)LAYOUT_NULL;

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
	/* for ULTRIX */
	same = streql( uwinp->hostname, layoutp->hostname ) &&
	    streql( uwinp->res_class, layoutp->res_class );
	same = same &&  streql( uwinp->res_name, layoutp->res_name ) &&
		 streql( uwinp->command, layoutp->command );

	if ( (! layoutp->found) && same)
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
	/* for ULTRIX */
	same = streql( "*", layoutp->hostname ) &&
	    streql( uwinp->res_class, layoutp->res_class );
	same = same && streql( uwinp->res_name, layoutp->res_name ) &&
	    streql( uwinp->command, layoutp->command );

        if ( (! layoutp->found) && same)
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

        /* Match host, res */

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
        if ( (! layoutp->found) &&
             streql( uwinp->hostname, layoutp->hostname ) &&
             streql( uwinp->res_class, layoutp->res_class ) &&
             streql( uwinp->res_name, layoutp->res_name ) )
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

    for ( layoutp = layout_head; layoutp != LAYOUT_NULL;
          layoutp = layoutp->next )
    {
        if ( (! layoutp->found) &&
             streql( "*", layoutp->hostname ) &&
             streql( uwinp->res_class, layoutp->res_class ) &&
             streql( uwinp->res_name, layoutp->res_name ) )
        {
            layoutp->found = TRUE;
            unmatched_layouts--;
            return (Layoutp)layoutp;
        }
    }

    return (Layoutp)LAYOUT_NULL;
}
    
/* ------------------------------------------------------------ */

void Layout_Set_Values( layoutp, uwinp )
    Userwin *uwinp;
    Layoutp layoutp;
{
    Size siz;
    Layout *layp = (Layout *)layoutp;

    if ( layp == LAYOUT_NULL ) return;
    uwinp->sized = TRUE;
    uwinp->positioned = layp->open || layp->positioned;
    uwinp->icon_positioned = (! layp->open ) || layp->positioned;
    siz = Tilwin_Get_Size( uwinp->iconp );
    Tilwin_Set_Min( uwinp->iconp, siz );
    Tilwin_Set_Max( uwinp->iconp, siz );
    Tilwin_Set( uwinp->iconp, layp->icon_center, siz );
    uwinp->zoomed = layp->zoomed;
    uwinp->regular_size = layp->regular_size;
    uwinp->regular_min = layp->regular_min;
    Tilwin_Set_Min( uwinp->winp, uwinp->regular_min );
    uwinp->zoom_size = layp->zoom_size;
    uwinp->zoom_min = layp->zoom_min;
    uwinp->rezoom_rect = layp->rezoom_rect;
    Tilwin_Set_Max( uwinp->winp, layp->max_size );
    Tilwin_Set( uwinp->winp, layp->center, layp->siz );
}
    
/* ------------------------------------------------------------ */

void Layout_Set_Options( layoutp, uwinp )
    Userwin *uwinp;
    Layoutp layoutp;
{
    Layout *layp = (Layout *)layoutp;

    if ( layp == LAYOUT_NULL ) return;

    Userwin_Set_Window_Option( uwinp,
        AUTO_OPEN_ON_CREATE, layp->open );

    if ( layp->explicit_fixed_completely )
    {
        Tilwin_Set_Window_Option( uwinp->winp, FIXED_COMPLETELY,
            layp->fixed_completely );
        Explicit_Window_Set( uwinp, EXPLICIT_FIXED_COMPLETELY );
    }
    if ( layp->explicit_fixed_height )
    {
        Tilwin_Set_Window_Option( uwinp->winp, FIXED_HEIGHT,
            layp->fixed_height );
        Explicit_Window_Set( uwinp, EXPLICIT_FIXED_HEIGHT );
    }
    if ( layp->explicit_fixed_width )
    {
        Tilwin_Set_Window_Option( uwinp->winp, FIXED_WIDTH,
            layp->fixed_width );
        Explicit_Window_Set( uwinp, EXPLICIT_FIXED_WIDTH );
    }
    if ( layp->explicit_require_manual_close )
    {
        Tilwin_Set_Window_Option( uwinp->winp, REQUIRE_MANUAL_CLOSE,
            layp->require_manual_close );
        Explicit_Window_Set( uwinp, EXPLICIT_REQUIRE_MANUAL_CLOSE );
    }
    if ( layp->explicit_listener_tied_to_zoom )
    {
        Userwin_Set_Window_Option( uwinp, LISTENER_TIED_TO_ZOOM,
            layp->listener_tied_to_zoom );
        Explicit_Window_Set( uwinp, EXPLICIT_LISTENER_TIED_TO_ZOOM );
    }
    if ( layp->explicit_zoom_tied_to_desire )
    {
        Userwin_Set_Window_Option( uwinp, ZOOM_TIED_TO_DESIRE,
            layp->zoom_tied_to_desire );
        Explicit_Window_Set( uwinp, EXPLICIT_ZOOM_TIED_TO_DESIRE );
    }
}

/* ------------------------------------------------------------ */

static void Layout_Write_Window( layout_stream, uwinp )
    FILE *layout_stream;
    Userwin *uwinp;
{
    if ( ( ( uwinp->res_class == NULL ) &&
           ( uwinp->res_name == NULL ) ) ||
         (! Userwin_Get_Window_Option( uwinp, SAVE_IN_LAYOUT ) ) )
         return;

    if ( uwinp->zoomed )
    {
        uwinp->zoom_size = Tilwin_Get_Desired( uwinp->winp );
        uwinp->zoom_min = Tilwin_Get_Min( uwinp->winp );
    }
    else
    {
        uwinp->regular_size = Tilwin_Get_Desired( uwinp->winp );
        uwinp->regular_min = Tilwin_Get_Min( uwinp->winp );
    }

    fprintf( layout_stream, "\n" );

    if ( uwinp->res_class == NULL )
        fprintf( layout_stream, "\n" );
    else
        fprintf( layout_stream, "%s\n", uwinp->res_class );

    if ( uwinp->res_name == NULL )
        fprintf( layout_stream, "\n" );
    else
        fprintf( layout_stream, "%s\n", uwinp->res_name );

    if ( uwinp->hostname == NULL )
        fprintf( layout_stream, "\n" );
    else if ( streql( hoststr, uwinp->hostname ) )
        fprintf( layout_stream, "*\n" );
    else
        fprintf( layout_stream, "%s\n", uwinp->hostname );

    if ( uwinp->identity == NULL )
        fprintf( layout_stream, "\n" );
    else
        fprintf( layout_stream, "%s\n", uwinp->identity );

    if ( uwinp->command == NULL )
        fprintf( layout_stream, "\n" );
    else
        fprintf( layout_stream, "%s\n", uwinp->command );

    fprintf( layout_stream,
        "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
        Tilwin_Is_Open( uwinp->winp ),
        Tilwin_Get_Size( uwinp->winp ).width,
        Tilwin_Get_Size( uwinp->winp ).height,
        Tilwin_Get_Center( uwinp->winp ).x,
        Tilwin_Get_Center( uwinp->winp ).y,
        Tilwin_Get_Center( uwinp->iconp ).x,
        Tilwin_Get_Center( uwinp->iconp ).y,
        uwinp->zoomed,
        uwinp->regular_size.width,
        uwinp->regular_size.height,
        uwinp->regular_min.width,
        uwinp->regular_min.height,
        uwinp->zoom_size.width,
        uwinp->zoom_size.height,
        uwinp->zoom_min.width,
        uwinp->zoom_min.height,
        Tilwin_Get_Max( uwinp->winp ).width,
        Tilwin_Get_Max( uwinp->winp ).height,
        uwinp->rezoom_rect.left_x,
        uwinp->rezoom_rect.right_x,
        uwinp->rezoom_rect.top_y,
        uwinp->rezoom_rect.bottom_y,
        ( Tilwin_Is_Open( uwinp->winp ) ?
              uwinp->icon_positioned :
              uwinp->positioned ) );

    fprintf( layout_stream,
        "%d %d %d %d %d %d %d %d %d %d %d %d\n",
        Explicit_Window_Get( uwinp, EXPLICIT_REQUIRE_MANUAL_CLOSE ),
        Tilwin_Get_Window_Option( uwinp->winp, REQUIRE_MANUAL_CLOSE),
        Explicit_Window_Get( uwinp, EXPLICIT_FIXED_WIDTH ),
        Tilwin_Get_Window_Option( uwinp->winp, FIXED_WIDTH ),
        Explicit_Window_Get( uwinp, EXPLICIT_FIXED_HEIGHT ),
        Tilwin_Get_Window_Option( uwinp->winp, FIXED_HEIGHT ),
        Explicit_Window_Get( uwinp, EXPLICIT_FIXED_COMPLETELY ),
        Tilwin_Get_Window_Option( uwinp->winp, FIXED_COMPLETELY ),
        Explicit_Window_Get( uwinp, EXPLICIT_LISTENER_TIED_TO_ZOOM ),
        Userwin_Get_Window_Option( uwinp, LISTENER_TIED_TO_ZOOM ),
        Explicit_Window_Get( uwinp, EXPLICIT_ZOOM_TIED_TO_DESIRE ),
        Userwin_Get_Window_Option( uwinp, ZOOM_TIED_TO_DESIRE ) );
}
    
/* ------------------------------------------------------------ */

bool Layout_Save()
{
    FILE *layout_stream;
    Tilwinp winp;
    Userwin *uwinp;

    layout_stream = Open_Write( ".rtl.layout" );
    if ( layout_stream == NULL ) return FALSE;

    if ( preference_file == NULL )
        fprintf( layout_stream, "\n\n" );
    else
        fprintf( layout_stream, "\n%s\n", preference_file );
    
    fprintf( layout_stream, "%d %d\n", 
                     Draw_Get_Frame_Width(), 
                     Draw_Get_Frame_Height() );

    Tilwin_Gen_Open_Windows();
    while ( (winp = Tilwin_Next_Window()) != WIN_NULL )
    {
        uwinp = Userwin_Get( winp );
        if ( uwinp->winp == winp )
            Layout_Write_Window( layout_stream, uwinp );
    }

    Tilwin_Gen_Closed_Windows();
    while ( (winp = Tilwin_Next_Window()) != WIN_NULL )
    {
        uwinp = Userwin_Get( winp );
        if ( uwinp->winp == winp )
            Layout_Write_Window( layout_stream, uwinp );
    }

    fclose( layout_stream );
    return TRUE;
}

