/*
 * Copyright (C) 2002 Sasha Vasko <sasha at aftercode.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define LOCAL_DEBUG
#include "../configure.h"
#include "asapp.h"

#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/un.h>

#include "afterstep.h"
#include "mystyle.h"
#include "screen.h"
#include "module.h"
#include "wmprops.h"
#include "../libAfterImage/afterimage.h"
#include "X11/XKBlib.h"
#ifdef XSHMIMAGE
# include <sys/ipc.h>
# include <sys/shm.h>
# include <X11/extensions/XShm.h>
#endif

static Bool   as_X_synchronous_mode = False;

/*************************************************************************/
/* Scr data members access functions to be used in linking DLLs          */
int get_screen_width(ScreenInfo *scr)
{
	if( scr == NULL ) 
		scr = ASDefaultScr;
	return scr->MyDisplayWidth;	
}	 

int get_screen_height(ScreenInfo *scr)
{
	if( scr == NULL ) 
		scr = ASDefaultScr;
	return scr->MyDisplayHeight;	
}	 

int get_screen_current_desk(ScreenInfo *scr)
{
	if( scr == NULL ) 
		scr = ASDefaultScr;
	return scr->CurrentDesk;	
}	 

struct MyLook *get_screen_look(ScreenInfo *scr)
{
	if( scr == NULL ) 
		scr = ASDefaultScr;
	return &(scr->Look);	
}	 

struct ASImageManager *get_screen_image_manager(ScreenInfo *scr)
{
	if( scr == NULL ) 
		scr = ASDefaultScr;
	return scr->image_manager;	
}	 

struct ASFontManager *get_screen_font_manager(ScreenInfo *scr)
{
	if( scr == NULL ) 
		scr = ASDefaultScr;
	return scr->font_manager;	
}	 

struct ASVisual *get_screen_visual(ScreenInfo *scr)
{
	if( scr == NULL ) 
		scr = ASDefaultScr;
	return scr->asv;	
}	 


ScreenInfo *get_current_screen()
{
	return ASDefaultScr ;
}	 

void
reload_screen_image_manager( ScreenInfo *scr, ASImageManager **old_imageman )
{
	char *env_path1 = NULL, *env_path2 = NULL ;
	if( scr == NULL ) 
		scr = ASDefaultScr ;
	if( old_imageman )
	{
		*old_imageman = scr->image_manager ;
	}else if( scr->image_manager )
      	destroy_image_manager( scr->image_manager, False );

	env_path1 = getenv( "IMAGE_PATH" ) ;
	env_path2 = getenv( "PATH" );
	if( env_path1 == NULL ) 
	{
		env_path1 = env_path2;
		env_path2 = NULL ;
	}
    scr->image_manager = create_image_manager( NULL, 2.2, Environment->pixmap_path?Environment->pixmap_path:"", env_path1, env_path2, NULL );
	set_xml_image_manager( scr->image_manager );
    show_progress("Pixmap Path changed to \"%s:%s:%s\" ...", Environment->pixmap_path?Environment->pixmap_path:"", env_path1?env_path1:"", env_path2?env_path2:"");
}

/*************************************************************************/

void
get_Xinerama_rectangles (ScreenInfo * scr)
{
#ifdef HAVE_XINERAMA
	if (XineramaQueryExtension (dpy, &(scr->XineEventBase), &(scr->XineErrorBase)))
	{
		register int  i;
		XineramaScreenInfo *s;

		if ((s = XineramaQueryScreens (dpy, &(scr->xinerama_screens_num))) != NULL)
		{
			static char buf[256] ; 
			scr->xinerama_screens = safemalloc (sizeof (XRectangle) * scr->xinerama_screens_num);
	    	asxml_var_insert("xroot.xinerama_screens_num", scr->xinerama_screens_num);
			for (i = 0; i < scr->xinerama_screens_num; ++i)
			{
				char *append_point = &buf[0]; 
				sprintf( append_point, "xroot.xinerama_screens[%d].", i );
				append_point += 23 ; 
				while( *append_point ) ++append_point ; 

				append_point[0] = 'x' ;append_point[1] = '\0' ;
		    	asxml_var_insert(&buf[0], s[i].x_org);
				scr->xinerama_screens[i].x = s[i].x_org;

				append_point[0] = 'y' ;
		    	asxml_var_insert(&buf[0], s[i].y_org);
				scr->xinerama_screens[i].y = s[i].y_org;

				strcpy( append_point, "width" );
		    	asxml_var_insert(&buf[0], s[i].width);
				scr->xinerama_screens[i].width = s[i].width;

				strcpy( append_point, "height" );
		    	asxml_var_insert(&buf[0], s[i].height);
				scr->xinerama_screens[i].height = s[i].height;
			}
			XFree (s);
		}
	}else
#endif
	{
		scr->xinerama_screens = NULL;
		scr->xinerama_screens_num = 0;
    	asxml_var_insert("xroot.xinerama_screens_num", 0);
	}
}

