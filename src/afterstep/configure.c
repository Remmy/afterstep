/*
 * Copyright (c) 2002,2003 Sasha Vasko <sasha at aftercode.net>
 * Copyright (c) 1998 Rafal Wierzbicki <rafal@mcss.mcmaster.ca>
 * Copyright (c) 1998 Sasha Vasko <sasha at aftercode.net>
 * Copyright (c) 1998 Michal Vitecek <fuf@fuf.sh.cvut.cz>
 * Copyright (c) 1998 Nat Makarevitch <nat@linux-france.com>
 * Copyright (c) 1998 Mike Venaccio <venaccio@aero.und.edu>
 * Copyright (c) 1998 Ethan Fischer <allanon@crystaltokyo.com>
 * Copyright (c) 1998 Mike Venaccio <venaccio@aero.und.edu>
 * Copyright (c) 1998 Chris Ridd <c.ridd@isode.com>
 * Copyright (c) 1997 Raphael Goulais <velephys@hol.fr>
 * Copyright (c) 1997 Guylhem Aznar <guylhem@oeil.qc.ca>
 * Copyright (C) 1996 Frank Fejes
 * Copyright (C) 1995 Bo Yang
 * Copyright (C) 1993 Robert Nation
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

/****************************************************************************
 *
 * Configure.c: reads the configuration files, interprets them,
 * and sets up menus, bindings, colors, and fonts as specified
 *
 ***************************************************************************/

#define LOCAL_DEBUG
#include "../../configure.h"

#include "asinternals.h" 

#include <unistd.h>

#include "dirtree.h"

#include "../../libAfterStep/session.h"
#include "../../libAfterStep/mystyle_property.h"
#include "../../libAfterStep/wmprops.h"
#include "../../libAfterStep/desktop_category.h"
#include "../../libAfterStep/kde.h"

#include "../../libAfterConf/afterconf.h"


typedef struct AfterStepConfig
{
	ASModuleConfig asmodule_config;
	ASFlagType	flags ;
	ASFlagType	set_flags ;

}AfterStepConfig;

#define AS_AFTERSTEP_CONFIG(p) AS_MODULE_CONFIG_TYPED(p,CONFIG_AfterStep_ID,AfterStepConfig)


static void
InitAfterStepConfig (ASModuleConfig *asm_config, Bool free_resources)
{
	AfterStepConfig *config = AS_AFTERSTEP_CONFIG(asm_config);
	if( config ) 
	{
		/* TODO */
		if( free_resources ) 
		{
		}
	}
}

void
AfterStep_fs2config( ASModuleConfig *asmodule_config, FreeStorageElem *Storage )
{
	FreeStorageElem *pCurr;
	ConfigItem    item;
	AfterStepConfig *config = AS_AFTERSTEP_CONFIG(asmodule_config) ;
	
	if( config == NULL ) 
		return ; 
	
	item.memory = NULL;
    for (pCurr = Storage; pCurr; pCurr = pCurr->next)
	{
		if (pCurr->term == NULL)
			continue;

		if (pCurr->term->type == TT_FLAG)
        {
        }else
		{
			if (!ReadConfigItem (&item, pCurr))
				continue;

			switch (pCurr->term->id)
			{
		        default:
        		    item.ok_to_free = 1;
			}
		}
	}
	
	ReadConfigItem (&item, NULL);
}

void
MergeAfterStepOptions ( ASModuleConfig *asm_to, ASModuleConfig *asm_from)
{
    /* int i ; */
    START_TIME(option_time);

	AfterStepConfig *to = AS_AFTERSTEP_CONFIG(asm_to);
	AfterStepConfig *from = AS_AFTERSTEP_CONFIG(asm_from);
	if( to && from )
	{

    	/* Need to merge new config with what we have already :*/
    	/* now lets check the config sanity : */
    	/* mixing set and default flags : */
    	ASCF_MERGE_FLAGS(to,from);

	}
    SHOW_TIME("to parsing",option_time);
}


flag_options_xref AfterStepConfigFlags[] = {
/*	ASCF_DEFINE_MODULE_FLAG_XREF(WINLIST,FillRowsFirst,WinListConfig), */
    {0, 0, 0}
};


int AfterStepBalloons[] = { TITLE_BALLOON_ID_START, MENU_BALLOON_ID_START, 0 };

static ASModuleConfigClass _afterstep_config_class = 
{	CONFIG_AfterStep_ID,
 	ASMC_HandlePublicLookOptions|
	ASMC_HandlePublicFeelOptions
	/* |ASMC_HandleLookMyStyles */,
	sizeof(AfterStepConfig),
 	"afterstep",
 	InitAfterStepConfig,
	AfterStep_fs2config,
	MergeAfterStepOptions,
	&AfterStepSyntax,
	&LookSyntax,
	&FeelSyntax,
	AfterStepConfigFlags,
	offsetof(AfterStepConfig,set_flags),
	
	AfterStepBalloons
 };
 
ASModuleConfigClass *AfterStepConfigClass = &_afterstep_config_class;


void 
ReloadConfig(ASFlagType what)
{
	ASBalloonLook *balloon_look ; 
	ASModuleConfig *config = parse_asmodule_config_all( AfterStepConfigClass );

	/* apply it  */
	Print_balloonConfig (config->balloon_configs[0] );
	balloon_look = create_balloon_look();
    balloon_config2look( &Scr.Look, balloon_look, config->balloon_configs[0], "TitleButtonBalloon" );
    set_balloon_state_look( TitlebarBalloons, balloon_look );
	destroy_balloon_look( balloon_look );

	balloon_look = create_balloon_look();
	Print_balloonConfig (config->balloon_configs[1] );
    balloon_config2look( &Scr.Look, balloon_look, config->balloon_configs[1], "MenuBalloon" );
	set_balloon_state_look( MenuBalloons,  balloon_look );
	destroy_balloon_look( balloon_look );
	
	PrintMyStyleDefinitions (config->style_defs);
	
	destroy_ASModule_config( config );
}

#ifdef AFTERSTEP_CONFIG_TEST

ASBalloonState *MenuBalloons = NULL ; 
ASBalloonState *TitlebarBalloons = NULL ; 

int main(int argc, char **argv )
{

    ASImageManager  *old_image_manager = NULL ;
    ASFontManager   *old_font_manager  = NULL ;
    InitMyApp( CLASS_AFTERSTEP, argc, argv, NULL, NULL, 0);

	AfterStepConfigClass->flags |= ASMC_HandleLookMyStyles;
	
	LinkAfterStepConfig();
    if( ConnectX( ASDefaultScr, 0 ) < 0  )
	{
		show_error( "Hostile X server encountered - unable to proceed :-(");
		return 1;/* failed to accure window management selection - other wm is running */
	}
	InitSession();
	MenuBalloons = create_balloon_state(); 
	TitlebarBalloons = create_balloon_state(); 
	ReloadASEnvironment( &old_image_manager, &old_font_manager, NULL, True, True );
	LoadColorScheme();
	ReloadConfig(0xFFFFFFFF);

}

#else


/* old look auxilary variables : */
static MyFont StdFont    = {NULL};         /* font structure */
static MyFont WindowFont = {NULL};      /* font structure for window titles */
static MyFont IconFont   = {NULL};        /* for icon labels */
/*
 * the old-style look variables
 */
static char         *Stdfont     = NULL;
static char         *Windowfont  = NULL;
static char         *Iconfont    = NULL;

static char         *WindowForeColor[BACK_STYLES]    = { NULL };
static char         *WindowBackColor[BACK_STYLES]    = { NULL };
static char         *WindowGradient[BACK_STYLES]     = { NULL };
static char         *WindowPixmap[BACK_STYLES]       = { NULL };
static char         *MenuForeColor[MENU_BACK_STYLES] = { NULL };
static char         *MenuBackColor[MENU_BACK_STYLES] = { NULL };
static char         *MenuGradient[MENU_BACK_STYLES]  = { NULL };
static char         *MenuPixmap[MENU_BACK_STYLES]    = { NULL };
static char         *IconBgColor = NULL;
static char         *IconTexColor = NULL;
static char         *IconPixmapFile = NULL;

static char         *TexTypes = NULL;
static int           TitleTextType = 0;
static int           TitleTextY = 0;
static int           IconTexType = TEXTURE_BUILTIN;

static char         *MenuPinOn = NULL ;
static int           MenuPinOnButton = -1 ;

static MyStyleDefinition *MyStyleList = NULL ;
static MyFrameDefinition *MyFrameList = NULL ;
static MyFrameDefinition *LegacyFrameDef = NULL ;

static balloonConfig BalloonConfig = {0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0 };
static balloonConfig MenuBalloonConfig = {0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0 };

static char         *MSWindowName[BACK_STYLES] = {NULL};
static char         *MSMenuName[MENU_BACK_STYLES] = {NULL };

/* parsing handling functions for different data types : */

void          SetInts               (char *text, FILE * fd, char **arg1, int *arg2);
void          SetInts2              (char *text, FILE * fd, char **arg1, int *arg2);
void          SetFlag               (char *text, FILE * fd, char **arg, int *another);
void		  SetLookFlag           (char *text, FILE * fd, char **arg, int *another);
void          SetFlag2              (char *text, FILE * fd, char **arg, int *var);
void          SetLookFlag           (char *text, FILE * fd, char **arg, int *junk);
void          SetBox                (char *text, FILE * fd, char **arg, int *junk);
void          SetCursor             (char *text, FILE * fd, char **arg, int *junk);
void          SetCustomCursor       (char *text, FILE * fd, char **arg, int *junk);
void          SetButtonList         (char *text, FILE * fd, char **arg1, int *arg2);
void          SetTitleText          (char *tline, FILE * fd, char **junk, int *junk2);
void          SetTitleButton        (char *tline, FILE * fd, char **junk, int *junk2);
void          SetFramePart          (char *text, FILE * fd, char **frame, int *id);
void          SetModifier           (char *text, FILE * fd, char **mod, int *junk2);
void          SetTButtonOrder       (char *text, FILE * fd, char **mod, int *junk2);

void          assign_string         (char *text, FILE * fd, char **arg, int *idx);
void          assign_path           (char *text, FILE * fd, char **arg, int *idx);
void          assign_themable_path  (char *text, FILE * fd, char **arg, int *idx);
void          assign_pixmap         (char *text, FILE * fd, char **arg, int *idx);
void          assign_geometry       (char *text, FILE * fd, char **geom, int *junk);
void          obsolete              (char *text, FILE * fd, char **arg, int *);

void          deskback_parse        (char *text, FILE * fd, char **junk, int *junk2);

/* main parsing function  : */
void          match_string (struct config *table, char *text, char *error_msg, FILE * fd);

/* menu loading code : */
int           MeltStartMenu (char *buf);

/* scratch variable : */
static int dummy;

static ASFeel TmpFeel ;
static MyLook TmpLook ;

/*
 * Order is important here! if one keyword is the same as the first part of
 * another keyword, the shorter one must come first!
 */
struct config main_config[] = {
	/* feel options */
	{"StubbornIcons", SetFlag2, (char **)StubbornIcons, (int *)0},
	{"StubbornPlacement", SetFlag2, (char **)StubbornPlacement, (int *)0},
	{"StubbornIconPlacement", SetFlag2, (char **)StubbornIconPlacement, (int *)0},
	{"StickyIcons", SetFlag2, (char **)StickyIcons, (int *)0},
	{"IconTitle", SetFlag2, (char **)IconTitle, (int *)0},
	{"KeepIconWindows", SetFlag2, (char **)KeepIconWindows, (int *)0},
	{"NoPPosition", SetFlag2, (char **)NoPPosition, (int *)0},
	{"CirculateSkipIcons", SetFlag2, (char **)CirculateSkipIcons, (int *)0},
    {"EdgeScroll", SetInts, (char **)&TmpFeel.EdgeScrollX, &TmpFeel.EdgeScrollY},
	{"RandomPlacement", SetFlag2, (char **)FEEL_DEPRECATED_RandomPlacement,(int *)&(TmpFeel.deprecated_flags)},
	{"SmartPlacement", SetFlag2, (char **)FEEL_DEPRECATED_SmartPlacement,(int *)&(TmpFeel.deprecated_flags)},
	{"DontMoveOff", obsolete, (char **)NULL, (int *)0},
	{"DecorateTransients", SetFlag2, (char **)DecorateTransients, (int *)0},
	{"CenterOnCirculate", SetFlag2, (char **)CenterOnCirculate, (int *)0},
    {"AutoRaise", SetInts, (char **)&TmpFeel.AutoRaiseDelay, &dummy},
    {"ClickTime", SetInts, (char **)&TmpFeel.ClickTime, &dummy},
    {"OpaqueMove", SetInts, (char **)&TmpFeel.OpaqueMove, &dummy},
    {"OpaqueResize", SetInts, (char **)&TmpFeel.OpaqueResize, &dummy},
    {"XorValue", obsolete, (char **)NULL, &dummy},
	{"Mouse", ParseMouseEntry, (char **)1, (int *)0},
    {"Popup", ParsePopupEntry, (char **)1, (int *)0},
    {"Function", ParseFunctionEntry, (char **)1, (int *)0},
	{"Key", ParseKeyEntry, (char **)1, (int *)0},
	{"ClickToFocus", SetFlag, (char **)ClickToFocus, (int *)EatFocusClick},
    {"EatFocusClick", SetFlag2, (char **)EatFocusClick, (int *)0},
    {"ClickToRaise", SetButtonList, (char **)NULL, (int *)0},
	{"MenusHigh", obsolete, (char **)NULL, (int *)0},
	{"SloppyFocus", SetFlag2, (char **)SloppyFocus, (int *)0},
    {"PagingDefault", obsolete, (char **)NULL, NULL},
    {"EdgeResistance", SetInts, (char **)&TmpFeel.EdgeResistanceScroll, &TmpFeel.EdgeResistanceMove},
    {"EdgeResistanceToDragging", SetInts, (char **)&TmpFeel.EdgeResistanceDragScroll, NULL},
	{"BackingStore", SetFlag2, (char **)BackingStore, (int *)0},
	{"AppsBackingStore", SetFlag2, (char **)AppsBackingStore, (int *)0},
	{"SaveUnders", SetFlag2, (char **)SaveUnders, (int *)0},
    {"Xzap", SetInts, (char **)&TmpFeel.Xzap, (int *)&dummy},
    {"Yzap", SetInts, (char **)&TmpFeel.Yzap, (int *)&dummy},
    {"AutoReverse", SetInts, (char **)&TmpFeel.AutoReverse, (int *)&dummy},
    {"AutoTabThroughDesks", SetFlag2, (char **)AutoTabThroughDesks, NULL},
    {"MWMFunctionHints", obsolete, (char **)0, NULL},
    {"MWMDecorHints", obsolete, (char **)0, NULL},
    {"MWMHintOverride", obsolete, (char **)0, NULL},
    {"FollowTitleChanges", SetFlag2, (char **)FollowTitleChanges, (int *)0},
    {"PersistentMenus", SetFlag2, (char **)PersistentMenus, (int *)0},
    {"NoSnapKey", SetModifier, (char **)&(TmpFeel.no_snaping_mod), (int *)0},
    {"ScreenEdgeAttraction", SetInts, (char **)&TmpFeel.EdgeAttractionScreen, &dummy},
    {"WindowEdgeAttraction", SetInts, (char **)&TmpFeel.EdgeAttractionWindow, &dummy},
    {"DontRestoreFocus", SetFlag2, (char **)DontRestoreFocus, NULL},
    {"WindowBox", windowbox_parse, (char**)NULL, (int*)NULL},
    {"DefaultWindowBox", assign_string, (char**)&(TmpFeel.default_window_box_name), (int*)0},
    {"RecentSubmenuItems", SetInts, (char**)&TmpFeel.recent_submenu_items, (int*)&dummy},
	{"WinListSortOrder", SetInts, (char**)&TmpFeel.winlist_sort_order, (int *)&dummy},
	{"WinListHideIcons", SetFlag2, (char**)WinListHideIcons, NULL},
	{"SuppressIcons", SetFlag2, (char**)SuppressIcons, NULL},
	{"WarpPointer", SetFlag2, (char**)WarpPointer, NULL},

