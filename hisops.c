/*
   File name: hisops.c
   Date:      2005/11/24 17:47
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

   Reads *.dis histograms (from AntHill etc.) and produces probability function.

   Based on code written by Jakub Valihrach, Pavel Praks, Petr Konecny
*/

#include "monte.h"

/* for systems with incorrect math.h files (Linux etc ) */
#ifndef REPLACE_RINTL
long double rintl(long double x);
#endif

#define isnum(c) (((c) >= '0' && (c) <= '9') || (c) == '.')

void null_histogram(tHis *his)
{
  his->name  = NULL ;
  his->type  = 0 ;
  his->len   = 0 ;
  his->min   = 0 ;
  his->max   = 0 ;
  his->total = 0 ;
	his->correlated = 0 ;
  his->data  = NULL ;
}

void free_histogram(tHis *his)
{
  free(his->name)  ;
  free(his->data) ;
  null_histogram(his) ;
}

void null_distrib(tDF *dis)
{
  dis->name  = NULL ;
  dis->type  = 0 ;
  dis->len   = 0 ;
  dis->dlen  = 0 ;
  dis->value = NULL ;
  dis->probd = NULL ;
  dis->distr = NULL ;
}

void free_distrib(tDF *dis)
{
  free(dis->name)  ;
  free(dis->value) ;
  free(dis->probd) ;
  free(dis->distr) ;
  null_distrib(dis) ;
}


/** Reads histogram description from file
 * @param fr stream pointer
 * @param histogram pointer to histogram
 * @return status
 */ 
int read_dis(FILE *fr, char *name, tHis *histogram)
{
  int bins, total, i ;
  double min, max ;
  char line[STR_LENGHT+1];
  char bigline[STR_LENGHT*5+1];
  int  distrib = H_UNKNOWN ;

  /* Reading of variables */
  fgets(line, STR_LENGHT, fr);             /* [Description] */
  fgets(bigline, STR_LENGHT*5, fr);        /* Identification... (some idiots use very long text here) */
  fgets(line, STR_LENGHT, fr);             /* [Type... => distrib */
  
  if (strstr(line, "iscrete") != NULL)
  {
     distrib = H_DISCRETE;    
  }
  else 
  {
    if (strstr(line, "ontinuous") != NULL)
    {
      distrib = H_CONTINUOUS;
    }
    else 
    {
      printf("Error! Unknown distribution type.\n");
      return(-1);
    }
  }
  
  fgets(line, STR_LENGHT, fr);             /* Form... */
  fgets(line, STR_LENGHT, fr);             /* empty line */
  fgets(line, STR_LENGHT, fr);             /* [Parameters] */
  fgets(line, STR_LENGHT, fr);             /* [Min... => min */
  i = 0;
  do
  {
    i++;
  } while((!isnum(line[i]))&&(line[i]!='-'));

  min = atof(&line[i]);

  fgets(line, STR_LENGHT, fr);             /* [Max... => max */
  i = 0;
  do
  {
    i++;
  } while((!isnum(line[i]))&&(line[i]!='-'));

  max = atof(&line[i]);
  
  fgets(line, STR_LENGHT, fr);             /* [Bins... => bins */
  i = 0;
  do
  {
    i++;
  } while(!isnum(line[i]));

  bins = atol(&line[i]);
  
  fgets(line, STR_LENGHT, fr);             /* [Total... => total */
  i = 0;
  do
  {
    i++;
  } while(!isnum(line[i]));

  total = atol(&line[i]);
  
  fgets(line, STR_LENGHT, fr);             /* empty line */
  fgets(line, STR_LENGHT, fr);             /* [Bins] */

  /* Allocate histogram data */
  if (bins <= 0)
  {
    return(-1) ;
  }
  else
  {
    if ((histogram->data = (long *)malloc(bins*sizeof(long)))==NULL)
    {
      return(-1) ;
    }
    else
    {
      histogram->type   = distrib ;
      histogram->len    = bins ;
      histogram->min    = min ;
      histogram->max    = max ;
      histogram->total  = total ;
    }
  }

  /* Reading of values: */
  for (i=0; i<bins; i++)
  {
    fgets(line, STR_LENGHT, fr);
    histogram->data[i] = atoi(line);
  }

  /* Setting of name */

  if ((bins=strlen(name)) > 0)
  {
    if((histogram->name = (char *)malloc((1+bins)*sizeof(char))) != NULL)
    {
      for (i=0; i<bins+1; i++) { histogram->name[i] = '\0' ; }
      strcpy(histogram->name, name) ;
    }
  }

  return(0) ;
}

/** Creates distribution function from histogram
 * @param histogram pointer to histogram
 * @distr pointer to empty distr
 * @return status
 */ 
