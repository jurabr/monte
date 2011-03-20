/*
   File name: simul.c
   Date:      2006/07/21 21:33
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

   Monte: simulation loop + some utilities (stuff moved from finput.c)
*/

#include "monte.h"

#ifdef USE_MMPI
extern double *pp_sum  ;
extern double *pp_sum2 ;

extern double *pp_min  ;
extern double *pp_max  ;

extern long pp_fail_num ;
#endif

FILE *frs = NULL ;

/** Does one simulation
 * @param rfld - result data field (size is num_ivars=num_ovars)
 */ 
int do_simulation(double *rfld)
{
  long i, val ;
	double x1, x2 ;

  for (i=0; i<num_ivars; i++)
  {
    switch (itype[i])
    {
      case DIS_CONSTANT: 
              rfld[i] = multhis[i] ; /* constants CAN NOT be correlated */
              break ;
      case DIS_HISTOGR: 
							if (histogram[i].correlated == 1)
							{
								/* some weird stuff (correlations): */
								x1 = get_rand(&rand_init) ;
								x2 = get_rand(&rand_init) ;
								rfld[i] = monte_normal_df(x1, x2) ;
							}
							else
							{
              	rfld[i] = multhis[i]*one_sim(&distfunc[i], get_rand(&rand_init)) ;
							}
              break ;
      case DIS_COPY: 
              val = (long)(multhis[i]) ;
              if ((val >= i)||(val < 0)) { val = i ; }
              rfld[i] = rfld[val] ; /* constants CAN NOT be correlated */
              break ;
    }
  }
  return(0);
}

/** Deterministic solution
 * @param ifld input fariables field)
 * @param ifld_len lenght of ifld
 * @param ofld output fariables field)
 * @param ofld_len lenght of ifld
 * note: ifld_len == ofld_len
 * @return status
 */
int monte_transform(double *ifld, long ifld_len, double *ofld, long ofld_len)
{
  long i ;

  switch(res_solver)
  {
    case SOL_COPY:
    default:
      if (ifld_len == ofld_len)
      {
        for (i=0; i<ofld_len; i++) { ofld[i] = ifld[i] ; }
      }
      else
      {
        for (i=0; i<ofld_len; i++) { ofld[i] = 0 ; }
        return(-1) ;
      }
      break;

    case SOL_LDL1:
      return( monte_solution(ifld, ofld) );
      break;

    case SOL_LDL2:
      return( monte_solution2(dlarg, ifld, ofld, if_type) );
      break;

    case SOL_EINP:
      for (i=0; i<ofld_len; i++)
      {
        fscanf(frs," %lf", &ofld[i]);
      }
      break;
  }
  
  return(0) ;
}

/** Prints approximation of total execution time based on first n simulations
 * @param t1 start time
 * @param t2 end time
 * @param dist number of simulations completed between t1 and t2
 * @param all total number of simulations
 */
void print_approx_exec_time(time_t t1, time_t t2, long dist, long all)
{
  double s,m,h,d, month, year ;

  s = (double)all * ((double)difftime(t2,t1)/((double) dist)) ;

  m = s / 60 ;
  h = m / 60 ; 
  d = m / 24 ; 
  month = d / 30 ;
  year =  d / 365 ;
  
  if (year > 1)
  {
    fprintf(msgout," %s: %3.1f %s\n",_("Approximated total run time"),year,_("year(s)"));
  }
  else
  {
    if (month > 1)
    {
      fprintf(msgout," %s: %4.1f %s\n",_("Approximated total run time"),month,_("month(s)"));
    }
    else
    {
      if (d > 1)
      {
        fprintf(msgout," %s: %4.1f %s\n",_("Approximated total run time"),d,_("days(s)"));
      }
      else
      {
        if (h > 1)
        {
          fprintf(msgout," %s: %4.1f %s\n",_("Approximated total run time"),h,_("hour(s)"));
        }
        else
        {
          if (m > 1)
          {
            fprintf(msgout," %s: %4.1f %s\n",_("Approximated total run time"),m,_("minute(s)"));
          }
          else
          {
            if (s > 1)
            {
              fprintf(msgout," %s: %6.0f %s\n",_("Approximated total run time"),s,_("seconds(s)"));
            }
          }
        }
      }
    }
  }

  if (day_run_limit < d) 
  {
    fprintf(msgout,"%s: %s (%.0f>%.0f)\n",_("Error"),_("Approximated run time exceeds given limits - program halted!"),
        d,day_run_limit);
    exit(-1);
  }
}

