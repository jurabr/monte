/*
   File name: mpifunc.c
   Date:      2006/07/06 14:56
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

   Auxiliary funcions for the MPI version of Monte
*/

#include "monte.h"

#ifdef USE_MMPI
extern int  ppRank ; /* process ID */
extern int  ppSize ; /* number of processes */
extern long ppSims ; /* number of simulations per process */

extern double *o_tst_min  ; /* min size for histograms */
extern double *o_tst_max  ; /* max size for histograms */
extern long   *o_tst_len  ; /* lenghts of histograms */
extern long   *o_tst_type ; /* type of output histogram */
extern long   *o_tst_from ; 
extern tHis   *o_his      ; /* output histograms */
extern long out_vals_needed ;

extern long    res_solver ;
extern long   *itype      ; /* input data types */
extern tHis   *histogram  ; /* used histograms */
extern double *multhis    ; /* histogram multiplier */
extern tDF    *distfunc   ; /* distribution functions */

extern double *pp_sum  ;
extern double *pp_sum2 ;
extern double *pp_min  ;
extern double *pp_max  ;

long   *pp_o_len = NULL ;
long   *pp_o_h_data = NULL ;

extern int alloc_input_flds(long len);
extern int alloc_output_flds(long len);
extern int alloc_out_vals_fld(long len);

/** Statistic data: ************************** */

void mpi_null_rstats(void)
{
  pp_sum  = NULL;
  pp_sum2 = NULL;
  pp_min  = NULL;
  pp_max  = NULL;
}

void mpi_free_rstats(void)
{
  free(pp_sum  );
  free(pp_sum2 );
  free(pp_min  );
  free(pp_max  );

  mpi_null_rstats();
}

/** allocates filelds for MPI_Reduce of statistic values
 * @param num_ovar total number of output variables
 * */
int mpi_alloc_rstats(long num_ovars)
{
  mpi_null_rstats();

  if ((pp_sum=(double *)malloc(num_ovars*sizeof(double))) == NULL) {goto memFree;}
  if ((pp_sum2=(double *)malloc(num_ovars*sizeof(double))) == NULL) {goto memFree;}
  if ((pp_min=(double *)malloc(num_ovars*sizeof(double))) == NULL) {goto memFree;}
  if ((pp_max=(double *)malloc(num_ovars*sizeof(double))) == NULL) {goto memFree;}

  return(0);
memFree:
  mpi_free_rstats();
  return(-1);
}

/** Output histograms: *********************** */
/* TODO */

int mpi_sync_maxmin(void)
{
  long i ;

  pp_max = NULL ;
  pp_min = NULL ;
  
  if ((pp_max = (double*)malloc(num_ovars*sizeof(double)))==NULL) { goto memFree; }
  if ((pp_min = (double*)malloc(num_ovars*sizeof(double)))==NULL) { goto memFree; }

  MPI_Allreduce(o_tst_min, pp_min, num_ovars,MPI_DOUBLE,MPI_MIN,MPI_COMM_WORLD);
  MPI_Allreduce(o_tst_max, pp_max, num_ovars,MPI_DOUBLE,MPI_MAX,MPI_COMM_WORLD);

  for (i=0; i<num_ovars; i++)
  {
    if (pp_min != NULL) { o_tst_min[i] = pp_min[i] ; }
    if (pp_max != NULL) { o_tst_max[i] = pp_max[i] ; }
  }

memFree:
  free(pp_max); pp_max = NULL;    
  free(pp_min); pp_min = NULL;    
  return(0);
}