Bool
set_synchronous_mode (Bool enable)
{
	Bool          old = as_X_synchronous_mode;

	XSynchronize (dpy, enable);
	as_X_synchronous_mode = enable;
	return old;
}

Bool
is_synchronous_request (int request_code)
{
	if (as_X_synchronous_mode)
		return True;
	switch (request_code)
	{
	 case X_CreateWindow:
	 case X_GetWindowAttributes:
	 case X_GetGeometry:
	 case X_QueryTree:
	 case X_InternAtom:
	 case X_GetAtomName:
	 case X_GetProperty:
	 case X_ListProperties:
	 case X_GetSelectionOwner:
	 case X_ConvertSelection:
	 case X_QueryPointer:
	 case X_GetMotionEvents:
	 case X_TranslateCoords:
	 case X_GetInputFocus:
	 case X_QueryKeymap:
	 case X_OpenFont:
	 case X_QueryFont:
	 case X_QueryTextExtents:
	 case X_ListFonts:
	 case X_ListFontsWithInfo:
	 case X_GetFontPath:
	 case X_CreatePixmap:
	 case X_CreateGC:
	 case X_GetImage:
	 case X_CreateColormap:
	 case X_CopyColormapAndFree:
	 case X_ListInstalledColormaps:
	 case X_AllocColor:
	 case X_AllocNamedColor:
	 case X_AllocColorCells:
	 case X_AllocColorPlanes:
	 case X_QueryColors:
	 case X_LookupColor:
	 case X_CreateCursor:
	 case X_CreateGlyphCursor:
	 case X_QueryBestSize:
	 case X_QueryExtension:
	 case X_ListExtensions:
	 case X_GetKeyboardMapping:
	 case X_GetKeyboardControl:
	 case X_GetPointerControl:
	 case X_GetScreenSaver:
	 case X_ListHosts:
	 case X_GetPointerMapping:
	 case X_GetModifierMapping:
		 return True;
		 break;
	 default:
		 break;
	}
	return False;
}

int
ASErrorHandler (Display * dpy, XErrorEvent * event)
{
	char         *err_text;

	fprintf (stderr, "%s has encountered a problem interacting with X Windows :\n", MyName);
	if (event && dpy)
	{
        if( event->error_code == BadWindow && ASDefaultScr->Windows != NULL )
            if( ASDefaultScr->on_dead_window )
                if( ASDefaultScr->on_dead_window( event->resourceid ) )
                    return 0;

        err_text = safemalloc (128);
		strcpy (err_text, "unknown error");
		XGetErrorText (dpy, event->error_code, err_text, 120);
		fprintf (stderr, "      Request: %d,    Error: %d(%s)\n", event->request_code, event->error_code, err_text);
		free (err_text);
		fprintf (stderr, "      in resource: 0x%lX\n", event->resourceid);
#ifndef __CYGWIN__
		if (is_synchronous_request (event->request_code))
			print_simple_backtrace ();
#endif
	}
	return 0;
}


void 
init_ScreenInfo(ScreenInfo *scr) 
{
	if( scr )
	{
		memset (scr, 0x00, sizeof (ScreenInfo));
		scr->Look.magic = MAGIC_MYLOOK ;
	}
}

