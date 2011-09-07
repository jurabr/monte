/*
   File name: cparam.c
   Date:      2005/11/24 21:26
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

   Command line parser
*/

#include "monte.h"

void print_help(char *name)
{
  fprintf(msgout,"\n %s\n\n", _("Simple Reliability Tool"));

  fprintf(msgout,"  %s: %s %s\n",_("Usage"),name, _("options"));
  fprintf(msgout,"\n  %s:\n", _("Options"));
  fprintf(msgout,"    %s .. %s\n", "-d DIR    ",_("directory with histograms"));
  fprintf(msgout,"    %s .. %s\n", "-i FILE   ",_("input file"));
  fprintf(msgout,"    %s .. %s\n", "-si       ",_("read input data from standard input"));
  fprintf(msgout,"    %s .. %s\n", "-fs FILE  ",_("file for results (statistics)"));
  fprintf(msgout,"    %s .. %s\n", "-fr FILE  ",_("file for results (simulation data)"));
  fprintf(msgout,"    %s .. %s\n", "-nofrh    ",_("do not write header in simulation results"));
  fprintf(msgout,"    %s .. %s\n", "-sifr     ",_("save also realizations of input variables to -fr FILE"));
  fprintf(msgout,"    %s .. %s\n", "-fc FILE  ",_("file for results (data correlation matrix)"));
  fprintf(msgout,"    %s .. %s\n", "-nofch    ",_("do not write header in data correlation results"));
  fprintf(msgout,"    %s .. %s\n", "-sr       ",_("write results (simulation data only) to standard output"));
  fprintf(msgout,"    %s .. %s\n", "-fpf FILE ",_("file for probability of failure results"));
  fprintf(msgout,"    %s .. %s\n", "-foh NAME ",_("names of output histogram files will start with NAME"));
  fprintf(msgout,"    %s .. %s\n", "-fon NUM  ",_("number of values for adjusting of output histograms"));
#ifndef USE_MMPI
  fprintf(msgout,"    %s .. %s\n", "-irsf FILE",_("do not process inputs - analyze simulations in FILE only"));
  fprintf(msgout,"    %s .. %s\n", "-sirs     ",_("do not process inputs - analyze simulations in stdin only"));
#endif
  fprintf(msgout,"    %s .. %s\n", "-s NUM    ",_("number of simulations"));
  fprintf(msgout,"    %s .. %s\n", "-wall DAYS",_("maximum possible run time (in days)"));
#ifdef USE_LSHARED
  fprintf(msgout,"    %s .. %s\n", "-ld LIB   ",_("load library LIB with solver"));
  fprintf(msgout,"    %s .. %s\n", "-lda ARG  ",_("argument for loaded library (if any)"));
  fprintf(msgout,"    %s .. %s\n", "-ldm NUM  ",_("mode parameter number for library (if any) "));
#endif

  fprintf(msgout,"    %s .. %s\n", "-ctp NUM  ",_("convergence test: designed probability of failure"));
  fprintf(msgout,"    %s .. %s\n", "-ctc NUM  ",_("convergence test: theoretical reliability value"));
  fprintf(msgout,"    %s .. %s\n", "-ctn NUM  ",_("convergence test: min. number of simulations"));

  fprintf(msgout,"    %s .. %s\n", "-noir     ",_("do not initialize randomizer (use for testing only!)"));
  fprintf(msgout,"    %s .. %s\n", "-norg     ",_("do not use random generator"));
  fprintf(msgout,"    %s .. %s\n", "-v        ",_("enable verbose mode"));
  fprintf(msgout,"    %s .. %s\n", "-vn NUM   ",_("print notice after every NUM simulations"));
  fprintf(msgout,"    %s .. %s\n", "-verr     ",_("write messages to STDERR instead of STDOUT"));
  fprintf(msgout,"    %s .. %s\n", "-h        ",_("this help"));
  fprintf(msgout,"    %s .. %s\n", "-hl       ",_("print license"));
}

