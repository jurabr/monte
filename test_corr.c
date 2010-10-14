/*
   File name: test_corr.c
   Date:      2006/07/20 15:56
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

   Monte: testing program for correlations
*/

#include "fem_math.h"

#define DEVEL_VERBOSE 1
#define DEVEL 1

extern int monte_eigen_jacobi(tMatrix *a, tVector *d, tMatrix *v, long *nrot);
extern int femMatCholFact(tMatrix *a);

long     size = 2 ;
long     nrot ;

tMatrix a ;
tVector d ;
tMatrix v ;


void fill_a(void)
{
  if (size != 2) { exit(-1); }

  femMatPut(&a, 1,1, 1.0) ;
  femMatPut(&a, 1,2, 0.660) ;

  femMatPut(&a, 2,1, 0.660) ;
  femMatPut(&a, 2,2, 1.0) ;

}

void print_input(void)
{
  int i,j,n ;

  n = a.rows ;
  
  printf("\nA:\n");
  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      printf(" %4.4f", femMatGet(&a, i,j));
    }
    printf("\n");
  }
  printf("\n------------------------\n");
}



void print_output(void)
{
  int i,j,n ;

  n = a.rows ;
  
  printf("\nA:\n");
  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      printf(" %4.4f", femMatGet(&a, i,j));
    }
    printf("\n");
  }

  printf("\nD:\n");
  for (i=1; i<=n; i++)
  {
    printf(" %4.4f\n", femVecGet(&d, i));
  }

  printf("\nV:\n");
  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      printf(" %4.4f", femMatGet(&v,i,j));
    }
    printf("\n");
  }


}

int main(int argc, char *argv[])
{
  femMatNull(&a);
  femMatNull(&v);
  femVecNull(&d);
  
  femFullMatInit(&a,    size, size);
  femVecFullInit(&d,    size);
  femFullMatInit(&v,    size, size);

  fill_a() ;

  print_input();

#if 0
  monte_eigen_jacobi(&a, &d, &v, &nrot);
#endif

  femMatCholFact(&a) ;

  print_output();

  return(0);
}

/* end of test_corr.c */
