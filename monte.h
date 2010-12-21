/*
   File name: monte.h
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


#ifndef __MONTE_H__
#define __MONTE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

#include "fem_mem.h"
#include "fem_math.h"

#ifdef USE_SPRNG
#define SIMPLE_SPRNG
#include <sprng.h>
#endif

#ifdef USE_LSHARED
#ifndef USE_WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif
#endif

#ifdef USE_MMPI
#include <mpi.h>
#endif

#define PI 3.1415926535897932384626433832795029L
#define   SLASH "/"

#define M_ROUND 0 /* rounding style */

#define RAND_BIG     1000000000
#define RAND_SEED    161803398
#define RAND_ZERO    0
#define RAND_FAC     (1.0/RAND_BIG)

#define STR_LENGHT   120

#define H_UNKNOWN   -1
#define H_DISCRETE   0
#define H_CONTINUOUS 1

#define DIS_UNKNOWN  -1
#define DIS_CONSTANT  0
#define DIS_HISTOGR   1
#define DIS_COPY      2
/* to be continued... */

#define H_OUT_NONE         0
#define H_OUT_AUTO         1
#define H_OUT_AUTO_DYN     2
#define H_OUT_AUTO_NUM     3
#define H_OUT_AUTO_NUM_DYN 4
#define H_OUT_LIMS         5
#define H_OUT_LIMS_DYN     6

#define SOL_COPY 1
#define SOL_LDL1 2
#define SOL_LDL2 3
#define SOL_EINP 4
/* to be continued */

/* Graphics: */
#define MDTERM_PS  1
#define MDTERM_GD  2
#define MDTERM_TK  3

typedef struct {
  char   *name ;
  int     type ;
  int     len ;
  double  min ;
  double  max ;
  long    total ;
	long    correlated;
  long   *data ;
} tHis ;

typedef struct {
  char   *name ;
  int     type ;
  int     len ;
  long    dlen ;
  double *value ; /* [len] */
  double *probd ; /* [len] density*/
  double *distr ; /* [dlen] */
} tDF ;


/* Variables: */
extern FILE *msgout;

extern int rand_gen_type ;  /* random generator type */
extern long    sim_number ; /* number of simulation */
extern long    num_ivars  ; /* number of input variables */
extern long    num_ovars  ; /* number of output variables */
extern long    fail_num   ;    /* used for computation of probability of failure */
extern long    ffunc_pos  ; /* position of failure function (if any) */
extern long   *itype      ; /* input data types */
extern tHis   *histogram  ; /* used histograms */
extern double *multhis    ; /* histogram multiplier */
extern tDF    *distfunc   ; /* distribution functions */
extern char   *inputfile  ; /* input file name */
extern char   *workdir    ; /* working directory */
extern char   *hisdir ; /* directory with histograms */
extern char   *ilogfile   ; /* file with realizations of variables */
extern char   *ologfile   ; /* file with simulation results */
extern char   *ocrmfile   ; /* file for output correlation matrix */
extern char   *irsfile    ; /* to read external simulation results */
extern char   *fpffile    ; /* file for reliability of failure data */
extern long    o_tst_sims ; /* number of simulations to adjust output histograms */
extern tHis   *o_his      ; /* output histograms */
extern char   *o_his_name ; /* template for output histogram file names */

extern long    corr_size ; /* size of correlation matrix */
extern long   *corr_desc ; /* indexes of variables that have to be correlated */
extern tMatrix corr_mat  ; /* correlation matrix (user-supplied) */
extern tMatrix corr_tran ; /* transformational matrix for correlation */
extern tMatrix corr_s    ; /* standard deviations */
extern tVector corr_u    ; /* mean values */
extern tVector corr_X    ; /* correlated data */
extern tVector corr_Y    ; /* uncorrelated (input) */
extern double *cc_x;
extern double *cc_xy;

extern long    inp_stdin  ;
extern long    res_stdout       ;
extern long    res_statistics   ;
extern long    res_realizations ;
extern long    res_rea_s_input  ;
extern long    res_rea_header   ;
extern long    res_rea_number   ;
extern long    res_solver       ;
extern double  day_run_limit    ;
extern long    verbose_mode     ;
extern long    verbose_nums     ;
extern long    use_cc_simple    ;
extern long    use_cc_header    ;
extern long    block_rand_init  ;

extern long    rand_init ;

/* Convergence tests: */
extern double   ct_c     ;
extern double   ct_t     ;
extern double   ct_pd    ;
extern int      ct_nmin  ;