    /* look options */
	/* obsolete stuff */
	{"Font", assign_string, &Stdfont, (int *)0},
	{"WindowFont", assign_string, &Windowfont, (int *)0},
	{"IconFont", assign_string, &Iconfont, (int *)0},
    {"MTitleForeColor", assign_string, &MenuForeColor[MENU_BACK_TITLE], (int *)0},
    {"MTitleBackColor", assign_string, &MenuBackColor[MENU_BACK_TITLE], (int *)0},
    {"MenuForeColor", assign_string, &MenuForeColor[MENU_BACK_ITEM], (int *)0},
    {"MenuBackColor", assign_string, &MenuBackColor[MENU_BACK_ITEM], (int *)0},
    {"MenuHiForeColor", assign_string, &MenuForeColor[MENU_BACK_HILITE], (int *)0},
    {"MenuHiBackColor", assign_string, &MenuBackColor[MENU_BACK_HILITE], (int *)0},
    {"MenuStippleColor", assign_string, &MenuForeColor[MENU_BACK_STIPPLE], (int *)0},
    {"StdForeColor", assign_string, &WindowForeColor[BACK_UNFOCUSED], (int *)0},
    {"StdBackColor", assign_string, &WindowBackColor[BACK_UNFOCUSED], (int *)0},
    {"StickyForeColor", assign_string, &WindowForeColor[BACK_STICKY], (int *)0},
    {"StickyBackColor", assign_string, &WindowBackColor[BACK_STICKY], (int *)0},
    {"HiForeColor", assign_string, &WindowForeColor[BACK_FOCUSED], (int *)0},
    {"HiBackColor", assign_string, &WindowBackColor[BACK_FOCUSED], (int *)0},
	{"TextureTypes", assign_string, &TexTypes, (int *)0},
    {"TextureMaxColors", obsolete, NULL, (int *)0},
    {"TitleTextureColor", assign_string, &WindowGradient[BACK_FOCUSED], (int *)0},    /* title */
    {"UTitleTextureColor", assign_string, &WindowGradient[BACK_UNFOCUSED], (int *)0},   /* unfoc tit */
    {"STitleTextureColor", assign_string, &WindowGradient[BACK_STICKY], (int *)0},   /* stic tit */
    {"MTitleTextureColor", assign_string, &MenuGradient[MENU_BACK_TITLE], (int *)0},   /* menu title */
    {"MenuTextureColor", assign_string, &MenuGradient[MENU_BACK_ITEM], (int *)0}, /* menu items */
    {"MenuHiTextureColor", assign_string, &MenuGradient[MENU_BACK_HILITE], (int *)0},  /* sel items */
    {"MenuPixmap", assign_string, &MenuPixmap[MENU_BACK_ITEM], (int *)0},  /* menu entry */
    {"MenuHiPixmap", assign_string, &MenuPixmap[MENU_BACK_HILITE], (int *)0},   /* hil m entr */
    {"MTitlePixmap", assign_string, &MenuPixmap[MENU_BACK_TITLE], (int *)0},   /* menu title */
    {"TitlePixmap", assign_string,  &WindowPixmap[BACK_FOCUSED], (int *)0}, /* foc tit */
    {"UTitlePixmap", assign_string, &WindowPixmap[BACK_UNFOCUSED], (int *)0},    /* unfoc tit */
    {"STitlePixmap", assign_string, &WindowPixmap[BACK_STICKY], (int *)0},    /* stick tit */
    {"MenuPinOff", obsolete, (char **)NULL, (int *)0},
    {"TexturedHandle", obsolete, (char **)NULL, (int *)0},
    {"TextGradientColor", obsolete, (char **)NULL, (int *)0}, /* title text */
    {"GradientText", obsolete, (char **)NULL, (int *)0},

    {"ButtonTextureType", SetInts, (char **)&IconTexType, (int*)&dummy},
	{"ButtonBgColor", assign_string, &IconBgColor, (int *)0},
	{"ButtonTextureColor", assign_string, &IconTexColor, (int *)0},
    {"ButtonMaxColors", obsolete, (char **)NULL, NULL},
	{"ButtonPixmap", assign_string, &IconPixmapFile, (int *)0},
    {"ButtonNoBorder", SetLookFlag, (char **)IconNoBorder, NULL},
    {"FrameNorth", SetFramePart, NULL, (int *)FR_N},
    {"FrameSouth", SetFramePart, NULL, (int *)FR_S},
    {"FrameEast",  SetFramePart, NULL, (int *)FR_E},
    {"FrameWest",  SetFramePart, NULL, (int *)FR_W},
    {"FrameNW", SetFramePart, NULL, (int *)FR_NW},
    {"FrameNE", SetFramePart, NULL, (int *)FR_NE},
    {"FrameSW", SetFramePart, NULL, (int *)FR_SW},
    {"FrameSE", SetFramePart, NULL, (int *)FR_SE},
    {"DecorateFrames", SetLookFlag, (char **)DecorateFrames, NULL},
	{"TitleButtonBalloonBorderWidth", obsolete, NULL, NULL },
	{"TitleButtonBalloonBorderColor", obsolete, NULL, NULL },
    {"TitleTextMode", SetTitleText, (char **)1, (int *)0},

    /* new stuff : */
	{"IconBox"							, SetBox, (char **)0, (int *)0},
    {"MyStyle"							, mystyle_parse, (char**)"afterstep", (int*)&MyStyleList},
    {"MyBackground"						, myback_parse, (char**)"asetroot", NULL},  /* pretending to be asteroot here */
    {"DeskBack"							, deskback_parse, NULL, NULL },
    {"*asetrootDeskBack"				, deskback_parse, NULL, NULL },        /* pretending to be asteroot here */
    {"MyFrame"							, myframe_parse, (char**)"afterstep", (int*)&MyFrameList},
    {"DefaultFrame"						, assign_string, (char**)&TmpLook.DefaultFrameName, (int*)0},
    {"DontDrawBackground"				, SetLookFlag, (char **)DontDrawBackground, NULL},
    {"CursorFore"						, assign_string, &TmpLook.CursorFore, (int *)0},    /* foreground color to be used for coloring pointer's cursor */
    {"CursorBack"						, assign_string, &TmpLook.CursorBack, (int *)0},    /* background color to be used for coloring pointer's cursor */
	/* this two a really from the feel */
	{"CustomCursor"						, SetCustomCursor, (char **)0, (int *)0},
	{"Cursor"							, SetCursor, (char **)0, (int *)0},
    /***********************************/
    {"IconsGrowVertically", SetLookFlag, (char **)IconsGrowVertically, (int *) 0},
	{"MenuPinOn"						, assign_string, &MenuPinOn, (int *)0},    /* menu pin */
    {"MArrowPixmap"						, assign_pixmap, (char **)&TmpLook.MenuArrow, (int *)0},   /* menu arrow */
    {"TitlebarNoPush"					, SetLookFlag, (char **)TitlebarNoPush, NULL},
    {"TextureMenuItemsIndividually"		, SetLookFlag, (char **)TxtrMenuItmInd,NULL},
    {"MenuMiniPixmaps"					, SetLookFlag, (char **)MenuMiniPixmaps, NULL},
	{"MenuShowUnavailable"				, SetLookFlag, (char **)MenuShowUnavailable, NULL },
	{"TitleTextAlign"					, SetInts, (char **)&TmpLook.TitleTextAlign, &dummy},
    {"TitleButtonSpacingLeft"			, SetInts, (char **)&TmpLook.TitleButtonSpacing[0], &dummy},
    {"TitleButtonSpacingRight"			, SetInts, (char **)&TmpLook.TitleButtonSpacing[1], &dummy},
    {"TitleButtonSpacing"				, SetInts2, (char **)&TmpLook.TitleButtonSpacing[0], &TmpLook.TitleButtonSpacing[1]},
    {"TitleButtonXOffsetLeft"			, SetInts, (char **)&TmpLook.TitleButtonXOffset[0], &dummy},
    {"TitleButtonXOffsetRight"			, SetInts, (char **)&TmpLook.TitleButtonXOffset[1], &dummy},
    {"TitleButtonXOffset"				, SetInts2, (char **)&TmpLook.TitleButtonXOffset[0], &TmpLook.TitleButtonXOffset[1]},
    {"TitleButtonYOffsetLeft"			, SetInts, (char **)&TmpLook.TitleButtonYOffset[0], &dummy},
    {"TitleButtonYOffsetRight"			, SetInts, (char **)&TmpLook.TitleButtonYOffset[1], &dummy},
    {"TitleButtonYOffset"				, SetInts2, (char **)&TmpLook.TitleButtonYOffset[0], &TmpLook.TitleButtonYOffset[1]},
    {"TitleButtonStyle"					, SetInts, (char **)&TmpLook.TitleButtonStyle, (int *)&dummy},
    {"TitleButtonOrder"					, SetTButtonOrder, NULL, NULL},
    {"ResizeMoveGeometry"				, assign_geometry, (char**)&TmpLook.resize_move_geometry, (int *)0},
    {"StartMenuSortMode"				, SetInts, (char **)&TmpLook.StartMenuSortMode, (int *)&dummy},
    {"DrawMenuBorders"					, SetInts, (char **)&TmpLook.DrawMenuBorders, (int *)&dummy},
    {"ButtonSize"						, SetInts, (char **)&TmpLook.ButtonWidth, (int *)&TmpLook.ButtonHeight},
    {"MinipixmapSize"					, SetInts, (char **)&TmpLook.minipixmap_width, (int *)&TmpLook.minipixmap_height},
    {"SeparateButtonTitle"				, SetLookFlag, (char **)SeparateButtonTitle, NULL},
    {"RubberBand"						, SetInts, (char **)&TmpLook.RubberBand, &dummy},
    {"DefaultStyle"						, assign_string, (char **)&MSWindowName[BACK_DEFAULT], (int *)0},
    {"FWindowStyle"						, assign_string, (char **)&MSWindowName[BACK_FOCUSED], (int *)0},
    {"UWindowStyle"						, assign_string, (char **)&MSWindowName[BACK_UNFOCUSED], (int *)0},
    {"SWindowStyle"						, assign_string, (char **)&MSWindowName[BACK_STICKY], (int *)0},
    {"MenuItemStyle"					, assign_string, (char **)&MSMenuName[MENU_BACK_ITEM], (int *)0},
    {"MenuTitleStyle"					, assign_string, (char **)&MSMenuName[MENU_BACK_TITLE], (int *)0},
    {"MenuHiliteStyle"					, assign_string, (char **)&MSMenuName[MENU_BACK_HILITE], (int *)0},
    {"MenuStippleStyle"					, assign_string, (char **)&MSMenuName[MENU_BACK_STIPPLE], (int *)0},
    {"MenuSubItemStyle"					, assign_string, (char **)&MSMenuName[MENU_BACK_SUBITEM], (int *)0},
    {"MenuHiTitleStyle"					, assign_string, (char **)&MSMenuName[MENU_BACK_HITITLE], (int *)0},
    {"MenuItemCompositionMethod"		, SetInts, (char **)&TmpLook.menu_icm, &dummy},
    {"MenuHiliteCompositionMethod"		, SetInts, (char **)&TmpLook.menu_hcm, &dummy},
    {"MenuStippleCompositionMethod"		, SetInts, (char **)&TmpLook.menu_scm, &dummy},
    {"MenuBalloonBorderHilite"			, bevel_parse, (char**)"afterstep", (int*)&(MenuBalloonConfig.BorderHilite)},
    {"MenuBalloonXOffset"				, SetInts, (char**)&(MenuBalloonConfig.XOffset), NULL},
    {"MenuBalloonYOffset"				, SetInts, (char**)&(MenuBalloonConfig.YOffset), NULL},
    {"MenuBalloonDelay"					, SetInts, (char**)&(MenuBalloonConfig.Delay), NULL},
    {"MenuBalloonCloseDelay"			, SetInts, (char**)&(MenuBalloonConfig.CloseDelay), NULL},
    {"MenuBalloonStyle"					, assign_string,   &(MenuBalloonConfig.Style), NULL},
    {"MenuBalloonTextPaddingX"			, SetInts, (char**)&(MenuBalloonConfig.TextPaddingX), NULL},
    {"MenuBalloonTextPaddingY"			, SetInts, (char**)&(MenuBalloonConfig.TextPaddingY), NULL},
    {"MenuBalloons"						, SetFlag2, (char**)BALLOON_USED, (int*)&(MenuBalloonConfig.set_flags)},
    {"ShadeAnimationSteps"				, SetInts, (char **)&TmpFeel.ShadeAnimationSteps, (int *)&dummy},
    {"TitleButtonBalloonBorderHilite"	, bevel_parse, (char**)"afterstep", (int*)&(BalloonConfig.BorderHilite)},
    {"TitleButtonBalloonXOffset"		, SetInts, (char**)&(BalloonConfig.XOffset), NULL},
    {"TitleButtonBalloonYOffset"		, SetInts, (char**)&(BalloonConfig.YOffset), NULL},
    {"TitleButtonBalloonDelay"			, SetInts, (char**)&(BalloonConfig.Delay), NULL},
    {"TitleButtonBalloonCloseDelay"		, SetInts, (char**)&(BalloonConfig.CloseDelay), NULL},
    {"TitleButtonBalloonStyle"			, assign_string, &(BalloonConfig.Style), NULL},
    {"TitleButtonBalloonTextPaddingX"	, SetInts, (char**)&(BalloonConfig.TextPaddingX), NULL},
    {"TitleButtonBalloonTextPaddingY"	, SetInts, (char**)&(BalloonConfig.TextPaddingY), NULL},
    {"TitleButtonBalloons"				, SetFlag2, (char**)BALLOON_USED, (int*)&(BalloonConfig.set_flags)},
    {"TitleButton"						, SetTitleButton, (char **)1, (int *)0},
    {"KillBackgroundThreshold"			, SetInts, (char**)&(TmpLook.KillBackgroundThreshold), NULL },
	{"DontAnimateBackground"			, SetFlag2, (char**)DontAnimateBackground, NULL},
	{"CoverAnimationSteps"				, SetInts, (char**)&(TmpFeel.desk_cover_animation_steps), NULL },
	{"CoverAnimationType"				, SetInts, (char**)&(TmpFeel.desk_cover_animation_type), NULL },
	{"AnimateDeskChange"				, SetFlag2, (char**)AnimateDeskChange, NULL },
	{"DontCoverDesktop"  				, SetFlag2, (char**)DontCoverDesktop, NULL },
	{"ButtonIconSpacing" 				, SetInts, (char **)&TmpLook.ButtonIconSpacing, &dummy},
    {"ButtonBevel"						, bevel_parse, (char**)"afterstep", (int*)&(TmpLook.ButtonBevel)},
    {"ButtonAlign"						, align_parse, (char**)"afterstep", (int*)&(TmpLook.ButtonAlign)},
	{"", 0, (char **)0, (int *)0}
};

