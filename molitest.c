/*
   File name: molitest.c
   Date:      2006/07/15 15:18
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

   Monte: utility for obtaining informations from dynamic libraries
*/

#include "monte.h"

#ifdef USE_LSHARED
char *(*monte_ivar_name)(char *, long );
char *(*monte_ovar_name)(char *, long );
int (*monte_solution)(double *, double *);
void (*monte_nums_of_vars2)(char *, long *, long *, long *);
int (*monte_init_lib_stuff2)(char *);
int (*monte_clean_lib_stuff2)(char *);
int (*monte_solution2)(char *,double *, double *);


long output_type = -1 ;
long ret_pos_index = -1 ;
long ignore_corr   = 0 ;
FILE *fi = NULL ;
FILE *fo = NULL ;

void print_help(char *name)
{
  fprintf(msgout,"\n %s\n\n", _("Simple Reliability Tool - Dynamic library utility"));

  fprintf(msgout,"  %s: %s %s\n",_("Usage"),name, _("options"));
  fprintf(msgout,"\n  %s:\n", _("Options"));

  fprintf(msgout,"    %s .. %s\n", "-ld LIB   ",_("load library LIB with solver (required!)"));
  fprintf(msgout,"    %s .. %s\n", "-lda ARG  ",_("argument for loaded library (if any)"));

  fprintf(msgout,"    %s .. %s\n", "-o   FILE ",_("write output to FILE"));
  fprintf(msgout,"    %s .. %s\n", "-so       ",_("send all outputs to standard output"));

  fprintf(msgout,"    %s .. %s\n", "-d        ",_("dump all available informations"));
  fprintf(msgout,"    %s .. %s\n", "-lt       ",_("return library interface type"));
  fprintf(msgout,"    %s .. %s\n", "-ni       ",_("return number of input arguments"));
  fprintf(msgout,"    %s .. %s\n", "-no       ",_("return number of input arguments"));
  fprintf(msgout,"    %s .. %s\n", "-nf       ",_("return index of failure function parameter (-1 if none)"));

  fprintf(msgout,"    %s .. %s\n", "-pi NUM   ",_("return name of NUMth input variable (numbered from 0)"));
  fprintf(msgout,"    %s .. %s\n", "-po NUM   ",_("return name of NUMth output variable (numbered from 0)"));

  fprintf(msgout,"    %s .. %s\n", "-if       ",_("return template of input file"));

  fprintf(msgout,"    %s .. %s\n", "-sd FILE  ",_("single set of input data in FILE - and compute results"));
  fprintf(msgout,"    %s .. %s\n", "-sdsi     ",_("single set of input data from stdin - and compute results"));
  fprintf(msgout,"    %s .. %s\n", "-ic       ",_("ignore correlations when computing"));

  fprintf(msgout,"    %s .. %s\n", "-v        ",_("enable verbose mode"));
  fprintf(msgout,"    %s .. %s\n", "-h        ",_("this help"));
  fprintf(msgout,"    %s .. %s\n", "-hl       ",_("print license"));
}

