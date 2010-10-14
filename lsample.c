/*
   File name: lsample.c
   Date:      2006/07/01 21:51
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri BRozovsky

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

   Description: This is a sample library for Monte.
*/

/*
   It HAVE TO include these functions:

   long monte_dlib_interface_type(void) .. returnes interface type number (1 for this)
   void monte_nums_of_vars(long *ilen, long *olen, long *ffunc) .. numbers of variables
   int monte_solution(double *ifld, double *ofld) .. provides computation

   There also can be other functions and data structures (if necessary) but
   they will be ignored Monte.
*/

#include <stdio.h>
#include <stdlib.h>


long monte_dlib_interface_type(void) 
{
  return(1) ; /* 1 is for the simple type */
}

void monte_nums_of_vars(long *ilen, long *olen, long *ffunc)
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

int monte_solution(double *ifld, double *ofld)
{
	long i ;

	for(i=0; i<1; i++)
	{
    ofld[0] = ifld[2] - ifld[1] ;

#if 1
    if (ifld[1] < 200) { ofld[1] = 1 ; }
    else { ofld[1] = 0 ; }
#else
    ofld[1] = ifld[2] - 0.25*ifld[1] ;
#endif
	}
  return(0);
}

/* end of lsample.c */
