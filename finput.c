/*
   File name: finput.c
   Date:      2005/11/26 14:23
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

   Reading of input data (specification of variables etc.) from file
*/

#include "monte.h"

/* Data */
long    sim_number = 1 ;    /* number of simulation */
long    num_ivars  = 0 ;    /* number of input variables */
long    num_ovars  = 0 ;    /* number of output variables */
long    ffunc_pos  = -1 ;   /* position of failure function (if any) */
long    fail_num   = 0 ;    /* used for computation of probability of failure */
long    if_type    = 0 ;    /* work mode for monte_solution2 */
long   *itype      = NULL ; /* input data types */
tHis   *histogram  = NULL ; /* used histograms */
double *multhis    = NULL ; /* histogram multiplier */
long   *deppos     = NULL ; /* dependence data (for DIS_DEPEND) */
tDF    *distfunc   = NULL ; /* distribution functions */
char   *hisdir     = NULL ; /* directory with histograms */
char   *workdir    = NULL ; /* working directory */
char   *inputfile  = NULL ; /* input file name */
char   *ilogfile   = NULL ; /* file with realizations of variables */
char   *ologfile   = NULL ; /* file with simulation results */
char   *ocrmfile   = NULL ; /* file for output correlation matrix */
char   *irsfile    = NULL ; /* to read external simulation results */
char   *fpffile    = NULL ; /* file for reliability of failure data */

/* Output histograms */
long    o_tst_sims = 1000 ; /* number of simulations to setup histograms */
long   *o_tst_type = NULL ; /* type of output histogram */
long   *o_tst_from = NULL ; /* starting point in o_tst_vals field */
long   *o_tst_len  = NULL ; /* lenghts of histograms */
double *o_tst_min  = NULL ; /* min size for histograms */
double *o_tst_max  = NULL ; /* max size for histograms */
tHis   *o_his      = NULL ; /* output histograms */
double *o_tst_vals = NULL ; /* computed values */
char   *o_his_name = NULL ; /* template for output histogram file names */
long out_vals_needed = 0 ;

/* Correlations: */
long    corr_size = 0 ;    /* size of correlation matrix */
long   *corr_desc = NULL ; /* indexes of variables that have to be correlated */
tMatrix corr_mat  ;        /* correlation matrix (user-supplied) */
tMatrix corr_tran ;        /* transformational matrix for correlation */
tMatrix corr_s    ;        /* standard deviations */
tVector corr_u    ;        /* mean values */
tVector corr_X    ;        /* correlated data */
tVector corr_Y    ;        /* uncorrelated (input) */

/* Corr. coeffs... */
double  *cc_x  = NULL ;
double  *cc_xy = NULL ;

/* Convergence tests: */
double   ct_c     = -1.0 ;
double   ct_t     = -1.0 ;
double   ct_pd    = 0.0 ;
int      ct_nmin  = -1 ;

/* Dynamic libraries */
#ifdef USE_LSHARED
char   *dllib      = NULL ; /* dynamic library name */
void   *dlarg      = NULL ; /* argument for dynamic library */
#ifndef USE_WIN32
void   *dlfile     = NULL ; /* dynamic library file */
#else
HANDLE dlfile;
#endif
#endif

/* feature switches: */
long    inp_stdin        = 0 ;
long    res_stdout       = 0 ;
long    res_statistics   = 0 ;
long    res_realizations = 0 ;
long    res_rea_s_input  = 0 ;
long    res_rea_header   = 1 ;
long    res_rea_number   = 1 ;
long    res_solver       = SOL_COPY ;
double  day_run_limit    = 1 ;
long    verbose_mode     = 0 ;
long    verbose_nums     = 10000 ;
long    use_cc_simple    = 0 ;
long    use_cc_header    = 1 ;
long    block_rand_init  = 0 ; /* do not init randomizers (for testing) */

#ifdef USE_MMPI
int  ppRank = 0 ; /* process ID */
int  ppSize = 0 ; /* number of processes */
long ppSims = 0 ; /* number of simulations per process */

double *pp_sum  = NULL ;
double *pp_sum2 = NULL ;
double *pp_min  = NULL ;
double *pp_max  = NULL ;

long pp_fail_num = 0 ;
#endif

/* random stuff */
long rand_init = 152 ;

void null_input_flds(void)
{
  itype      = NULL ;
  histogram  = NULL ;
  multhis    = NULL ;
  deppos     = NULL ;
  distfunc   = NULL ;
}