int
ConnectXDisplay (Display *display, ScreenInfo * scr, Bool as_manager)
{
    int width_mm, height_mm ;
	int display_dpcmx = 0, display_dpcmy = 0;
	int button_w, button_h, mini_w, mini_h ;
	
	if( display == NULL ) 
		return -1;

	dpy = display ;
	set_current_X_display (display); /* for libAfterBase X Wrappers */
	
	if( scr == NULL ) 
	{
		if( ASDefaultScr == NULL ) 
			ASDefaultScr = safecalloc( 1, sizeof(ScreenInfo));
		scr = ASDefaultScr ;
	}

	init_ScreenInfo(scr);	

	x_fd = XConnectionNumber (dpy);

    if( x_fd < 0 )
    {
        show_error("failed to initialize X Windows session. Aborting!");
        exit(1);
    }
    if (fcntl (x_fd, F_SETFD, 1) == -1)
        show_warning ("close-on-exec for X Windows connection failed");

	XSetErrorHandler (ASErrorHandler);

    if( get_flags(MyArgs.flags, ASS_Debugging) )
        set_synchronous_mode (True);

    intern_hint_atoms ();
    intern_wmprop_atoms ();

	scr->screen = DefaultScreen (dpy);
	scr->Root = RootWindow (dpy, scr->screen);
	if (scr->Root == None)
	{
        show_error("Screen %d is not valid. Exiting.", (int)scr->screen);
		exit (1);
	}

    scr->NumberOfScreens = NumberOfScreens = ScreenCount (dpy);
	scr->MyDisplayWidth = DisplayWidth (dpy, scr->screen);
	scr->MyDisplayHeight = DisplayHeight (dpy, scr->screen);

	asxml_var_insert("xroot.width", scr->MyDisplayWidth);
    asxml_var_insert("xroot.height", scr->MyDisplayHeight);

	width_mm = DisplayWidthMM(dpy, scr->screen);
	height_mm = DisplayHeightMM(dpy, scr->screen);
	asxml_var_insert("xroot.widthmm", width_mm);
    asxml_var_insert("xroot.heightmm", height_mm);
	
	display_dpcmx = (scr->MyDisplayWidth * 10 )/ width_mm;
	display_dpcmy = (scr->MyDisplayHeight * 10 )/ height_mm;

	button_w = (display_dpcmx < 50)?48:64;
	button_h = (display_dpcmy < 50)?48:64;
	mini_w = (display_dpcmx <= 44)?max(display_dpcmx/2,12):display_dpcmx-20;
	mini_h = (display_dpcmy <= 44)?max(display_dpcmy/2,12):display_dpcmy-20;
	asxml_var_insert(ASXMLVAR_IconButtonWidth, button_w);
	asxml_var_insert(ASXMLVAR_IconButtonHeight, button_h);
	asxml_var_insert(ASXMLVAR_IconWidth, (button_w*3)/4);
	asxml_var_insert(ASXMLVAR_IconHeight, (button_h*3)/4);
	asxml_var_insert(ASXMLVAR_MinipixmapWidth, mini_w);
	asxml_var_insert(ASXMLVAR_MinipixmapHeight, mini_h);
	asxml_var_insert(ASXMLVAR_TitleFontSize, (mini_h*7)/12);
	asxml_var_insert(ASXMLVAR_MenuFontSize, (mini_h*7)/12+1);
	asxml_var_insert("font.size", ((mini_h*7)/12));

	scr->CurrentDesk = -1;

    scr->RootClipArea.width = scr->MyDisplayWidth;
    scr->RootClipArea.height = scr->MyDisplayHeight;

    if( (scr->wmprops = setup_wmprops( scr, as_manager, 0xFFFFFFFF, NULL )) == NULL )
        return -1;

    scr->asv = create_asvisual (dpy, scr->screen, DefaultDepth(dpy,scr->screen), NULL);
	if (scr->asv == NULL)
	{
        show_error("Failed to find suitable visual for screen %d. Exiting.", (int)scr->screen);
		exit (1);
	}

	scr->d_depth = scr->asv->visual_info.depth;

    scr->last_Timestamp = CurrentTime ;
    scr->menu_grab_Timestamp = CurrentTime ;

    setup_modifiers ();

	get_Xinerama_rectangles (scr);

#ifdef SHAPE
	XShapeQueryExtension (dpy, &(scr->ShapeEventBase), &(scr->ShapeErrorBase));
#endif /* SHAPE */
#ifdef XSHMIMAGE
	scr->ShmCompletionEventType = XShmGetEventBase(dpy) + ShmCompletion;
#endif /* SHAPE */

	init_screen_gcs(scr);

	return x_fd;
}