#define PARSE_BUFFER_SIZE 	MAXLINELENGTH
char         *orig_tline = NULL;

/* the following values must not be reset other then by main
   config reading routine
 */
int           curr_conf_line = -1;
char         *curr_conf_file = NULL;

void
error_point ()
{
	fprintf (stderr, "AfterStep");
	if (curr_conf_file)
		fprintf (stderr, "(%s:%d)", curr_conf_file, curr_conf_line);
	fprintf (stderr, ":");
}

void
tline_error (const char *err_text)
{
	error_point ();
	fprintf (stderr, "%s in [%s]\n", err_text, orig_tline);
}

/***************************************************************
 **************************************************************/
void
obsolete (char *text, FILE * fd, char **arg, int *i)
{
    tline_error ("This option is obsolete. ");
}

/***************************************************************
 * get an icon
 **************************************************************/
void CheckImageManager()
{
    if( Scr.image_manager == NULL )
	{
		char *ppath = Environment->pixmap_path ;
		if( ppath == NULL )
			ppath = getenv( "IMAGE_PATH" );
		if( ppath == NULL )
			ppath = getenv( "PATH" );
		Scr.image_manager = create_image_manager( NULL, 2.2, ppath, getenv( "IMAGE_PATH" ), getenv( "PATH" ), NULL );
	}
}

Bool
GetIconFromFile (char *file, MyIcon * icon, int max_colors)
{
    CheckImageManager();
    memset( icon, 0x00, sizeof(icon_t));
    return load_icon(icon, file, Scr.image_manager );
}

ASImage *
GetASImageFromFile (char *file)
{
    ASImage *im;
    CheckImageManager();
	LOCAL_DEBUG_OUT( "loading image from file \"%s\"", file );
    im = get_asimage( Scr.image_manager, file, ASFLAGS_EVERYTHING, 100 );
    if( im == NULL )
        show_error( "failed to locate icon file \"%s\" in the IconPath and PixmapPath", file );
    return im;
}

/*
 * Copies a string into a new, malloc'ed string
 * Strips all data before the second quote. and strips trailing spaces and
 * new lines
 */

char         *
stripcpy3 (const char *source, const Bool Warn)
{
	const char *orig_source = source ;
	while ((*source != '"') && (*source != 0))
		source++;
	if (*source != 0)
		source++;
	while ((*source != '"') && (*source != 0))
		source++;
	if (*source == 0)
	{
		if (Warn)
			show_warning ("bad binding [%s]", orig_source);
		return 0;
	}
	source++;
	return stripcpy (source);
}


/*
 * initialize the old-style look variables
 */
void
init_old_look_variables (Bool free_resources)
{
    int i ;
	if (free_resources)
	{
        /* the fonts */
		if (Stdfont != NULL)
			free (Stdfont);
		if (Windowfont != NULL)
			free (Windowfont);
		if (Iconfont != NULL)
			free (Iconfont);
        for( i = 0 ; i < BACK_STYLES ; ++i )
        {
            if( WindowForeColor[i] )
                free( WindowForeColor[i] );
            if( WindowBackColor[i] )
                free( WindowBackColor[i] );
            if( WindowGradient[i] )
                free( WindowGradient[i] );
            if( WindowPixmap[i] )
                free( WindowPixmap[i] );
        }
        for( i = 0 ; i < MENU_BACK_STYLES ; ++i )
        {
            if( MenuForeColor[i] )
                free(MenuForeColor[i]);
            if( MenuBackColor[i] )
                free(MenuBackColor[i]);
            if( MenuGradient[i] )
                free(MenuGradient[i]);
            if( MenuPixmap[i] )
                free(MenuPixmap[i]);
        }

        if( IconBgColor )
            free(IconBgColor);
        if( IconTexColor )
            free(IconTexColor);
        if( IconPixmapFile )
            free(IconPixmapFile);
        if( TexTypes )
            free(TexTypes);
    }

	/* the fonts */
	Stdfont = NULL;
	Windowfont = NULL;
	Iconfont = NULL;

    for( i = 0 ; i < BACK_STYLES ; ++i )
    {
        WindowForeColor[i] = NULL ;
        WindowBackColor[i] = NULL ;
        WindowGradient[i] = NULL ;
        WindowPixmap[i] = NULL ;
    }
    for( i = 0 ; i < MENU_BACK_STYLES ; ++i )
    {
        MenuForeColor[i] = NULL ;
        MenuBackColor[i] = NULL ;
        MenuGradient[i] = NULL ;
        MenuPixmap[i] = NULL ;
    }
    IconBgColor = NULL ;
    IconTexColor = NULL ;
    IconPixmapFile = NULL ;
    /* miscellaneous stuff */
	TexTypes = NULL;
    TitleTextType = 0;
    TitleTextY = 0;
    IconTexType = TEXTURE_BUILTIN;

}


/*
 * merge the old variables into the new styles
 * the new styles have precedence
 */
void
merge_old_look_variables (MyLook *look)
{
    char         *button_style_names[BACK_STYLES] = 
													{AS_ICON_TITLE_MYSTYLE, 
													 AS_ICON_TITLE_UNFOCUS_MYSTYLE, 
													 AS_ICON_TITLE_STICKY_MYSTYLE, 
													 NULL,
													 "ButtonTitleDefault" };
    MyStyle      *button_styles[BACK_STYLES];
    int i ;

    for( i = 0 ; i < BACK_STYLES ; ++i )
        button_styles[i] = mystyle_list_find(look->styles_list, button_style_names[i]);

	/* the fonts */
	if (Stdfont != NULL)
	{
        if (load_font (Stdfont, &StdFont) == False)
            exit(1);
        else
            for( i = MENU_BACK_ITEM ; i < MENU_BACK_STYLES ; ++i )
                mystyle_inherit_font (look->MSMenu[i], &StdFont);
    }
	if (Windowfont != NULL)
	{
        if (load_font (Windowfont, &WindowFont) == False)
            exit(1);
        for( i = 0 ; i < BACK_STYLES ; ++i )
            mystyle_inherit_font (look->MSWindow[i], &WindowFont);
        mystyle_inherit_font (look->MSMenu[MENU_BACK_TITLE], &WindowFont);
	}
	if (Iconfont != NULL)
	{
        if (load_font (Iconfont, &IconFont) == False)
            exit (1);
        for( i = 0 ; i < BACK_STYLES ; ++i )
            mystyle_inherit_font (button_styles[i], &IconFont);
    }
	/* the text type */
    if (TitleTextType != 0)
	{
        for( i = 0 ; i < BACK_STYLES ; ++i )
			if( look->MSWindow[i] )
				if (!get_flags(look->MSWindow[i]->set_flags, F_TEXTSTYLE))
            	{
                	set_flags(look->MSWindow[i]->text_style, TitleTextType);
                	set_flags(look->MSWindow[i]->user_flags, F_TEXTSTYLE);
                	set_flags(look->MSWindow[i]->set_flags, F_TEXTSTYLE);
            	}
    }
	/* the colors */
	/* for black and white - ignore user choices */
	/* for color - accept user choices */
	if (Scr.d_depth > 1)
	{
        int wtype[BACK_STYLES] = {0};
        int mtype[MENU_BACK_STYLES] = {0};

        for( i = 0 ; i < BACK_STYLES ; ++i )
            wtype[i] = -1 ;
        for( i = 0 ; i < MENU_BACK_STYLES ; ++i )
            mtype[i] = -1 ;

		if (TexTypes != NULL)
            sscanf (TexTypes, "%i %i %i %i %i %i", &wtype[BACK_FOCUSED], &wtype[BACK_UNFOCUSED], &wtype[BACK_STICKY],
                                                   &mtype[MENU_BACK_TITLE], &mtype[MENU_BACK_ITEM], &mtype[MENU_BACK_HILITE]);

		if (IconTexType == TEXTURE_BUILTIN)
			IconTexType = -1;

		/* check for missing 1.4.5.x keywords */
        if (MenuForeColor[MENU_BACK_TITLE] == NULL && WindowForeColor[BACK_FOCUSED] != NULL)
            MenuForeColor[MENU_BACK_TITLE] = mystrdup(WindowForeColor[BACK_FOCUSED]);
        if (MenuBackColor[MENU_BACK_TITLE] == NULL && WindowBackColor[BACK_FOCUSED] != NULL)
            MenuBackColor[MENU_BACK_TITLE] = mystrdup(WindowBackColor[BACK_FOCUSED]);
        if (MenuForeColor[MENU_BACK_HILITE] == NULL && WindowForeColor[BACK_FOCUSED] != NULL)
            MenuForeColor[MENU_BACK_HILITE] = mystrdup(WindowForeColor[BACK_FOCUSED]);
        if (MenuBackColor[MENU_BACK_HILITE] == NULL && MenuBackColor[MENU_BACK_ITEM] != NULL)
		{
            mtype[MENU_BACK_HILITE] = mtype[MENU_BACK_ITEM];
            MenuBackColor[MENU_BACK_HILITE] = mystrdup (MenuBackColor[MENU_BACK_ITEM]);
            if (MenuGradient[MENU_BACK_HILITE] == NULL && MenuGradient[MENU_BACK_ITEM] != NULL)
                MenuGradient[MENU_BACK_HILITE] = mystrdup(MenuGradient[MENU_BACK_ITEM]);
            if (MenuPixmap[MENU_BACK_HILITE] == NULL && MenuPixmap[MENU_BACK_ITEM] != NULL)
                MenuPixmap[MENU_BACK_HILITE] = mystrdup(MenuPixmap[MENU_BACK_ITEM]);
        }
        for( i = 0 ; i < BACK_STYLES ; ++i )
            mystyle_merge_colors (look->MSWindow[i], wtype[i], WindowForeColor[i], WindowBackColor[i], WindowGradient[i], WindowPixmap[i]);
        for( i = 0 ; i < MENU_BACK_STYLES ; ++i )
            mystyle_merge_colors (look->MSMenu[i], mtype[i], MenuForeColor[i], MenuBackColor[i], MenuGradient[i], MenuPixmap[i]);

		{
            MyStyle      *button_pixmap = mystyle_list_find (look->styles_list, AS_ICON_MYSTYLE);

			/* icon styles automagically inherit from window title styles */
			if (button_pixmap != NULL)
			{
                mystyle_merge_styles (look->MSWindow[BACK_FOCUSED], button_pixmap, 0, 0);
                mystyle_merge_colors (button_pixmap, IconTexType, NULL, IconBgColor, IconTexColor, IconPixmapFile);
			}
		}
        for( i = 0 ; i < BACK_STYLES ; ++i )
            if (button_styles[i] != NULL)
                mystyle_merge_styles (look->MSWindow[i], button_styles[i], 0, 0);
    }
    init_old_look_variables (True);            /* no longer need those strings !!!! */
}


