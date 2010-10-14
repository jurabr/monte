/*
   File name: correl.c
   Date:      2006/07/20 15:44
   Author:    Jiri Brozovsky, Petr Konecny

   Copyright (C) 2006 Jiri Brozovsky, Petr Konecny

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

   Monte: Correlations
*/

#include "monte.h"
#include "fem_math.h"
#include "fem.h"

void rotate (tMatrix *a, int i, int j, int k, int l, 
    double g, double h, double s, double tau)
{
  g = femMatGet(a, i,j);
  h = femMatGet(a, k,l);

  femMatPut(a, i,j, g - s*(h+g*tau) );
  femMatPut(a, k,l, h + s*(g-h*tau) );
}

/* compute eigen numbers and vectors (Jacobi method) */
int monte_eigen_jacobi(tMatrix *a, tVector *d, tMatrix *v, long *nrot)
{
  long    iters = 100 ;
  int     i,iq, ip, j, n ;
  double  sm ;
  double  tresh, g, h, t, c, theta, s, tau ;
  double  checkp, checkq, checkh ;
  tVector b; 
  tVector z;

  *nrot = 0 ;

  n = a->rows ;

  femVecNull(&b);
  femVecNull(&z);

  femVecFullInit(&b, n);
  femVecFullInit(&z, n);

  for (i=1; i<=n; i++)
  {
    femVecPut( &b, i, femMatGet(a, i, i) );
    femVecPut(  d, i, femMatGet(a, i, i) );
    femVecPut( &z, i, 0.0 );

    femMatPut( v, i, i, 1.0 );
  }

  for (i=1; i<=iters; i++)
  {
    sm = 0.0 ;

    for (ip=0; ip<=(n-1); ip++)
    {
      for (iq=(ip+1); iq<=n; iq++)
      {
        sm += fabs( femMatGet(a, ip, iq) );
      }
    }

    if (sm <= FEM_ZERO) /* sum <= 0 so we are finished */
    {
      /*printf("iterations: %li\n", *nrot);*/
      femVecFree(&b);
      femVecFree(&z);
      return(0);
    }

    if (i < 4)
    {
      tresh = (0.2 * sm) / ((double)(n*n)) ;
    }
    else
    {
      tresh = 0.0 ;
    }

    for (ip=1; ip<=(n-1); ip++)
    {
      for (iq=(ip+1); iq<=n; iq++)
      {
        g = 100.0 * fabs(femMatGet(a,ip,iq));

        checkp = fabs(g*fabs(femVecGet(d, ip)) - fabs(femVecGet(d,ip)));
        checkq = fabs(g*fabs(femVecGet(d, iq)) - fabs(femVecGet(d,iq)));
        if ((i > 4) && (checkp <= FEM_ZERO) && (checkq <= FEM_ZERO))
        {
          /* off-diagonal elements are small */
          femMatPut (a, ip,iq, 0.0) ;
        }
        else
        {
          /* still are big.. */
          h = femVecGet(d, iq) - femVecGet(d, ip) ;

          checkh = fabs( fabs(h)+g - fabs(h) );

          if (checkh < FEM_ZERO)
          {
            if (h != 0.0) { t = femMatGet(a, ip, iq) / h ; }
            else          { t = 0 ; }
          }
          else
          {
            theta = (0.5*h) / femMatGet(a, ip, iq) ;
            t = 1.0 / ( fabs (theta) + sqrt(1.0 + pow(theta, 2)));
            if (theta < 0.0) { t = (-1.0)*t ; }
          }

          c = 1.0 / sqrt(1.0 + pow(t, 2) );
          s = t * c ;
          tau = s / (1.0 + c);
          h = t* femMatGet(a, ip, iq);

          femVecPut(&z, ip, femVecGet(&z, ip) - h );
          femVecPut(&z, iq, femVecGet(&z, iq) + h );

          femVecPut(d, ip, femVecGet(d, ip) - h );
          femVecPut(d, iq, femVecGet(d, iq) + h );
          
          femMatPut(a, ip, iq, 0.0);

          for (j=1; j<=ip-1; j++)
          {
            rotate(a, j,ip,j,iq, g,h,s,tau);
          }

          for (j=ip+1; j<=iq-1; j++)
          {
            rotate(a, ip,j,j,iq, g,h,s,tau);
          }

          for (j=iq+1; j<=n; j++)
          {
            rotate(a, ip,j,iq,j, g,h,s,tau);
          }

          for (j=1; j<=n; j++)
          {
            rotate(v, j,ip,j,iq, g,h,s,tau);
          }
          *nrot = *nrot + 1 ;
        }
      }
    }

    for (ip=1; ip<=n; ip++)
    {
      femVecAdd (&b, ip, femVecGet(&z, ip) );
      femVecPut ( d, ip, femVecGet(&b, ip) );
      femVecPut (&z, ip, 0.0 );

    }
  }

  if (verbose_mode == 1)
  {
    fprintf(msgout,"%s - %s\n", _("Error"), _("out of iterations for eigendata"));
  }

  return(-1);
}

/** NULLs correlation data */
void null_corr_flds(void)
{
  corr_size = 0 ; 
  corr_desc = NULL ; 

  femMatNull(&corr_mat); 
  femMatNull(&corr_tran); 
  femMatNull(&corr_s); 

  femVecNull(&corr_u); 
  femVecNull(&corr_X); 
  femVecNull(&corr_Y); 
}

/** Frees correlation data */
void free_corr_flds(void)
{
  corr_size = 0 ; 
  free(corr_desc); 

  femMatFree(&corr_mat); 
  femMatFree(&corr_tran); 
  femMatFree(&corr_s); 

  femVecFree(&corr_u); 
  femVecFree(&corr_X); 
  femVecFree(&corr_Y); 

  null_corr_flds();
}