void free_input_flds(void)
{
  free(itype      ) ;
  free(histogram  ) ;
  free(multhis    ) ;
  free(deppos     ) ;
  free(distfunc   ) ;

  null_input_flds() ;
}

int alloc_input_flds(long len)
{
  null_input_flds() ;

  if ((itype = (long *)malloc(len*sizeof(long))) == NULL)
     { free_input_flds() ; return(-1); }

  if ((histogram = (tHis *)malloc(len*sizeof(tHis))) == NULL)
     { free_input_flds() ; return(-1); }

  if ((multhis = (double *)malloc(len*sizeof(double))) == NULL)
     { free_input_flds() ; return(-1); }

  if ((deppos = (long *)malloc(len*sizeof(long))) == NULL)
     { free_input_flds() ; return(-1); }

  if ((distfunc = (tDF *)malloc(len*sizeof(tDF))) == NULL)
     { free_input_flds() ; return(-1); }

  return(0);
}

void null_output_flds(void)
{
  /* o_tst_sims have not to be here !!! */
  o_tst_type = NULL ;
  o_tst_from = NULL ;
  o_tst_vals = NULL ;
  o_tst_len  = NULL ;
  o_tst_min  = NULL ;
  o_tst_max  = NULL ;
  o_his      = NULL ;
}

void free_output_flds(void)
{
  o_tst_sims = 0 ;
  free(o_tst_type ) ;
  free(o_tst_from ) ;
  free(o_tst_vals ) ;
  free(o_tst_len  ) ;
  free(o_tst_min  ) ;
  free(o_tst_max  ) ;
  free(o_his      ) ;

  null_output_flds() ;
}

int alloc_output_flds(long len)
{
  long i ;

  null_output_flds() ;

  if (len < 1) { return(0); } /* nothing to do */

  if ((o_tst_type = (long *)malloc(len*sizeof(long))) == NULL)
     { free_output_flds() ; return(-1); }

  if ((o_tst_from = (long *)malloc(len*sizeof(long))) == NULL)
     { free_output_flds() ; return(-1); }
  
  if ((o_tst_len = (long *)malloc(len*sizeof(long))) == NULL)
     { free_output_flds() ; return(-1); }


  if ((o_tst_min = (double *)malloc(len*sizeof(double))) == NULL)
     { free_output_flds() ; return(-1); }

  if ((o_tst_max = (double *)malloc(len*sizeof(double))) == NULL)
     { free_output_flds() ; return(-1); }


  if ((o_his = (tHis *)malloc(len*sizeof(tHis))) == NULL)
     { free_output_flds() ; return(-1); }

  for (i=0; i<len; i++)
  {
    o_his[i].name  = NULL ;
    o_his[i].type  = 0 ;
    o_his[i].len   = 0 ;
    o_his[i].min   = 0.0 ;
    o_his[i].max   = 0.0 ;
    o_his[i].total = 0 ;
    o_his[i].data  = NULL ;

    o_tst_type[i] = H_OUT_NONE ;
    o_tst_from[i] = -1 ;
    o_tst_len[i] =  0 ;
    o_tst_min[i] = 0.0 ;
    o_tst_max[i] = 0.0 ;
  }

  /* o_tst_vals still NOT ALLOCATED here!!! */

  return(0);
}

int alloc_out_vals_fld(long len)
{
  long i;

  if ((o_tst_vals = (double *)malloc(len*sizeof(double))) == NULL)
     { free_output_flds() ; return(-1); }

  for (i=0; i<len; i++) {o_tst_vals[i] = 0.0;}
  return(0);
}

/** Adds output variables to histogram-analysis fields */
int add_out_setup_vars(long sim_number, double *rfld)
{
  long i ;
  long pos ;
  long force ;
  long rv = 0 ;

  if (o_tst_type == NULL) {return(-1);}

  for (i=0; i<num_ovars; i++)
  {
    if ((o_tst_type[i] == H_OUT_AUTO)
        ||(o_tst_type[i] == H_OUT_AUTO_DYN)
        ||(o_tst_type[i] == H_OUT_AUTO_NUM)
        ||(o_tst_type[i] == H_OUT_AUTO_NUM_DYN)
        )
    {
      pos = o_tst_from[i]+sim_number ;
      o_tst_vals[pos] = rfld[i] ;
    }
    else
    {
      if ((o_tst_type[i] == H_OUT_LIMS)||(o_tst_type[i] == H_OUT_LIMS_DYN))
      {
        if (o_tst_type[i] == H_OUT_LIMS_DYN)
        {
          force = 0 ;
        }
        else
        {
          force = 1 ;
        }

        rv = put_val_to_histogram(&o_his[i], rfld[i], force);


        if ((rv != 0)&&(force == 0))
        {
          enlarge_histogram(&o_his[i], rfld[i]) ;
          rv=put_val_to_histogram(&o_his[i], rfld[i], force);
        }
      }
    }
  }
  return(0);
}