#ifdef USE_LSHARED
/* dynamic library stuff */
extern char   *dllib;
extern void   *dlarg;
#ifndef USE_WIN32
extern void   *dlfile;
long (*monte_dlib_interface_type)(void) ;
void (*monte_nums_of_vars)(long *, long *, long *);
int (*monte_solution)(double *, double *);

int (*monte_init_lib_stuff2)(char *);
int (*monte_clean_lib_stuff2)(char *);
void (*monte_nums_of_vars2)(char *, long *, long *, long *);
int (*monte_solution2)(char *,double *, double *);
#else
extern HANDLE dlfile; 
#define random rand
#if 0
#define EXPORT __declspec(dllimport)
EXPORT int monte_solution(char *, double *, double *);
EXPORT int monte_solution2(char *, double *, double *);
EXPORT long monte_dlib_interface_type(void);
EXPORT void monte_nums_of_vars(long *, long *, long *);
EXPORT void monte_nums_of_vars2(long *, long *, long *);
EXPORT int monte_init_lib_stuff2(char *);
EXPORT int monte_clean_lib_stuff2(char *);
#endif

typedef void (*pfunc)(); 
typedef int (*ipfunc)(); 
typedef long (*lpfunc)(); 

lpfunc monte_dlib_interface_type ;
ipfunc monte_solution ;
pfunc monte_nums_of_vars ;
ipfunc monte_solution2 ;
pfunc monte_nums_of_vars2 ;
ipfunc monte_init_lib_stuff2 ;
ipfunc monte_clean_lib_stuff2 ;
#endif
#endif

#ifdef USE_MMPI
extern int ppRank ;
extern int ppSize ;
extern long ppSims ;
#endif

/* functions: */
extern int parse_command_line(int argc, char *argv[]);

extern double get_rand(long *inpl);
extern int init_knuth(void);

extern void null_histogram(tHis *his);
extern void free_histogram(tHis *his);
extern void null_distrib(tDF *dis);
extern void free_distrib(tDF *dis);

extern int read_dis(FILE *fr, char *name, tHis *histogram);
extern int compute_pf(tHis *histogram, tDF *distr);
extern double one_sim(tDF *distr, double rand);

extern int enlarge_histogram(tHis *his, double newval);
extern int put_val_to_histogram(tHis *his, double val, int force);
extern int write_histogram(tHis *his, char *fname);
extern int write_histogram_for_gnuplot(tHis *his, char *fname);
extern int add_out_setup_vars(long sim_number, double *rfld);
extern int add_out_vars(double *rfld);
extern int setup_out_histograms(void);
extern void free_input_flds(void);
extern void free_output_flds(void);

extern char *fnamecat(char *dir, char *fname);
extern char *outnamecat(char *dir, char *fname, char *ext);
extern int read_simple_input(FILE *fr);
extern int read_simple_input_out_hists(FILE *fr);
extern int read_simple_input_correl(FILE *fr);

extern int monte_MC(FILE *fstat, FILE *fsim, FILE *fcrm);

extern int save_o_his(void);

#ifdef PSGUI
extern void set_ps_output_file(FILE *fw);
extern void get_draw_size_ps(int *x0,int *y0,int *width,int *height);
extern void mdps_draw_point(int x,int  y);
extern void mdps_draw_line(int x1,int  y1,int  x2,int  y2,int  width);
extern void mdps_draw_string(int x,int  y, char *str);
#endif

#ifdef USE_MMPI
extern void mpi_free_rstats(void);
extern int mpi_alloc_rstats(long num_ovars);
extern int mpi_sync_maxmin(void);
extern int mpi_sync_maxminlen_final(void);

extern int mpi_send_input_data(void);
extern int mpi_send_input_out_h_data(void);
#endif

extern long monte_round(double val, int type); /* rint replacement */


extern int monte_eigen_jacobi(tMatrix *a, tVector *d, tMatrix *v, long *nrot);
extern int femMatCholFact(tMatrix *a);
extern void null_corr_flds(void);
extern void free_corr_flds(void);
extern int alloc_corr_flds(long len);
extern int do_correlation(tVector *Y, tMatrix *T, tVector *X, long *corr_desc);
extern int pack_val_corr(long num_corr, long *corr, long num_ifld, double *ifld, tVector *Y);
extern int unpack_val_corr( tVector *X, long num_corr, long *corr, long num_ifld, double *ifld);
extern int compute_corr_tran_mat(tMatrix *cmat, tMatrix *tran);
extern double monte_errf(double x);
extern double monte_normal_df(double x1, double x2);

extern void free_cc(void);
extern int alloc_cc(long num);
extern void fill_cc(long ilen, double *ifld, long olen, double *ofld);
extern int write_cc_matrix(FILE *fw, long inum, long onum, long sims, int use_header);

#endif

/* end of monte.h */