/** Allocates (empty) correlation data
 * @len number of data
 * @return status
 */
int alloc_corr_flds(long len)
{
  if (len < 1)
  {
    return(-1);
  }

  if ((corr_desc= femIntAlloc(len)) == NULL)
  {
    goto memFree;
  }

  femFullMatInit(&corr_mat, len, len); 
  femFullMatInit(&corr_tran, len, len); 
  femFullMatInit(&corr_s, len, len); 

  femVecFullInit(&corr_u, len); 
  femVecFullInit(&corr_X, len); 
  femVecFullInit(&corr_Y, len); 

  corr_size = len ; 

  return(0);

memFree:
  free_corr_flds();
  return(-1);
}


/** Does the actual correlation: X = sigma*T*Y + u
 * @param Y uncorrelated reduced variables
 * @param 
 */ 
int do_correlation(tVector *Y, tMatrix *T, tVector *X, long *corr_fld)
{
  long n, pos, i ;
	double s2 ;
	double y ;

  n = T->rows ;

	s2 = sqrt(2);

  femVecMatMult(Y, T, X); 

	for (i=1; i<=n; i++)
	{
		pos = corr_fld[i-1] ;
		y = monte_errf( femVecGet(X,i) /s2 ) ;
		femVecPut(X, i, multhis[pos]*one_sim(&distfunc[pos], y));
	}

  return(0);
}

/** realizations -> vector for correlation
 * @param num_corr lenght of corr[]
 * @param corr indexes of values to be correlated
 * @param num_ifld lenght if ifld[]
 * @param ifld realizations
 * @param u mean values
 * @param Y vector to be filled
 * @return status
 */
int pack_val_corr(long num_corr, long *corr, long num_ifld, double *ifld, tVector *Y)
{
  long i, pos ;
  
  if ((num_corr <= 0) || (num_ifld <= 0))
  {
    return(0); /* nothing to do */
  }

  pos = 1 ;

  for (i=0; i<num_corr; i++)
  {
    femVecPut(Y, pos, ifld[i]);
    pos++;
  }
  
  return(0);
}

/** vector for correlation -> realizations 
 * @param X vector with correlated values
 * @param num_corr lenght of corr[]
 * @param corr indexes of values to be correlated
 * @param num_ifld lenght if ifld[]
 * @param ifld realizations
 * @return status
 */
int unpack_val_corr( tVector *X, long num_corr, long *corr, long num_ifld, double *ifld)
{
  long i, pos ;
  
  if ((num_corr <= 0) || (num_ifld <= 0))
  {
    return(0); /* nothing to do */
  }

  pos = 1 ;

  for (i=0; i<num_corr; i++)
  {
    ifld[i] = femVecGet(X, pos);
    pos++;
  }
 
  return(0);
}

/** makes transformation matrix from a correlation matrix */
int compute_corr_tran_mat(tMatrix *cmat, tMatrix *tran)
{
  int rv = 0 ;
	long i, j ;
  long n_iter = 0;
  long n ;
  tVector e_v ;
  tMatrix evec ;

  if (corr_size <= 0) {return(0);}  /* nothing to do */

  n = cmat->rows ;

  if (n <= 0) {return(-1);} /* it must be nonzero! */

#if 1
  femMatPrn(cmat, "CORRELATION");
#endif

  femVecNull(&e_v);
  femMatNull(&evec);

  femVecFullInit(&e_v, n) ;
  femFullMatInit(&evec, n,n) ;
	
	for (i=1; i<=n; i++)
	{
		for (j=1; j<=n; j++)
		{
printf("# %li %li\n",i, j);
			femMatPut(tran, i,j, femMatGet(cmat, i,j)) ;
		}
	}

	rv = femMatCholFact(tran);
  
  rv = monte_eigen_jacobi(cmat, &e_v, &evec, &n_iter);

	for (i=1; i<=n; i++)
	{
		if (femVecGet(&e_v, i) <= 0.0)
		{
			fprintf(msgout,"%s - %s!\n",_("Error"),_("correlation matrix IS NOT positive definite"));
			rv = -1 ;
		}
	}

#if 1
  femVecPrn(&e_v, "EIGENVALUES");
  femMatPrn(tran, "TRANSFORMATION");
#endif

  femVecFree(&e_v) ;
  femMatFree(&evec) ;

  return(rv);
}

double monte_fact(double x)
{
	long i ;
	double y = 1 ;
	
	if (x <= 1) 
	{
		return(1);
	}
	
	for (i=1; i<=x; i++)
	{
		y *= (double)i ;
	}

	return(y);
}

double monte_errf(double x)
{
	long   i ;
	double y = 0.0 ;
	long   num = 60 ;

	if (x < -5.0) { return(0.0); }
	if (x > 5.0) { return(1.0); }

  if (fabs(x) <= 1.5) {num = 12 ;}
  if (fabs(x) <= 1.8) {num = 20 ;}
  if (fabs(x) <= 3.0) {num = 40 ;}
	
	for (i=0; i <=num; i++)
	{
		y += 
			(pow((-1.0), i) * pow(x, 2*i+1)) 
			/
			(monte_fact(i) * (2*i+1))
			;
	}

	y = 0.5 + 0.5* (2.0 /sqrt(PI)) * y ;
	
	return(y);
}

double monte_normal_df(double x1, double x2)
{
  double a, b ;

	if (x1 < FEM_ZERO) { x1 = FEM_ZERO; }

  a = -2.0*log(x1) ;
  b = sin(2.0*PI*x2) ;

  if (a < 0) {a = 0;}

	return( sqrt(a) * b);
}

/* end of correl.c */