void print_license(char *name)
{
  fprintf(msgout,"\n %s\n", _("Simple Reliability Tool"));
  fprintf(msgout," %s %s\n\n",_("(C) 2006,2010"), _("Jiri Brozovsky, Petr Konecny, Jakub Valihrach"));

  fprintf(msgout,"  %s\n", _("This program is free software; you can redistribute it and/or"));
  fprintf(msgout,"  %s\n", _("modify it under the terms of the GNU General Public License as"));
  fprintf(msgout,"  %s\n", _("published by the Free Software Foundation; either version 2 of the"));
  fprintf(msgout,"  %s\n", _("License, or (at your option) any later version."));
  fprintf(msgout,"  %s\n", _(" "));
  fprintf(msgout,"  %s\n", _("This program is distributed in the hope that it will be useful, but"));
  fprintf(msgout,"  %s\n", _("WITHOUT ANY WARRANTY; without even the implied warranty of"));
  fprintf(msgout,"  %s\n", _("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU"));
  fprintf(msgout,"  %s\n", _("General Public License for more details."));
  fprintf(msgout,"  %s\n", _(" "));
  fprintf(msgout,"  %s\n", _("You should have received a copy of the GNU General Public License"));
  fprintf(msgout,"  %s\n", _("in a file called COPYING along with this program; if not, write to"));
  fprintf(msgout,"  %s\n", _("the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA"));
  fprintf(msgout,"  %s\n", _("02139, USA. "));
}

char *get_cmd_str(int argc, char *argv[], char *clswitch)
{
  char *str = NULL ;
  long i, j ;

  if (argc <= 1) { print_help(argv[0]); exit(0); }

  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],clswitch) == 0)
    {
      if (argc >= (i+2))
      {
        if (argv[i+1] != NULL)
        {
          if (strlen(argv[i+1]) >= 1)
          {
            if ((str = (char *)malloc(sizeof(char)*(strlen(argv[i+1])+2))) != NULL)
            {
              for (j=0; j<(strlen(argv[i+1])+2); j++) { str[j] = '\0' ; }
              strcpy(str,argv[i+1]) ;
            }
          }
        }
      }
    }
  }
  return(str) ;
}

int get_cmd_single_par(int argc, char *argv[], char *clswitch)
{
  long i ;
  
  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],clswitch) == 0) { return(1) ; }
  }

  return(0);
}

long get_cmd_int(int argc, char *argv[], char *clswitch)
{
  long i ;
  
  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],clswitch) == 0)
    {
      if (argc >= (i+2))
      {
        if (argv[i+1] != NULL)
        {
          return(abs(atoi(argv[i+1]))) ;
        }
      }
    }
  }
  
  return(0);
}

double get_cmd_dbl(int argc, char *argv[], char *clswitch)
{
  long i ;
  
  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],clswitch) == 0)
    {
      if (argc >= (i+2))
      {
        if (argv[i+1] != NULL)
        {
          return((atof(argv[i+1]))) ;
        }
      }
    }
  }
  
  return(0.0);
}

/** sets data for convergence testing */
int set_ct_data(void)
{
  if (ct_c < 0.5) 
  {
    ct_c    = -1.0 ;
    ct_nmin = -1 ;
    return(0);
  }
  else
  {
    if (ct_c <= 0.9)
    {
      ct_c = 0.9 ;
      ct_t = 1.644853476 ;
    }
    else
    {
      if (ct_c <= 0.99)
      {
        ct_c = 0.99 ;
        ct_t = 2.575831338 ;
      }
      else
      {
         ct_c = 0.999 ;
         ct_t = 3.890687140 ;
      }
    }
  }

  if (ct_nmin < 1)
  {
    ct_nmin = (long)(sim_number/10) ;
  }
  return(0);
}