/** Provides convergence testing */
int monte_test_convergence(long simulation, long fail_num)
{
  double eps ;
  double pf ;

  eps = ct_t * sqrt( (ct_pd*(1.0-ct_pd)) /((double)simulation)) ;
  pf  = ((double)fail_num) / ((double)simulation) ;

  if (pf < (ct_pd - eps)) 
  {
    if (verbose_mode == 1) {
      fprintf(msgout,"\n%s: %s %li %s\n",_("Finished"),_("Approximate solution converged in"),simulation, _("steps"));
    }
    return( 1);
  }  /* done   */
  if (pf > (ct_pd + eps)) 
  {
    if (verbose_mode == 1) {
      fprintf(msgout,"\n%s: %s %li.\n",_("Finished"), _("Unconvergent solution in simulation"), simulation);
    }
    return(-1);
  } /* failed */

  return(0);
}

/** Does sequential Monte Carlo simulation process
 */ 
int monte_MC(FILE *fstat, FILE *fsim, FILE *fcrm)
{
  int rv = 0 ;
  long i, j ;
  double *sfld = NULL ;
  double *rfld = NULL ;
  double *sum  = NULL ;
  double *sum2 = NULL ;
  double *max  = NULL ;
  double *min  = NULL ;
  double *pp_sumx = NULL ;
  double *pp_sumxy = NULL ;
  double  dispersion, vcoeff, average, sdeviatition ;
  time_t  t1, t2, t3 ;
  div_t   vsnum ;

  if (num_ovars <= 0) { num_ovars = num_ivars ; }

  if ((sfld=(double *)malloc(num_ivars*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 
  if ((rfld=(double *)malloc(num_ovars*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 

  /* allocate necessary fields */
  if (res_statistics == 1)
  {
    if ((sum =(double *)malloc(num_ovars*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 
    if ((sum2=(double *)malloc(num_ovars*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 
    if ((max =(double *)malloc(num_ovars*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 
    if ((min =(double *)malloc(num_ovars*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 

    for (i=0; i<num_ivars; i++)
    {
      sfld[i] = 0 ;
    }

    for (i=0; i<num_ovars; i++)
    {
      sum[i]  = 0 ;
      sum2[i] = 0 ;
      max[i]  = 0 ;
      min[i]  = 0 ;
    }
  }

#ifdef USE_MMPI
  if (use_cc_simple == 1)
  {
    if ((pp_sumx =(double *)malloc((num_ovars+num_ivars)*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 
    if ((pp_sumxy=(double *)malloc(pow(num_ovars+num_ivars,2)*sizeof(double)))==NULL) { rv= -1 ; goto memFree ; } 

    for (i=0; i<num_ivars+num_ovars; i++) { pp_sumx[i] = 0 ; }
    for (i=0; i<pow(num_ivars+num_ovars,2); i++) { pp_sumxy[i] = 0 ; }
  }
#endif
 
  for (i=0; i<num_ivars; i++) { sfld[i] = 0 ; }
  for (i=0; i<num_ovars; i++) { rfld[i] = 0 ; }

#ifndef USE_MMPI
  if (res_realizations == 1)
  {
    if (res_rea_header == 1)
    {
      if ((res_rea_s_input == 1) && (res_solver != SOL_COPY))
      {
        fprintf(fsim, "%li %li\n", num_ivars+num_ovars, sim_number);
      }
      else
      {
        fprintf(fsim, "%li %li\n", num_ovars, sim_number);
      }
    }
  }
#endif

#ifdef USE_MMPI
      vsnum = div(sim_number, ppSize) ;

      if (vsnum.quot < 1)
      {
        fprintf(msgout,
            "Error - number of simulations is lower than number of processes!\n");
        exit(-1);
      }
      
      if (ppRank == 0) { ppSims = vsnum.quot + vsnum.rem ; }
      else { ppSims = vsnum.quot ; }
#endif

  time(&t1) ;

  /* main simulation loop: */
#ifdef USE_MMPI
  for (i=0; i<ppSims; i++)
#else
  for (i=0; i<sim_number; i++)
#endif
  {
    if (res_solver != SOL_EINP)
    {
      /* realizations: */
      do_simulation(sfld);

      /* data correlation */
      if (corr_size > 0)
      {
        pack_val_corr(corr_size, corr_desc, num_ivars, sfld, &corr_Y);
        do_correlation( &corr_Y, &corr_tran, &corr_X, corr_desc);
        unpack_val_corr(&corr_X, corr_size, corr_desc, num_ivars, sfld);
      }
      /* end of correlation... */
    }

    /* deterministic solution: */
    monte_transform(sfld, num_ivars, rfld, num_ovars) ;

    if (res_solver != SOL_EINP)
    {
      /* data for correlation coefficient */
      if (use_cc_simple == 1) { fill_cc(num_ivars, sfld, num_ovars, rfld); } 
    }

    /* number of failures: */
    if (ffunc_pos > -1) { if (rfld[ffunc_pos] > 0) { fail_num++; } }

#ifdef USE_MMPI
  if (ppRank == 0) {
#endif
    if (verbose_mode == 1)
    {
      if (i == 0) { time(&t2) ; print_approx_exec_time(t1, t2, 1, sim_number); }
      if (i == 9) { time(&t3) ; print_approx_exec_time(t1, t3, 10, sim_number); }
      if (i == 99) { time(&t3) ; print_approx_exec_time(t1, t3, 100, sim_number); }
      if (i == 999) { time(&t3) ; print_approx_exec_time(t1, t3, 1000, sim_number); }
      if (i == 9999) { time(&t3) ; print_approx_exec_time(t1, t3, 10000, sim_number); }
      if (i == 99999) { time(&t3) ; print_approx_exec_time(t1, t3, 100000, sim_number); }
      if (i == 499999) { time(&t3) ; print_approx_exec_time(t1, t3, 500000, sim_number); }

      vsnum = div((i+1), verbose_nums) ;
      if (vsnum.rem==0)
      {
        fprintf(msgout,"  %s: %14li",_("Simulation"),i+1);
				if (ffunc_pos > -1)
				{
					fprintf(msgout, ",   pf = %1.8f    (%li/%li)\n",
							((double)fail_num/(double)(i+1.0)), fail_num, i+1
							);
				}
				else
				{
					fprintf(msgout,"\n");
				}
      } 
    }
#ifdef USE_MMPI
  } /* end of ppRank == 0 */
#endif

    /* output histograms */
    if ((o_his != NULL) && (o_his_name != NULL))
    {
      if (i < (o_tst_sims-1)) 
      { 
        add_out_setup_vars(i, rfld); 
      }
      else
      {
        if (i == (o_tst_sims-1))
        {
          /* create output histograms */
          if (setup_out_histograms() != 0)
          {
            if (verbose_mode == 1)
            { fprintf(msgout,"%s - %s!\n",_("Error"),_("unable to make setup histograms")); }
            rv = -1 ;
            goto memFree;
          }
        }
        /* use output histograms */
        add_out_vars(rfld); 
      }
    }


#ifndef USE_MMPI /* no sense for MPI */
    if (res_realizations == 1)
    {
      if ((res_rea_s_input == 1) && (res_solver != SOL_COPY))
      {
        for (j=0; j<num_ivars; j++) { fprintf(fsim, " %e",sfld[j]); }
      }

      for (j=0; j<num_ovars; j++) { fprintf(fsim, " %e",rfld[j]); }
      fprintf(fsim, "\n");
    }
#endif

    if (res_statistics == 1)
    {
      for (j=0; j<num_ovars; j++)
      {
        if (i == 1)
        {
          max[j] = rfld[j] ;
          min[j] = rfld[j] ;
        }
        else
        {
          if (max[j] < rfld[j]) {max[j] = rfld[j];}
          if (min[j] > rfld[j]) {min[j] = rfld[j];}
        }
        sum[j]  += rfld[j] ;
        sum2[j] += pow(rfld[j],2) ;
      }
    }


    /* convergence testing: */
    if (ct_pd > 0.0) {
      if (i >= (ct_nmin-1))
      {
        if ((monte_test_convergence(i+1,fail_num)) != 0)
        {
          sim_number = i+1 ;
          break;
        }
      }
    }
  }

  /* compute and save results */
#ifdef USE_MMPI
  mpi_alloc_rstats(num_ovars); /* no testing :-\ */

  MPI_Reduce(sum, pp_sum, num_ovars,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(sum2, pp_sum2, num_ovars,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(min, pp_min, num_ovars,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);
  MPI_Reduce(max, pp_max, num_ovars,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

  if (ppRank == 0)
  {
    for (i=0; i<num_ovars; i++)
    {
      sum[i] = pp_sum[i] ;
      sum2[i] = pp_sum2[i] ;
      min[i] = pp_min[i] ;
      max[i] = pp_max[i] ;
    }
  }

  mpi_free_rstats();

  
  if (ppRank == 0) {
#endif
  if (res_statistics == 1)
  {
    if (res_rea_number == 1)
    {
      fprintf(fstat,"%li %li\n", num_ovars, sim_number);
    }

    for (i=0; i<num_ovars; i++)
    {
      average     = sum[i] / ((double)sim_number) ;
      if (sim_number > 1)
      {
#if 0
        /* probably incorrect: */
        dispersion  = (1.0/((double)sim_number-1.0))*                                       
           ( sum2[i] - (1.0/(double)sim_number)*pow(sum[i],2));
#else
        /* corrected (Barch et al, page 748): */
        dispersion  = (1.0/((double)sim_number-1.0))*                                       
           (sum2[i] - average*sum[i]);
#endif
      }
      else
      {
        dispersion = 0 ;
      }

      sdeviatition = sqrt(dispersion) ; 
    
      if (average != 0.0)
      {
        vcoeff = fabs(sdeviatition / average) ;
      }
      else
      {
        vcoeff = 0 ;
      }
      
      fprintf(fstat, "%e %e %e %e %e %e\n",
        min[i],
        max[i],
        average,
        dispersion,
        sdeviatition,
        vcoeff
        );
    }
  }

#ifdef USE_MMPI
  } /* end of ppRank == 0 */

  if (ffunc_pos > -1)
  {
    MPI_Reduce(&fail_num, &pp_fail_num, 1,MPI_LONG,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
  }

  if (use_cc_simple == 1)
  {
    MPI_Reduce(&cc_x, &pp_sumx, (num_ivars+num_ovars),MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&cc_xy, &pp_sumxy, pow(num_ivars+num_ovars,2),MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  }

  if (ppRank == 0) {

  if (ffunc_pos > -1)
  {
    fail_num = pp_fail_num ;
  }

  if (use_cc_simple == 1)
  {
    for (i=0; i<num_ivars+num_ovars; i++) 
    { 
      cc_x[i] = pp_sumx[i] ; 
    }
    for (i=0; i<pow(num_ivars+num_ovars,2); i++) 
    { 
      cc_xy[i] = pp_sumxy[i] ; 
    }
  }
#endif

  /* probability of failure */
  if (verbose_mode == 1)
  {
    if (ffunc_pos > -1)
    {
      fprintf(msgout,"\n %s: (%li/%li) = %f\n\n",
          _("Probability of failure"),
          fail_num, sim_number, (double)fail_num/(double)sim_number
          );
    }
  }

  if (use_cc_simple == 1)
  {
    /* todo Change stdout to something different */
    write_cc_matrix(fcrm, num_ivars, num_ovars, sim_number, use_cc_header);
  }

#ifdef USE_MMPI
  } /* end of ppRank == 0 */
#endif

#if 1
	return(rv); /* TODO: this makes memory leak but no segfault ! */
#endif
memFree:
  free(sfld); sfld = NULL ;
  free(sum ); sum  = NULL ;
  free(sum2); sum2 = NULL ;
  if (use_cc_simple == 1)
  {
    free(pp_sumx); pp_sumx = NULL ;
    free(pp_sumxy); pp_sumxy = NULL ;
  }
  return(rv);
}


/* end of simul.c */