/** TODO */
int setup_out_histograms(void)
{
  long i, j ;
  long pos ;
  double max, min ;
  long rv = 0 ;

  if (o_tst_type == NULL) {return(-1);}

  for (i=0; i<num_ovars; i++)
  {
    if ((o_tst_type[i] == H_OUT_AUTO)
        ||(o_tst_type[i] == H_OUT_AUTO_DYN)
        ||(o_tst_type[i] == H_OUT_AUTO_NUM)
        ||(o_tst_type[i] == H_OUT_AUTO_NUM_DYN)
        )
    {
      max = o_tst_vals[o_tst_from[i]] ;
      min = o_tst_vals[o_tst_from[i]] ;

      for (j=1; j<(o_tst_sims-1); j++)
      {
        pos = o_tst_from[i] + j ;
        if (max < o_tst_vals[pos]) {max =  o_tst_vals[pos]; }
        if (min > o_tst_vals[pos]) {min =  o_tst_vals[pos]; }
      }

      /* some checks: */
      if (max == min) { max = fabs(min)*10.0 ; }

      if ((max == min) && (max == 0.0))
      {
        min = -1 ;
        max =  1 ;
      }
      o_tst_max[i] = max ;
      o_tst_min[i] = min ;

    }
  }

#ifdef USE_MMPI
  mpi_sync_maxmin();
#endif

  for (i=0; i<num_ovars; i++)
  {
    if ((o_tst_type[i] == H_OUT_AUTO)
        ||(o_tst_type[i] == H_OUT_AUTO_DYN)
        ||(o_tst_type[i] == H_OUT_AUTO_NUM)
        ||(o_tst_type[i] == H_OUT_AUTO_NUM_DYN)
        )
    {
      o_his[i].max = o_tst_max[i] ;
      o_his[i].min = o_tst_min[i] ;
      o_his[i].len = o_tst_len[i] ; 
      o_his[i].total = 0 ;

      /* allocate histogram data here! */
      if ((o_his[i].data = (long *)malloc(o_tst_len[i]*sizeof(long)))==NULL)
      {
        fprintf(msgout,"%s - %s: %s!\n",_("Error"),_("no memory to allocate result field for"),o_his[i].name);
        o_tst_type[i] = H_OUT_NONE ;
      }
      for (j=0; j<o_tst_len[i]; j++) {o_his[i].data[j] = 0;}
      
      /* finally we add the values to output histograms: */
      for (j=0; j<o_tst_sims-1; j++)
      {
        pos = o_tst_from[i] + j ;
        rv = put_val_to_histogram(&o_his[i],o_tst_vals[pos], 1);
      }
    }
  }

  /** some debug prints ----------------------------- **/
#ifdef DEVEL_VERBOSE
  for (i=0; i<num_ovars; i++)
  {
    fprintf(msgout,
      "[D] OH[%li] \"%s\": type=%i len=%i min=%f max=%f, total=%li\n",
        i,
        o_his[i].name,
        o_his[i].type,
        o_his[i].len,
        o_his[i].min,
        o_his[i].max,
        o_his[i].total
        );
  }
#endif
  /** ----------------------------------------------- **/

  /* o_tst_vals no longer needed... */
  free(o_tst_vals);
  o_tst_vals = NULL;
  return(0);
}

/** Adds output variables to output histograms */
int add_out_vars(double *rfld)
{
  long i ;
  long force ;
  long rv = 0 ;

  for (i=0; i<num_ovars; i++)
  {
    if ((o_tst_type[i] == H_OUT_LIMS_DYN)||(o_tst_type[i] == H_OUT_AUTO_NUM_DYN)||(o_tst_type[i] == H_OUT_AUTO_DYN))
    {
      force = 0 ;
      rv = put_val_to_histogram(&o_his[i], rfld[i], force);
  
      if (rv != 0)
      {
        enlarge_histogram(&o_his[i], rfld[i]) ;
        rv = put_val_to_histogram(&o_his[i], rfld[i], force);
      }
    }
    
    if ((o_tst_type[i] == H_OUT_LIMS)||(o_tst_type[i] == H_OUT_AUTO)||(o_tst_type[i] == H_OUT_AUTO_NUM))
    {
      force = 1 ;
      rv = put_val_to_histogram(&o_his[i], rfld[i], force);
    }
    
  }

  return(rv);
}