int parse_command_line(int argc, char *argv[])
{
  if (get_cmd_single_par(argc, argv, "-h") == 1)
  {
    print_help(argv[0]) ;
    exit(0);
  }

  if (get_cmd_single_par(argc, argv, "-hl") == 1)
  {
    print_license(argv[0]) ;
    exit(0);
  }

  
  hisdir  = get_cmd_str(argc, argv, "-d") ;

  if ((ilogfile = get_cmd_str(argc, argv, "-fr")) != NULL)
  {
    res_realizations = 1 ;
  }

  if ((ologfile = get_cmd_str(argc, argv, "-fs")) != NULL)
  {
    res_statistics = 1 ;
  }

  if ((o_his_name = get_cmd_str(argc, argv, "-foh")) != NULL)
  {
    if (strlen(o_his_name) > 1)
    {
      /* nothing to do */
    }
    else
    {
      free(o_his_name); 
      o_his_name = NULL ;
    }
  }

  if ((ocrmfile = get_cmd_str(argc, argv, "-fc")) != NULL)
  {
    use_cc_simple = 1 ;
  }

  if (get_cmd_single_par(argc, argv, "-nofch") == 1)
  {
    use_cc_header = 0 ;
  }

  if ((fpffile = get_cmd_str(argc, argv, "-fpf")) != NULL)
  {
    /* nothing to do */
  }
  else
  {
    fpffile = NULL ;
  }


  if ((o_tst_sims = get_cmd_int(argc, argv, "-fon")) <= 0)
  {
    o_tst_sims = 1000 ;
  }

  if ((inputfile = get_cmd_str(argc, argv, "-i")) != NULL)
  {
    inp_stdin = 0 ;
  }

  if (get_cmd_single_par(argc, argv, "-si") == 1)
  {
    if (inputfile != NULL)
    {
      free(inputfile); 
      inputfile = NULL ;
    }
    inp_stdin = 1 ;
  }

  if (get_cmd_single_par(argc, argv, "-sr") == 1)
  {
    if (ilogfile != NULL)
    {
      free(ilogfile); 
      ilogfile = NULL ;
    }
    res_realizations = 1 ;
    res_stdout = 1 ;
    msgout = stderr ;
  }

  if (get_cmd_single_par(argc, argv, "-verr") == 1)
  {
    msgout = stderr ;
  }

  if (get_cmd_single_par(argc, argv, "-noir") == 1)
  {
    block_rand_init = 1 ;
  }

  if (get_cmd_single_par(argc, argv, "-norg") == 1)
  {
    rand_gen_type = 0 ;
  }


  if ((sim_number = get_cmd_int(argc, argv, "-s")) <= 0)
  {
    fprintf(msgout,"%s: %s\n",_("Error"), _("Number of simulation is required!"));
    exit(-1);
  }
  else
  {
    if (o_tst_sims > (sim_number-1))
    {
      o_tst_sims = sim_number-1 ;
    }
  }

  if ((day_run_limit = (double) get_cmd_int(argc, argv, "-wall")) <= 0)
  {
    day_run_limit = 1000 ;
  }

  if (get_cmd_single_par(argc, argv, "-nofrh") == 1)
  {
    res_rea_header = 0 ;
  }
  else
  {
    res_rea_header = 1 ;
  }

  if (get_cmd_single_par(argc, argv, "-sifr") == 1)
  {
    res_rea_s_input = 1 ;
  }
  else
  {
    res_rea_s_input = 0 ;
  }


  if (get_cmd_single_par(argc, argv, "-v") == 1)
  {
    verbose_mode = 1 ;
  }
  else
  {
    verbose_mode = 0 ;
  }

  if ((verbose_nums = get_cmd_int(argc, argv, "-vn")) >= 1)
  {
    verbose_mode = 1 ;
  }
  else
  {
    verbose_nums = 10000 ;
  }


  if ((ct_pd = get_cmd_dbl(argc, argv, "-ctp")) <= 0.0)
  {
    ct_pd   = -1.0 ;
    ct_c    = -1.0;
    ct_nmin = -1 ;
  }

  if ((ct_c = get_cmd_dbl(argc, argv, "-ctc")) < 0.5)
  {
    ct_pd   = -1.0 ;
    ct_c    = -1.0;
    ct_nmin = -1 ;
  }
  if ((ct_nmin = get_cmd_int(argc, argv, "-ctn")) < 1)
  {
    ct_nmin = -1 ;
  }


#ifdef USE_LSHARED
#ifndef USE_WIN32
  dlarg = get_cmd_str(argc, argv, "-lda") ;
  if_type = get_cmd_int(argc, argv, "-ldm") ;

  if ((dllib = get_cmd_str(argc, argv, "-ld")) == NULL)
  {
    res_solver = SOL_COPY ;
  }
  else
  {
    if ((dlfile = dlopen (dllib, RTLD_NOW)) == NULL)
    {
      fprintf(msgout,"Error - unable to open dynamic library (%s)!\n", dllib);
      free(dllib) ; dllib = NULL ;
    	res_solver = SOL_COPY ;
      return(-1);
    }
    else
    {
      monte_dlib_interface_type = dlsym(dlfile, "monte_dlib_interface_type");
      if (dlerror() != NULL)
      {
        fprintf(msgout,"Error - invalid dynamic library!\n");
        free(dllib) ; dllib = NULL ;
        free(dlfile) ; dlfile = NULL ;
    		res_solver = SOL_COPY ;
        return(-1);
      }
      {
        switch(monte_dlib_interface_type())
        {
          case 1:
            monte_solution = dlsym(dlfile, "monte_solution");
            if (dlerror() != NULL)
            {
              fprintf(msgout,"Error - invalid dynamic library!\n");
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }
            monte_nums_of_vars = dlsym(dlfile, "monte_nums_of_vars");
            if (dlerror() != NULL)
            {
              fprintf(msgout,"Error - invalid dynamic library!\n");
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }

            monte_nums_of_vars(&num_ivars, &num_ovars, &ffunc_pos);

            res_solver = SOL_LDL1 ;
            break;

          case 2:
            monte_solution2 = dlsym(dlfile, "monte_solution");
            if (dlerror() != NULL)
            {
              if (verbose_mode == 1) {fprintf(msgout,"%s - %s!\n", _("Error"), _("invalid dynamic library"));}
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }
            monte_nums_of_vars2 = dlsym(dlfile, "monte_nums_of_vars");
            if (dlerror() != NULL)
            {
              if (verbose_mode == 1) {fprintf(msgout,"%s - %s!\n", _("Error"), _("invalid dynamic library"));}
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }
            
            monte_init_lib_stuff2 = dlsym(dlfile, "monte_init_lib_stuff");
            if (dlerror() != NULL)
            {
              if (verbose_mode == 1) {fprintf(msgout,"%s - %s!\n", _("Error"), _("invalid dynamic library"));}
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }
 
            monte_clean_lib_stuff2 = dlsym(dlfile, "monte_clean_lib_stuff");
            if (dlerror() != NULL)
            {
              if (verbose_mode == 1) {fprintf(msgout,"%s - %s!\n", _("Error"), _("invalid dynamic library"));}
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }

            if (monte_init_lib_stuff2(dlarg) != 0) /* must be BEFORE nums_of_vars!*/
            {
              if (verbose_mode == 1) {fprintf(msgout,"%s - %s!\n", _("Error"), _("unable so initialize library data"));}
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }

            monte_nums_of_vars2(dlarg, &num_ivars, &num_ovars, &ffunc_pos);

            if ((num_ivars < 1)||(num_ovars < 1))
            {
              if (verbose_mode ==1){fprintf(msgout,"%s - %s!\n",_("Error"), _("no variables declared in dynamic library"));}
    					res_solver = SOL_COPY ;
              return(-1);
            }
            res_solver = SOL_LDL2 ;
            break;



          default: 
                  fprintf(msgout,"%s - %s!~\n", _("Error"), _("invalid dynamic library (unknown interface)"));
                  free(dllib) ; dllib = NULL ;
                  free(dlfile) ; dlfile = NULL ;
    							res_solver = SOL_COPY ;
                  return(-1);
                  break;
        }
      }
    }
  }
#else  /* USE_WIN32 */
	/* for windblows... */
  dlarg = get_cmd_str(argc, argv, "-lda") ;
  if_type = get_cmd_int(argc, argv, "-ldm") ;

  if ((dllib = get_cmd_str(argc, argv, "-ld")) == NULL)
  {
    res_solver = SOL_COPY ;
  }
  else
  {
    if ((int)(dlfile = LoadLibrary (dllib)) <= HINSTANCE_ERROR)
    {
      fprintf(msgout,"Error - unable to open dynamic library (%s)!\n", dllib);
      free(dllib) ; dllib = NULL ;
    	res_solver = SOL_COPY ;
      return(-1);
    }
    else
    {
      monte_dlib_interface_type = (lpfunc)GetProcAddress(dlfile, "monte_dlib_interface_type");
      {
        switch(monte_dlib_interface_type())
        {
          case 1:
            monte_solution = (ipfunc)GetProcAddress(dlfile, "monte_solution");
            monte_nums_of_vars = (pfunc)GetProcAddress(dlfile, "monte_nums_of_vars");

            monte_nums_of_vars(&num_ivars, &num_ovars, &ffunc_pos);

            res_solver = SOL_LDL1 ;
            break;

          case 2:
            monte_solution2 = (ipfunc)GetProcAddress(dlfile, "monte_solution");
            monte_nums_of_vars2 = (pfunc)GetProcAddress(dlfile, "monte_nums_of_vars");
            monte_init_lib_stuff2 = (ipfunc)GetProcAddress(dlfile, "monte_init_lib_stuff");
 
            monte_clean_lib_stuff2 = (ipfunc)GetProcAddress(dlfile, "monte_clean_lib_stuff");

            if (monte_init_lib_stuff2(dlarg) != 0) /* must be BEFORE nums_of_vars!*/
            {
              if (verbose_mode == 1) {fprintf(msgout,"%s - %s!\n", _("Error"), _("unable so initialize library data"));}
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }

            monte_nums_of_vars2(dlarg, &num_ivars, &num_ovars, &ffunc_pos);

            if ((num_ivars < 1)||(num_ovars < 1))
            {
              if (verbose_mode ==1){fprintf(msgout,"%s - %s!\n",_("Error"), _("no variables declared in dynamic library"));}
    					res_solver = SOL_COPY ;
              return(-1);
            }
            res_solver = SOL_LDL2 ;
            break;

          default: 
                  fprintf(msgout,"%s - %s!~\n", _("Error"), _("invalid dynamic library (unknown interface)"));
                  free(dllib) ; dllib = NULL ;
                  free(dlfile) ; dlfile = NULL ;
    							res_solver = SOL_COPY ;
                  return(-1);
                  break;
        }
      }
    }
  }
#endif /* USE_WIN32 */
#endif /* USE_LSHARED */

  if ((irsfile = get_cmd_str(argc, argv, "-irsf")) != NULL)
  {
    res_solver = SOL_EINP ;
  }

  if (get_cmd_single_par(argc, argv, "-sirs") == 1)
  {
    if (irsfile != NULL) {free(irsfile); irsfile=NULL;}
    res_solver = SOL_EINP ;
  }



  /* adjust some things: */
  if (o_tst_sims > sim_number)
  {
    o_tst_sims = sim_number ;
  }

  if ((use_cc_simple == 1) && (sim_number < 3))
  {
    use_cc_simple = 0 ;
    fprintf(msgout,"%s - %s!\n", _("Error"), _("result correlations are available only for more than 3 simulations"));
  }

  /* set convergence testing data */
  set_ct_data() ;

#ifdef DEVEL_VERBOSE
  if (workdir != NULL) { fprintf(msgout, "[D] Working directory: %s\n", workdir); }
  if (inputfile != NULL) { fprintf(msgout, "[D] Input file: %s\n", inputfile); }
  if (ilogfile != NULL) { fprintf(msgout, "[D] Simulations: %s\n", ilogfile); }
  if (ologfile != NULL) { fprintf(msgout, "[D] Statistics: %s\n", ologfile); }
  if (o_his_name != NULL) { fprintf(msgout, "[D] Output histograms: %s*.*\n", o_his_name); }
#endif

  return(0) ;
}

/* end of cparam.c */
