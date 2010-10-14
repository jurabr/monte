/*
   File name: dis2gp.c
   Date:      2006/07/05 22:28
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
*/

#include "monte.h"
#include <strings.h>

FILE *fr = NULL ;

tHis his ;

int main(int argc, char *argv[])
{

  if (argc > 1)
  {
    if ((strcmp(argv[1],"-h") == 0)||(strcmp(argv[1],"-H") == 0))
    {
      fprintf(stderr,"dis2gp: converts *.dis files to a raw form (acceptable by Gnuplot).\n\n");
      fprintf(stderr,"  Usage: %s file.dis file.txt\n",argv[0]);
      fprintf(stderr,"\n  If no parameters are given then output is obtained from a standard input\n");
      fprintf(stderr,"  and output is send to a standard output.\n");
      exit(0);
    }
  }
  
  if (argc > 1)
  {
    if (strlen(argv[1]) < 1) { exit(-1); }
    if ((fr = fopen(argv[1],"r")) == NULL) { exit(-1); }
  }
  else
  {
    fr = stdin ;
  }

  his.name  = NULL ;
  his.type  = 0 ;
  his.len   = 0 ;
  his.min   = 0.0 ;
  his.max   = 0.0 ;
  his.total = 0 ;
  his.data  = NULL ;
 
  if (argc > 1)
  {
    if (read_dis(fr, argv[1], &his) !=0) { fclose(fr); exit(-2); }
  }
  else
  {
    if (read_dis(fr, "unnamed", &his) !=0) { fclose(fr); exit(-2); }
  }

  if (argc > 2)
  {
    if (strlen(argv[2]) < 1) 
    { 
      write_histogram_for_gnuplot(&his, NULL);
    }
    else
    {
      write_histogram_for_gnuplot(&his, argv[2]);
    }
  }
  else
  {
    write_histogram_for_gnuplot(&his, NULL);
  }

  return(0);
}


/* end of dis2gp.c */