/*--------------------------- */
/* Result correlation coefficient operations: */
void null_cc(void)
{
  cc_x  = NULL ;
  cc_xy = NULL ;
}

void free_cc(void)
{
  free(cc_x)  ;
  free(cc_xy) ;

  null_cc();
}

int alloc_cc(long num)
{
  if ((cc_x  = femDblAlloc(num)) == NULL) {goto memFree;}
  if ((cc_xy = femDblAlloc(num*num)) == NULL) {goto memFree;}

  return(0);
memFree:
  free_cc();
  return(-1);
}

double get_cc_xx(long i, long j, long num)
{
  return( cc_xy[i*num + j]);
}

void put_cc_xx(long i, long j, long num, double val)
{
  cc_xy[i*num + j] = val ;
}

void add_cc_xx(long i, long j, long num, double val)
{
  cc_xy[i*num + j] += val ;
}



void fill_cc(long ilen, double *ifld, long olen, double *ofld)
{
  long i,j ;
  double ival, jval ;

  for (i=0; i<ilen; i++) { cc_x[i] += ifld[i] ; }
  for (i=0; i<olen; i++) { cc_x[i+ilen] += ofld[i] ; }

  for (i=0; i<(ilen+olen); i++)
  {
    if (i >= ilen) 
    {
      ival = ofld[i-ilen] ;
    }
    else
    {
      ival = ifld[i] ;
    }
    for (j=0; j<(ilen+olen); j++)
    {
      if (j >= ilen) 
      {
        jval = ofld[j-ilen]; 
      }
      else
      {
        jval = ifld[j] ;
      }
      
      add_cc_xx(i,j, ilen+olen, ival*jval) ;
    }
  }
}

int write_cc_matrix(FILE *fw, long inum, long onum, long sims, int use_header)
{
  long i, j, num;
  double i_av, j_av ;
  double xx, xy, yy ;
  double val ;

  num = inum + onum ;

  if (use_header == 1)
  {
    fprintf(fw,"%li %li\n", inum+onum, inum+onum);
  }

  for (i=0; i<num; i++)
  {
    i_av = cc_x[i] / (double)sims ;

    xx = get_cc_xx(i, i, num) ;
    xx -= (double)sims*i_av*i_av ;

    for (j=0; j<num; j++)
    {
      /* compute the stuff first! */
      j_av = cc_x[j] / (double)sims ;

      xy = get_cc_xx(i, j, num) ;
      yy = get_cc_xx(j, j, num) ;

      xy -= (double)sims*i_av*j_av ;
      yy -= (double)sims*j_av*j_av ;

      if ((xx*xy) == 0.0) 
      {
        val = 0 ;
      }
      else
      {
        val = xy*xy / (xx*yy) ;
      }

      fprintf(fw, " %e", sqrt(val));
    }
    fprintf(fw,"\n");
  }

  return(0);
}
/*--------------------------- */


/** Replaces "," with "." in str
 *  @param str string to be modified
 *  @param len lenght of "str"
 */
void replace_all_commas(char *str, long len)
{
  long j ;

  for (j=0; j<len; j++) { if (str[j]==',') {str[j]='.' ;} }
}

/** Finds space (or other separator) in string 
 * @param str string to be searched
 * @param len lenght of str
 * @param from start position in str
 * @param pos position of separator (result) - or end of string
 */
void test_space(char *str, long len, long from, long *pos)
{
  long j ;

  if ((from+1)>=len)
  {
    *pos = strlen(str)-1 ;
    return;
  }

  for (j=from+1; j<len; j++)
  {
    if ((str[j]==' ')||
        (str[j]=='\0')||
        (str[j]=='\t')||
        (str[j]=='\n')||
        (str[j]==';')||
        /* (str[j]==',')|| */
        (str[j]=='|')||
        (str[j]=='!')||
        (str[j]==':')||
        (str[j]=='?')
      )
    {
      *pos = j ;
      return ;
    }
  }

  *pos = strlen(str)-1 ;
}

/** Makes full path for filename from dir and fname
 * @param dir directory WITHOUT ending slash (example: "/tmp")
 * @param fname filename (example: "fantasmagoria.dis")
 * @return full filename with path (example: "/tmp/fantasmagoria.dis")
 */ 
