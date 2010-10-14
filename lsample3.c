/*
   File name: lsample3.c
   Date:      2006/07/15 21:39
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

              F1 |   F2 |
   /|           \/     \/                     
   /|----------------------------------- (w)
   /|            a1     a2
  (V, M)

    |<------------ L ------------------>|

    Variables:
    Input (7): E I L a1 a2 F1 F2
    Output(3): Mmax, Vmax, wmax
    No failure function

    a1, a2 are from <0,1>
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


long monte_dlib_interface_type(void) 
{
  return(1) ; /* 1 is for the simple type */
}

void monte_nums_of_vars(long *ilen, long *olen, long *ffunc)
{
  *ilen = 7 ; /* required number of input variables */
  *olen = 3 ; /* returned number of output variables */
  *ffunc = -1 ; /* what parameter represents failure function, -1 for none */
}

int monte_solution(double *ifld, double *ofld)
{
  double V, M, w ;
  double E, I, L, a1, a2, F1, F2, ar1,ar2 ;

  E   = ifld[0] ;
  I   = ifld[1] ;
  L   = ifld[2] ;
  ar1 = ifld[3] ;
  ar2 = ifld[4] ;
  F1  = ifld[5] ;
  F2  = ifld[6] ;

  a1 = ar1 * L ;
  a2 = ar2 * L ;

  /* this is an extreme difficult and scientific analysis: */
  M = a1*F1 + a2*F2 ;
  V = F1 + F2 ;

#if 0
  printf("E=%e I=%e L=%e a1=%e a2=%e F1=%e F2=%e\n", E, I, L, a1, a2, F1, F2);
#endif

  w = 
    (F1*a1*a1/(6.0*E*I))*(3.0*L-a1) 
    + 
    (F2*a2*a2/(6.0*E*I))*(3.0*L-a2) 
    ;

  ofld[0] = M ;
  ofld[1] = V ;
  ofld[2] = w ;

#if 0
  printf("M = %e V = %e w = %e\n",M, V, w); ;
#endif

  return(0);
}

/* end of lsample3.c */
