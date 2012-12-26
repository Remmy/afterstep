/*
 * Copyright (C) 1998 Sasha Vasko <sasha at aftercode.net>
 * Copyright (C) 1996 Frederic Cordier <cordier@cui.unige.ch>
 * Copyright (C) 1998 Guylhem Aznar <guylhem@oeil.qc.ca>
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

#include "../../configure.h"

#include "types.h"

#ifdef DEBUG			/* For debugging */
#include <unchecked.h>
#endif

/* Variables utilise par l'analyseur syntaxique */
extern ScriptProp *scriptprop;
extern int nbobj;		/* Nombre d'objets */
extern int numligne;		/* Numero de ligne */
extern TabObj *tabobj;		/* Tableau d'objets, limite=100 */
extern char **TabVVar;		/* Tableau des variables du sript */
extern int TabIdObj[101];
extern Bloc **TabIObj;
extern CaseObj *TabCObj;
#ifdef DEBUG
extern int __bounds_debug_no_checking;
#endif

/* Constante de couleurs utilise dans le tableau TabColor */
#define black 0
#define white 1
#define back 2
#define fore 3
#define shad 4
#define li 5


/* Variables globales */
char *ScriptName;		/* Nom du fichier contenat le script decrivant le GUI */
char *MyName;
int screen;
Display *dpy;
char *pixmapPath = NULL;	/* path to find scripts & pixmaps */
char *ScriptPath = NULL;

int fd[2];			/* pipe pair */
int fd_err;
int x_fd;			/* fd for X */
Window ref;

X11base *x11base;		/* Pour le serveur X */
TypeBuffSend BuffSend;		/* Pour les communication entre script */

int grab_server = 0;
struct XObj *tabxobj[100];
char *Scrapt;

Atom propriete, type;
static Atom wm_del_win;
/* static void WMProtocolsHandler(); */

extern void InitCom ();
extern void (*TabCom[30]) (int NbArg, long *TabArg);

ScreenInfo Scr;

void
Debug ()
{
  int i, j;

  for (j = 1; j <= nbobj; j++)
    for (i = 0; i <= TabCObj[TabIdObj[j]].NbCase; i++)
      {
	/* Execution du bloc d'instruction */
	fprintf (stderr, "Id de l'objet %d\n", TabIdObj[j]);
	fprintf (stderr, "Nb Instruction %d\n", TabIObj[TabIdObj[j]][i].NbInstr);
      }

}

void yyparse (void);

/* Lecture du fichier contenant le scipt */
void
ReadConfig (char *ScriptName)
{
  extern FILE *yyin;
  char *s;

  s = (char *) find_file(ScriptName, ScriptPath, 4);

  yyin = fopen (s, "r");
  if (yyin == NULL)
    {
      fprintf (stderr, "Can't open the script %s", s);
      exit (1);
    }
  /* On ne redefini pas yyout qui est la sortie standard */

  /* Application de l'analyseur syntaxique et lexical */
  yyparse ();
  /* Fermeture du script */

  fclose (yyin);
}

/* Quitter par l'option Delete du bouton de la fenetre */
void
DeadPipe (int nonsense)
{
  Quit (0, NULL);
}

/* Lecture du fichier base */
void
ParseOptions (char *filename)
{
  FILE *fd = (FILE *) 0;
  char line[MAXLINELENGTH];
  char *tline, *orig_tline;
  int Clength;

  fd = fopen (filename, "r");
  if (fd == (FILE *) 0)
    {
      fprintf (stderr, "%s: can't open config file %s\n", MyName, filename);
      exit (1);
    }
  tline = fgets (line, (sizeof line) - 1, fd);
  orig_tline = tline;
  Clength = strlen (MyName);
  while (tline != (char *) 0)
    {
      while (isspace (*tline))
	tline++;
      if ((strlen (&tline[0]) > 1) && (mystrncasecmp (tline, "PixmapPath", 10) == 0))
    pixmapPath = stripcpy(&tline[10]);
      else if ((strlen (&tline[0]) > 1) && (mystrncasecmp (tline, "*ScriptPath", 11) == 0))
    ScriptPath = stripcpy( &tline[11]);

      tline = fgets (line, (sizeof line) - 1, fd);
      orig_tline = tline;
    }
  fclose (fd);
  return;
}

