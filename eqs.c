/*
 * File name: eqs.c
 * Date:      Tue Jul 25 23:02:06 CEST 2006
 * Author:    Jiri Brozovsky
 *
 * Cholesky factorization
 * 
 */


#include "fem_mem.h"
#include "fem_math.h"

/** Cholesky factorization only */
int femMatCholFact(tMatrix *a)
{
  int rv = 0 ;
  tVector    C ;
	double  sum ;
	long    n ;
	long    i,j,k;

	n = a->rows ;

  femVecNull(&C);
  if (femVecFullInit(&C,   n) != AF_OK) {goto memFree;}

  for (i=1; i<=n; i++)
  {
    for (j=i; j<=n; j++)
    {
      sum = femMatGet(a, i,j);
      for (k=i-1; k>=1; k--)
      {
        sum -= femMatGet(a, i,k) * femMatGet(a, j, k) ;
      }
      if (i == j)
      {
        if (sum <= 0.0)
        {
          rv = -1 ;
          goto memFree ;
        }
        femVecPut(&C,i, sqrt(sum));
      }
      else
      {
        femMatPut(a, j, i, sum / femVecGet(&C,i));
      }
    }
  }

  for (i=1; i<=n; i++)
  {
    for (j=i; j<=n; j++)
    {
      if (i != j)
      {
        femMatPut(a,i,j, femMatGet(a,j,i));
        femMatPut(a,j,i, 0);
      }
      else
      {
        femMatPut(a,j,i, femVecGet(&C,i));
      }
    }
  }

#if 0
  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      printf("_%e", femMatGet(a, i,j));
    }
    printf("\n ");
  }
#endif

	/* freeing of memory: */
memFree:
	femVecFree(&C  );
  
  return(rv);
}

/* end of geo_eqs.c */
