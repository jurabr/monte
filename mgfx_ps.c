/*
   File name: mgfx_ps.c
   Date:      2006/02/18 18:30
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.

   Postscript output functions for Monte
*/

#include "monte.h"

#ifdef PSGUI

extern int md_draw(void);
extern int get_mdterm(void);

extern int gfxAction ;

int        mdps_x0 = 0 ;
int        mdps_y0 = 0 ;
int        mdps_width  = 800 ;
int        mdps_height = 600 ;

FILE      *mdpsFile = NULL ;

void set_ps_output_file(FILE *fw)
{
  mdpsFile = fw ;
}

/* Universal drawing functions: */

void get_draw_size_ps(int *x0,int *y0,int *width,int *height)
{
  *x0     = mdps_x0 ;
  *y0     = mdps_y0 ;
  *width  = mdps_width ;
  *height = mdps_height ;
}

void mdps_draw_point(int x,int  y)
{
  fprintf(mdpsFile,"%i %i moveto %i %i lineto\nclosepath\n",x-1,mdps_height-y,x+1,mdps_height-y+1);
  fprintf(mdpsFile,"1 stroke\n");
}

void mdps_draw_line(int x1,int  y1,int  x2,int  y2,int  width)
{
  fprintf(mdpsFile,"%i %i moveto %i %i lineto\nclosepath\n",x1,mdps_height-y1,x2,mdps_height-y2);
  fprintf(mdpsFile,"%i stroke\n", width);
}

void mdps_draw_string(int x,int  y, char *str)
{
  fprintf(mdpsFile,"%i %i moveto\n (%s)show\n",x,mdps_height-y,str);
}

/* end of "universal drawing functions" */

/* ------------------------------------------ */

int mdps_draw(int width, int height, char *fname)
{
  mdps_x0 = 0 ;
  mdps_y0 = 0 ;
  mdps_width  = width ;
  mdps_height = height ;

  if ((mdpsFile = fopen(fname, "wb")) == NULL)
  {
    return(-1) ;
  }

  fprintf(mdpsFile,"%%!PS-Adobe-2.0\n");
  fprintf(mdpsFile,"%%%%Creator: MicroDef \n");
  fprintf(mdpsFile,"%%%%BoundingBox: %i %i %i %i\n",
      mdps_x0,mdps_y0, mdps_width,mdps_height);
  fprintf(mdpsFile,"/Helvetica\nfindfont\n");
  fprintf(mdpsFile,"12 scalefont setfont\n");
  fprintf(mdpsFile,"0 setgray\n");

  md_draw();

  fprintf(mdpsFile,"showpage\n");

  fclose(mdpsFile);

  return(0);
}

#endif

/* end of mgfx_ps.c */