/* Procedure d'initialisation du serveur X et des variables globales */
void
Xinit (int IsFather)
{
  char *name;
  Atom myatom;
  int i = 16;

  /* Connextion au serveur X */
#ifdef DEBUG
  __bounds_debug_no_checking = True;
#endif

  x_fd = ConnectX( ASDefaultScr, display_name, PropertyChangeMask );
  x11base->display = dpy;
#ifdef DEBUG
  __bounds_debug_no_checking = False;
#endif

  if (IsFather)
    {
      name = (char *) calloc (sizeof (char), strlen ("Script") + 5);
      do
	{
	  sprintf (name, "%c%xScript", 161, i);
	  i++;
	  myatom = XInternAtom (x11base->display, name, False);
	}
      while (XGetSelectionOwner (x11base->display, myatom) != None);
      x11base->TabScriptId[1] = name;
      x11base->TabScriptId[0] = NULL;
    }
  x11base->NbChild = 0;
  screen = x11base->screen = Scr.screen;
  x11base->WhitePix = Scr.asv->white_pixel ;
  x11base->BlackPix = Scr.asv->black_pixel ;
  x11base->depth = Scr.d_depth;
  x11base->colormap = Scr.asv->colormap;
  x11base->root = Scr.Root;
}

/***********************/
/* Lecture d'un icone  */
/***********************/
void
LoadIcon (struct XObj *xobj)
{
	if ((xobj->icon) != NULL)
    {
		ASImage *im ;
		if( Scr.image_manager == NULL ) 
			Scr.image_manager = create_image_manager( NULL, 2.2, pixmapPath, NULL );
	
		im = get_asimage( Scr.image_manager, xobj->icon, 0xFFFFFFFF, 100 );
		if( im == NULL ) 
		{
			xobj->icon_w = 0;
			xobj->icon_h = 0;
			xobj->iconPixmap = None;
			xobj->icon_maskPixmap = None;
		}else
		{
			xobj->iconPixmap = asimage2pixmap( Scr.asv, Scr.Root, im, NULL, True );
			xobj->icon_maskPixmap = asimage2mask( Scr.asv, Scr.Root, im, NULL, True );
			xobj->icon_w = im->width;
			xobj->icon_h = im->height;
			safe_asimage_destroy( im );
		}
    }
}

unsigned long
MyAllocNamedColor2 (char *colorname )
{
	ARGB32 argb ;
	unsigned long pixel = Scr.asv->white_pixel ;
	if( parse_argb_color( colorname, &argb ) != colorname )
		ARGB2PIXEL(Scr.asv,argb, &pixel);
	return pixel ;
}

int 
MyAllocNamedColor (char *colorname, XColor *col )
{
	ARGB32 argb ;
	col->pixel = Scr.asv->white_pixel ;
	if( parse_argb_color( colorname, &argb ) != colorname )
	{
		ARGB2PIXEL(Scr.asv,argb, &(col->pixel));
		return 1 ;
	}
	return 0 ;
}