void print_license(char *name)
{
  fprintf(msgout,"\n %s\n", _("Simple Reliability Tool (sharet library analyzing utility)"));
  fprintf(msgout," %s %s\n\n",_("(C) 2005"), _("Jiri Brozovsky, Petr Konecny, Jakub Valihrach"));

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

int parse_command_line(int argc, char *argv[])
{
  verbose_mode = 0 ;

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

  if (get_cmd_single_par(argc, argv, "-v") == 1)
  {
    verbose_mode = 1 ;
  }
  else
  {
    verbose_mode = 0 ;
  }

#ifdef USE_LSHARED
#ifndef USE_WIN32
  dlarg = get_cmd_str(argc, argv, "-lda") ;

  if ((dllib = get_cmd_str(argc, argv, "-ld")) == NULL)
  {
    res_solver = SOL_COPY ;
  }
  else
  {
    /* dlfile = dlopen ("dllib", RTLD_LAZY); */
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

            monte_ivar_name = dlsym(dlfile, "monte_ivar_name");
            if (dlerror() != NULL)
            {
              if (verbose_mode == 1) {fprintf(msgout,"%s - %s!\n", _("Error"), _("invalid dynamic library"));}
              free(dllib) ; dllib = NULL ;
              free(dlfile) ; dlfile = NULL ;
    					res_solver = SOL_COPY ;
              return(-1);
            }

            monte_ovar_name = dlsym(dlfile, "monte_ovar_name");
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

 
            if (monte_init_lib_stuff2(dlarg) != 0)  /* must be BEFORE nums_of_vars!*/
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
#endif

  if (get_cmd_single_par(argc, argv, "-d") == 1)
  {
    output_type = 1 ; /* dump info */
  }

  if (get_cmd_single_par(argc, argv, "-lt") == 1)
  {
    output_type = 2 ; /* library interface type */
  }

  if (get_cmd_single_par(argc, argv, "-ni") == 1)
  {
    output_type = 3 ; /* number of input variables */
  }

  if (get_cmd_single_par(argc, argv, "-no") == 1)
  {
    output_type = 4 ; /* number of output variables */
  }

  if (get_cmd_single_par(argc, argv, "-nf") == 1)
  {
    output_type = 5 ; /* index of failure function */
  }

  if (get_cmd_single_par(argc, argv, "-if") == 1)
  {
    output_type = 6 ; /* create input file for Monte */
  }

  if (get_cmd_single_par(argc, argv, "-pi") == 1)
  {
    if ((ret_pos_index = get_cmd_int(argc, argv, "-pi")) >= 0)
    {
      output_type = 7 ; /* name of i-th input variable */
    }
  }

  if (get_cmd_single_par(argc, argv, "-po") == 1)
  {
    if ((ret_pos_index = get_cmd_int(argc, argv, "-po")) >= 0)
    {
      output_type = 8 ; /* name of i-th output variable */
    }
  }

  if ((ilogfile = get_cmd_str(argc, argv, "-sd")) != NULL)
  {
    output_type = 9 ; /* write single solution, get input from ilogfile */
  }
  
  if (get_cmd_single_par(argc, argv, "-sdsi") == 1)
  {
    free(ilogfile) ;
    ilogfile = NULL ;
    output_type = 9 ; /* write single solution, get input from ilogfile */
  }
  
  if (get_cmd_single_par(argc, argv, "-ic") == 1)
  {
    ignore_corr = 1 ;
  }

  if (get_cmd_single_par(argc, argv, "-so") == 1)
  {
    free(ologfile); ologfile = NULL ;
  }

  if ((ologfile = get_cmd_str(argc, argv, "-o")) != NULL)
  {
    /* output file*/
  }
  else
  {
    ologfile = NULL ;
  }

  return(0) ;
}

int open_files(void)
{
  if (output_type == 9)
  {
    if (ilogfile != NULL)
    {
      if ((fi=fopen(ilogfile, "r")) == NULL)
      {
        fprintf(msgout, "Error - can not read solution input!\n");
        exit(-1);
      }
    }
    else
    {
      fi = stdin ;
    }
  }

  if (ologfile == NULL)
  {
    fo = stdout ;
  }
  else
  {
    if ((fo = fopen(ologfile,"w")) == NULL)
    {
      fo = stdout ;
    }
  }

  return(0);
}

void close_files(void)
{
  if (fi != NULL) {fclose(fi);}
  if (fo != NULL) {fclose(fo);}
}

int get_output(void)
{
  long i ;
  double *ifld = NULL ;
  double *ofld = NULL ;

  switch (output_type)
  {
    case 1:
      fprintf(fo, "Interface type  : %li\n\n", res_solver);
      fprintf(fo, "Input variables : %li\n", num_ivars);
      fprintf(fo, "Output variables: %li\n", num_ovars);
      if (ffunc_pos > -1)
      {
        fprintf(fo, "Failure function: %li\n", ffunc_pos);
      }
      else
      {
        fprintf(fo, " No failure function available.\n");
      }

      if (res_solver == SOL_LDL2)
      {
        fprintf(fo, "\nInput variables: %li\n", res_solver);
        for (i=0; i<num_ivars; i++)
        {
          fprintf(fo,"%6li: %s\n", i, monte_ivar_name(dlarg, i));
        }

        fprintf(fo, "\nOutput variables: %li\n", res_solver);
        for (i=0; i<num_ovars; i++)
        {
          fprintf(fo,"%6li: %s\n", i, monte_ovar_name(dlarg, i));
        }
      }
      
      break;
    case 2: fprintf(fo, "%li\n", res_solver); break;
    case 3: fprintf(fo, "%li\n", num_ivars); break;
    case 4: fprintf(fo, "%li\n", num_ovars); break;
    case 5: fprintf(fo, "%li\n", ffunc_pos); break;
    case 6:
            fprintf(fo,"%li\n", num_ivars) ;
            for (i=0; i<num_ivars; i++)
            {
              if (res_solver == SOL_LDL1)
              {
                fprintf(fo,"ivar%li 1.0 0 \n",i+1);
              }
              else
              {
                fprintf(fo,"%s 1.0 0 \n", monte_ivar_name(dlarg, i));
              }
            }

            fprintf(fo,"%li\n", num_ovars) ;
            for (i=0; i<num_ovars; i++)
            {
              if (res_solver == SOL_LDL1)
              {
                fprintf(fo,"ovar%li %i\n",i+1, (int)H_OUT_AUTO);
              }
              else
              {
                fprintf(fo,"%s %i\n", monte_ovar_name(dlarg, i), (int)H_OUT_AUTO);
              }
            }
      break;
    case 7: fprintf(fo,"%s\n", monte_ivar_name(dlarg, ret_pos_index)); break;
    case 8: fprintf(fo,"%s\n", monte_ovar_name(dlarg, ret_pos_index)); break;
    case 9:
            if ((ifld=(double*)malloc(sizeof(double)*num_ivars))==NULL)
            {
              fprintf(msgout,"Error - out of memory!\n");
              return(-1);
            }
            if ((ofld=(double*)malloc(sizeof(double)*num_ovars))==NULL)
            {
              free (ifld);
              fprintf(msgout,"Error - out of memory!\n");
              return(-1);
            }

            for (i=0; i<num_ivars; i++) { ifld[i] = 0 ; }
            for (i=0; i<num_ovars; i++) { ofld[i] = 0 ; }

            for (i=0; i<num_ivars; i++) 
            { 
              fscanf(fi,"%lf", &ifld[i]) ; 
            }

            /* correlations: */
            if ((ignore_corr == 0)&& (corr_size > 0))
            {
              null_corr_flds();
              alloc_corr_flds(corr_size);

							if (compute_corr_tran_mat(&corr_mat, &corr_tran) != 0)
    					{
        				fprintf(msgout,"%s - %s!\n", _("Error"), _("computation of correlation data failed") );
      					return(-1);
    					}

              pack_val_corr(corr_size, corr_desc, num_ivars, ifld, &corr_Y);
              do_correlation( &corr_Y, &corr_tran, &corr_X, corr_desc);
              unpack_val_corr(&corr_X, num_ivars, corr_desc, num_ivars, ifld);
            }

            switch (res_solver)
            {
              case SOL_LDL1:
                monte_solution(ifld, ofld);
                break;
              case SOL_LDL2:
                monte_solution2(dlarg, ifld, ofld);
                break;
            }

            for (i=0; i<num_ovars; i++) 
            { 
              fprintf(fo,"%e\n", ofld[i]) ; 
            }

            free(ifld);
            free(ofld);

            if ((ignore_corr == 0)&& (corr_size > 0))
            {
              free_corr_flds();
            }
      break;
  }

  return(0);
}
#endif



/* Main routine: ############################################# */
int main(int argc, char *argv[])
{
  msgout = stderr ;
#ifdef USE_LSHARED
  if (argc <= 1) { print_help(argv[0]); return(0); }

  if (parse_command_line(argc, argv) != 0)
  {
    fprintf(msgout,"Error - bad parameters!\n");
    return(-1);
  }

  open_files();

  get_output();
  
  close_files();

  if (res_solver == SOL_LDL2)
  {
    monte_clean_lib_stuff2(dlarg); /* clean library data (if any) */
  }

#ifndef USE_WIN32
  if (dlfile != NULL) { dlclose(dlfile); }
#endif

#endif
  return(0);
}

/* end of molitest.c */