char *fnamecat(char *dir, char *fname)
{
  int lend, lenf, len, i ;
  char *tmp = NULL ;

  if (dir != NULL) { lend = strlen(dir) ; }
  else { lend = 0 ; }

  for (i=0; i<lend; i++) 
  {
    if (dir[i] == '\n') {dir[i] = '\0';}
    if (dir[i] == '\t') {dir[i] = '\0';}
  }

  if (fname != NULL) { lenf = strlen(fname) ; }
  else 
  { 
    return(NULL) ; /* empty filename !!! */
  }

  for (i=0; i<lenf; i++) 
  {
    if (fname[i] == '\n') {fname[i] = '\0';}
    if (fname[i] == '\t') {fname[i] = '\0';}
  }

  if (lend <= 0) { len = 1 + lenf ; }
  else           { len = lend + 2 + lenf ; }

  if ((tmp = (char *)malloc(len*sizeof(char)))==NULL) { return(NULL) ; }
  for (i=0; i<len; i++) { tmp[i] = '\0' ; }

  if (lend > 0)
  {
    strcpy(tmp,dir);
    strcat(tmp,SLASH);
    strcat(tmp,fname);
  }
  else
  {
    strcpy(tmp,fname);
  }


#ifdef DEVEL_VERBOSE
  fprintf(msgout,"[D] Name is: _%s_\n", tmp);
#endif

  return(tmp) ;
}

/** Makes filename from template, fname and extension
 * @param dir template for name (example: "/tmp/mess-")
 * @param fname name of thing (example "out123")
 * @param ext extension (example "dis")
 * @return filename (example: "/tmp/mess-out123.dis")
 */ 
char *outnamecat(char *dir, char *fname, char *ext)
{
  int lend, lenf, lene, len, i ;
  char *tmp = NULL ;

  if (dir != NULL) { lend = strlen(dir) ; }
  else 
  { 
    return(NULL);
  }

  for (i=0; i<lend; i++) 
  {
    if (dir[i] == '\n') {dir[i] = '\0';}
    if (dir[i] == '\t') {dir[i] = '\0';}
  }

  if (fname != NULL) { lenf = strlen(fname) ; }
  else 
  { 
    return(NULL) ; /* empty filename !!! */
  }

  for (i=0; i<lenf; i++) 
  {
    if (fname[i] == '\n') {fname[i] = '\0';}
    if (fname[i] == '\t') {fname[i] = '\0';}
  }

  if (ext != NULL) { lene = strlen(ext) ; }
  else 
  { 
    return(NULL) ; /* empty extension !!! */
  }

  for (i=0; i<lene; i++) 
  {
    if (ext[i] == '\n') {ext[i] = '\0';}
    if (ext[i] == '\t') {ext[i] = '\0';}
  }

  if (lend < 1) { return(NULL); }
  if (lenf < 1) { return(NULL); }
  if (lene < 1) { return(NULL); }

  len = lend + lenf + 2 + lene ;


  if ((tmp = (char *)malloc(len*sizeof(char)))==NULL) { return(NULL) ; }
  for (i=0; i<len; i++) { tmp[i] = '\0' ; }

  if (lend > 0)
  {
    strcpy(tmp,dir);
    strcat(tmp,fname);
    strcat(tmp,".");
    strcat(tmp,ext);
  }
  else
  {
    strcpy(tmp,fname);
  }


#ifdef DEVEL_VERBOSE
  fprintf(msgout,"[D] Name is: _%s_\n", tmp);
#endif

  return(tmp) ;
}

void strclean(char *str)
{
  int i ;

  if (str == NULL) {return;}
  if (strlen(str) < 1) {return;}

  for (i=0; i<strlen(str); i++)
  {
    switch (str[i])
    {
      case ' ' : str[i] = '\0'; break;
      case '\t': str[i] = '\0'; break;
      case '\n': str[i] = '\0'; break;
      case ';' : str[i] = '\0'; break;
    }
  }
}

/** Reads input data from file
 * @param fr pointer to input file
 */