/* Ouvre une fenetre pour l'affichage du GUI */
void
OpenWindow ()
{
  XTextProperty Name;
  XWMHints *IndicWM;
  XSizeHints *IndicNorm;
  unsigned long mask;
  XSetWindowAttributes Attr;

  /* Allocation des couleurs */
  x11base->TabColor[fore].pixel = MyAllocNamedColor2 ( x11base->forecolor );

  x11base->TabColor[back].pixel = MyAllocNamedColor2 (x11base->backcolor);
  x11base->TabColor[shad].pixel = MyAllocNamedColor2 (x11base->shadcolor);
  x11base->TabColor[li].pixel =  MyAllocNamedColor2 (x11base->licolor  );
  x11base->TabColor[black].pixel = Scr.asv->black_pixel ;
  x11base->TabColor[white].pixel = Scr.asv->white_pixel ;

  /* Definition des caracteristiques de la fentre */
  mask = 0;
  mask |= CWBackPixel;
  Attr.background_pixel = x11base->TabColor[back].pixel;

  x11base->win = XCreateWindow (x11base->display,
				DefaultRootWindow (x11base->display),
				x11base->size.x,
				x11base->size.y,
				x11base->size.width,
				x11base->size.height, 0,
				CopyFromParent,
				InputOutput,
				CopyFromParent,
				mask, &Attr);

  XSetWindowColormap (x11base->display, x11base->win, x11base->colormap);
  x11base->gc = XCreateGC (x11base->display, x11base->win, 0, NULL);

  /* Choix des evts recus par la fenetre */
  XSelectInput (x11base->display, x11base->win, KeyPressMask | ButtonPressMask
		| ExposureMask | ButtonReleaseMask | EnterWindowMask
		| LeaveWindowMask | ButtonMotionMask);

  /* Specification des parametres utilises par le gestionnaire de fenetre */
  if (XStringListToTextProperty (&x11base->title, 1, &Name) == 0)
    fprintf (stderr, "Can't use icon name\n");
  IndicNorm = XAllocSizeHints ();
  if (x11base->size.x != -1)
    {
      IndicNorm->x = x11base->size.x;
      IndicNorm->y = x11base->size.y;
      IndicNorm->flags = PSize | PMinSize | PMaxSize | PResizeInc | PBaseSize | PPosition;
    }
  else
    IndicNorm->flags = PSize | PMinSize | PMaxSize | PResizeInc | PBaseSize;
  IndicNorm->width = x11base->size.width;
  IndicNorm->height = x11base->size.height;
  IndicNorm->min_width = x11base->size.width;
  IndicNorm->min_height = x11base->size.height;
  IndicNorm->max_width = x11base->size.width;
  IndicNorm->max_height = x11base->size.height;
  IndicWM = XAllocWMHints ();
  IndicWM->input = True;
  IndicWM->initial_state = NormalState;
  IndicWM->flags = InputHint | StateHint;
  XSetWMProperties (x11base->display, x11base->win, &Name,
		    &Name, NULL, 0, IndicNorm, IndicWM, NULL);
  Scrapt = (char *) calloc (sizeof (char), 1);

  /* Construction des atomes pour la communication inter-application */
  propriete = XInternAtom (x11base->display, "Prop_selection", False);
  wm_del_win = XInternAtom (x11base->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (x11base->display, x11base->win, &wm_del_win, 1);

}

/***********************************************/
/* Execution d'une sequence d'instruction      */
/***********************************************/
void
ExecBloc (Bloc * bloc)
{
  int i;

  for (i = 0; i <= bloc->NbInstr; i++)
    {
      TabCom[bloc->TabInstr[i].Type] (bloc->TabInstr[i].NbArg, bloc->TabInstr[i].TabArg);
    }
}


/* Construction de l'interface graphique */
void
BuildGUI (int IsFather)
{
  int i;


  if (scriptprop->font == NULL)
    x11base->font = strdup ("fixed");
  else
    x11base->font = scriptprop->font;

  if (scriptprop->forecolor == NULL)
    x11base->forecolor = strdup ("black");
  else
    x11base->forecolor = scriptprop->forecolor;

  if (scriptprop->backcolor == NULL)
    x11base->backcolor = strdup ("white");
  else
    x11base->backcolor = scriptprop->backcolor;

  if (scriptprop->shadcolor == NULL)
    x11base->shadcolor = strdup ("black");
  else
    x11base->shadcolor = scriptprop->shadcolor;

  if (scriptprop->licolor == NULL)
    x11base->licolor = strdup ("black");
  else
    x11base->licolor = scriptprop->licolor;

  x11base->icon = scriptprop->icon;

  x11base->size.x = scriptprop->x;
  x11base->size.y = scriptprop->y;
  x11base->size.width = scriptprop->width;
  x11base->size.height = scriptprop->height;
  x11base->title = scriptprop->titlewin;

  /* Initialisation du serveur X et de la fenetre */

  OpenWindow ();


  /* Parcour de tous les objets graphiques */
  nbobj++;
  for (i = 0; i < nbobj; i++)
    {
      tabxobj[i] = (struct XObj *) calloc (1, sizeof (struct XObj));
      tabxobj[i]->id = (*tabobj)[i].id;
      tabxobj[i]->x = (*tabobj)[i].x;
      tabxobj[i]->y = (*tabobj)[i].y;
      tabxobj[i]->width = (*tabobj)[i].width;
      tabxobj[i]->height = (*tabobj)[i].height;
      if (tabxobj[i]->width == 0)
	tabxobj[i]->width = 1;
      if (tabxobj[i]->height == 0)
	tabxobj[i]->height = 1;
      tabxobj[i]->value = (*tabobj)[i].value;
      tabxobj[i]->value2 = (*tabobj)[i].value2;
      tabxobj[i]->value3 = (*tabobj)[i].value3;
      tabxobj[i]->flags[0] = (*tabobj)[i].flags[0];
      tabxobj[i]->flags[1] = (*tabobj)[i].flags[1];
      tabxobj[i]->flags[2] = (*tabobj)[i].flags[2];
      tabxobj[i]->icon = (*tabobj)[i].icon;
      tabxobj[i]->swallow = (*tabobj)[i].swallow;

      if ((*tabobj)[i].title == NULL)
	tabxobj[i]->title = (char *) calloc (1, 200);
      else
	tabxobj[i]->title = (*tabobj)[i].title;

      if ((*tabobj)[i].font == NULL)
	tabxobj[i]->font = (char *) strdup (x11base->font);
      else
	tabxobj[i]->font = (*tabobj)[i].font;

      if ((*tabobj)[i].forecolor == NULL)
	tabxobj[i]->forecolor = (char *) strdup (x11base->forecolor);
      else
	tabxobj[i]->forecolor = (*tabobj)[i].forecolor;

      if ((*tabobj)[i].backcolor == NULL)
	tabxobj[i]->backcolor = (char *) strdup (x11base->backcolor);
      else
	tabxobj[i]->backcolor = (*tabobj)[i].backcolor;

      if ((*tabobj)[i].shadcolor == NULL)
	tabxobj[i]->shadcolor = (char *) strdup (x11base->shadcolor);
      else
	tabxobj[i]->shadcolor = (*tabobj)[i].shadcolor;

      if ((*tabobj)[i].licolor == NULL)
	tabxobj[i]->licolor = strdup (x11base->licolor);
      else
	tabxobj[i]->licolor = (*tabobj)[i].licolor;

      ChooseFunction (tabxobj[i], (*tabobj)[i].type);
      tabxobj[i]->gc = x11base->gc;
      tabxobj[i]->display = x11base->display;
      tabxobj[i]->ParentWin = &(x11base->win);
      tabxobj[i]->Screen = x11base->screen;
      tabxobj[i]->colormap = &(x11base->colormap);
      tabxobj[i]->iconPixmap = None;
      tabxobj[i]->icon_maskPixmap = None;

      LoadIcon (tabxobj[i]);	/* Chargement de l'icone du widget */

      tabxobj[i]->InitObj (tabxobj[i]);
    }

  /* Enregistrement du bloc de taches periodic */
  x11base->periodictasks = scriptprop->periodictasks;

  /*Si un bloc d'initialisation du script existe, on l'execute ici */
  if (scriptprop->initbloc != NULL)
    {
      ExecBloc (scriptprop->initbloc);
      free (scriptprop->initbloc->TabInstr);
      free (scriptprop->initbloc);
    }
  free (tabobj);
  free (scriptprop);
  XMapRaised (x11base->display, x11base->win);
  for (i = 0; i < nbobj; i++)
    if (tabxobj[i]->flags[0] != True)
      XMapWindow (x11base->display, tabxobj[i]->win);

}


/***********************************************/
/* Fonction de traitement des msg entre objets */
/***********************************************/
void
SendMsg (struct XObj *xobj, int TypeMsg)
{
  int i;

  for (i = 0; i <= TabCObj[TabIdObj[xobj->id]].NbCase; i++)
    if (TabCObj[TabIdObj[xobj->id]].LstCase[i] == TypeMsg)
      {
	/* Execution du bloc d'instruction */
	ExecBloc (&TabIObj[TabIdObj[xobj->id]][i]);
      }
}

/*******************************************/
/* Appeler lors d'une demande de selection */
/*******************************************/
void
SendMsgToScript (XEvent event)
{
  Atom Sender, Receiver = None;
  static XEvent evnt_sel;
  int i;

  Sender = XInternAtom (x11base->display, x11base->TabScriptId[1], True);

  if (event.xselectionrequest.selection == Sender)
    {
      i = 0;
      while ((i < BuffSend.NbMsg) && (event.xselectionrequest.target != Receiver))
	{
	  Receiver = XInternAtom (x11base->display, BuffSend.TabMsg[i].R, True);
	  i++;
	}
      i--;

      evnt_sel.type = SelectionNotify;
      evnt_sel.xselection.requestor = event.xselectionrequest.requestor;
      evnt_sel.xselection.selection = event.xselectionrequest.selection;
      evnt_sel.xselection.target = Receiver;
      evnt_sel.xselection.time = event.xselectionrequest.time;

      if (event.xselectionrequest.target == Receiver)
	{			/* On a trouve le recepteur */
	  evnt_sel.xselection.property = event.xselectionrequest.property;
	  XChangeProperty (x11base->display, evnt_sel.xselection.requestor,
			   evnt_sel.xselection.property,
			   evnt_sel.xselection.target,
			   8, PropModeReplace, BuffSend.TabMsg[i].Msg, strlen (BuffSend.TabMsg[i].Msg) + 1);
	  BuffSend.NbMsg--;
	  free (BuffSend.TabMsg[i].Msg);
	  if (BuffSend.NbMsg > 0)
	    {
	      memmove (&BuffSend.TabMsg[i], &BuffSend.TabMsg[i + 1], (BuffSend.NbMsg - i) * sizeof (TypeName));
	    }
	}
      else
	{			/* Cas ou le recepteur demande un message et qu'il n'y en a pas */
	  evnt_sel.xselection.property = None;
	}
      XSendEvent (x11base->display, evnt_sel.xselection.requestor, False, 0, &evnt_sel);
    }
}

/* read an X event */
void
ReadXServer ()
{
  static XEvent event, evnt_sel;
  int i;
  char *octet;

  while (XEventsQueued (x11base->display, QueuedAfterReading))
    {
      XNextEvent (x11base->display, &event);
      switch (event.type)
	{
	case Expose:
	  if (event.xexpose.count == 0)
	    for (i = 0; i < nbobj; i++)
	      tabxobj[i]->DrawObj (tabxobj[i]);
	  break;
	case KeyPress:
	  /* Touche presse dans un objet */
	  if (event.xkey.subwindow != 0)
	    {
	      /* Envoi de l'evt � l'objet */
	      for (i = 0; i < nbobj; i++)
		if (tabxobj[i]->win == event.xkey.subwindow)
		  tabxobj[i]->EvtKey (tabxobj[i], &event.xkey);
	    }
	  break;
	case ButtonPress:
	  /* Clique dans quel fenetre? */
	  if (event.xbutton.subwindow != 0)
	    {
	      i = 0;
	      while ((tabxobj[i]->win != event.xbutton.subwindow) && (i < nbobj - 1))
		i++;
	      tabxobj[i]->EvtMouse (tabxobj[i], &event.xbutton);
	    }
	  break;
	case ButtonRelease:
	  break;
	case EnterNotify:
	  break;
	case LeaveNotify:
	  break;
	case MotionNotify:
	  break;
	case MappingNotify:
	  XRefreshKeyboardMapping ((XMappingEvent *) & event);
	  break;
	case SelectionRequest:
	  if (event.xselectionrequest.selection == XA_PRIMARY)
	    {
	      evnt_sel.type = SelectionNotify;
	      evnt_sel.xselection.requestor = event.xselectionrequest.requestor;
	      evnt_sel.xselection.selection = event.xselectionrequest.selection;
	      evnt_sel.xselection.target = event.xselectionrequest.target;
	      evnt_sel.xselection.time = event.xselectionrequest.time;
	      evnt_sel.xselection.property = event.xselectionrequest.property;
	      switch (event.xselectionrequest.target)
		{
		case XA_STRING:
		  XChangeProperty (x11base->display, evnt_sel.xselection.requestor,
				   evnt_sel.xselection.property,
				   evnt_sel.xselection.target,
			   8, PropModeReplace, Scrapt, strlen (Scrapt) + 1);
		  break;
		default:
		  evnt_sel.xselection.property = None;
		}
	      XSendEvent (x11base->display, evnt_sel.xselection.requestor, False, 0, &evnt_sel);
	    }
	  else
	    SendMsgToScript (event);
	  break;
	case SelectionClear:
	  if (event.xselectionclear.selection == XA_PRIMARY)
	    UnselectAllTextField (tabxobj);
	  break;
	case ClientMessage:
	  if ((event.xclient.format == 32) && (event.xclient.data.l[0] == wm_del_win))
	    DeadPipe (1);
	  break;
	case PropertyNotify:
	  if (event.xproperty.atom == XA_CUT_BUFFER0)
	    octet = XFetchBuffer (x11base->display, &i, 0);
	  else if (event.xproperty.atom == XA_CUT_BUFFER1)
	    octet = XFetchBuffer (x11base->display, &i, 1);
	  else if (event.xproperty.atom == XA_CUT_BUFFER2)
	    octet = XFetchBuffer (x11base->display, &i, 2);
	  else if (event.xproperty.atom == XA_CUT_BUFFER3)
	    octet = XFetchBuffer (x11base->display, &i, 3);
	  else if (event.xproperty.atom == XA_CUT_BUFFER4)
	    octet = XFetchBuffer (x11base->display, &i, 4);
	  else if (event.xproperty.atom == XA_CUT_BUFFER5)
	    octet = XFetchBuffer (x11base->display, &i, 5);
	  else if (event.xproperty.atom == XA_CUT_BUFFER6)
	    octet = XFetchBuffer (x11base->display, &i, 6);
	  else if (event.xproperty.atom == XA_CUT_BUFFER7)
	    octet = XFetchBuffer (x11base->display, &i, 7);
	  else
	    break;
	  if (i > 0)
	    {
	      Scrapt = (char *) realloc ((void *) Scrapt, sizeof (char) * (i + 1));
	      Scrapt = strcpy (Scrapt, octet);
	    }
	  break;
	}
    }
}

/* main event loop */
void
MainLoop ()
{
  fd_set in_fdset;
  unsigned long header[HEADER_SIZE];
  unsigned long *body;
  int count, i;
  struct timeval tv;
  int res;

  while (1)
    {
      FD_ZERO (&in_fdset);
      FD_SET (x_fd, &in_fdset);
      FD_SET (fd[1], &in_fdset);

      XFlush (x11base->display);

      tv.tv_sec = 1;
      tv.tv_usec = 0;

      if (x11base->periodictasks != NULL)
	res = select (32, &in_fdset, NULL, NULL, &tv);
      else
	res = select (32, &in_fdset, NULL, NULL, NULL);

      if (res > 0)
	{
	  if (FD_ISSET (x_fd, &in_fdset))
	    ReadXServer ();

	  if (FD_ISSET (fd[1], &in_fdset))
	    {
	      if ((count = ReadASPacket (fd[1], header, &body)) > 0)
		{
		  for (i = 0; i < nbobj; i++)
		    tabxobj[i]->ProcessMsg (tabxobj[i], header[1], body);
		  free (body);
		}
	    }
	}
      if (x11base->periodictasks != NULL)	/* Execution des taches periodics */
	ExecBloc (x11base->periodictasks);
    }
}

void
ReadASScriptArg (int argc, char **argv, int IsFather)
{
  int i;
  Atom myatom;
  int FisrtArg;

  BuffSend.NbMsg = 0;		/* Aucun message dans le buffer */

  for (i = 2; i < 98; i++)
    x11base->TabScriptId[i] = NULL;

  if (IsFather)
    {				/* Cas du pere */
      myatom = XInternAtom (x11base->display, x11base->TabScriptId[1], True);
      XSetSelectionOwner (x11base->display, myatom, x11base->win, CurrentTime);
      FisrtArg = 9;
    }
  else
    {				/* Cas du fils */
      x11base->TabScriptId[0] = (char *) calloc (sizeof (char), strlen (argv[7]));
      x11base->TabScriptId[0] = strncpy (x11base->TabScriptId[0], argv[7], strlen ("Script") + 3);
      x11base->TabScriptId[1] = argv[7];
      myatom = XInternAtom (x11base->display, x11base->TabScriptId[1], True);
      XSetSelectionOwner (x11base->display, myatom, x11base->win, CurrentTime);
      FisrtArg = 8;
    }
}

void
version (void)
{
  printf ("%s version %s\n", MyName, VERSION);
  exit (0);
}

void
usage (void)
{
  printf ("Usage:\n"
	  "%s [-v|--version] [-h|--help] [--window window-id] script\n", MyName);
  exit (0);
}

/* main procedure */
int
main (int argc, char **argv)
{
  int IsFather;
  char *realconfigfile;
  char *temp;
  int i, k;
  char *global_config_file = NULL;

  /* Save our program name - for error messages */
  set_application_name( argv[0] );
  temp = strrchr (argv[0], '/');
  MyName = temp ? temp + 1 : argv[0];

  for (i = 1; i < argc && *argv[i] == '-'; i++)
    {
      if (!strcmp (argv[i], "-h") || !strcmp (argv[i], "--help"))
	usage ();
      else if (!strcmp (argv[i], "-v") || !strcmp (argv[i], "--version"))
	version ();
      else if (!strcmp (argv[i], "-w") || !strcmp (argv[i], "--window"))
	ref = strtol (argv[++i], NULL, 16);
      else if (!strcmp (argv[i], "-c") || !strcmp (argv[i], "--context"))
	i++;
      else if (!strcmp (argv[i], "-f") && i + 1 < argc)
	global_config_file = argv[++i];
    }

  if (i == argc)
    usage ();
  /* On determine si le script a un pere */
  IsFather = (i >= argc || *argv[i] != (char) 161);
  i += !IsFather;
  ScriptName = argv[i++];

  /* Dead pipe == dead AfterStep */
  signal (SIGPIPE, DeadPipe);
  set_signal_handler (SIGSEGV);
  signal (SIGINT, DeadPipe);	/* cleanup on other ways of closing too */
  signal (SIGHUP, DeadPipe);
  signal (SIGQUIT, DeadPipe);
  signal (SIGTERM, DeadPipe);

  /* Enregistrement des arguments du script */
  x11base = (X11base *) calloc (1, sizeof (X11base));
  x11base->TabArg[0] = MyName;
  for (k = 1; i < argc; i++, k++)
    x11base->TabArg[k] = argv[i];

  Xinit (IsFather);

  /* connect to AfterStep */
  fd[0] = fd[1] = ConnectAfterStep(M_NEW_DESK | M_END_WINDOWLIST | M_MAP | M_RES_NAME |
								    M_RES_CLASS | M_WINDOW_NAME);

#ifdef I18N
  if (!setlocale (LC_CTYPE, AFTER_LOCALE))
    {
      fprintf (stderr, "%s : cannot set locale.\n", MyName);
      exit (1);
    }
#endif

  if (global_config_file != NULL)
    ParseOptions (global_config_file);
  else
    {
      char configfile[255];
      sprintf (configfile, "%s/base.%dbpp", AFTER_DIR, DefaultDepth (dpy, screen));
      realconfigfile = (char *) PutHome (configfile);
      ParseOptions (realconfigfile);
    }

  SendInfo (fd, "Send_WindowList", 0);

  ReadConfig (ScriptName);	/* Lecture et analyse du script */

  InitCom ();			/* Fonction d'initialisation de TabCom et TabFunc   */

  BuildGUI (IsFather);		/* Construction des boutons et de la fenetre */

  ReadASScriptArg (argc, argv, IsFather);

  MainLoop ();

  return 0;
}
