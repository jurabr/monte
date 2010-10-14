/*
   File name: mgraph.c
   Date:      2006/02/18 18:14
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

   Graphics output of histograms (Postscript, TCL,...)
*/

#include "monte.h"

int mdTerm = MDTERM_PS ;
FILE *oFile = NULL ;

/* Universal drawing functions: ################################## */

int get_mdterm(void) { return(mdTerm); }

int init_mdterm(int width, int lenght, FILE *fw)
{
  switch (get_mdterm())
  {
#ifdef TKGUI
    case MDTERM_TK: break;
#endif
#ifdef GDGUI
    case MDTERM_GD: break;
#endif
#ifdef PSGUI
    case MDTERM_PS: break;
#endif
  }
  return(0);
}

void md_get_size(int *x0, int *y0, int *width, int *height)
{
  switch (get_mdterm())
  {
#ifdef TKGUI
    case MDTERM_TK: get_draw_size(x0, y0, width, height); break ;
#endif
#ifdef GDGUI
    case MDTERM_GD:  get_draw_size_gd(x0,y0,width,height); break ;
#endif
#ifdef PSGUI
    case MDTERM_PS:  get_draw_size_ps(x0,y0,width,height); break ;
#endif
  }
}

void md_draw_point(int x, int y)
{
  switch (get_mdterm())
  {
#ifdef TKGUI
    case MDTERM_TK: mdgtk_draw_point(x, y); break ;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_point(x, y); break ;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_point(x, y); break ;
#endif
  }
}

void md_draw_line(int x1, int y1, int x2, int y2, int width)
{
  switch (get_mdterm())
  {
#ifdef TKGUI
    case MDTERM_TK: mdgtk_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_line(x1, y1, x2, y2, width); break;
#endif
  }
}

void md_draw_string(int x, int y, char *str)
{
  switch (get_mdterm())
  {
#ifdef TKGUI
    case MDTERM_TK: mdgtk_draw_string(x, y, str); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_string(x, y, str); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_string(x, y, str); break;
#endif
  }
}


int md_open_file(char *fname)
{
  if ((oFile = fopen(fname,"w")) == NULL)
  {
    return(-1); /* I/O error :-/ */
  }
  else
  {
    switch (get_mdterm())
    {
#ifdef TKGUI
      case MDTERM_TK:  set_tk_output_file(oFile); break;
#endif

#ifdef GDGUI
      case MDTERM_GD:  set_gd_output_file(oFile); break;
#endif

#ifdef PSGUI
      case MDTERM_PS: set_ps_output_file(oFile); break;
#endif
    }
  }

  return(0);
}

int md_close_file(void)
{
  return (fclose(oFile));
}

/* end of "universal drawing functions" ########################### */


/** Actual drawing code: ################## */

int plot_histogram(tHis *a)
{
  return(0);
}

void md_draw(void)
{
  /* TODO: some code here... */
}

/* end of mgraph.c */
