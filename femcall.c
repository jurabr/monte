/*
   File name: femcall.c
   Date:      2008/10/04 13:48
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

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

   FEM wrapper for Monte for : 

    - gets data from Monte, 
    - prepares material data input file,
    - runs uFEM in NON-LINEAR (!)
    - checks L-D curve
    - returns results (material data + difference measure)

    Number of parameters to variate (must be in this order!):
    1. E0
    2. nu
    3. fyc
    4. fybc
    5. fyt
    6. fuc
    7. fubc
    8. fut
    9. width (real set data)

    Number of return data (must be inthis order!):
    1. L-D difference ratio
    2. E0
    3. nu
    4. fyc
    5. fybc
    6. fyt
    7. fuc
    8. fubc
    9. fut
   10. k_c
   11. n_c
   12. k_bc
   13. n_bc
   14. k_t
   15. n_t
    
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int    nl_steps = 10 ;
static double *disp_orig = NULL ;
static double *disp_comp = NULL ;

long monte_dlib_interface_type(void) 
{
  return(2) ; /* 2 is for the advanced type */
}


void monte_nums_of_vars(char *param, long *ilen, long *olen, long *ffunc)
{
  *ilen = 9 ; /* required number of input variables */
  *olen = 15 ; /* returned number of output variables */
  *ffunc = -1 ; /* what parameter represents failure function, -1 for none */
  return ;
}

char *monte_ivar_name(char *param, long pos)
{
  switch (pos)
  {
    case 0: return("EX"); break ;
    case 1: return("PRXY"); break ;
    case 2: return("FYC"); break ;
    case 3: return("FYBC"); break ;
    case 4: return("FYT"); break ;
    case 5: return("FUC"); break ;
    case 6: return("FUBC"); break ;
    case 7: return("FUT"); break ;
    case 8: return("WIDTH"); break ;
    default: return("empty");
  }

  return("empty");
}


char *monte_ovar_name(char *param, long pos)
{
  switch (pos)
  {
    case  0: return("DIFF"); break ;
    case  1: return("EX"); break ;
    case  2: return("PRXY"); break ;
    case  3: return("FYC"); break ;
    case  4: return("FYBC"); break ;
    case  5: return("FYT"); break ;
    case  6: return("FUC"); break ;
    case  7: return("FUBC"); break ;
    case  8: return("FUT"); break ;
    case  9: return("KC"); break ;
    case 10: return("NC"); break ;
    case 11: return("KBC"); break ;
    case 12: return("NBC"); break ;
    case 13: return("KT"); break ;
    case 14: return("NT"); break ;
    default: return("empty");
  }

  return("empty");
}

int monte_init_lib_stuff(char *param)
{
  FILE *frc = NULL ;
  int i ;

  if (param == NULL) 
  {
    fprintf(stderr, "Invalid configuration file!\n") ;
    exit (-2);
  }

  if ((frc = fopen(param, "r")) == NULL)
  {
    fprintf(stderr, "Bad or damaged configuration file!\n") ;
    exit (-2);
  }

  fscanf(frc,"%i", &nl_steps) ;
  if (nl_steps > 2)
  {
    fprintf(stderr, "Bad number of non-linear steps!\n") ;
    exit (-1);
  }

  /* TODO allocate and read L-D curve here */
  /* read configuration here */

  if ((disp_orig = (double *)malloc(nl_steps*sizeof(double))) == NULL)
  {
    fprintf(stderr, "Out of memory!\n") ;
    exit (-1);
  }
  
  if ((disp_comp = (double *)malloc(nl_steps*sizeof(double))) == NULL)
  {
    fprintf(stderr, "Out of memory!\n") ;
    exit (-1);
  }

  for (i=0;i<nl_steps; i++)
  {
    fscanf(frc,"%lf", &disp_orig[i]) ;
  }
    
  fclose(frc) ;
  return(0);
}

