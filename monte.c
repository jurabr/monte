/*
   File name: monte.c
   Date:      2005/11/23 20:37
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

   Random number generator and structural reliability tool
*/

#include "monte.h"

extern FILE *frs ; /* from simul.c */

int main(int argc, char *argv[])
{
  FILE *fr   = NULL ;
  FILE *fwr  = NULL ;
  FILE *fws  = NULL ;
  FILE *fwc  = NULL ;
  FILE *fwpf = NULL ;

  msgout = stdout ;

  /* parse command line parameters */
  if (parse_command_line(argc, argv) != 0)
	{
		fprintf(msgout,"%s!\n","Input error - exiting");
		exit(-1);
	}

#ifdef USE_MMPI
  MPI_Init(&argc, &argv) ;
  MPI_Comm_rank(MPI_COMM_WORLD, &ppRank);
  MPI_Comm_size(MPI_COMM_WORLD, &ppSize);

  if (ppRank == 0) {
#endif

  if (verbose_mode == 1)
     { fprintf(msgout,"%s ..\n",_("Reading input")); }

  /* read input data: */
  if (inp_stdin == 1)
  {
    fr = stdout ;
  }
  else
  {
    if ((fr = fopen(inputfile,"r"))==NULL)
    {
      fprintf(msgout,"%s: %s\n",_("Error"), _("Can not open input file"));
      return(-1);
    }
  }

  /* read histograms */
  if (read_simple_input(fr) != 0)
  {
    fprintf(msgout,"%s: %s\n",_("Error"), _("Invalid input file (input data)"));
    fclose(fr);
    return(-1);
  }

  /* read description of output histograms */
  if (read_simple_input_out_hists(fr) != 0)
  {
    fprintf(msgout,"%s: %s\n",_("Error"), _("Invalid input file (output data)"));
    fclose(fr);
    return(-1);
  }

  /* read description of correlations */
  if (read_simple_input_correl(fr) != 0)
  {
    fprintf(msgout,"%s: %s\n",_("Error"), _("Invalid input file (correlation data)"));
    fclose(fr);
    return(-1);
  }

  /* file for reading of realizations (if needed) */
  if (res_solver == SOL_EINP)
  {
    if (irsfile != NULL)
    {
      if((frs = fopen(irsfile,"r"))==NULL)
      {
        fprintf(msgout,"%s: %s\n",_("Error"), _("Can not open file with realizations"));
        return(-1);
      }
    }
    else
    {
      frs = stdin ;
    }
  }

  if ((fr != stdin) && (frs != stdin))
  {
    fclose(fr);
  }

#ifdef USE_MMPI
  } /* end of ppRank == 0*/

  if (mpi_send_input_data() != 0) { exit(-2); };

  if (mpi_send_input_out_h_data() != 0) {exit(-3);}

#endif

  if (alloc_cc(num_ivars+num_ovars) != 0) {use_cc_simple = 0;}

#ifdef USE_MMPI
  if (ppRank == 0) {
#endif

  if (verbose_mode == 1) { fprintf(msgout," %s.\n",_("Done")); }

  if (verbose_mode == 1)
     { fprintf(msgout,"%s ..\n",_("Opening outputs")); }

#ifndef USE_MMPI /* no sense for MPI */
  /* file for realizations of random variables */
  if (res_realizations == 1)
  {
    if (res_stdout == 1)
    {
      fwr = stdout ;
    }
    else
    {
      if((fwr = fopen(ilogfile,"w"))==NULL)
      {
        fprintf(msgout,"%s: %s\n",_("Error"), _("Can not open log file"));
        return(-1);
      }
    }
  }
#endif

  /* file for statistics */
  if (res_statistics == 1)
  {
    if((fws = fopen(ologfile,"w"))==NULL)
    {
      fprintf(msgout,"%s: %s\n",_("Error"), _("Can not open output file (statistics)"));
      return(-1);
    }
  }
  
  /* file for correlation results */
  if (use_cc_simple == 1)
  {
    if((fwc = fopen(ocrmfile,"w"))==NULL)
    {
      fprintf(msgout,"%s: %s\n",_("Error"), _("Can not open output file (correlations)"));
      return(-1);
    }
  }

  /* file for correlation results */
  if (fpffile != NULL)
  {
    if((fwpf = fopen(fpffile,"w"))==NULL)
    {
      fprintf(msgout,"%s: %s\n",_("Error"), _("Can not open output file (failure data)"));
      return(-1);
    }
  }


  if (verbose_mode == 1) { fprintf(msgout," %s.\n",_("Done")); }

  if (verbose_mode == 1)
     { fprintf(msgout,"%s: \n",_("Running simulations")); }

#ifdef USE_MMPI
  } /* end of ppRank == 0 */
#endif


  if (corr_size > 0)
  {
    if (compute_corr_tran_mat(&corr_mat, &corr_tran) != 0)
    {
      if (verbose_mode == 1)
      {
        fprintf(msgout,"%s - %s!\n", _("Error"), _("computation of correlation data failed") );
      }
      return(-1);
    }
  }

  if (block_rand_init != 1)
  {
#ifdef USE_SPRNG
#ifdef USE_SPRNG1
  init_sprng(make_sprng_seed(),SPRNG_DEFAULT);
#endif
#ifdef USE_SPRNG2
  init_sprng(DEFAULT_RNG_TYPE,make_sprng_seed(),SPRNG_DEFAULT);
#endif
#else
  rand_init = init_knuth() ;
#endif
  }

  /* runs simulations: */
  monte_MC(fws, fwr, fwc) ;

#ifdef USE_MMPI
  if (ppRank == 0) {
#endif

	if (res_statistics == 1)
  { 
    fclose(fws);
  }

	if (use_cc_simple == 1)
  { 
    fclose(fwc);
  }

  if (fpffile != NULL)
  {
    /* saves probability of failure: */
    fprintf(fwpf,"%e 1.0\n", (double)fail_num/(double)sim_number);
    fclose(fwpf);
  }


#ifdef USE_MMPI
  } /* end of ppRank == 0 */
#endif

#ifndef USE_MMPI /* no sense for MPI */
	if (res_realizations == 1)
	{
  	fclose(fwr);
	}
#endif

  /* save output histograms: */
  if ((o_his_name != NULL) && (o_his != NULL))
  {
    if (save_o_his() != 0)
    {
      if (verbose_mode == 1)
      {
        fprintf(msgout,"%s - %s!\n", _("Error"),_("failed to save results in histogram form"));
      }
    }

    free_output_flds();
  }

#ifdef USE_MMPI
  if (ppRank == 0) {
#endif

  if (verbose_mode == 1) 
  { 
    fprintf(msgout," %s\n",_("All work done. Have a nice day!")); 
  }

  if (res_solver == SOL_EINP)
  {
    fclose(frs);
  }

#ifdef USE_MMPI
  } /* end of ppRank == 0 */
  MPI_Finalize() ;
#endif

#ifdef USE_LSHARED
  if (res_solver == SOL_LDL2)
  {
    monte_clean_lib_stuff2(dlarg); /* clean library data (if any) */
  }
#endif

  free_input_flds(); /* final cleanup */

  if (use_cc_simple == 1) { free_cc(); }

#ifdef USE_LSHARED
#ifndef USE_WIN32
  if (dlfile != NULL) { dlclose(dlfile); }
#endif
#endif
  return(0);
}


/* end of monte.c */