/** synchronises sizea and content of output histograms */
int mpi_sync_maxminlen_final(void)
{
  long i, j, len, dlen ;
  double gap ;
  long *tmp = NULL ;
  long  total;

  pp_max = NULL ;
  pp_min = NULL ;
  pp_o_len = NULL;
  
  if ((pp_max = (double*)malloc(num_ovars*sizeof(double)))==NULL) { goto memFree; }
  if ((pp_min = (double*)malloc(num_ovars*sizeof(double)))==NULL) { goto memFree; }
  if ((pp_o_len = (long*)malloc(num_ovars*sizeof(long)))==NULL) { goto memFree; }

  for (i=0; i<num_ovars; i++)
  {
    o_tst_min[i] = o_his[i].min;
    o_tst_max[i] = o_his[i].max;
    o_tst_len[i] = o_his[i].len;
  }

  MPI_Allreduce(o_tst_min, pp_min, num_ovars,MPI_DOUBLE,MPI_MIN,MPI_COMM_WORLD);
  MPI_Allreduce(o_tst_max, pp_max, num_ovars,MPI_DOUBLE,MPI_MAX,MPI_COMM_WORLD);
  MPI_Allreduce(o_tst_len, pp_o_len, num_ovars,MPI_LONG,MPI_MAX,MPI_COMM_WORLD);

  for (i=0; i<num_ovars; i++)
  {
    if (o_tst_type[i] > H_OUT_NONE)
    {
      if ((o_tst_type[i] != H_OUT_AUTO)
        && (o_tst_type[i] != H_OUT_AUTO_NUM)
        && (o_tst_type[i] != H_OUT_LIMS))
      {
      
#if 1
      if ((pp_o_len[i] != o_his[i].len)
        ||(pp_min[i] != o_his[i].min)
        ||(pp_max[i] != o_his[i].max))
#endif
        {
          gap = (o_his[i].max-o_his[i].min) / ((double)o_his[i].len) ;
#ifdef REPLACE_RINTL
          len = monte_round( ((pp_max[i]-pp_min[i]) / gap), M_ROUND) ;
#else
          len = rintl((pp_max[i]-pp_min[i]) / gap) ;
#endif
        
          /* reallocate to this size, fill this size */
          if ((tmp = ((long *)malloc(len*sizeof(long))))==NULL) {continue;}
          for (j=0; j<len; j++) {tmp[j] = 0;}

          if (pp_min[i] < o_his[i].min)
#ifdef REPLACE_RINTL
              { dlen = monte_round( (fabs((pp_min[i]-o_his[i].min)/gap)), M_ROUND) ; }
#else
              { dlen = rintl(fabs((pp_min[i]-o_his[i].min)/gap)) ; }
#endif
          else { dlen = 0 ; }

          for (j=0; j<o_his[i].len; j++)
          {
            tmp[j+dlen] = o_his[i].data[j] ;
          }
          free(o_his[i].data) ;
          o_his[i].data = tmp ;
          tmp = NULL ;

          o_his[i].max = pp_max[i];
          o_his[i].min = pp_min[i];
          o_his[i].len = len ;
        }
      }

      tmp = NULL;
      /* for all types of histograms: */
      if ((tmp = ((long *)malloc(o_his[i].len*sizeof(long))))==NULL) {continue;}
      for (j=0; j<o_his[i].len; j++) {tmp[j] = o_his[i].data[j];}

      MPI_Reduce(tmp, o_his[i].data,o_his[i].len,MPI_LONG,MPI_SUM,0,MPI_COMM_WORLD);
      free(tmp); tmp=NULL;
      total = o_his[i].total;
      MPI_Reduce(&total, &o_his[i].total,1,MPI_LONG,MPI_SUM,0,MPI_COMM_WORLD);
    }
  }

memFree:
  free(pp_max); pp_max = NULL;    
  free(pp_min); pp_min = NULL;    
  free(pp_o_len); pp_o_len = NULL;    
  tmp = NULL ;
  return(0);
}




/** Input data: ****************************** */