int read_simple_input(FILE *fr)
{
  long i,j, posa, posb, val ;
  long num_ivars_test = -1 ;
  char line[STR_LENGHT*4+1] ;
  char name[2*STR_LENGHT+1] ;
  char hfile[2*STR_LENGHT+1] ;
  char tmp[2*STR_LENGHT+1] ;
  FILE *hfr = NULL ;

  fscanf(fr, "%li", &num_ivars_test) ;

  if (num_ivars == 0)
  {
    num_ivars = num_ivars_test ; /* when no function is given */
  }

  if (num_ivars_test < num_ivars)
  {
    fprintf(msgout,"%s - %s!\n",_("Error"), _("tool low number of variables"));
    return(-1);
  }

  /* when no library is specified: */
  if (res_solver == SOL_COPY) { num_ovars = num_ivars ; }

  /* allocate space for data: */
  if (alloc_input_flds(num_ivars) != 0) {return(-1);}

  fgets(line, STR_LENGHT*4, fr); /* to bypass the first line... */

  for (i=0; i<num_ivars; i++)
  {
    for (j=0; j<4*STR_LENGHT+1; j++) { line[j]='\0'; }

    for (j=0; j<2*STR_LENGHT+1; j++) 
    { 
      name[j]='\0'; 
      hfile[j]='\0';
    }

    /* read whole line */
    fgets(line, STR_LENGHT*4, fr);
    
    /* change ALL "," to "." (correct numbers) */
    replace_all_commas(line, strlen(line));

    posa = 0 ;
    posb = 0 ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout, "[D] line[%li] is: %s",i, line);
#endif

    /* read variable name */
    test_space(line, strlen(line), posa, &posb) ;
    for (j=posa; j<=posb; j++) { name[j-posa] = line[j] ; }
    name[posb+1] = '\0' ;

    strclean(name);
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[D] variable[%li] = (%s) [posa=%li, posb=%li]\n", i, name, posa, posb);
#endif

    posa = posb ;

    /* find constant */
    test_space(line, strlen(line), posa, &posb) ;
    for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
    for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
    tmp[posb+1] = '\0' ;
    multhis[i] = atof(tmp) ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[D] multhis[%li] = %e (%s) [pos %li,%li]\n",i,multhis[i],tmp,posa,posb);
#endif

    posa = posb ;

    /* find type */
    test_space(line, strlen(line), posa, &posb) ;
    for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
    for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
    tmp[posb+1] = '\0' ;
    itype[i] = atof(tmp) ;

    posa = posb ;

		deppos[i] = -1 ;
    
    switch (itype[i])
    {
      case DIS_UNKNOWN:
        break ;
      case DIS_HISTOGR: /* read histogram name */
        test_space(line, strlen(line), posa, &posb) ;
        for (j=posa; j<posb; j++) { hfile[j-posa] = line[j+1] ; }
        hfile[posb+1] = '\0' ;

        /* read histogram: */
        if (fnamecat(hisdir, hfile) == NULL) 
        { 
          fprintf(msgout,"Error - fnamecat failed!\n");
          goto memFree ; 
        }
        strclean(hfile);
        if ((hfr=fopen(fnamecat(hisdir, hfile),"r"))==NULL) 
        { 
          fprintf(msgout,"Error - cannot open histogram file: %s (\"%s\")\n",hfile,fnamecat(hisdir, hfile));
          goto memFree ; 
        }
        if ( read_dis(hfr, name, &histogram[i]) != 0)
        {
          fprintf(msgout,"Error - cannot read histogram: %s!\n",name);
          goto memFree ;
        }
				histogram[i].correlated = 0 ;
        if ( compute_pf(&histogram[i], &distfunc[i]) != 0)
        {
          goto memFree ;
        }
        break ;
      case DIS_CONSTANT:
        /* nothing to do */
        break ;
      case DIS_COPY:
        /* var. index (0,1,..) is inside multhis[i] */
        val = (long)(multhis[i]) ;
        if ((val >= i)||(val < 0)) 
        { 
          val = i ; 
          fprintf(msgout,"Error - index of dependence is incorrect!\n");
          goto memFree ;
        }
        break;

      case DIS_DEPEND:
        test_space(line, strlen(line), posa, &posb) ;
 				for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
    		for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
    		tmp[posb+1] = '\0' ;
    		deppos[i] = atoi(tmp) ;

        /* var. index (0,1,..) is inside deppos[i] */
        val = deppos[i] ;
        if ((val >= i)||(val < 0)) 
        { 
          val = -1 ; 
          fprintf(msgout,"[D] Index of f-dependence is incorrect!\n");
          goto memFree ;
        }
        break;

      /* to be continued for other types.. */
    }

  }
  
  return(0);

memFree:
  /* TODO - on error.. */
  free_input_flds();
  return(-1);
}

/** Reads description of output histograms
 * @param fr pointer to input file
 * @return status
 */ 