int
ConnectX (ScreenInfo * scr, unsigned long event_mask)
{
    if (!(dpy = XOpenDisplay (MyArgs.display_name)))
	{
        show_error("Can't open display \"%s\". Exiting!", XDisplayName (MyArgs.display_name));
		exit (1);
	}
	
	ConnectXDisplay (dpy, scr, (event_mask&SubstructureRedirectMask));
    XSelectInput (dpy, scr->Root, event_mask);
	return x_fd;
}


/***********************************************************************
 *  Procedure:
 *  SetupEnvironment - setup environment variables DISPLAY and HOSTDISPLAY
 *  for our children to use.
 ************************************************************************/
void
make_screen_envvars( ScreenInfo *scr )
{
    int           len ;
    char         *display = ":0.0";
    char          client[MAXHOSTNAME];
/* We set environmanet vars and we keep memory used for those allocated
 * as some OS's don't copy the environment variables properly
 */
	if( scr == NULL )
		scr = ASDefaultScr;

	if( scr->display_string )
        free( scr->display_string );
    if( scr->rdisplay_string )
        free( scr->rdisplay_string );

    /*  Add a DISPLAY entry to the environment, incase we were started
	 * with afterstep -display term:0.0
	 */
	display = XDisplayString (dpy);
    len = strlen (display);
    scr->display_string = safemalloc (len + 10);
    sprintf (scr->display_string, "DISPLAY=%s", display);

	/* Add a HOSTDISPLAY environment variable, which is the same as
	 * DISPLAY, unless display = :0.0 or unix:0.0, in which case the full
	 * host name will be used for ease in networking . */
    if ( scr->display_string[8] == ':' ||
        strncmp (&(scr->display_string[8]), "unix:", 5) == 0)
	{
        register char * ptr = &(scr->display_string[8]);
        if( *ptr != ':' ) ptr += 5 ;
		mygethostname (client, MAXHOSTNAME);
		scr->localhost = True ;
		scr->rdisplay_string = safemalloc (len + 14 + strlen (client));
        sprintf (scr->rdisplay_string, "HOSTDISPLAY=%s:%s", client, ptr);
    }else
    {   /* X Server is likely to be runnig on other computer: */
		scr->localhost = False ;
		scr->rdisplay_string = safemalloc (len + 14);
        sprintf (scr->rdisplay_string, "HOSTDISPLAY=%s", display);
	}
}

void
init_screen_gcs(ScreenInfo *scr)
{
	XGCValues     gcv;
   	unsigned long gcm = GCGraphicsExposures;
	
	if( scr == NULL )
		scr = ASDefaultScr;
	
	gcv.graphics_exposures = False;

    scr->RootGC = XCreateGC( dpy, scr->Root, gcm, &gcv );
	scr->DrawGC = create_visual_gc( scr->asv, scr->Root, gcm, &gcv );
}

void
destroy_screen_gcs(ScreenInfo *scr)
{
	if( scr == NULL )
		scr = ASDefaultScr;
	
	if( scr->DrawGC )
	{	
		XFreeGC(dpy, scr->DrawGC);
		scr->DrawGC = NULL ;
	}
	if( scr->RootGC )
	{	
		XFreeGC(dpy, scr->RootGC);
		scr->RootGC = NULL ;
	}
}		

/* Setting up global variables  nonlock_mods, and lock_mods, defined in asapp.c : */
void
setup_modifiers ()
{
	int           m, i, knl;
	char         *kn;
	KeySym        ks;
	KeyCode       kc, *kp;
	unsigned      lockmask, *mp;
	XModifierKeymap *mm = XGetModifierMapping (dpy);

	lockmask = LockMask;
	if (mm)
	{
		kp = mm->modifiermap;
		for (m = 0; m < 8; m++)
		{
			for (i = 0; i < mm->max_keypermod; i++)
			{
				if ((kc = *kp++) && ((ks = XkbKeycodeToKeysym (dpy, kc, 0,0)) != NoSymbol))
				{
					kn = XKeysymToString (ks);
					knl = strlen (kn);
					if ((knl > 6) && (mystrcasecmp (kn + knl - 4, "lock") == 0))
						lockmask |= (1 << m);
				}
			}
		}
		XFreeModifiermap (mm);
	}
/* forget shift & control locks */
	lockmask &= ~(ShiftMask | ControlMask);
	nonlock_mods = ((ShiftMask | ControlMask | Mod1Mask | Mod2Mask
	 				 | Mod3Mask | Mod4Mask | Mod5Mask) & ~lockmask);
	
	LOCAL_DEBUG_OUT( "lockmask = %X, nonlock_mods = %X", lockmask, nonlock_mods );

	mp = lock_mods;
	for (m = 0, i = 1; i < MAX_LOCK_MODS; i++)
	{
		if ((i & lockmask) > m)
			m = *mp++ = (i & lockmask);
	}
	*mp = 0;
}