/* TODO distribute data to all cluster nodes */
int mpi_send_input_data(void)
{
  long i_main[5] ;
  long *i_len = NULL ;
  long *i_total = NULL ;
  double *i_min = NULL;
  double *i_max = NULL;

  long i;


  if (ppRank == 0)
  {
    i_main[0] = sim_number ;
    i_main[1] = num_ivars ;
    i_main[2] = num_ovars ;
    i_main[3] = ffunc_pos ;
    i_main[4] = res_solver ;
  }

  MPI_Bcast(i_main, 5, MPI_LONG, 0, MPI_COMM_WORLD);


  if (ppRank != 0)
  {
    sim_number = i_main[0]; 
    num_ivars  = i_main[1];
    num_ovars  = i_main[2];
    ffunc_pos  = i_main[3];
    res_solver = i_main[4];

    if (alloc_input_flds(num_ivars) != 0) {return(-1);}
  }

  if ((i_len=(long *)malloc(num_ivars*sizeof(long)))==NULL){return(-1);}
  if ((i_total=(long *)malloc(num_ivars*sizeof(long)))==NULL){return(-1);}

  if ((i_min=(double *)malloc(num_ivars*sizeof(double)))==NULL){return(-1);}
  if ((i_max=(double *)malloc(num_ivars*sizeof(double)))==NULL){return(-1);}

  if (ppRank == 0)
  {
    for (i=0; i<num_ivars; i++)
    {
      i_len[i] = histogram[i].len ;
      i_total[i] = histogram[i].total ;

      i_min[i] = histogram[i].min ;
      i_max[i] = histogram[i].max ;
    }
  }


  MPI_Bcast(itype, num_ivars, MPI_LONG, 0, MPI_COMM_WORLD);
  MPI_Bcast(i_len, num_ivars, MPI_LONG, 0, MPI_COMM_WORLD);
  MPI_Bcast(i_total, num_ivars, MPI_LONG, 0, MPI_COMM_WORLD);

  MPI_Bcast(multhis, num_ivars, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(i_min, num_ivars, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(i_max, num_ivars, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  
  MPI_Barrier(MPI_COMM_WORLD);

  if (ppRank != 0)
  {
    for (i=0; i<num_ivars; i++)
    {
      histogram[i].len = i_len[i] ;
      histogram[i].total = i_total[i] ;

      histogram[i].min = i_min[i] ;
      histogram[i].max = i_max[i] ;
    }
  }

  free(i_len);
  free(i_total);

  free(i_min);
  free(i_max);


  for (i=0; i<num_ivars; i++)
  {
    if ((histogram[i].len > 0) && (itype[i] > DIS_CONSTANT))
    {
      if (ppRank != 0)
      {
        /* allocate histogram data here */
        if ((histogram[i].data = (long *)malloc(histogram[i].len*sizeof(long)))==NULL){return(-1);}
      }

      MPI_Barrier(MPI_COMM_WORLD); /* maybe unnecessary? */

      MPI_Bcast(histogram[i].data, histogram[i].len, MPI_LONG, 0, MPI_COMM_WORLD);
    }
  }

  if (ppRank != 0)
  {

    for (i=0; i<num_ivars; i++)
    {
      switch(itype[i])
      {
        case DIS_UNKNOWN:
        break ;
        
        case DIS_CONSTANT:
        break ;

        case DIS_HISTOGR:
          if ( compute_pf(&histogram[i], &distfunc[i]) != 0) {return(-1);}
        break;
      }
    }
  }


  return(0);
}


/* TODO distribute o_h data to all cluster nodes */
int mpi_send_input_out_h_data(void)
{
  long o_pp_data[3];
  long i, j;

  if (o_his_name == NULL) {return(0);}

  if (ppRank == 0)
  {
    o_pp_data[0] = num_ovars ;
    o_pp_data[1] = o_tst_sims ;
    o_pp_data[2] = out_vals_needed ;
  }

  MPI_Bcast(o_pp_data, 3, MPI_LONG, 0, MPI_COMM_WORLD);

  if (ppRank != 0)
  {
    num_ovars       = o_pp_data[0] ;
    o_tst_sims      = o_pp_data[1] ;
    out_vals_needed = o_pp_data[2] ;

    if (alloc_output_flds(num_ovars) != 0) {return(-1);}
    
    if (out_vals_needed > 0)
    {
      if (alloc_out_vals_fld(out_vals_needed) != 0)
      {
        return(-1);
      }
    }
  }

  MPI_Bcast(o_tst_len, num_ovars, MPI_LONG,0, MPI_COMM_WORLD);
  MPI_Bcast(o_tst_type, num_ovars, MPI_LONG,0, MPI_COMM_WORLD);
  MPI_Bcast(o_tst_from, num_ovars, MPI_LONG,0, MPI_COMM_WORLD);

  MPI_Bcast(o_tst_min, num_ovars, MPI_DOUBLE,0, MPI_COMM_WORLD);
  MPI_Bcast(o_tst_max, num_ovars, MPI_DOUBLE,0, MPI_COMM_WORLD);

  for (i=0; i<num_ovars; i++)
  {
    if (ppRank != 0)
    {
      o_his[i].len = o_tst_len[i] ;
      o_his[i].type = 0 ;
      o_his[i].total = 0 ;
      o_his[i].min = o_tst_min[i] ;
      o_his[i].max = o_tst_max[i] ;
    }


    if (o_tst_len[i] > 0)
    {
      if (ppRank != 0)
      {
        if ((o_tst_type[i] == H_OUT_LIMS)||(o_tst_type[i] == H_OUT_LIMS_DYN))
        {
          if ((o_his[i].data = (long *)malloc(o_tst_len[i]*sizeof(long)))==NULL) { return(-1); }
          for (j=0; j<o_tst_len[i]; j++) 
          {
            o_his[i].data[j] = 0;
          }
        }
      }

    }
  }

  return(0);
}

#endif

/* end of mpifunc.c */
