/*
   File name: knuth.c
   Date:      2005/11/23 20:48
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

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

#ifdef USE_SPRNG
int rand_gen_type = 4 ; /* type of random number generator */
#else
int rand_gen_type = 3 ;
#endif

int init_knuth(void)
{
  time_t t1;

  time(&t1);

  return((int)t1);
}

/** Random number generator (see Knuth: Art of Computer Programming 3) 
 * @param inpl initial number (152 for example)
 * @return random value from  <0;1>
 */
double get_rand_knuth(long *inpl)
{
  static int ind   ;
  static int inda  ;
  static long fld[56] ;
  static int stf = 0 ;
  long   valj, valk ;
  long   i, j, k ;

  /* initialization */
  if ( (*inpl < 0) || (stf == 0) )
  {
    stf = 1 ;
    valj  = labs(RAND_SEED - labs(*inpl)) ;
    valj %= RAND_BIG ;
    fld[55] = valj ;
    valk = 1 ;

    for (i=1; i<=54; i++)
    {
      j = (21*i) % 55 ;
      fld[j] = valk ;
      valk = valj - valk ;

      if (valk < RAND_ZERO) { valk += RAND_BIG ; }

      valj = fld[j] ;
    } /* for i */

    for (k=1; k<=4; k++)
    {
      for (i=1; i<=55; i++)
      {
        fld[i] -= fld[1+(i+30) % 55] ;
        if (fld[i] < RAND_ZERO) { fld[i] += RAND_BIG ; }
      }
    }
    *inpl  = 1 ;
    ind  = 0 ;
    inda = 31 ;
  }

  /* normal run */
  if (++ind >= 56)
  {
    ind = 1 ;
  }

  if (++inda >= 56)
  {
    inda = 1 ;
  }

  valj = fld[ind] - fld[inda] ;

  if (valj < RAND_ZERO)
  {
    valj += RAND_BIG ;
  }

  fld[ind] = valj ;

  return(valj*RAND_FAC);
}

double get_rand_rand(long *inpl)
{
  return ( ((double)rand()) / pow(2.0,15) );
}

double get_rand_random(long *inpl)
{
  return ( ((double)random()) / pow(2.0,31) );
}

double get_rand(long *inpl)
{
  switch (rand_gen_type)
  {
    case 1: return(get_rand_rand(inpl)); break ;
    case 2: return(get_rand_random(inpl)); break ;
    case 3: return(get_rand_knuth(inpl)); break ;
#ifdef USE_SPRNG
    case 4: return( sprng() ); break;
#endif

    default: return(get_rand_rand(inpl)); break ;
  }
}

/* end of knuth.c */