int compute_pf(tHis *histogram, tDF *distr)
{
  double d_step ;
  long   i, j, k ;
#ifndef USE_MMPI
  long len;
#endif

  /* allocate distribution function data: */
  distr->len = histogram->len ;
  distr->dlen = histogram->total ;


  if ((distr->value=(double *)malloc(histogram->len*sizeof(double)))==NULL)
  {
    return(-1);
  }

  if ((distr->probd=(double *)malloc(histogram->len*sizeof(double)))==NULL)
  {
    free(distr->probd) ;
    distr->probd = NULL ;
    return(-1);
  }

  if ((distr->distr=(double *)malloc(histogram->total*sizeof(double)))==NULL)
  {
    free(distr->value) ;
    distr->value = NULL ;
    free(distr->probd) ;
    distr->probd = NULL ;
    return(-1);
  }

  if (histogram->type == H_CONTINUOUS)
  {
    d_step = (double)(histogram->max-histogram->min)/(double)(histogram->len); ;
    for (i=0; i<histogram->len; i++)
    {
      distr->value[i]= (double)histogram->min + ((double)i+0.5)*d_step;
    }
  }
  else
  {
    d_step = (double)(histogram->max - histogram->min)/(double)(histogram->len-1); ;
    for (i=0; i<histogram->len; i++)
    {
      distr->value[i]= (double)histogram->min + ((double)i)*d_step;
    }
  }

  k = 0 ;
  for (i=0; i<histogram->len; i++)
  {
    if (histogram->data[i] > 0)
    {
      for(j=0; j<histogram->data[i]; j++)
      {
        distr->distr[k] = distr->value[i] ;
        k++ ;
      }
    }
  }

  for (i=0; i<histogram->len; i++)
  {
    distr->probd[i] = histogram->data[i] / (double) histogram->total ;
  }

#ifndef USE_MMPI
  if ((len=strlen(histogram->name)) > 0)
  {
    if((distr->name = (char *)malloc((1+len)*sizeof(char))) != NULL)
    {
      for (i=0; i<len+1; i++) { distr->name[i] = '\0' ; }
      strcpy(distr->name, histogram->name) ;
    }
  }
#endif

  distr->type = histogram->type ;

  return(0) ;
}

/** Computes one realization of random value from histogram nad get_rand()
 * @distr pointer to empty distr
 * @distr rand_int  initialization number for Knuth's generator
 * @return realisation of rand. val. of course
 */ 
double one_sim(tDF *distr, double rand)
{
  double rval = 0 ;
  double order_r, len, step ;
  long   pos ;
	double s ;
	long is ;

  len = (double)distr->dlen ;
  step = distr->value[1] - distr->value[0] ;

  switch (distr->type)
  {
    case H_CONTINUOUS:
			is = (long)((rand*len))  ;
			s = distr->distr[is] - 0.5*step ;
			order_r = rand * (double)len - (double)is ;
			rval = s  + order_r*step ;
      break ;

    case H_DISCRETE: 
#ifdef REPLACE_RINTL
      pos = monte_round((rand*(len)-0.5), M_ROUND) ;
#else
      pos = (long)rintl(rand*(len)-0.5) ;
#endif
      if (pos >= distr->dlen) {pos = distr->dlen - 1 ;}
      if (pos <= 0) {pos = 0 ;}
      rval = distr->distr[ pos ] ;
      break ;
  }

  return(rval) ;
}

/** Allocates an empty histrogram structure 
 * @param his pointer to an empty histrogram structure
 * @param name name of the histogram
 * @param type type (use 0 for discrete)
 * @param len number of intervals ("bins")
 * @param min minimal allowed value
 * @param max maximal allowed value
 * @return status
 */ 
int alloc_empty_histogram(tHis *his, char *name, int type, int len, double min, double max)
{
  int nlen, i ;

  null_histogram(his);

  if (name == NULL)
  {
    if (verbose_mode == 1)
    { fprintf(msgout,"%s - %s!\n",_("Error"),_("unnamed histogram requested")); }
    return(-1);
  }
  
  if ((nlen=strlen(name)) < 1)
  {
    if (verbose_mode == 1)
    { fprintf(msgout,"%s - %s!\n",_("Error"),_("unnamed histogram requested")); }
    return(-1);
  }

  if ((len <= 0) ||(max <= min))
  {
    if (verbose_mode == 1)
    { fprintf(msgout,"%s - %s: %s!\n",_("Error"),_("invalid histogram requested"), name); }
    return(-1);
  }

  if ((his->data = (long *)malloc(len*sizeof(long))) == NULL)
  {
    if (verbose_mode == 1)
    { fprintf(msgout,"%s - %s: %s!\n",_("Error"),_("out of memory for histogram"),name); }
    return(-1);
  }
  for (i=0; i<len; i++) {his->data[i]=0;}
  
  if ((his->name = (char *) malloc((nlen+1)*sizeof(char))) == NULL)
  {
    if (verbose_mode == 1)
    { fprintf(msgout,"%s - %s: %s!\n",_("Error"),_("out of memory for histogram name"),name); }
    null_histogram(his);
    return(-1);
  }
  for (i=0; i<=nlen; i++) {his->name[i]='\0';}
  strcpy(his->name, name);

  his->type = type ;
  his->max = max ;
  his->min = min ;
  his->total = 0 ;
  
  return(0);
}