int read_simple_input_out_hists(FILE *fr)
{
  long out_h_len = 0 ;
  long out_vals_from = 0 ;
  char line[STR_LENGHT*4+1] ;
  char name[2*STR_LENGHT+1] ;
  char tmp[2*STR_LENGHT+1] ;
  long i, j, posa, posb ;
  double  sw ;

  null_output_flds();

  fgets(line, STR_LENGHT*4, fr); /* to bypass the first line... */
  out_h_len = atoi(line);
  if (out_h_len < 0) {goto memFree;}

  if (alloc_output_flds(num_ovars) != 0) {goto memFree;}

  if (out_h_len > num_ovars) /* to be sure */
  {
    out_h_len = num_ovars ;
  }

  for (i=0; i<out_h_len; i++)
  {
    for (j=0; j<4*STR_LENGHT+1; j++) { line[j]='\0'; }
    for (j=0; j<2*STR_LENGHT+1; j++) { name[j]='\0'; }

    /* read whole line */
    fgets(line, STR_LENGHT*4, fr);
  
    /* change ALL "," to "." (correct numbers) */
    replace_all_commas(line, strlen(line));

    posa = 0 ;
    posb = 0 ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout, "[D] OUT line[%li] is: %s",i, line);
#endif

    /* read variable name */
    test_space(line, strlen(line), posa, &posb) ;
    for (j=posa; j<=posb; j++) { name[j-posa] = line[j] ; }
    name[posb] = '\0' ;
    name[posb+1] = '\0' ;
    strclean(name);

    if ((o_his[i].name = (char *)malloc(sizeof(char)*(strlen(name)+1)))==NULL)
    { goto memFree ; }
    else
    {
      for (j=0; j<(strlen(name)+1); j++) { o_his[i].name[j] = '\0'; }
      strcpy(o_his[i].name, name);
    }

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[D] OUT variable[%li] = (%s) [posa=%li, posb=%li]\n", i, o_his[i].name, posa, posb);
#endif

    posa = posb ;

    /* find type */
    test_space(line, strlen(line), posa, &posb) ;
    for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
    for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
    tmp[posb+1] = '\0' ;
    o_tst_type[i] = atof(tmp) ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[D] OUT type[%li] = (%li)\n", i, o_tst_type[i]);
#endif

    posa = posb ;

    switch(o_tst_type[i])
    {
      case H_OUT_NONE : break; /* nothing to do */
      case H_OUT_AUTO : 
      case H_OUT_AUTO_DYN : 
                        o_tst_from[i] = out_vals_from ;
                        out_vals_needed += o_tst_sims ;
                        out_vals_from = out_vals_needed ;

                        o_tst_len[i] = sqrt((double)sim_number);
                        break; 
      case H_OUT_AUTO_NUM : 
      case H_OUT_AUTO_NUM_DYN : 
                        o_tst_from[i] = out_vals_from ;
                        out_vals_needed += o_tst_sims ;
                        out_vals_from = out_vals_needed ;

                /* find len */
                test_space(line, strlen(line), posa, &posb) ;
                for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
                for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
                tmp[posb+1] = '\0' ;
                o_tst_len[i] = atof(tmp) ;
#ifdef DEVEL_VERBOSE
                fprintf(msgout,"[D] len[%li] = %li\n",i,o_tst_len[i]);
#endif
                        break; 
      case H_OUT_LIMS : 
      case H_OUT_LIMS_DYN : 

            /* find len */
            test_space(line, strlen(line), posa, &posb) ;
            for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
            for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
            tmp[posb+1] = '\0' ;
            o_tst_len[i] = atof(tmp) ;
#ifdef DEVEL_VERBOSE
                fprintf(msgout,"[D] len[%li] = %li\n",i,o_tst_len[i]);
#endif
            posa = posb ;


            /* find min */
            test_space(line, strlen(line), posa, &posb) ;
            for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
            for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
            tmp[posb+1] = '\0' ;
            o_tst_min[i] = atof(tmp) ;
#ifdef DEVEL_VERBOSE
            fprintf(msgout,"[D] min[%li] = %e\n",i,o_tst_min[i]);
#endif

            posa = posb ;

            /* find max */
            test_space(line, strlen(line), posa, &posb) ;
            for (j=0; j<2*STR_LENGHT+1; j++) { tmp[j]='\0'; }
            for (j=posa; j<=posb; j++) { tmp[j-posa] = line[j] ; }
            tmp[posb+1] = '\0' ;
            o_tst_max[i] = atof(tmp) ;
#ifdef DEVEL_VERBOSE
            fprintf(msgout,"[D] max[%li] = %e\n",i,o_tst_max[i]);
#endif

            posa = posb ;
                       
            if (o_tst_len[i] < 1) {o_tst_len[i] = 1 ;}
              
            /* incorrect order of limits: */
            if (o_tst_min[i] > o_tst_max[i])
            {
              sw = o_tst_max[i] ;
              o_tst_max[i] = o_tst_min[i] ;
              o_tst_min[i] = sw ;
            }

            /* faulty limits: */
            if (o_tst_min[i] == o_tst_max[i])
            {
              o_tst_from[i] = out_vals_from ;
              out_vals_needed += o_tst_sims ;
              out_vals_from = out_vals_needed ;

              if (o_tst_type[i] == H_OUT_LIMS_DYN)
                { o_tst_type[i] = H_OUT_AUTO_DYN ; }
              else
                { o_tst_type[i] = H_OUT_AUTO ; }
            }

            o_his[i].min =  o_tst_min[i] ;
            o_his[i].max =  o_tst_max[i] ;
            o_his[i].len =  o_tst_len[i] ;

            /* allocate histogram data here! */
            if ((o_his[i].data = (long *)malloc(o_tst_len[i]*sizeof(long)))==NULL)
            {
              fprintf(msgout,"%s - %s!\n",_("Error"),_("no memory to allocate result field"));
              goto memFree;
            }
            for (j=0; j<o_tst_len[i]; j++) { o_his[i].data[j] = 0; }
                        break ;
      default:
                        o_tst_type[i] = H_OUT_NONE ; /* unknown -> unused */
                        break ;
    }
  }

  if (out_vals_needed > 0)
  {
    if (alloc_out_vals_fld(out_vals_needed) != 0)
    {
      fprintf(msgout,"%s - %s!\n",_("Error"),_("no memory to allocate result field"));
      goto memFree;
    }
  }

  if (o_his_name == NULL)
  {
    /* no such stuff needed by user! */
    free_output_flds();
  }

  return(0);

