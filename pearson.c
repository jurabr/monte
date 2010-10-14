/*
   File name: pearson2.c
   Date:      2006/07/27 15:55
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

   Computes correlation matrix from given data using the Pearson's coefficient

   Usage: pearson <input >output

   Input must include first line with number of variables and
   number of simulations; other lines include values
   (one simulation of all variables per line).
   A raw correlation matrix is returned.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793238462643383279502884

FILE *fr = NULL ;
FILE *fw = NULL ;
long  vars, sims ;
double *data = NULL ;
double *sdev = NULL ;
double *mean = NULL ;
double *sum1 = NULL ;
double *sum2 = NULL ;
double *mat  = NULL;

int read_data(FILE *fr)
{
  long i, j;
  fscanf(fr,"%li %li", &vars, &sims);

  if ((data = (double *)malloc(vars*sims*sizeof(double))) == NULL) {return(-1);}

  if ((sdev = (double *)malloc(vars*sizeof(double))) == NULL) {return(-1);}
  if ((mean = (double *)malloc(vars*sizeof(double))) == NULL) {return(-1);}
  if ((sum1 = (double *)malloc(vars*sizeof(double))) == NULL) {return(-1);}
  if ((sum2 = (double *)malloc(vars*sizeof(double))) == NULL) {return(-1);}

  if ((mat = (double *)malloc(vars*vars*sizeof(double))) == NULL) {return(-1);}

  for (i=0; i< (sims*vars); i++)
  {
    data[i] = 0.0 ;
  }

  for (i=0; i< vars; i++)
  {
    sum1[i] = 0.0 ;
    sum2[i] = 0.0 ;
  }

  for (i=0; i<sims; i++)
  {
    for (j=0; j<vars; j++)
    {
      fscanf(fr,"%lf", &data[j*sims+i]);

#if 0
      if (j > 0) { data[j*sims+i] = data[0*sims+i] ; }
#endif
      
      sum1[j] +=  data[j*sims+i] ;
      sum2[j] +=  pow(data[j*sims+i], 2) ;
    }
  }

  for (i=0; i<vars; i++)
  {
    mean[i] = sum1[i] / ((double)sims) ;
#if 0
    sdev[i] = sqrt(1.0/((double)sims-1.0))*                                       
           ( sum2[i] - (1.0/(double)sims)*pow(sum1[i],2));
#endif

    sdev[i] = sqrt(
        (1/((double)sims-1)) * (sum2[i] - mean[i]*sum1[i])
        );
  }
  return(0);
}

int get_matrix(void)
{
  long i,j,k ;
  double sum, val, vali, valj ;

  for (i=0; i<vars; i++)
  {
    for (j=0; j<vars; j++)
    {
      sum = 0.0 ;
      for (k=0; k<sims; k++)
      {
        vali = data[i*sims+k] ;
        valj = data[j*sims+k] ;
        sum += (vali-mean[i])*(valj-mean[j]);
      }
      val = sum / (((double)sims-1.0)*(sdev[i]*sdev[j])) ;

      mat[j + i*vars] = val ;
    }
  }
  return(0);
}

int print_matrix(FILE *fw)
{
  int i,j;
  
  for (i=0; i<vars; i++)
  {
    for (j=0; j<vars; j++)
    {
      fprintf(fw," %3.6e", 2.0*sin(PI*mat[j + i*vars]/6.0) );
    }
    fprintf(fw,"\n");
  }

  return(0);
}

int main(int argc, char *argv[])
{
  fr = stdin ;
  fw = stdout ;

  if ( read_data(fr) != 0) {exit(-1);}
  if ( get_matrix() != 0) {exit(-1);}
  if ( print_matrix(fw) != 0) {exit(-1);}

  return(0);
}

/* end of pearson2.c */
