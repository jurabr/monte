/*
   File name: lsample2.c
   Date:      2006/07/15 10:11
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

   Description: This is a sample library for Monte [advanced interface].
*/


/*
   It HAVE TO include these functions:

   long monte_dlib_interface_type(void) .. returnes interface type number (1 for this)
   void monte_nums_of_vars(char *param, long *ilen, long *olen, long *ffunc) .. numbers of variables
   int monte_init_lib_stuff(char *param); .. to init custom data
   int monte_clean_lib_stuff(char *param); .. to clean custom data
   char *monte_ivar_name(char *param, long pos) .. returnes name of pos-th input variable (pos starts from 0)
   char *monte_ovar_name(char *param, long pos) .. returnes name of pos-th output variable (pos starts from 0)
   int monte_solution(char *param, double *ifld, double *ofld) .. provides computation

   Note: the *param can also be NULL, so you HAVE TO check it first!


   There also can be other functions and data structures (if necessary) but
   they will be ignored Monte.
*/



#include <stdio.h>
#include <stdlib.h>

static double mult = 0 ;

long monte_dlib_interface_type(void) 
{
  return(2) ; /* 2 is for the advanced type */
}


void monte_nums_of_vars(char *param, long *ilen, long *olen, long *ffunc)
{
  *ilen = 3 ; /* required number of input variables */
  *olen = 2 ; /* returned number of output variables */
#if 0
  *ffunc = -1 ; /* what parameter represents failure function, -1 for none
                   paramaters are numbered FROM ZERO (0) !!!! */
#else
  *ffunc = 1 ;
#endif
  return ;
}

char *monte_ivar_name(char *param, long pos)
{
  switch (pos)
  {
    case 0: return("var1"); break ;
    case 1: return("var2"); break ;
    case 2: return("var3"); break ;
    default: return("empty");
  }

  return("empty");
}


char *monte_ovar_name(char *param, long pos)
{
  switch (pos)
  {
    case 0: return("out1"); break ;
    case 1: return("out2"); break ;
    default: return("empty");
  }

  return("empty");
}

int monte_init_lib_stuff(char *param)
{
  mult = 1.0 ;
  return(0);
}

int monte_clean_lib_stuff(char *param)
{
  mult = 0.0 ;
  return(0);
}


int monte_solution(char *param, double *ifld, double *ofld)
{
	long i ;

	for(i=0; i<10; i++)
	{
    ofld[0] = (ifld[2] - ifld[1]) * mult ;

#if 0
    if (ifld[1] < 200) { ofld[1] = 1 ; }
    else { ofld[1] = 0 ; }
#else
    ofld[1] = ifld[2] - 0.25*ifld[1] ;
#endif
	}
  return(0);
}

/* end of lsample2.c */
