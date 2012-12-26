/*
 * Copyright (c) 2002 Sasha Vasko <sasha@aftercode.net>
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

#include "../configure.h"

#include <unistd.h>

#define LOCAL_DEBUG
#include "asapp.h"
#include "afterstep.h"
#include "../libAfterImage/afterimage.h"
#include "screen.h"
#include "event.h"

void
make_icon_pixmaps (icon_t * icon, Bool ignore_alpha)
{
	if (icon->image)
	{
		icon->pix = asimage2pixmap (ASDefaultVisual, ASDefaultRoot, icon->image, NULL, False);
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif
		if (!ignore_alpha)
		{
			int           depth = check_asimage_alpha (ASDefaultVisual, icon->image);

			if (depth > 0)
				icon->mask = asimage2alpha (ASDefaultVisual, ASDefaultRoot, icon->image, NULL, False, True);
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif
            if (depth == 8)
				icon->alpha = asimage2alpha (ASDefaultVisual, ASDefaultRoot, icon->image, NULL, False, False);
#ifdef LOCAL_DEBUG
    LOCAL_DEBUG_OUT( "syncing %s","");
    ASSync(False);
#endif

        }
        LOCAL_DEBUG_OUT(" icon pixmaps: %lX,%lX,%lX", icon->pix, icon->mask, icon->alpha );
        flush_asimage_cache(icon->image);
	}
}

void
icon_from_pixmaps( MyIcon *icon, Pixmap pix, Pixmap mask, Pixmap alpha )
{
	if( icon ) 
	{
		free_icon_resources( icon );
		icon->pix = pix ; 
		icon->mask = mask ; 
		icon->alpha = alpha ;
		icon->image = NULL ;
		
		if ( pix != None)
		{
			unsigned int  width, height;
			if (!get_drawable_size( pix, &width, &height))
			{
				icon->pix = None;
				icon->width = 0;
				icon->height = 0;
			}else
			{
				icon->width = width;
				icon->height = height;
				if (mask)
					icon->image = picture2asimage ( ASDefaultVisual, pix, mask, 0, 0, icon->width, icon->height,
													AllPlanes, False, 0);
				else
					icon->image = picture2asimage ( ASDefaultVisual, pix, alpha, 0, 0, icon->width, icon->height,
													AllPlanes, False, 0);
			}
		}
	}		   
}

void
asimage2icon (ASImage * im, icon_t * icon)
{
	icon->image = im;
	if (im)
	{
		icon->width = im->width;
		icon->height = im->height;
        flush_asimage_cache(im);
	}
}

Bool
load_icon (icon_t *icon, const char *filename, ASImageManager *imman )
{
    if (icon && filename)
	{
        ASImage *im = get_asimage( imman, filename, ASFLAGS_EVERYTHING, 100 );
        if( im == NULL )
            show_error( "failed to locate icon file %s in the IconPath and PixmapPath", filename );
        else
		{
            asimage2icon (im, icon);
            LOCAL_DEBUG_OUT("icon file \"%s\" loaded into ASImage %p(imman %p) using imageman = %p and has size %dx%d", filename, im, im->imageman, imman, icon->width, icon->height );
		}
        return (icon->image != NULL);
	}
	return False;
}

Bool
scale_icon (icon_t *icon, int width, int height, ASVisual *asv)
{
	if (icon && icon->image && width > 0 && height > 0
			&& (icon->image->width != width || icon->image->height != height)) {
		ASImage *tmp = scale_asimage (asv, icon->image, width, height, ASA_ASImage, 100, ASIMAGE_QUALITY_DEFAULT);
		if (tmp) {
			safe_asimage_destroy (icon->image);
			asimage2icon (tmp, icon);
			return True;
		}
	}
	return False;
}

void
free_icon_resources (icon_t *icon)
{
	if( icon ) 
	{
    	LOCAL_DEBUG_OUT( "icon's pixmap to be freed: %lX,%lX,%lX", icon->pix, icon->mask, icon->alpha );
		destroy_visual_pixmap(ASDefaultVisual, &(icon->pix));
    	destroy_visual_pixmap(ASDefaultVisual, &(icon->mask));
    	destroy_visual_pixmap(ASDefaultVisual, &(icon->alpha));
		if (icon->image)
		{
			safe_asimage_destroy (icon->image);
			icon->image = NULL ; 
		}
	}
}

void
destroy_icon(icon_t **picon)
{
    icon_t *pi = *picon ;
    if( pi )
    {
		free_icon_resources (pi) ;
        free( pi );
        *picon = NULL ;
    }
}


void destroy_asbutton( ASButton *btn, Bool reusable )
{
    if( btn )
    {
        int i = ASB_StateCount;
        while( --i >= 0 )
            if( btn->shapes[i] )
            {
                free( btn->shapes[i] );
                btn->shapes[i] = NULL ;
            }
        if( !reusable )
            free( btn );
    }
}

static void 
update_button_size (button_t *button)
{
  button->width = 0 ;
  button->height = 0 ;

  if( button->unpressed.image )
  {
      button->width = button->unpressed.image->width ;
      button->height = button->unpressed.image->height ;
  }
  if( button->pressed.image )
  {
      if( button->pressed.image->width > button->width )
          button->width = button->pressed.image->width ;
      if( button->pressed.image->height > button->height )
          button->height = button->pressed.image->height ;
  }

}

Bool
load_button( button_t *button, char **filenames, ASImageManager *imman )
{
    if( button && imman && filenames )
    {
        if( filenames[0] )
            load_icon(&(button->unpressed), filenames[0], imman );
        if( filenames[1] )
            load_icon(&(button->pressed), filenames[1], imman );
				update_button_size (button);
        return ( button->width > 0 && button->height > 0 );
    }
    return False;
}

Bool
scale_button( button_t *button, int width, int height, ASVisual *asv)
{
    if( button && asv && width > 0 && height > 0 )
    {
        scale_icon(&(button->unpressed), width, height, asv);
        scale_icon(&(button->pressed), width, height, asv);
				update_button_size (button);
        return ( button->width > 0 && button->height > 0 );
    }
    return False;
}

void
free_button_resources( button_t *button )
{
    free_icon_resources( &(button->unpressed) );
    free_icon_resources( &(button->pressed) );
}