memFree:
  free_input_flds();
  free_output_flds();
  return(-1);
}

int read_simple_input_correl(FILE *fr)
{
  long num_vals = 0 ;
  double val ;
  long i, j;

  null_corr_flds();

  /* no fget possible after this point !!! */
  fscanf(fr, "%li", &num_vals);

  if (num_vals <= 0) /* nothing to correlate*/
  {
    corr_size = 0 ;
    return(0);
  }
  else
  {
    if (alloc_corr_flds(num_vals) != 0)
    {
      fprintf(msgout,"%s - %s!\n",
          _("Error"),
          _("unable to allocate correlation space")
          );
      goto memFree;
    }
  }

  /* read variable indexes, numbered form 0 */
  for (i=0; i<num_vals; i++)
  {
    fscanf(fr, "%li", &corr_desc[i]) ;

		if ((corr_desc[i] < num_ivars) && (corr_desc[i] >= 0))
		{
			histogram[i].correlated = 1 ;
		}
  }

  /* read matrix: */
  for (i=1; i<=num_vals; i++)
  {
    for (j=1; j<=num_vals; j++)
    {
      fscanf(fr, "%lf", &val);
      femMatPut(&corr_mat, i, j, val);
    }
  }

  return(0);
memFree:
  free_input_flds();
  free_output_flds();
  free_corr_flds();
  return(-1);
}

/* ********************************************************** */


/** saves output histogram to files
 * */
int save_o_his(void)
{
  long i ;

  if (o_his_name == NULL) {return(0);} /* no job for us */
  if (o_his == NULL) {return(0);} /* no job for us */

#ifdef USE_MMPI
  if (mpi_sync_maxminlen_final() != 0) {return(-1);} /* sync o_h stuff */

  if (ppRank == 0) {
#endif

  for (i=0; i<num_ovars; i++)
  {
    if (o_tst_type[i] == H_OUT_NONE) {continue;} /* nothing to save */
    
    if (o_his[i].name == NULL) {continue;} /* invalid name */
    if (strlen(o_his[i].name) < 1) {continue;} /* too short name */

    if (outnamecat(o_his_name,o_his[i].name,"dis") == NULL)
    {
      /* something is wrong! */
      return(-1);
    }
    else
    {
      if (write_histogram(&o_his[i], outnamecat(o_his_name,o_his[i].name,"dis")) != 0)
      {
        return(-2);
      }
#if 0
      if (write_histogram_for_gnuplot(&o_his[i], outnamecat(o_his_name,o_his[i].name,"tds")) != 0)
      {
        return(-2);
      }
#endif
    }
  }
#ifdef USE_MMPI
  }
#endif
  return(0);
}

/* end of finput.c */