/*
 * Initialize feel variables
 */
void
InitFeel (ASFeel *feel, Bool free_resources)
{
	if( feel ) 
	{	
    	if (free_resources)
			destroy_asfeel( feel, True );
    	feel->magic = MAGIC_ASFEEL ;
		init_asfeel( feel );
	}
}

void 
merge_feel( ASFeel *to, ASFeel *from )
{
	to->deprecated_flags = from->deprecated_flags ;
    to->EdgeScrollX = from->EdgeScrollX ; 
	to->EdgeScrollY = from->EdgeScrollY ; 
    to->AutoRaiseDelay = from->AutoRaiseDelay ; 
    to->ClickTime = from->ClickTime ; 
    to->OpaqueMove = from->OpaqueMove ; 
    to->OpaqueResize = from->OpaqueResize ; 
    to->EdgeResistanceScroll = from->EdgeResistanceScroll  ; 
	to->EdgeResistanceMove = from->EdgeResistanceMove ; 
    to->EdgeResistanceDragScroll = from->EdgeResistanceDragScroll  ; 
    to->Xzap = from->Xzap ; 
    to->Yzap = from->Yzap ; 
    to->AutoReverse = from->AutoReverse ; 
    to->no_snaping_mod = from->no_snaping_mod ; 
    to->EdgeAttractionScreen = from->EdgeAttractionScreen ; 
    to->EdgeAttractionWindow = from->EdgeAttractionWindow ; 
    to->default_window_box_name = from->default_window_box_name ; 
    to->recent_submenu_items = from->recent_submenu_items ; 
	to->winlist_sort_order = from->winlist_sort_order ; 
    to->ShadeAnimationSteps = from->ShadeAnimationSteps ; 
    to->desk_cover_animation_steps = from->desk_cover_animation_steps ; 
	to->desk_cover_animation_type = from->desk_cover_animation_type ; 

}	 

void
ApplyFeel( ASFeel *feel )
{
    check_screen_panframes(ASDefaultScr);
	
}

/*
 * Initialize look variables
 */
void
InitLook (MyLook *look, Bool free_resources)
{
	int i ;
    /* actuall MyLook cleanup : */

    mylook_init (look, free_resources, LL_Everything );

    /* other related things : */
    if (free_resources)
	{
		/* icons */
        if( MenuPinOn != NULL )
            free( MenuPinOn );
        if( Scr.default_icon_box )
            destroy_asiconbox( &(Scr.default_icon_box));
        if( Scr.icon_boxes )
            destroy_ashash( &(Scr.icon_boxes));

        /* temporary old-style fonts : */
        unload_font (&StdFont);
        unload_font (&WindowFont);
        unload_font (&IconFont);
        DestroyMyStyleDefinitions (&MyStyleList);
        DestroyMyFrameDefinitions (&MyFrameList);
		if( LegacyFrameDef )
			DestroyMyFrameDefinitions( &LegacyFrameDef );
        if( BalloonConfig.Style )
            free( BalloonConfig.Style );
		for( i = 0  ; i < BACK_STYLES ; ++i )
			if( MSWindowName[i] )
				free( MSWindowName[i] );
		for( i = 0  ; i < MENU_BACK_STYLES ; ++i )
			if( MSMenuName[i] )
				free( MSMenuName[i] );
    }
    MenuPinOn = NULL;
    MenuPinOnButton = -1;

    Scr.default_icon_box = NULL ;
    Scr.icon_boxes = NULL ;

    /* temporary old-style fonts : */
    memset(&StdFont, 0x00, sizeof(MyFont));
    memset(&WindowFont, 0x00, sizeof(MyFont));
    memset(&IconFont, 0x00, sizeof(MyFont));

	MyStyleList = NULL ;
	for( i = 0  ; i < BACK_STYLES ; ++i )
		MSWindowName[i] = NULL ;
	for( i = 0  ; i < MENU_BACK_STYLES ; ++i )
		MSMenuName[i] = NULL ;

	MyFrameList = NULL ;
	LegacyFrameDef = NULL ;
    memset( &BalloonConfig, 0x00, sizeof(BalloonConfig));
}

void
merge_look( MyLook *to, MyLook *from )
{
	int i  ;
  	to->CursorFore = from->CursorFore ; 
    to->CursorBack = from->CursorBack ; 
	to->MenuArrow = from->MenuArrow ; 
	to->TitleTextAlign = from->TitleTextAlign;
	for( i = 0 ; i < 2 ; ++i ) 
	{
    	to->TitleButtonSpacing[i] = from->TitleButtonSpacing[i] ;
	    to->TitleButtonXOffset[i] = from->TitleButtonXOffset[i] ;
	    to->TitleButtonYOffset[i] = from->TitleButtonYOffset[i] ;
	}
	to->TitleButtonStyle = from->TitleButtonStyle ; 
    to->resize_move_geometry = from->resize_move_geometry ; 
    to->StartMenuSortMode = from->StartMenuSortMode ; 
    to->DrawMenuBorders = from->DrawMenuBorders ; 
    to->ButtonWidth  = (from->ButtonWidth == 0)? 64 :from->ButtonWidth  ; 
	to->ButtonHeight = (from->ButtonHeight== 0)? to->ButtonWidth :from->ButtonHeight ; 
	to->ButtonIconSpacing = from->ButtonIconSpacing;
	to->ButtonAlign  = from->ButtonAlign;
	to->ButtonBevel  = from->ButtonBevel;

    to->minipixmap_width  = (from->minipixmap_width == 0)? 24 : from->minipixmap_width ; 
	to->minipixmap_height = (from->minipixmap_height == 0)?to->minipixmap_width:from->minipixmap_height; 
	

	to->DefaultFrameName = from->DefaultFrameName ; 

    to->RubberBand = from->RubberBand ; 
    to->menu_icm = from->menu_icm ; 
    to->menu_hcm = from->menu_hcm ; 
    to->menu_scm = from->menu_scm ;
    to->KillBackgroundThreshold = from->KillBackgroundThreshold ; 
	to->desktop_animation_tint = from->desktop_animation_tint ;
	LOCAL_DEBUG_OUT( "desk_anime_tint = %lX", from->desktop_animation_tint );
}


void
make_styles (MyLook *look)
{
/* make sure the globals are defined */
    char *style_names[BACK_STYLES] = { "FWindow", "UWindow", "SWindow", NULL, "default"  };
    char *menu_style_names[MENU_BACK_STYLES] = { "MenuTitle", "MenuItem", "MenuHilite", "MenuStipple", "MenuSubItem", "MenuHiTitle" };
    int i ;

    for( i = 0 ; i < BACK_STYLES ; ++i )
		if( MSWindowName[i] )
			look->MSWindow[i] = mystyle_find_or_get_from_file (look->styles_list, MSWindowName[i]);
    if (look->MSWindow[BACK_DEFAULT] == NULL)
		look->MSWindow[BACK_DEFAULT] = mystyle_find_or_get_from_file (look->styles_list, "default");
	
	/* this is the last resort : */
    if (look->MSWindow[BACK_DEFAULT] == NULL)
	    look->MSWindow[BACK_DEFAULT] = mystyle_list_find_or_default (look->styles_list, "default");
		
    for( i = 0 ; i < BACK_STYLES ; ++i )
        if (look->MSWindow[i] == NULL && style_names[i] )
            look->MSWindow[i] = mystyle_list_new (look->styles_list, style_names[i]);

    for( i = 0 ; i < MENU_BACK_STYLES ; ++i )
		if( MSMenuName[i] )
			look->MSMenu[i] = mystyle_find_or_get_from_file (look->styles_list, MSMenuName[i]);

    for( i = 0 ; i < MENU_BACK_STYLES ; ++i )
        if (look->MSMenu[i] == NULL)
        {
            if( i == MENU_BACK_SUBITEM )
                look->MSMenu[i] = look->MSMenu[MENU_BACK_ITEM];
            else if( i == MENU_BACK_HITITLE )
                look->MSMenu[i] = look->MSWindow[BACK_FOCUSED];
            else
                look->MSMenu[i] = mystyle_list_new (look->styles_list, menu_style_names[i]);
        }
    if (mystyle_find_or_get_from_file (look->styles_list, "ButtonPixmap") == NULL)
        mystyle_list_new (look->styles_list, "ButtonPixmap");
    if (mystyle_find_or_get_from_file (look->styles_list, "ButtonTitleFocus") == NULL)
        mystyle_list_new (look->styles_list, "ButtonTitleFocus");
    if (mystyle_find_or_get_from_file (look->styles_list, "ButtonTitleSticky") == NULL)
        mystyle_list_new (look->styles_list, "ButtonTitleSticky");
    if (mystyle_find_or_get_from_file (look->styles_list, "ButtonTitleUnfocus") == NULL)
        mystyle_list_new (look->styles_list, "ButtonTitleUnfocus");
}

MyFrame *add_myframe_from_def(MyLook *look, MyFrameDefinition *fd, ASFlagType default_title_align )
{
	ASHashTable *list = look->FramesList;
    MyFrame *frame;
    int i ;

	frame = get_flags(fd->flags,MYFRAME_INHERIT_DEFAULTS)? 
				create_default_myframe(default_title_align) : create_myframe();
    
	frame->name = mystrdup( fd->name );
    for( i = 0 ; i < fd->inheritance_num ; ++i )
    {
		ASHashData hdata ;
        if( get_hash_item( list, AS_HASHABLE(fd->inheritance_list[i]), &hdata.vptr ) == ASH_Success )
            inherit_myframe( frame, hdata.vptr );
    }
    frame->parts_mask = (frame->parts_mask&(~fd->set_parts))|fd->parts_mask;
	LOCAL_DEBUG_OUT( "parts_mask == 0x%lX", frame->parts_mask );
    frame->set_parts |= fd->set_parts ;
    for( i = 0 ; i < FRAME_PARTS ; ++i )
    {
        if( fd->parts[i] )
            set_string(&(frame->part_filenames[i]), mystrdup(fd->parts[i]));
        if( get_flags( fd->set_part_size, 0x01<<i ) )
        {
            frame->part_width[i] = max(fd->part_width[i],1);
            frame->part_length[i] = max(fd->part_length[i],1);
        }
        if( IsPartFBevelSet(fd,i) )
            frame->part_fbevel[i] = fd->part_fbevel[i];
        if( IsPartUBevelSet(fd,i) )
            frame->part_ubevel[i] = fd->part_ubevel[i];
        if( IsPartSBevelSet(fd,i) )
            frame->part_sbevel[i] = fd->part_sbevel[i];
        if( get_flags( fd->set_part_align, 0x01<<i ) )
            frame->part_align[i] = fd->part_align[i];
    }
    frame->set_part_size  |= fd->set_part_size ;
    frame->set_part_bevel |= fd->set_part_bevel ;
    frame->set_part_align |= fd->set_part_align ;

    for( i = 0 ; i < FRAME_PARTS ; ++i )
        if( frame->part_filenames[i] )
            if( !get_flags( frame->set_part_align, 0x01<<i ) )
            {
                frame->part_align[i] = RESIZE_V|RESIZE_H ;
                set_flags( frame->set_part_align, 0x01<<i );
            }

    for( i = 0 ; i < BACK_STYLES ; ++i )
    {
        if( fd->title_styles[i] )
		{
            set_string(&(frame->title_style_names[i]), mystrdup(fd->title_styles[i]) );
			/* force load the MyStyle in question */
			mystyle_find_or_get_from_file (look->styles_list, fd->title_styles[i]);
		}
        if( fd->frame_styles[i] )
		{
            set_string(&(frame->frame_style_names[i]), mystrdup(fd->frame_styles[i]) );
			/* force load the MyStyle in question */
			mystyle_find_or_get_from_file (look->styles_list, fd->title_styles[i]);
		}
    }
    if( get_flags( fd->set_title_attr, MYFRAME_TitleFBevelSet ) )
        frame->title_fbevel = fd->title_fbevel;
    if( get_flags( fd->set_title_attr, MYFRAME_TitleUBevelSet ) )
        frame->title_ubevel = fd->title_ubevel;
    if( get_flags( fd->set_title_attr, MYFRAME_TitleSBevelSet ) )
        frame->title_sbevel = fd->title_sbevel;
    if( get_flags( fd->set_title_attr, MYFRAME_TitleAlignSet ) )
        frame->title_align = fd->title_align;
	if( get_flags( fd->set_title_attr, MYFRAME_LeftBtnAlignSet ) )
        frame->left_btn_align = fd->left_btn_align;
	if( get_flags( fd->set_title_attr, MYFRAME_RightBtnAlignSet ) )
        frame->right_btn_align = fd->right_btn_align;

    if( get_flags( fd->set_title_attr, MYFRAME_CondenseTitlebarSet ) )
        frame->condense_titlebar = fd->condense_titlebar;
    if( get_flags( fd->set_title_attr, MYFRAME_LeftTitlebarLayoutSet ) )
	{
        frame->left_layout = fd->left_layout;
		LOCAL_DEBUG_OUT( "LeftTitlebarLayout = 0x%lX", fd->left_layout );		
	}
    if( get_flags( fd->set_title_attr, MYFRAME_RightTitlebarLayoutSet ) )
        frame->right_layout = fd->right_layout;
	if( get_flags( fd->set_title_attr, MYFRAME_TitleFCMSet ) )
        frame->title_fcm = fd->title_fcm;
    if( get_flags( fd->set_title_attr, MYFRAME_TitleUCMSet ) )
        frame->title_ucm = fd->title_ucm;
    if( get_flags( fd->set_title_attr, MYFRAME_TitleSCMSet ) )
        frame->title_scm = fd->title_scm;

    if( get_flags( fd->set_title_attr, MYFRAME_TitleFHueSet ) )
		parse_hue( fd->title_fhue, &(frame->title_fhue) );
    if( get_flags( fd->set_title_attr, MYFRAME_TitleUHueSet ) )
        parse_hue( fd->title_uhue, &(frame->title_uhue) );
    if( get_flags( fd->set_title_attr, MYFRAME_TitleSHueSet ) )
        parse_hue( fd->title_shue, &(frame->title_shue) );
	
    if( get_flags( fd->set_title_attr, MYFRAME_TitleFSatSet ) )
        frame->title_fsat = fd->title_fsat;
    if( get_flags( fd->set_title_attr, MYFRAME_TitleUSatSet ) )
        frame->title_usat = fd->title_usat;
    if( get_flags( fd->set_title_attr, MYFRAME_TitleSSatSet ) )
        frame->title_ssat = fd->title_ssat;
	
    if( get_flags( fd->set_title_attr, MYFRAME_TitleHSpacingSet ) )
		frame->title_h_spacing = fd->title_h_spacing;
	if( get_flags( fd->set_title_attr, MYFRAME_TitleVSpacingSet ) )
		frame->title_v_spacing = fd->title_v_spacing;

	
	for( i = 0 ; i < MYFRAME_TITLE_BACKS ; ++i )
	{
		if( get_flags( fd->set_title_attr, MYFRAME_TitleBackAlignSet_Start<<i ) )
        	frame->title_backs_align[i] = fd->title_backs_align[i];

    	if( fd->title_backs[i] )
    	{
        	set_string(&(frame->title_back_filenames[i]), mystrdup(fd->title_backs[i]) );
        	if( !get_flags( fd->set_title_attr, MYFRAME_TitleBackAlignSet_Start<<i ) )
        	{
            	frame->title_backs_align[i] = FIT_LABEL_WIDTH ;
            	set_flags( fd->set_title_attr, MYFRAME_TitleBackAlignSet_Start<<i );
        	}
    	}
	}
    frame->set_title_attr |= fd->set_title_attr ;

    /* wee need to make sure that frame has such a
     * neccessary attributes as title align and title bevel : */
    if( !get_flags(frame->set_title_attr, MYFRAME_TitleFBevelSet ) )
        frame->title_fbevel = DEFAULT_TBAR_HILITE;
    if( !get_flags(frame->set_title_attr, MYFRAME_TitleUBevelSet ) )
        frame->title_ubevel = DEFAULT_TBAR_HILITE;
    if( !get_flags(frame->set_title_attr, MYFRAME_TitleSBevelSet ) )
        frame->title_sbevel = DEFAULT_TBAR_HILITE;
    if( !get_flags(frame->set_title_attr, MYFRAME_TitleAlignSet ) )
        frame->title_align = default_title_align;
	
	if( !get_flags( frame->set_title_attr, MYFRAME_LeftBtnAlignSet ) )
        frame->left_btn_align = ALIGN_VCENTER;
	if( !get_flags( frame->set_title_attr, MYFRAME_RightBtnAlignSet ) )
        frame->right_btn_align = ALIGN_VCENTER;

    set_flags( frame->set_title_attr, MYFRAME_TitleBevelSet|MYFRAME_TitleAlignSet );

	frame->set_flags = fd->set_flags ;
	frame->flags = fd->flags ;


    if( add_hash_item( list, AS_HASHABLE(frame->name), frame ) != ASH_Success )
	{
		LOCAL_DEBUG_OUT( "failed to add frame with the name \"%s\", currently list holds %ld frames", frame->name, list->items_num );
        destroy_myframe( &frame );
    }else
    {
        LOCAL_DEBUG_OUT( "added frame with the name \"%s\"", frame->name );
    }
    return frame;
}