/** TODO Allocates an empty histrogram structure 
 * @param newval - nev value that is outside histogram
 * @return status
 */ 
int enlarge_histogram(tHis *his, double newval)
{
  long nlen, dlen, i ;
  double gap ;
  long *tmp = NULL ;

  if ((newval >= his->min) && (newval <= his->max)) { return(0); } /* nothing to do */
  if (his->len <= 0) { return(-1); }


  gap = (his->max - his->min) / ((double)his->len) ;


  if (newval < his->min)
  {
    /* smaller */
    dlen = (long)(fabs( his->min - newval) / gap) + 1 ;
    nlen = his->len + dlen ;

    if ((tmp = (long *)malloc((nlen)*sizeof(long))) == NULL)
    {
      if (verbose_mode == 1)
      { fprintf(msgout,"%s - %s: %s!\n", _("Error"),_(" no memory for enlarged histogram"), his->name); }
      return(-1);
    }
    for (i=0; i<nlen; i++) {tmp[i] = 0 ;}

    for (i=0; i<his->len; i++)
    {
      tmp[i+dlen] = his->data[i] ;
    }
    free(his->data);
    his->data = tmp;  tmp=NULL;

    his->min -= gap*dlen ;
  }
  else
  {
    /* higher */
    dlen = (long)(fabs(newval - his->max) / gap) + 1 ;
    nlen = his->len + dlen ;
    if ((tmp = (long *)malloc(nlen*sizeof(long))) == NULL)
    {
      if (verbose_mode == 1)
      { fprintf(msgout,"%s - %s: %s!\n", _("Error"),_(" no memory for enlarged histogram"), his->name); }
      return(-1);
    }
    for (i=0; i<nlen; i++) {tmp[i] = 0 ;}

    for (i=0; i<his->len; i++) 
    { 
      tmp[i] = his->data[i] ;
    }

    free(his->data);
    his->data = tmp;  tmp=NULL;

    his->max += gap*dlen ;
  }

  his->len = nlen ;

  return(0);
}

/** Adds value to the histogram
 * @param his histogram
 * @param val value to be inserted
 * @param force - if != 0 then outside value is added to the boundary interval
 * @return 0 if OK, -1 if value is too small, 1 if value is too big
 */ 
int put_val_to_histogram(tHis *his, double val, int force)
{
  long i ;
  double gap ;

  /* testing is value is outside */
  if (val > his->max) 
  {
    if (force != 0)
    {
      his->data[his->len-1]++ ;
      his->total++;
      return(0);
    }
    else
    {
      return(1);
    }
  }
  if (val < his->min) 
  {
    if (force != 0)
    {
      his->data[0]++ ;
      his->total++;
      return(0);
    }
    else
    {
      return(-1);
    }
  }

  /* interval ("bin") size: */
  gap = (his->max - his->min) / ((double)his->len) ;

	i = (long)( monte_round (0.5 + ((val-his->min) / gap), M_ROUND)) - 1 ;

	if (i < 0) {i = 0;}
	if (i >= his->len) {i = his->len -1;}

	his->data[i]++ ;
	his->total++ ;

  return(0);
}

/** Write histogram to file
 * @param his histogram
 * @param fname name of output file
 * @return status
 */