int monte_clean_lib_stuff(char *param)
{
  /* probably nothing to do here */
  free(disp_orig); disp_orig = NULL ;
  free(disp_comp); disp_comp = NULL ;
  return(0);
}


int monte_solution(char *param, double *ifld, double *ofld)
{
  FILE *fw = NULL ;
  double Ex, nu, fyc, fybc, fyt, fuc, fubc, fut ;
  double kc, nc, kbc, nbc, kt, nt, s_a, s_b, e_a, e_b ;
  int i ;

  /* open simulational file */
  if ((fw = fopen("chensim.fem","w")) == NULL)
  {
    fprintf(stderr,"I/O error on temporary file - EXITING!\n") ;
    exit(-1);
  }
  fflush(fw) ;

  Ex = ifld[0] ;
  nu = ifld[1];
  fyc = ifld[2] ;
  fybc = ifld[3] ;
  fyt = ifld[4] ;
  fuc = ifld[5] ;
  fubc = ifld[6] ;
  fut = ifld[7] ;

  /* data checking: */
  if (fyc  >= fuc)  { fuc  = 0.01 * fyc  ; }
  if (fybc >= fubc) { fubc = 0.01 * fybc ; }
  if (fyt  >= fut)  { fut  = 0.01 * fyt  ; }

  /* file header */
  fprintf(fw, "1 -1\n") ;
  fprintf(fw, "1 2 10\n") ; /* linsolver, nlsolver, steps*/
  fprintf(fw, "1 2 1\n %e\n", ifld[8]) ; /* real set info + width value */

  fprintf(fw, "1 3 16\n"); /*material data definition */
  fprintf(fw, "0 %e %e\n", Ex, nu); /* EX PRXY */
  fprintf(fw, "%e %e %e\n", fyc, fybc, fyt);
  fprintf(fw, "%e %e %e 0\n", fuc, fubc, fut); /* last is ereduc */

  /* create material data for ramberg-osgood : */

  /* C: */
  s_a = fabs(fyc) ;
  e_a = 1.001 * fabs (s_a / Ex) ;

  s_b = fabs(fuc) ;
  e_b = 1.002 * fabs(s_b/s_a) * fabs (s_b / Ex) ;

	nc = log( (Ex*e_a - s_a) / (Ex*e_b-s_b) ) / log(s_a/s_b) ;
	kc = pow(s_a/Ex,1.0-nc) * ((Ex*e_a)/s_a - 1.0) ;

  for (i=1 ; i<20; i++)
  {
    /*printf ()*/
  }

  /* BC: */
  s_a = fabs(fybc) ;
  e_a = 1.001 * fabs (s_a / Ex) ;

  s_b = fabs(fubc) ;
  e_b = 1.002 * fabs(s_b/s_a) * fabs (s_b / Ex) ;

	nbc = log( (Ex*e_a - s_a) / (Ex*e_b-s_b) ) / log(s_a/s_b) ;
	kbc = pow(s_a/Ex,1.0-nbc) * ((Ex*e_a)/s_a - 1.0) ;


  /* T: */
  s_a = fabs(fyt) ;
  e_a = 1.001 * fabs (s_a / Ex) ;

  s_b = fabs(fut) ;
  e_b = 1.002 * fabs(s_b/s_a) * fabs (s_b / Ex) ;

	nt = log( (Ex*e_a - s_a) / (Ex*e_b-s_b) ) / log(s_a/s_b) ;
	kt = pow(s_a/Ex,1.0-nt) * ((Ex*e_a)/s_a - 1.0) ;


  /* TODO */

  /* run solver */
  /* TODO */

  /* get and compare results: */
  /* TODO */

  if (fclose(fw) != 0)
  {
    fprintf(stderr,"I/O error on temporary file - EXITING!\n") ;
    exit(-1);
  }
  fw = NULL ;
  return(0);
}



/* end of femcall.c */