void
fix_menu_pin_on( MyLook *look )
{
    if (MenuPinOn != NULL)
    {
        if( MenuPinOnButton < 0 || MenuPinOnButton >= TITLE_BUTTONS )
        {
            register int i = TITLE_BUTTONS ;

            while ( --i >= 0 )
            {
                if( look->buttons[i].unpressed.image == NULL && look->buttons[i].pressed.image == NULL )
                    break;
            }
			
            if( i >= 0 )
            {
                if( GetIconFromFile (MenuPinOn, &(Scr.Look.buttons[i].unpressed), 0) )
                {
                    int context = C_TButton0<<i ;
                    register int k ;
                    Scr.Look.buttons[i].width = Scr.Look.buttons[i].unpressed.image->width ;
                    Scr.Look.buttons[i].height = Scr.Look.buttons[i].unpressed.image->height ;
                    MenuPinOnButton = i ;

                    if( Scr.Look.buttons[i].context == C_NO_CONTEXT )
                        Scr.Look.buttons[i].context = context ;
                    context = Scr.Look.buttons[i].context ;
                    for( k = 0 ; k < TITLE_BUTTONS ; ++k )
                        if( Scr.Look.button_xref[k] == context )
                            break;
                    if( k == TITLE_BUTTONS )
                    {
                        while( --k >= 0 )
                            if( Scr.Look.button_xref[k] == C_NO_CONTEXT )
                            {
                                Scr.Look.button_xref[k] =context ;
                                break;
                            }
                    }
                    if( k >= 0 && k < TITLE_BUTTONS )
                        Scr.Look.ordered_buttons[k] = &(Scr.Look.buttons[i]) ;
                    else
                        show_warning( "there is no slot on the titlebar to place button %d into. Check yout TitleButtonOrder setting.", i );
                }else
                    MenuPinOnButton = -1 ;
            }
        }
        if( MenuPinOnButton >= 0 )
        {
            static char binding[128];
            sprintf( binding, "1 %d A PinMenu\n", MenuPinOnButton );
            /* also need to add mouse binding for this one */
            ParseMouseEntry (binding, NULL, NULL, NULL);
        }
        show_warning( "MenuPinOn setting is depreciated - instead add a Title button and bind PinMenu function to it." );
    }
}

void
FixLook( MyLook *look )
{
    ASFlagType default_title_align = ALIGN_LEFT ;
	int menu_font_size = 0 ;
    int i ;
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif
    /* make sure all needed styles are created */
#if defined(LOCAL_DEBUG) && !defined(NO_DEBUG_OUTPUT)
    PrintMyStyleDefinitions (MyStyleList);
#endif
    LOCAL_DEBUG_OUT( "MyStyleList %p", MyStyleList );
    if( MyStyleList )
	{
		MyStyleDefinition *sd ;
	    for( sd = MyStyleList ; sd != NULL ; sd = sd->next )
        {
            LOCAL_DEBUG_OUT( "processing MyStyleDefinition %p", sd );
            mystyle_create_from_definition( look->styles_list, sd );
        }
        DestroyMyStyleDefinitions (&MyStyleList);
	}
    make_styles (look);
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif

    /* merge pre-1.5 compatibility keywords */
    merge_old_look_variables (look);
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif

    /* fill in remaining members with the default style */
    mystyle_list_fix_styles (look->styles_list);
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif

	mylook_set_font_size_var (look);
	
	for( i = 0 ; i < MENU_BACK_STYLES ; ++i ) 
		if( look->MSMenu[i] )
		{
			int font_size = mystyle_get_font_height( look->MSMenu[i] );
			if( font_size > menu_font_size ) 
					menu_font_size = font_size ;
		}
	asxml_var_insert(ASXMLVAR_MenuFontSize, menu_font_size);


    mystyle_list_set_property (Scr.wmprops, look->styles_list);
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif

    if(look->TitleTextAlign == JUSTIFY_RIGHT )
        default_title_align = ALIGN_RIGHT ;
    else if(look->TitleTextAlign == JUSTIFY_CENTER )
        default_title_align = ALIGN_CENTER ;

	if( look->DefaultFrameName == NULL ) 
		look->DefaultFrameName = mystrdup("default");
    check_myframes_list( look );
    
	/* update frame geometries */
    if (get_flags( look->flags, DecorateFrames))
    {
        MyFrameDefinition *fd ;
        MyFrame *frame ;
        /* TODO: need to load the list as well (if we have any )*/
#if defined(LOCAL_DEBUG) && !defined(NO_DEBUG_OUTPUT)
        PrintMyFrameDefinitions (MyFrameList, 1);
#endif
        LOCAL_DEBUG_OUT( "MyFrameList %p", MyFrameList );
        for( fd = MyFrameList ; fd != NULL ; fd = fd->next )
        {
            LOCAL_DEBUG_OUT( "processing MyFrameDefinition %p", fd );
            if( (frame = add_myframe_from_def( look, fd, default_title_align|ALIGN_VCENTER )) != NULL ) 
            	myframe_load ( frame, Scr.image_manager );
        }
		if( LegacyFrameDef )
		{
            LOCAL_DEBUG_OUT( "processing legacy MyFrameDefinition %p", LegacyFrameDef );
			LegacyFrameDef->name = mystrdup(look->DefaultFrameName);
            if( (frame = add_myframe_from_def( look, LegacyFrameDef, default_title_align|ALIGN_VCENTER )) != NULL )
            	myframe_load ( frame, Scr.image_manager );
		}
        DestroyMyFrameDefinitions (&MyFrameList);
		DestroyMyFrameDefinitions (&LegacyFrameDef);
        LOCAL_DEBUG_OUT( "DefaultFrameName is \"%s\".", look->DefaultFrameName );
    }

	if( myframe_find( look->DefaultFrameName ) == NULL ) 
	{	
        MyFrame *dmf = create_default_myframe(default_title_align|ALIGN_VCENTER);
		dmf->name = mystrdup(look->DefaultFrameName);
		if( add_hash_item( look->FramesList, AS_HASHABLE(dmf->name), dmf ) != ASH_Success ) 
			destroy_myframe( &dmf );
	}

#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif

    /* checking that all the buttons have assigned slots in the button xref : */
    for( i = 0 ; i < TITLE_BUTTONS ; ++i )
    {
        if( Scr.Look.buttons[i].unpressed.image != NULL )
        {
            int context = C_TButton0<<i ;
            register int k ;
            if( Scr.Look.buttons[i].context == C_NO_CONTEXT )
                Scr.Look.buttons[i].context = context ;
            context = Scr.Look.buttons[i].context ;
            for( k = 0 ; k < TITLE_BUTTONS ; ++k )
                if( Scr.Look.button_xref[k] == context )
                    break;
            if( k == TITLE_BUTTONS )
            {
                while( --k >= 0 )
                    if( Scr.Look.button_xref[k] == C_NO_CONTEXT )
                    {
                        Scr.Look.button_xref[k] =context ;
                        break;
                    }
            }
            if( k >= 0 && k < TITLE_BUTTONS )
                Scr.Look.ordered_buttons[k] = &(Scr.Look.buttons[i]) ;
            else
                show_warning( "there is no slot on the titlebar to place button %d into. Check yout TitleButtonOrder setting.", i );
        }
    }

    /* checking sanity of the move-resize window geometry :*/
    if( (look->resize_move_geometry.flags&(HeightValue|WidthValue)) != (HeightValue|WidthValue))
	{
		unsigned int width = 0;
		unsigned int height = 0 ;
		mystyle_get_text_size( look->MSWindow[BACK_FOCUSED], " +88888 x +88888 ", &width, &height );
    	if( !get_flags(look->resize_move_geometry.flags, WidthValue ) )
	        look->resize_move_geometry.width = width + SIZE_VINDENT * 2;

	    if( !get_flags(look->resize_move_geometry.flags, HeightValue ) )
    	    look->resize_move_geometry.height = height + SIZE_VINDENT * 2;

	    set_flags( look->resize_move_geometry.flags, HeightValue|WidthValue );
	}
    if( look->supported_hints == NULL )
    {
        look->supported_hints = create_hints_list();
        enable_hints_support( look->supported_hints, HINTS_ICCCM );
        enable_hints_support( look->supported_hints, HINTS_Motif );
        enable_hints_support( look->supported_hints, HINTS_Gnome );
        enable_hints_support( look->supported_hints, HINTS_KDE );
        enable_hints_support( look->supported_hints, HINTS_ExtendedWM );
        enable_hints_support( look->supported_hints, HINTS_ASDatabase );
        enable_hints_support( look->supported_hints, HINTS_GroupLead );
        enable_hints_support( look->supported_hints, HINTS_Transient );
    }
    switch( look->TitleButtonStyle )
    {
        case 0 :
            look->TitleButtonXOffset[0] = look->TitleButtonXOffset[1] = 3;
            look->TitleButtonYOffset[0] = look->TitleButtonYOffset[1] = 3;
            break ;
        case 1 :
            look->TitleButtonXOffset[0] = look->TitleButtonXOffset[1] = 1;
            look->TitleButtonYOffset[0] = look->TitleButtonYOffset[1] = 1;
            break ;
    }

    /* now we need to go through all the deskconfigs and create generic
     * MyBackground for those that alreadyu do not have one */
    if( look->desk_configs )
    {
        ASHashIterator it ;
        if( start_hash_iteration( look->desk_configs, &it ) )
            do
            {
                MyDesktopConfig *dc = (MyDesktopConfig *)curr_hash_data( &it );
                MyBackground *myback = mylook_get_back(look, dc->back_name);
				LOCAL_DEBUG_OUT( "myback = %p, back_name = \"%s\"", myback, dc->back_name?dc->back_name:"<NULL>" );
                if( myback == NULL && dc->back_name != NULL )
                {
                    myback = create_myback( dc->back_name );
                    myback->type = MB_BackImage ;
                    myback->data = mystrdup( dc->back_name );
                    add_myback( look, myback );
                }
            }while( next_hash_item( &it ) );
    }else if( !get_flags( Scr.Look.flags, DontDrawBackground ) )
	{
		MyDesktopConfig *dc ;
		MyBackground *myback ;
		char * buf =safemalloc( strlen(DEFAULT_BACK_NAME)+1 );

		sprintf( buf, DEFAULT_BACK_NAME, 0 );
	    dc = create_mydeskconfig( 0, buf );
		free( buf );

		add_deskconfig( &(Scr.Look), dc );
       	myback = mylook_get_back(look, dc->back_name);
        if( myback == NULL  )
        {
            myback = create_myback( dc->back_name );
            myback->type = MB_BackImage ;
            add_myback( look, myback );
		}
	}
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif
}