int write_histogram(tHis *his, char *fname)
{
  FILE *fw=NULL ;
  int i ;

  if (his->type == H_UNKNOWN)
  {
    if (verbose_mode == 1)
    { fprintf(msgout,"%s %s:%s!\n", _("Error"),_("unknown type of histogram"),his->name); }
    return(-1);
  }

  if (fname == NULL) {return(-1);}

  if ((fw=fopen(fname,"w")) == NULL)
  {
    if (verbose_mode == 1)
    { fprintf(msgout,"%s %s:%s!\n", _("Error"),_(" cannot open file"),fname); }
    return(-1);
  }

  fprintf(fw,"[Description]\n");
  fprintf(fw,"Identitication=Monte output histogram %s\n",his->name);
  if (his->type == H_DISCRETE) { fprintf(fw,"Type=Discrete\n"); }
  else { fprintf(fw,"Type=Continuous\n"); }
  fprintf(fw,"Form=Frequency\n\n");

  fprintf(fw,"[Parameters]\n");
  fprintf(fw,"Min=%f\n",his->min);
  fprintf(fw,"Max=%f\n",his->max);
  fprintf(fw,"Bins=%i\n",his->len);
  fprintf(fw,"Total=%li\n\n",his->total);

  fprintf(fw,"[Bins]\n");
  for (i=0; i<his->len; i++) { fprintf(fw,"%li\n",his->data[i]); }
  fprintf(fw,"\n");

  fclose(fw);
  return(0);
}


/** Write histogram to file in form acceptable for gnuplot 
 * @param his histogram
 * @param fname name of output file
 * @return status
 */
int write_histogram_for_gnuplot(tHis *his, char *fname)
{
  FILE *fw=NULL ;
  int i ;
  double gap ;

  if (fname == NULL) 
  {
    fw = stdout ;
  }
  else
  {
    if ((fw=fopen(fname,"w")) == NULL)
    {
      if (verbose_mode == 1)
      { fprintf(msgout,"%s %s:%s!\n", _("Error"),_(" cannot open file"),fname); }
      return(-1);
    }
  }

  if (his->len == 0)
  {
    if (fw != stdout)
    {
      fclose(fw);
    }
    return(-1);
  }
    
  gap = (his->max - his->min) / ((double)his->len) ;

  fprintf(fw,"# Monte output histogram %s\n",his->name);
  fprintf(fw,"# Min=%f\n",his->min);
  fprintf(fw,"# Max=%f\n",his->max);
  fprintf(fw,"# Bins=%i\n",his->len);
  fprintf(fw,"# Total=%li\n\n",his->total);

  fprintf(fw,"%e %i\n",his->min+0.0, 0); 
  for (i=0; i<his->len-1; i++) 
  { 
    fprintf(fw,"%e %li\n",his->min+(double)i*gap, his->data[i]); 
    fprintf(fw,"%e %li\n",his->min+(double)(i+1)*gap, his->data[i]); 
  }
  fprintf(fw,"%e %li\n",his->min+(double)(his->len-1)*gap, his->data[his->len-1]); 
  fprintf(fw,"%e %li\n",his->min+(double)(his->len)*gap, his->data[his->len-1]); 
  fprintf(fw,"%e %i\n",his->min+(double)(his->len)*gap, 0); 

  if (fw != stdout)
  {
    fclose(fw);
  }
  return(0);
}

/** tests if "val" is odd (1) or even (0) number 
 * @aparam val value
 * @return 1 if val is odd, 0 if val is even
 */
long monte_is_odd(long val)
{
  if (val == 0) {return(0);}

  if ((((long)(val/2))*2) == val)
  {
    return(0);
  }
  else
  {
    return(1);
  }

  return(1);
}

/** Roundes doubles to integers
 * @param val value to be rounded
 * @type rounding style: 0..5 always up, 1..odd down, 2..odd up
 * @return rounded value
 */ 
long monte_round(double val, int type)
{
  long   tmp1, tmp2, tmp, res ;
  double tmpd ;
  long   sign; ;

  if (val == 0.0) {return(0);}

  if (val > 0.0) { sign = (1) ; }
  else           { sign = (-1) ; }
  
  tmpd = fabs(val);
  tmp  = (long) tmpd;
  tmp1 = (long) (10*tmpd) ;

  res = tmp1 - (10*tmp) ;

  if (res == 0)
  {
    return(sign*tmp);
  }
  else
  {
    if (res < 5) { return(sign*tmp) ; }
    if (res > 5) { return(sign*(tmp+1)) ; }

    /* the worst case: */
    switch (type)
    {
      case 0: /* round up: */
        return (sign*(tmp+1)); break ;
      case 1: /* odd down, even up */
        tmp2 = (long) (100*tmpd) ;
        res = tmp2 - (100*tmp) ;

        if (monte_is_odd(res) == 1) { return(sign*tmp); }
        else                        { return(sign*(tmp+1)); }

        break;
      case 2: /* odd up even down */
        tmp2 = (long) (100*tmpd) ;
        res = tmp2 - (100*tmp) ;

        if (monte_is_odd(res) == 0) { return(sign*tmp); }
        else                        { return(sign*(tmp+1)); }

        break;
    }
  }

  return(sign*tmp); /* we CAN NOT reach this point, but... */
}

/* end of hisops.c */