/****************************************************************************
 * Pan Frames : windows for edge-scrolling
 * the root window is surrounded by four window slices, which are InputOnly.
 * So you can see 'through' them, but they eat the input. An EnterEvent in
 * one of these windows causes a Paging. The windows have the according cursor
 * pointing in the pan direction or are hidden if there is no more panning
 * in that direction. This is mostly intended to get a panning even atop
 * of Motif applictions, which does not work yet. It seems Motif windows
 * eat all mouse events.
 *
 * Hermann Dunkel, HEDU, dunkel@cul-ipn.uni-kiel.de 1/94
 ****************************************************************************/
void
init_screen_panframes(ScreenInfo *scr)
{
#ifndef NO_VIRTUAL
    XRectangle  frame_rects[PAN_FRAME_SIDES];

    XSetWindowAttributes attributes;           /* attributes for create */
    register int i ;

	if( scr == NULL ) 
		scr =  ASDefaultScr ;

    frame_rects[0].x = frame_rects[0].y = 0;
    frame_rects[0].width = scr->MyDisplayWidth;
    frame_rects[0].height = SCROLL_REGION;

    frame_rects[1].x = scr->MyDisplayWidth - SCROLL_REGION;
    frame_rects[1].y = SCROLL_REGION;
    frame_rects[1].width = SCROLL_REGION;
    frame_rects[1].height = scr->MyDisplayHeight - 2 * SCROLL_REGION;

    frame_rects[2].x = 0;
    frame_rects[2].y = scr->MyDisplayHeight - SCROLL_REGION;
    frame_rects[2].width = scr->MyDisplayWidth;
    frame_rects[2].height = SCROLL_REGION;

    frame_rects[3].x = 0;
    frame_rects[3].y = SCROLL_REGION;
    frame_rects[3].width = SCROLL_REGION;
    frame_rects[3].height = scr->MyDisplayHeight - 2 * SCROLL_REGION;

    frame_rects[2].width = frame_rects[0].width = scr->MyDisplayWidth;
    frame_rects[1].x = scr->MyDisplayWidth - SCROLL_REGION;
    frame_rects[3].height = frame_rects[1].height =
      scr->MyDisplayHeight - (SCROLL_REGION*2) ;

    attributes.event_mask = AS_PANFRAME_EVENT_MASK;
    for( i = 0 ; i < PAN_FRAME_SIDES ; i++ )
    {
        scr->PanFrame[i].win =
			create_screen_window( scr, None,
								  frame_rects[i].x, frame_rects[i].y,
              		              frame_rects[i].width, frame_rects[i].height, 0, /* no border */
	                              InputOnly, (CWEventMask), &attributes);
        LOCAL_DEBUG_OUT( "creating PanFrame %d(%lX) at %dx%d%+d%+d", i, scr->PanFrame[i].win,
                         frame_rects[i].width, frame_rects[i].height, frame_rects[i].x, frame_rects[i].y );
        scr->PanFrame[i].isMapped = False ;
    }
#endif /* NO_VIRTUAL */
}

/***************************************************************************
 * checkPanFrames hides PanFrames if they are on the very border of the
 * VIRTUELL screen and EdgeWrap for that direction is off.
 * (A special cursor for the EdgeWrap border could be nice) HEDU
 ****************************************************************************/