/*
 * Initialize database variables
 */

void
InitDatabase (Bool free_resources)
{
	if (free_resources)
    {
        destroy_asdb( &Database );
        /* XResources : */
        destroy_user_database();
    }else
        Database = NULL ;
}

/*
 * Create/destroy window titlebar/buttons as necessary.
 */
Bool
redecorate_aswindow_iter_func(void *data, void *aux_data)
{
    ASWindow *asw = (ASWindow*)data;
    if(asw )
	{
		/* need to invalidate all MyStyles at this point ??? */
		
		invalidate_window_mystyles( asw );
		
		redecorate_window( asw, False );
        if( asw->internal && asw->internal->on_look_feel_changed )
            asw->internal->on_look_feel_changed( asw->internal, &Scr.Feel, &Scr.Look, ASFLAGS_EVERYTHING );
        on_window_status_changed( asw, True );
		set_flags( asw->internal_flags, ASWF_PendingShapeRemoval );
    }
    return True;
}

void 
advertise_tbar_props()
{
	ASTBarProps props ; 
	MyFrame *frame = myframe_find(NULL);
	MouseButton  *btn;
	int i, k ; 

	MyButton *close_btn = NULL;
	MyButton *maximize_btn = NULL;
	MyButton *minimize_btn = NULL;
	MyButton *shade_btn = NULL;
	MyButton *menu_btn = NULL;
	struct 
	{
		Atom *kind, *kind_down;
	 	int func;
		MyButton **pbtn;
	}buttons[] = 
	{	{ &_AS_BUTTON_CLOSE, 	&_AS_BUTTON_CLOSE_PRESSED, 		F_CLOSE, 	&close_btn },
		{ &_AS_BUTTON_CLOSE, 	&_AS_BUTTON_CLOSE_PRESSED, 		F_DELETE, 	&close_btn },
		{ &_AS_BUTTON_CLOSE, 	&_AS_BUTTON_CLOSE_PRESSED, 		F_DESTROY, 	&close_btn },
		{ &_AS_BUTTON_MAXIMIZE, &_AS_BUTTON_MAXIMIZE_PRESSED, 	F_MAXIMIZE, &maximize_btn },
		{ &_AS_BUTTON_MINIMIZE, &_AS_BUTTON_MINIMIZE_PRESSED, 	F_ICONIFY, 	&minimize_btn },
		{ &_AS_BUTTON_SHADE, 	&_AS_BUTTON_SHADE_PRESSED, 		F_SHADE, 	&shade_btn },
		{ &_AS_BUTTON_MENU, 	&_AS_BUTTON_MENU_PRESSED, 		F_POPUP, 	&menu_btn },
		{0,0,0,NULL}};
		

	memset( &props, 0x00, sizeof(props));
	if( get_flags( frame->set_title_attr, MYFRAME_TitleAlignSet ) )
	{
		props.align = frame->title_align ;
#if (NO_ALIGN==0)		
		if( props.align == 0 )  		   
			props.align = ~ALIGN_MASK ;
#endif
	}
	if( get_flags( frame->set_title_attr, MYFRAME_TitleFBevelSet|MYFRAME_TitleUBevelSet ) )
	{	
		props.bevel = frame->title_fbevel|frame->title_ubevel ;
#if (NO_HILITE==0)
		if( props.bevel == 0 )
			props.bevel = ~HILITE_MASK;
#endif			
	}
	props.title_h_spacing = frame->title_h_spacing ;
	props.title_v_spacing = frame->title_v_spacing ;
	props.buttons_h_border = max(Scr.Look.TitleButtonXOffset[0],Scr.Look.TitleButtonXOffset[1]);
	props.buttons_v_border = max(Scr.Look.TitleButtonYOffset[0],Scr.Look.TitleButtonYOffset[1]);
	props.buttons_spacing = max(Scr.Look.TitleButtonSpacing[0],Scr.Look.TitleButtonSpacing[1]);
		
    for (btn = Scr.Feel.MouseButtonRoot; btn != NULL; btn = btn->NextButton)
		if ( (btn->Context & C_TButtonAll) )
		{
			for( i = 0 ; buttons[i].pbtn != NULL ; ++i )
			{
				if( btn->fdata->func == buttons[i].func && *(buttons[i].pbtn) == NULL )
				{/* now lets find that button in look : */
				 	for( k = 0  ; k < TITLE_BUTTONS ; ++k ) 
						if ( Scr.Look.buttons[k].unpressed.image != NULL &&
						     (btn->Context&Scr.Look.buttons[k].context) != 0 )
						{
							*(buttons[i].pbtn) = &(Scr.Look.buttons[k]);
							break;	
						}
					break;	
				}	 
			}
		}
	props.buttons_num = 0 ; 
	for( i = 0 ; buttons[i].pbtn != NULL ; ++i )
	{
		MyButton *b = *(buttons[i].pbtn);
		if( b ) 
		{
			++props.buttons_num	;
			if( b->pressed.image != NULL && b->pressed.image != b->unpressed.image )
				++props.buttons_num	;
		}	 
	}
	props.buttons = safemalloc( props.buttons_num*sizeof(struct ASButtonPropElem));
	k = 0 ;
	for( i = 0 ; buttons[i].pbtn != NULL ; ++i ) 
	{
		MyButton *b = *(buttons[i].pbtn);
		if( b ) 
		{
			MyIcon *icon = &(b->unpressed);
			if( icon->pix == None ) 
				make_icon_pixmaps( icon, False );
	
			props.buttons[k].kind = *(buttons[i].kind);
			props.buttons[k].pmap = icon->pix;
			props.buttons[k].mask = icon->mask;
			props.buttons[k].alpha = icon->alpha;
	   		++k ;
			if( b->pressed.image && b->pressed.image != b->unpressed.image )
			{	  
				icon = &(b->pressed);
				if( icon->pix == None ) 
					make_icon_pixmaps( icon, False );
				props.buttons[k].kind = *(buttons[i].kind_down);
				props.buttons[k].pmap = icon->pix;
				props.buttons[k].mask = icon->mask;
				props.buttons[k].alpha = icon->alpha;
				++k ;
			}
		}	 
	}		

	set_astbar_props( Scr.wmprops, &props ); 
	free( props.buttons );
}


/*************************************************************************/
/* reading confiug files now :                                           */
/*************************************************************************/
int
ParseConfigFile (const char *file, char **tline)
{
    FILE         *fp = NULL;
	register char *ptr;

	/* memory management for parsing buffer */
	if (file == NULL)
		return -1;

    /* this should not happen, but still checking */
    if ((fp = fopen (file, "r")) == (FILE *) NULL)
	{
        show_error("can't open config file [%s] - skipping it for now.\nMost likely you have incorrect permissions on the AfterStep configuration dir.",
             file);
        return -1;
	}

	if (*tline == NULL)
		*tline = safemalloc (MAXLINELENGTH + 1);

	curr_conf_file = (char *)file;
	curr_conf_line = 0;
	while (fgets (*tline, MAXLINELENGTH, fp))
	{
		curr_conf_line++;
		/* prventing buffer overflow */
		*((*tline) + MAXLINELENGTH) = '\0';
		/* remove comments from the line */
		ptr = stripcomments (*tline);
		/* parsing the line */
		orig_tline = ptr;
        if (*ptr != '\0' && *ptr != '#')
			match_string (main_config, ptr, "error in config:", fp);
	}
	curr_conf_file = NULL;
	fclose (fp);
	return 1;
}

/*****************************************************************************
 *****************************************************************************
 * This routine is responsible for reading and parsing the config file
 ****************************************************************************
 ****************************************************************************/
/* MakeMenus - for those who can't remember LoadASConfig's real name        */
void
LoadASConfig (int thisdesktop, ASFlagType what)
{
    char            *tline = NULL;
    ASImageManager  *old_image_manager = NULL ;
    ASFontManager   *old_font_manager  = NULL ;
	FunctionData gtkrc_signal_func ;
	FunctionData kde_signal_func ;
	ASEvent dummy_event = {0};
	
	init_func_data( &gtkrc_signal_func );
    gtkrc_signal_func.func = F_SIGNAL_RELOAD_GTK_RCFILE ;
	init_func_data( &kde_signal_func );
    kde_signal_func.func = F_KIPC_SEND_MESSAGE_ALL ;
	kde_signal_func.func_val[0] = KIPC_PaletteChanged ; 
	kde_signal_func.func_val[1] = 0 ; 

    cover_desktop();

#ifndef DIFFERENTLOOKNFEELFOREACHDESKTOP
	/* only one look & feel should be used */
	thisdesktop = 0;
#endif /* !DIFFERENTLOOKNFEELFOREACHDESKTOP */

    show_progress("Loading configuration files ...");
    display_progress( True, "Loading configuration files ...");
    if (Session->overriding_file == NULL )
	{
        char *configfile;
        const char *const_configfile;
        if (get_flags(what, PARSE_BASE_CONFIG))
		{
			if( ReloadASEnvironment( &old_image_manager, &old_font_manager, NULL, get_flags(what, PARSE_LOOK_CONFIG), True ) )
			{
				if( !get_flags(what, PARSE_LOOK_CONFIG) )
				{
					if( old_image_manager != NULL || old_font_manager != NULL )
					{
                    	InitLook (&Scr.Look, True);
                    	set_flags(what, PARSE_LOOK_CONFIG);
					}
				}else
                	clear_flags(what, PARSE_BASE_CONFIG);
            }
        }else if(get_flags(what, PARSE_LOOK_CONFIG))
		{  /* must reload Image manager so that changed images would get updated */
			reload_screen_image_manager( ASDefaultScr, &old_image_manager );
		}

        if (get_flags(what, PARSE_LOOK_CONFIG))
		{
			stop_all_background_xfer();
			LoadColorScheme();

			/* now we can proceed to loading them look and theme */
            if( (const_configfile = get_session_file (Session, thisdesktop, F_CHANGE_LOOK, False) ) != NULL )
            {
                InitLook (&Scr.Look, True);

				memset( &TmpLook, 0x00, sizeof(TmpLook));
				TmpLook.magic = MAGIC_MYLOOK ;
				InitLook (&TmpLook, False );

				LOCAL_DEBUG_OUT( "desk_anime_tint = %lX", TmpLook.desktop_animation_tint );
                ParseConfigFile (const_configfile, &tline);


				LOCAL_DEBUG_OUT( "desk_anime_tint = %lX", TmpLook.desktop_animation_tint );
                show_progress("LOOK configuration loaded from \"%s\" ...", const_configfile);
                display_progress( True, "LOOK configuration loaded from \"%s\".", const_configfile);
#ifdef ASETROOT_FILE
				if( Scr.Look.desk_configs == NULL )
				{/* looks like there is no background information in the look file and we should be
				    getting it from the asetroot file : */

		            if( (configfile = make_session_file(Session, ASETROOT_FILE, False )) != NULL )
      			    {
      			        ParseConfigFile (configfile, &tline);
              			/* Save base filename to pass to modules */
				        show_progress("ROOT BACKGROUND configuration loaded from \"%s\" ...", configfile);
                        display_progress( True, "ROOT BACKGROUND configuration loaded from \"%s\" .", configfile);
              			free( configfile );
		            }
				}
#endif
				merge_look( &Scr.Look, &TmpLook );
				destroy_string( &(BalloonConfig.Style));
				destroy_string( &(MenuBalloonConfig.Style));
            }else
            {
                show_warning("LOOK configuration file cannot be found!");
                display_progress( True, "LOOK configuration file cannot be found!");
                clear_flags(what, PARSE_LOOK_CONFIG);
            }
        	if( UpdateGtkRC(Environment) ) 
				ExecuteFunction (&gtkrc_signal_func, &dummy_event, -1);
			if( !get_flags(Environment->flags,ASE_NoKDEGlobalsTheming ) ) 
				if( UpdateKCSRC() ) 
					ExecuteFunction (&kde_signal_func, &dummy_event, -1);
		}
        if (get_flags(what, PARSE_FEEL_CONFIG))
		{
            if( (const_configfile = get_session_file (Session, thisdesktop, F_CHANGE_FEEL, False) ) != NULL )
            {
				const char *ws_file = get_session_ws_file( Session, True );
				memset( &TmpFeel , 0x00, sizeof(TmpFeel));
				InitFeel (&TmpFeel, True);
                InitFeel (&Scr.Feel, True);
                if (tline == NULL)
                    tline = safemalloc (MAXLINELENGTH + 1);
				
				display_progress( True, "Reloading and merging desktop categories ...");
				ReloadCategories(False);
				UpdateCategoriesCache();
                
				display_progress( True, "Parsing menu entries and checking availability ...");
                MeltStartMenu (tline);
                display_progress( False, "Done..");
                ParseConfigFile (const_configfile, &tline);
                show_progress("FEEL configuration loaded from \"%s\" ...", const_configfile);
                display_progress( True, "FEEL configuration loaded from \"%s\" .", const_configfile);
                if( (configfile = make_session_file(Session, AUTOEXEC_FILE, False )) != NULL )
                {
                    ParseConfigFile (configfile, &tline);
                    show_progress("AUTOEXEC configuration loaded from \"%s\" ...", configfile);
                    display_progress( True, "AUTOEXEC configuration loaded from \"%s\" .", configfile);
                    free( configfile );
                }else
                {    
                    show_warning("AUTOEXEC configuration file cannot be found!");
                    display_progress( True, "AUTOEXEC configuration file cannot be found!");
                }        
				if( ws_file != NULL )
				{
                    ParseConfigFile (ws_file, &tline);
                    show_progress("WORKSPACE STATE configuration loaded from \"%s\" ...", ws_file);
                    display_progress( True, "WORKSPACE STATE configuration loaded from \"%s\".", ws_file);
				}else
                {    
                    show_progress("WORKSPACE STATE file cannot be found!");
                    display_progress( True, "WORKSPACE STATE file cannot be found!");
                }
				merge_feel( &Scr.Feel, &TmpFeel );
            }else
            {
                show_warning("FEEL configuration file cannot be found!");
                display_progress( True, "FEEL configuration file cannot be found!");
                clear_flags(what, PARSE_FEEL_CONFIG);
            }
        }
        if (get_flags(what, PARSE_DATABASE_CONFIG))
		{
			if( !ReloadASDatabase() )
			{
		        display_progress( True, "DATABASE configuration file cannot be found!");
                clear_flags(what, PARSE_DATABASE_CONFIG);
			}else
			{
				configfile  = make_session_file(Session, DATABASE_FILE, False );
	    	    display_progress( True, "DATABASE configuration loaded from \"%s\" .", configfile);
				free( configfile );
			}
		}
	} else
	{
		ReloadASEnvironment( &old_image_manager, &old_font_manager, NULL, True, True );

		LoadColorScheme();

		memset( &TmpLook, 0x00, sizeof(TmpLook));
		InitLook (&TmpLook, False );
		InitLook (&Scr.Look, True);
		memset( &TmpFeel, 0x00, sizeof(TmpFeel));
		InitFeel (&TmpFeel, False );
        InitFeel (&Scr.Feel, True);
        ParseConfigFile (Session->overriding_file, &tline);
		merge_look( &Scr.Look, &TmpLook );
		merge_feel( &Scr.Feel, &TmpFeel );

        ReloadASDatabase();
        show_progress("AfterStep configuration loaded from \"%s\" ...", Session->overriding_file);
        display_progress( True, "AfterStep configuration loaded from \"%s\".", Session->overriding_file);
        what = PARSE_EVERYTHING ;
		if( UpdateGtkRC(Environment) ) 
			ExecuteFunction (&gtkrc_signal_func, &dummy_event, -1);
		if( !get_flags(Environment->flags,ASE_NoKDEGlobalsTheming ) ) 
			UpdateKCSRC();
    }

	/* let's free the memory used for parsing */
	if (tline)
		free (tline);
    show_progress("Done loading configuration.");
    display_progress( True, "Done loading configuration.");

    check_desksize_sanity( ASDefaultScr );
	set_desktop_geometry_prop ( Scr.wmprops, Scr.VxMax+Scr.MyDisplayWidth, Scr.VyMax+Scr.MyDisplayHeight);

    if (get_flags(what, PARSE_FEEL_CONFIG))
	{
	    display_progress( True, "Applying Feel.");
		check_feel_sanity( &Scr.Feel );
        ApplyFeel( &Scr.Feel );
		asxml_var_insert(ASXMLVAR_MenuRecentSubmenuItems, Scr.Feel.recent_submenu_items );
	}

    if (get_flags(what, PARSE_LOOK_CONFIG))
    {
        FixLook( &Scr.Look );

		asxml_var_insert(ASXMLVAR_IconButtonWidth, Scr.Look.ButtonWidth);
		asxml_var_insert(ASXMLVAR_IconButtonHeight, Scr.Look.ButtonHeight);

		asxml_var_insert(ASXMLVAR_MinipixmapWidth, Scr.Look.minipixmap_width);
		asxml_var_insert(ASXMLVAR_MinipixmapHeight, Scr.Look.minipixmap_height);

		asxml_var_insert(ASXMLVAR_MenuShowMinipixmaps, get_flags(Scr.Look.flags, MenuMiniPixmaps)?1:0);
		asxml_var_insert(ASXMLVAR_MenuShowUnavailable, get_flags(Scr.Look.flags, MenuShowUnavailable)?1:0);
		asxml_var_insert(ASXMLVAR_MenuTxtItemsInd,     get_flags(Scr.Look.flags, TxtrMenuItmInd)?1:0);

        if( thisdesktop == Scr.CurrentDesk )
		{
		    MyBackground *new_back = get_desk_back_or_default( Scr.CurrentDesk, False );
        	SendPacket( -1, M_NEW_BACKGROUND, 1, 1);
			if( new_back && new_back->loaded_im_name ) 
			{
				free( new_back->loaded_im_name );  
				new_back->loaded_im_name = NULL ;
			}
            change_desktop_background( Scr.CurrentDesk );
		}
    }
	
	if (get_flags(what, PARSE_LOOK_CONFIG|PARSE_FEEL_CONFIG))
	{
		ReloadConfig(what);
    }

    if( get_flags(what, PARSE_BASE_CONFIG|PARSE_LOOK_CONFIG|PARSE_FEEL_CONFIG))
	{
    	int count = 0 ;
	    ASHashIterator  i;
		ARGB32 cursor_fore = ARGB32_White ;
		ARGB32 cursor_back = ARGB32_Black ;

		fix_menu_pin_on( &Scr.Look );
		
		/* also need to recolor cursors ! */
		if( Scr.Look.CursorFore )
			parse_argb_color( Scr.Look.CursorFore, &cursor_fore );
		if( Scr.Look.CursorBack )
	   		parse_argb_color( Scr.Look.CursorBack, &cursor_back );
		recolor_feel_cursors( &Scr.Feel, cursor_fore, cursor_back );
		XDefineCursor (dpy, Scr.Root, Scr.Feel.cursors[ASCUR_Default]);

	    display_progress( True, get_flags( Scr.Look.flags, MenuMiniPixmaps )?"Reloading menu pixmaps :":"Unloading menu pixmaps :");
		if( start_hash_iteration (Scr.Feel.Popups, &i) )
            do
            {
				MenuData *md = curr_hash_data(&i);
				if (!get_flags (Scr.Look.flags, MenuMiniPixmaps))
					free_menu_pmaps (md);
				else
				{
					char *name = md->name;
					Bool newline = ( count%10 == 0 );
					if( isdigit(name[0]) )
						if( md->first != NULL && md->first->fdata->func == F_TITLE ) 
				    		name = md->first->item;
					display_progress( newline, newline?"    %s":"%s", name);
					++count;

					reload_menu_pmaps( md, get_flags(what, PARSE_BASE_CONFIG) );
				}

            }while( next_hash_item( &i ));
		
	    display_progress( True, "Advertising titlebar properties ...");
		advertise_tbar_props();
	    display_progress( False, "Done.");
	}
	   
    /* force update of window frames */
    if (get_flags(what, PARSE_BASE_CONFIG|PARSE_LOOK_CONFIG|PARSE_FEEL_CONFIG|PARSE_DATABASE_CONFIG))
    {
        display_progress( True, "Redecorating client windows...");    
        iterate_asbidirlist( Scr.Windows->clients, redecorate_aswindow_iter_func, NULL, NULL, False );
        display_progress( False, "Done.");    
    }

    if( old_image_manager && old_image_manager != Scr.image_manager )
    {
        display_progress( True, "Unloading old images...");    
        if(Scr.RootImage && Scr.RootImage->imageman == old_image_manager )
        {
            safe_asimage_destroy(Scr.RootImage);
            Scr.RootImage = NULL ;
        }
        destroy_image_manager( old_image_manager, False );
        display_progress( False, "Done.");    
    }
    if( old_font_manager && old_font_manager != Scr.font_manager )
    {
        display_progress( True, "Unloading old fonts...");        
        destroy_font_manager( old_font_manager, False );
        display_progress( False, "Done.");    
    }

    ConfigureNotifyLoop();

    remove_desktop_cover();

	validate_rootpmap_props(Scr.wmprops);
	
	LOCAL_DEBUG_OUT( "TmpFeel.flags = 0x%lX, Scr.Feel.flags = 0x%lX", TmpFeel.flags, Scr.Feel.flags );
}

/*****************************************************************************
 *  This series of functions do actuall parsing of config options on per-line
 *  basis :
 *****************************************************************************/
/*****************************************************************************
 * Copies a text string from the config file to a specified location
 ****************************************************************************/

void
assign_string (char *text, FILE * fd, char **arg, int *junk)
{
	if( *arg )
		free( *arg );
	*arg = stripcpy2 (text, 0);
}

/*****************************************************************************
 *
 * Copies a PATH string from the config file to a specified location
 *
 ****************************************************************************/

void
assign_themable_path (char *text, FILE * fd, char **arg, int *junk)
{
    char         *tmp = stripcpy (text);
	int           tmp_len;
    char         *as_theme_data = NULL ; /*make_session_dir(Session, ICON_DIR, False); */

	replaceEnvVar (&tmp);
    if( as_theme_data )
    {
        tmp_len = strlen (tmp);
        *arg = safemalloc (tmp_len + 1 + strlen (as_theme_data) + 1);
        strcpy (*arg, tmp);
        (*arg)[tmp_len] = ':';
        strcpy ((*arg) + tmp_len + 1, as_theme_data);
        free (tmp);
        free (as_theme_data);
    }else
        *arg = tmp;
}


void
assign_path (char *text, FILE * fd, char **arg, int *junk)
{
	*arg = stripcpy (text);
	replaceEnvVar (arg);
}

void
assign_geometry (char *text, FILE * fd, char **arg, int *junk)
{
    ASGeometry *geom = (ASGeometry*)arg ;

    geom->x = geom->y = 0;
    geom->width = geom->height = 1;
    geom->flags = 0 ;
    parse_geometry (text, &(geom->x), &(geom->y), &(geom->width), &(geom->height), &(geom->flags));
}

/*****************************************************************************
 * Loads a pixmap to the assigned location
 ****************************************************************************/
void
assign_pixmap (char *text, FILE * fd, char **arg, int *junk)
{
    char *fname = NULL ;
    if( parse_filename(text, &fname) != text )
    {
        MyIcon **picon = (MyIcon**)arg ;
        *picon = safecalloc( 1, sizeof(icon_t));
        GetIconFromFile (fname, *picon, -1);
        free (fname);
    }
}

/****************************************************************************
 *  Read TitleText Controls
 ****************************************************************************/

void
SetTitleText (char *tline, FILE * fd, char **junk, int *junk2)
{
	int           ttype, y;

    sscanf (tline, "%d %d", &ttype, &y);
    TitleTextType = ttype;
    TitleTextY = y;
}

/****************************************************************************
 *
 *  Read Titlebar pixmap button
 *
 ****************************************************************************/

void
SetTitleButton (char *tline, FILE * fd, char **junk, int *junk2)
{
	int           num;
    char          *files[2] = {NULL, NULL};
    int           offset = 0;
	int           n;

    if ((n = sscanf (tline, "%d", &num)) <= 0)
	{
        show_error("wrong number of parameters given with TitleButton in [%s]", tline );
		return;
	}
    if (num < 0 || num >= TITLE_BUTTONS)
	{
        show_error("invalid Titlebar button number: %d", num);
		return;
	}

    /* going the hard way to prevent buffer overruns */
    while (isspace (tline[offset]))   offset++;
    while (isdigit (tline[offset]))   offset++;
    while (isspace (tline[offset]))   offset++;

    tline = parse_filename( &(tline[offset]), &(files[0]));
    offset = 0 ;
    while( isspace(tline[offset]) ) ++offset;
    if( tline[offset] != '\0' )
        parse_filename( &(tline[offset]), &(files[1]));

    if( !load_button( &(Scr.Look.buttons[num]), files, Scr.image_manager ) )
        show_error( "Failed to load image files specified for TitleButton %d [%s]", num, tline);
    if( files[0] )
        free(files[0] ) ;
    if( files[1] )
        free(files[1] ) ;
}

/*****************************************************************************
 *
 * Changes a cursor def.
 *
 ****************************************************************************/

void
SetCursor (char *text, FILE * fd, char **arg, int *junk)
{
	int           num, cursor_num, cursor_style;

	num = sscanf (text, "%d %d", &cursor_num, &cursor_style);
	if ((num != 2) || (cursor_num >= MAX_CURSORS) || (cursor_num < 0))
		show_warning( "bad Cursor number in [%s]", text );
	else
    {
        Cursor new_c = XCreateFontCursor (dpy, cursor_style);
        if( new_c )
        {
            if( Scr.Feel.cursors[cursor_num] && Scr.Feel.cursors[cursor_num] != Scr.standard_cursors[cursor_num])
                XFreeCursor( dpy, Scr.Feel.cursors[cursor_num] );
            Scr.Feel.cursors[cursor_num] = new_c ;
			LOCAL_DEBUG_OUT( "New X Font cursor %lX created for cursor_num %d", new_c, cursor_num );
        }
    }
}