void
check_screen_panframes(ScreenInfo *scr)
{
#ifndef NO_VIRTUAL
    int           wrapX ;
    int           wrapY ;
    Bool          map_frame[PAN_FRAME_SIDES] = {False, False, False, False};
    register int  i;

	if( scr == NULL )
		scr = ASDefaultScr;
 ;

    wrapX = get_flags(scr->Feel.flags, EdgeWrapX);
    wrapY = get_flags(scr->Feel.flags, EdgeWrapY);

    if( get_flags(scr->Feel.flags, DoHandlePageing) )
    {
        if (scr->Feel.EdgeScrollY > 0)
        {
            if (scr->Vy > 0 || wrapY )
                map_frame[FR_N] = True ;
            if (scr->Vy < scr->VyMax || wrapY )
                map_frame[FR_S] = True ;
        }
        if (scr->Feel.EdgeScrollX > 0 )
        {
            if (scr->Vx < scr->VxMax || wrapX )
                map_frame[FR_E] = True ;
            if (scr->Vx > 0 || wrapX )
                map_frame[FR_W] = True ;
        }
    }
    /* Remove Pan frames if paging by edge-scroll is permanently or
	 * temporarily disabled */
    for( i = 0 ; i < PAN_FRAME_SIDES ; i++ )
    {
        if( scr->PanFrame[i].win )
        {
            if( map_frame[i] != scr->PanFrame[i].isMapped )
            {
                if( map_frame[i] )
                {
                    LOCAL_DEBUG_OUT( "mapping PanFrame %d(%lX)", i, scr->PanFrame[i].win );
                    XMapRaised (dpy, scr->PanFrame[i].win);
                }else
                    XUnmapWindow (dpy, scr->PanFrame[i].win);
                scr->PanFrame[i].isMapped = map_frame[i];
            }

            if( map_frame[i] )
            {
                /* to maintain stacking order where first mapped pan frame is the lowest window :*/
                LOCAL_DEBUG_OUT( "rasing PanFrame %d(%lX)", i, scr->PanFrame[i].win );
                XRaiseWindow( dpy, scr->PanFrame[i].win );
                XDefineCursor (dpy, scr->PanFrame[i].win, scr->Feel.cursors[ASCUR_Top+i]);
            }
        }
    }
#endif
}

/****************************************************************************
 * Gotta make sure these things are on top of everything else, or they
 * don't work!
 ***************************************************************************/
void
raise_scren_panframes (ScreenInfo *scr)
{
#ifndef NO_VIRTUAL
    register int i ;
	if( scr == NULL )
		scr = ASDefaultScr ;
    for( i = 0 ; i < PAN_FRAME_SIDES ; i++ )
        if( scr->PanFrame[i].isMapped )
        {
            LOCAL_DEBUG_OUT( "rasing PanFrame %d(%lX)", i, scr->PanFrame[i].win );
            XRaiseWindow (dpy, scr->PanFrame[i].win);
        }
#endif
}

Window
get_lowest_panframe(ScreenInfo *scr)
{
    register int i;
	if( scr == NULL )
		scr = ASDefaultScr ;
    for( i = 0 ; i < PAN_FRAME_SIDES ; i++ )
        if( scr->PanFrame[i].isMapped )
            return scr->PanFrame[i].win;
    return None;
}

/* utility function for geometry merging : */
void merge_geometry( ASGeometry *from, ASGeometry *to )
{
    if ( get_flags(from->flags, WidthValue) )
        to->width = from->width ;
    if ( get_flags(from->flags, HeightValue) )
        to->height = from->height ;
    if ( get_flags(from->flags, XValue) )
    {
        to->x = from->x ;
        if( !get_flags(from->flags, XNegative) )
            clear_flags(to->flags, XNegative);
    }
    if ( get_flags(from->flags, YValue) )
    {
        to->y = from->y ;
        if( !get_flags(from->flags, YNegative) )
            clear_flags(to->flags, YNegative);
    }
    to->flags |= from->flags ;
}

void
check_desksize_sanity( ScreenInfo *scr )
{
	if( scr == NULL )
		scr = ASDefaultScr ;

    if( scr->VxMax <= 0 )
        scr->VxMax = 0 ;
    else if( scr->VxMax < 32000/scr->MyDisplayWidth )
        scr->VxMax = (scr->VxMax * scr->MyDisplayWidth) - scr->MyDisplayWidth ;

    if( scr->VyMax <= 0 )
        scr->VyMax = 0 ;
    else if( scr->VyMax < 32000/scr->MyDisplayHeight )
        scr->VyMax = (scr->VyMax * scr->MyDisplayHeight) - scr->MyDisplayHeight ;
}