void
SetCustomCursor (char *text, FILE * fd, char **arg, int *junk)
{
	int           num, cursor_num;
	char          f_cursor[1024], f_mask[1024];
	Pixmap        cursor = None, mask = None;
	unsigned int  width, height;
	int x, y;
	XColor        fore, back;
	char         *path;
    Cursor new_c ;

	num = sscanf (text, "%d %s %s", &cursor_num, f_cursor, f_mask);
	if ((num != 3) || (cursor_num >= MAX_CURSORS) || (cursor_num < 0))
	{
		show_warning( "bad Cursor number in [%s]", text );
		return;
	}

	path = find_file(f_mask, Environment->cursor_path, R_OK);
	if (path)
	{
		XReadBitmapFile (dpy, Scr.Root, path, &width, &height, &mask, &x, &y);
		free (path);
	} else
	{
		show_warning( "Cursor mask requested in [%s] could not be found", text );
		return;
	}

	path = find_file(f_cursor, Environment->cursor_path, R_OK);
	if (path)
	{
		XReadBitmapFile (dpy, Scr.Root, path, &width, &height, &cursor, &x, &y);
		free (path);
	} else
	{
		show_warning( "Cursor bitmap requested in [%s] could not be found", text );
		return;
	}

	fore.pixel = Scr.asv->black_pixel;
	back.pixel = Scr.asv->white_pixel;
	XQueryColor (dpy, Scr.asv->colormap, &fore);
	XQueryColor (dpy, Scr.asv->colormap, &back);

	if (cursor == None || mask == None)
	{
		show_warning( "Cursor mask or bitmap requested in [%s] could not be loaded", text );
		return;
	}

    new_c = XCreatePixmapCursor (dpy, cursor, mask, &fore, &back, x, y);
    if( new_c )
    {
        if( Scr.Feel.cursors[cursor_num] && Scr.Feel.cursors[cursor_num] != Scr.standard_cursors[cursor_num])
            XFreeCursor( dpy, Scr.Feel.cursors[cursor_num] );
        Scr.Feel.cursors[cursor_num] = new_c ;
		LOCAL_DEBUG_OUT( "New Custom X cursor created for cursor_num %d", cursor_num );
    }
    XFreePixmap (dpy, mask);
    XFreePixmap (dpy, cursor);
    ASSync(False);
    LOCAL_DEBUG_OUT( "mask %lX and cursor %lX freed", mask, cursor );
}

/*****************************************************************************
 *
 * Sets a boolean flag to true
 *
 ****************************************************************************/

void
SetFlag (char *text, FILE * fd, char **arg, int *another)
{
    Scr.Feel.flags |= (unsigned long)arg;
	if (another)
	{
		long          i = strtol (text, NULL, 0);
		if (i)
            Scr.Feel.flags |= (unsigned long)another;
	}
}

void
SetLookFlag (char *text, FILE * fd, char **arg, int *another)
{
	unsigned long *flags = (unsigned long *)another;
	char         *ptr;
	int           val = strtol (text, &ptr, 0);

	if (flags == NULL)
        flags = &Scr.Look.flags;
	if (ptr != text && val == 0)
		*flags &= ~(unsigned long)arg;
	else
		*flags |= (unsigned long)arg;
}

void
SetFlag2 (char *text, FILE * fd, char **arg, int *var)
{
	unsigned long *flags = (unsigned long *)var;
	char         *ptr;
	int           val = strtol (text, &ptr, 0);

	if (flags == NULL)
        flags = &Scr.Feel.flags;
	if (ptr != text && val == 0)
		*flags &= ~(unsigned long)arg;
	else
		*flags |= (unsigned long)arg;
}

/*****************************************************************************
 *
 * Reads in one or two integer values
 *
 ****************************************************************************/

void
SetInts (char *text, FILE * fd, char **arg1, int *arg2)
{
	if (arg2 == NULL)
	    sscanf (text, "%d", (int *)arg1);
	else
	    sscanf (text, "%d%*c%d", (int *)arg1, (int *)arg2);
/*    LOCAL_DEBUG_OUT( "text=[%s], arg1=%p, Scr.Feel.Autoreverse = %p, res = %d", text, arg1, &(Scr.Feel.AutoReverse), *((int*)arg1) );*/
}

void
SetInts2 (char *text, FILE * fd, char **arg1, int *arg2)
{
    sscanf (text, "%d", (int *)arg1);
	if( arg2 )
		*arg2 = *(int*)arg1 ;
/*    LOCAL_DEBUG_OUT( "text=[%s], arg1=%p, Scr.Feel.Autoreverse = %p, res = %d", text, arg1, &(Scr.Feel.AutoReverse), *((int*)arg1) );*/
}

/*****************************************************************************
 *
 * Reads in a list of mouse button numbers
 *
 ****************************************************************************/

void
SetButtonList (char *text, FILE * fd, char **arg1, int *arg2)
{
	int           i, b;
	char         *next;

	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		b = (int)strtol (text, &next, 0);
		if (next == text)
			break;
		text = next;
		if (*text == ',')
			text++;
		if ((b > 0) && (b <= MAX_MOUSE_BUTTONS))
            Scr.Feel.RaiseButtons |= 1 << b;
	}
    set_flags(Scr.Feel.flags, ClickToRaise);
}


/*****************************************************************************
 *
 * Reads Dimensions for an icon box from the config file
 *
 ****************************************************************************/

void
SetBox (char *text, FILE * fd, char **arg, int *junk)
{
    int x1 = 0, y1 = 0, x2 = Scr.MyDisplayWidth, y2 = Scr.MyDisplayHeight ;
    int num;

    /* not a standard X11 geometry string :*/
	num = sscanf (text, "%d%d%d%d", &x1, &y1, &x2, &y2);

	/* check for negative locations */
	if (x1 < 0)
		x1 += Scr.MyDisplayWidth;
	if (y1 < 0)
		y1 += Scr.MyDisplayHeight;

	if (x2 < 0)
		x2 += Scr.MyDisplayWidth;
	if (y2 < 0)
		y2 += Scr.MyDisplayHeight;

    if (x1 >= x2 || y1 >= y2 ||
		x1 < 0 || x1 > Scr.MyDisplayWidth || x2 < 0 || x2 > Scr.MyDisplayWidth ||
		y1 < 0 || y1 > Scr.MyDisplayHeight || y2 < 0 || y2 > Scr.MyDisplayHeight)
    {
        show_error("invalid IconBox '%s'", text);
    }else
	{
        int box_no = Scr.Look.configured_icon_areas_num;
        Scr.Look.configured_icon_areas = realloc( Scr.Look.configured_icon_areas, (box_no+1)*sizeof(ASGeometry));
        Scr.Look.configured_icon_areas[box_no].x = x1 ;
        Scr.Look.configured_icon_areas[box_no].y = y1 ;
        Scr.Look.configured_icon_areas[box_no].width = x2-x1 ;
        Scr.Look.configured_icon_areas[box_no].height = y2-y1 ;
        Scr.Look.configured_icon_areas[box_no].flags = XValue|YValue|WidthValue|HeightValue ;
        if( x1 > Scr.MyDisplayWidth-x2 )
            Scr.Look.configured_icon_areas[box_no].flags |= XNegative ;
        if( y1 > Scr.MyDisplayHeight-y2 )
            Scr.Look.configured_icon_areas[box_no].flags |= YNegative ;
        ++Scr.Look.configured_icon_areas_num;
    }
}

void
SetFramePart (char *text, FILE * fd, char **frame, int *id)
{
    char *fname = NULL;
    if( parse_filename (text, &fname) != text )
    {
		union {
			int *ptr ;
			int id ;
		}ptr_id ;
		ptr_id.ptr = id ;
		if( LegacyFrameDef == NULL )
		{
			AddMyFrameDefinition(&LegacyFrameDef);
			LegacyFrameDef->name = mystrdup("default");
		}
		show_warning( "Frame* definitions are deprecated in look. Please use MyFrame ... ~MyFrame structures instead.%s","");
		set_string_value (&(LegacyFrameDef->parts[ptr_id.id]), fname, &(LegacyFrameDef->set_parts), (0x01<<ptr_id.id));
        set_flags( LegacyFrameDef->parts_mask, (0x01<<ptr_id.id));
    }
}

void
SetModifier (char *text, FILE * fd, char **mod, int *junk2)
{
    int *pmod = (int*)mod ;
    if( pmod )
        *pmod = parse_modifier( text );
}

void
SetTButtonOrder(char *text, FILE * fd, char **unused1, int *unused2)
{
    unsigned int *xref = (unsigned int*)&(Scr.Look.button_xref[0]);
    unsigned int *rbtn = (unsigned int*)&(Scr.Look.button_first_right) ;
    if( xref && rbtn )
    {
        register int i = 0, btn = 0;
        *rbtn = TITLE_BUTTONS ;
        while( !isspace(text[i]) && text[i] != '\0' )
        {
            int context = C_NO_CONTEXT ;
            switch(text[i])
            {
                case '0' : context = C_TButton0;  break ;
                case '1' : context = C_TButton1;  break ;
                case '2' : context = C_TButton2;  break ;
                case '3' : context = C_TButton3;  break ;
                case '4' : context = C_TButton4;  break ;
                case '5' : context = C_TButton5;  break ;
                case '6' : context = C_TButton6;  break ;
                case '7' : context = C_TButton7;  break ;
                case '8' : context = C_TButton8;  break ;
                case '9' : context = C_TButton9;  break ;
                case 'T' :
                case 't' : context = C_TITLE ; break;
                default:
                    show_warning("invalid context specifier '%c' in TitleButtonOrder setting", text[i] );
            }
            if( context == C_TITLE )
            {
                *rbtn = btn ;
            }else if( context != C_NO_CONTEXT )
            {
                xref[btn] = context ;

                if( ++btn >= TITLE_BUTTONS )
                    break;
            }
            ++i;
        }
        while( btn < TITLE_BUTTONS )
        {
            xref[btn] = C_NO_CONTEXT ;
            ++btn ;
        }
    }
}


/****************************************************************************
 *
 * These routines put together files from start directory
 *
 ***************************************************************************/

void dirtree_print_tree (dirtree_t * tree, int depth);

int
MeltStartMenu (char *buf)
{
	char         *as_start = NULL;
	dirtree_t    *tree;

    switch (Scr.Look.StartMenuSortMode)
	{
	 case SORTBYALPHA:
		 dirtree_compar_list[0] = dirtree_compar_base_order;
		 dirtree_compar_list[1] = dirtree_compar_order;
		 dirtree_compar_list[2] = dirtree_compar_type;
		 dirtree_compar_list[3] = dirtree_compar_alpha;
		 dirtree_compar_list[4] = NULL;
		 break;

	 case SORTBYDATE:
		 dirtree_compar_list[0] = dirtree_compar_base_order;
		 dirtree_compar_list[1] = dirtree_compar_order;
		 dirtree_compar_list[2] = dirtree_compar_type;
		 dirtree_compar_list[3] = dirtree_compar_mtime;
		 dirtree_compar_list[4] = NULL;
		 break;

	 default:
		 dirtree_compar_list[0] = NULL;
		 break;
	}

	/*
	 *    Here we test the existence of various
	 *    directories used for the generation.
	 */

    as_start = make_session_dir (Session, START_DIR, False);
    tree = dirtree_new_from_dir (as_start);
    show_progress("MENU loaded from \"%s\" ...", as_start);
	free (as_start);
	
#ifdef FIXED_DIR
	{
        char         *as_fixeddir = make_session_dir (Session, FIXED_DIR, False);

		if (CheckDir (as_fixeddir) == 0)
		{
			dirtree_t    *fixed_tree = dirtree_new_from_dir (as_fixeddir);

			free (as_fixeddir);
			
			dirtree_move_children (tree, fixed_tree);
			dirtree_delete (fixed_tree);
            show_progress("FIXED MENU loaded from \"%s\" ...", as_fixeddir);
		} else
            show_error("unable to locate the fixed menu directory at \"%s\"", as_fixeddir);
		free (as_fixeddir);
	}
#endif /* FIXED_DIR */

	dirtree_parse_include (tree);
	dirtree_remove_order (tree);
	dirtree_merge (tree);
	dirtree_sort (tree);
/*	dirtree_print_tree( tree, 0) ; */
	dirtree_set_id (tree, 0);
	/* make sure one copy of the root menu uses the name "0" */
	(*tree).flags &= ~DIRTREE_KEEPNAME;

    dirtree_make_menu2 (tree, buf, True);
	/* to keep backward compatibility, make a copy of the root menu with
	 * the name "start" */
	{
		if ((*tree).name != NULL)
			free ((*tree).name);
		(*tree).name = mystrdup ("start");
		(*tree).flags |= DIRTREE_KEEPNAME;
        dirtree_make_menu2 (tree, buf, False);
	}
	/* cleaning up cache of the searcher */
	is_executable_in_path (NULL);

	dirtree_delete (tree);
	return 0;
}

void deskback_parse(char *text, FILE * fd, char **junk, int *junk2)
{
    register int i = 0;
    int desk = atoi(text);
    char *data = NULL ;
    MyDesktopConfig *dc = NULL ;

    if( !IsValidDesk(desk) )
    {
        show_error( "invalid desktop number in: \"%s\"", text);
        return;
    }

    while( isdigit(text[i]) ) ++i ;
    if( i == 0 || !isspace(text[i]) )
    {
        show_error( "missing desktop number in: \"%s\"", text);
        return;
    }

    while( isspace(text[i]) ) ++i ;
    if( text[i] == '#' )
        return;

    data = stripcpy2 (&(text[i]), 0);
    if( data == NULL )
    {
        show_error( "DeskBack option with no name of the relevant MyBackground: \"%s\"", text);
        return ;
    }
    LOCAL_DEBUG_OUT("desk(%d)->data(\"%s\")->text(%s)", desk, data, text );
    dc = create_mydeskconfig( desk, data );
    add_deskconfig( &(Scr.Look), dc );
    free( data );
}

/****************************************************************************
 *
 * Matches text from config to a table of strings, calls routine
 * indicated in table.
 *
 ****************************************************************************/

void
match_string (struct config *table, char *text, char *error_msg, FILE * fd)
{
    register int i ;
    table = find_config (table, text);
	if (table != NULL)
	{
        i = strlen (table->keyword);
        while(isspace(text[i])) ++i;
        table->action (&(text[i]), fd, table->arg, table->arg2);
	} else
		tline_error (error_msg);
}

#endif

