/* $Id: ip_perfdata.C,v 1.1 1997/06/25 14:41:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* 
 * This file contains routines which read in performance data from a file and
 * into a data structure to make it available to the performance estimator.
 * Also included is a description of the file format used to store the
 * performance data.
 * 
 * Briefly, the file format is as follows:
 * 
 * - A header comment identifying the file
 * 
 * - a tag identifying the architecture for which the data in the file was
 * collected on (example: "@architecture SEQUENT_SYMMETRY")
 * 
 * - a version number (making it possible for the program reading the file to
 * recognize ancient data files)
 * 
 * - one or more sections of performance data. Each section begins with a
 * keyword identifying the data type (example: "@computation_data") and ends
 * with the keyword "@end_data".
 */

/* 
 * $Log: ip_perfdata.C,v $
 * Revision 1.1  1997/06/25 14:41:40  carr
 * Support 64-bit Pointers
 *
 * Revision 1.7  1997/03/11  14:32:09  carr
 * newly checked in as revision 1.7
 *
 * Revision 1.7  93/12/17  14:55:46  rn
 * made include paths relative to the src directory. -KLC
 * 
 * Revision 1.6  93/11/18  16:54:35  curetonk
 * changes to make ip_perfdata.c ANSI-C compliant
 * filename changed from ip_perfdata.c to ip_perfdata.ansi.c
 * RCS filename changed from RCS/ip_perfdata.c,v to RCS/ip_perfdata.ansi.c,v
 * 
 * Revision 1.6  93/06/11  14:59:55  patton
 * made changes to allow compilation on Solaris' CC compiler
 * 
 * Revision 1.5  93/04/27  12:11:51  curetonk
 * *** empty log message ***
 * 
 * Revision 1.4  93/03/31  10:45:12  mcintosh
 * Fix some bugs and some memory leaks. 
 * 
 * Revision 1.3  92/11/20  14:10:56  joel
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/03  16:27:23  rn
 * fix include file pathnames, change EXTERN to EXTERN,
 * add RCS header, and minor additional cleanup -- JMC & Don
 * 
 * Revision 1.1  92/09/21  23:34:42  mcintosh
 * Initial revision
 * 
 * Revision 1.1  92/06/23  22:42:53  mcintosh
 * Initial revision
 * 
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <sys/param.h>

#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/frontEnd/ast/builtins.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perfdata.h>

extern char *getenv();

/* Pass this string to get_mem() to tell it what routine called it.
*/
#define PERF_READ_DATA "perf_read_data"

/* This determines what the first part of error messages will be
*/
#define PERF_ERR_NAME PERF_READ_DATA

/*
 * The following structure is returned by the routine
 * perf_datafile_readhdr(). It contains information read from the header
 * section of the file. It is subsequently passed to other routines which
 * read the data file.
 */

typedef struct {
  Perf_archtype arch;  /* architecture type of file */
  char *arch_str;      /* string containing name of architecture. */
  int file_version;    /* version number of file */
  int max_parloop_iters;      /* | information about the       */
  int max_parloop_procs;      /* |  size of the parallel loop  */
  int max_parloop_workrange;  /* |  training set               */
  int max_parloop_workincr;
  FILE *fp;            /* FILE ptr of open file */
} Perf_datafilehdr;

Perf_datafilehdr *alloc_Perf_datafilehdr();
void free_Perf_datafilehdr();

/* ---------------------------- */

typedef struct {
  Perf_archtype arch;
  char *str;
} Arch_to_string_type;

static Arch_to_string_type perf_arch_to_string_map[] = {
  { PE_SEQUENT_SYMMETRY, PERF_EST_ARCH_SEQUENT_SYMMETRY },
  { PE_BBN_TC2000, PERF_EST_ARCH_BBN_TC2000 },
  { PE_DUMMY, PERF_EST_ARCH_DUMMY }  /* Must be last entry (sentinel) */
};

static char *perf_map_archtype_to_string(Perf_archtype arch)
{
  int i;
  
  for (i = 0; perf_arch_to_string_map[i].arch != PE_DUMMY; i++)
    if (arch == perf_arch_to_string_map[i].arch)
      return perf_arch_to_string_map[i].str;
  
  return 0;
}

/*
 * Read a line from the performance data file. Ignore comments.
 */

static char *getline(char *buf, FILE *fp)
{
  while (fgets(buf, PERF_DATA_MAXLINE, fp) == buf)
    if (buf[0] != '#')  /* if it's not a comment */
      return buf;       /* then return it */
  return NULL;		/* end of file. */
}

static int perf_datafile_readhdr(Perf_archtype arch, Perf_datafilehdr *hdr)
{
  char filename[MAXPATHLEN];
  char buf[PERF_DATA_MAXLINE];  /* line buffer */
  char keyword[64];
  char *archname;
  FILE *fp;
  
  (void) bzero((char *) hdr, sizeof(*hdr));

  /*
   * Map arch enum to string
   */
  if (!(archname = perf_map_archtype_to_string(arch))) {
    (void) fprintf(stderr, "%s: unknown architecture requested.\n",
		   PERF_ERR_NAME);
    return -1;
  }
  hdr->arch = arch;

  /*
   * Check to see if the 'PERF_DATA_FILES_HOME' environment variable is
   * set (this is something for Parascope developers). If it is set, then
   * get data files from this location.
   */
  if (getenv(PERF_DATA_FILES_HOME_ENV_VAR)) {
    fprintf(stderr,
	    "Using '%s' as base directory for performance data files.\n",
	    getenv(PERF_DATA_FILES_HOME_ENV_VAR));
    (void) sprintf(filename, "%s/%s/%s",
		   getenv(PERF_DATA_FILES_HOME_ENV_VAR),
		   archname, PERF_DATA_FILE_NAME);
  } else {
    (void) sprintf(filename, "%s/%s/%s/%s", getenv("RN_HOME"),
		   PERF_DATA_FILES_HOME, archname, PERF_DATA_FILE_NAME);
  }
  
  /* Open the file
  */
  if ((fp = fopen(filename, "r")) == NULL) {
    (void) fprintf(stderr, "%s: cannot open data file (%s)\n", PERF_ERR_NAME,
	    filename);
    perror("status:");
    return -1;
  }
  hdr->fp = fp;

  /* Read architecture, version, etc., then return header.
  */
  /* version number */
  if (getline(buf, fp) == NULL ||
      sscanf(buf, "%s %d", keyword, &(hdr->file_version)) != 2 ||
      strncmp(PERF_VERSION_KW, keyword, strlen(PERF_VERSION_KW)) ||
      hdr->file_version != PERF_DATAFILE_VERSION) {
    (void) fprintf(stderr, "%s: nonexistent/stale version # (%s)\n",
	    PERF_ERR_NAME, filename);
    return -1;
  }
  /* architecture */
  if (getline(buf, fp) == NULL ||
      strncmp(buf, PERF_ARCH_KW, strlen(PERF_ARCH_KW))) {
    (void) fprintf(stderr, "%s: no %s tag (%s)\n",
	    PERF_ERR_NAME, PERF_ARCH_KW, filename);
    hdr->arch_str = (char *) get_mem(sizeof("unknown"), PERF_READ_DATA);
    (void) strcpy(hdr->arch_str, "unknown");
  } else {
    char *ind1, *ind2;
    ind1 = strchr(buf, '"')+1;
    ind2 = strchr(ind1 ? ind1 : buf, '"');
    if (ind1 && ind2 && ind2 > ind1) {
      hdr->arch_str = (char *) get_mem(ind2-ind1+1, PERF_READ_DATA);
      (void) strncpy(hdr->arch_str, ind1, ind2-ind1);
      hdr->arch_str[ind2-ind1] = '\0'; /* make sure it's terminated */
    } else {
      (void) fprintf(stderr, "%s: mangled %s tag (%s)\n",
	             PERF_ERR_NAME, PERF_ARCH_KW, filename);
    }
  }

  /* Read in a series of fields describing the parallel loop data
  */
  
  if (getline(buf, fp) == NULL ||
      sscanf(buf, "%s %d", keyword, &(hdr->max_parloop_iters)) != 2 ||
      strncmp(PERF_MAXPARLOOPITERS_KW, keyword,
	      strlen(PERF_MAXPARLOOPITERS_KW))) {
    (void) fprintf(stderr, "%s: nonexistent/illegal %s field (%s)\n",
	    PERF_ERR_NAME, PERF_MAXPARLOOPITERS_KW, filename);
    return -1;
  }

  if (getline(buf, fp) == NULL ||
      sscanf(buf, "%s %d", keyword, &(hdr->max_parloop_procs)) != 2 ||
      strncmp(PERF_MAXPARLOOPPROCS_KW, keyword,
	      strlen(PERF_MAXPARLOOPPROCS_KW))) {
    (void) fprintf(stderr, "%s: nonexistent/illegal %s field (%s)\n",
	    PERF_ERR_NAME, PERF_MAXPARLOOPPROCS_KW, filename);
    return -1;
  }

  if (getline(buf, fp) == NULL ||
      sscanf(buf, "%s %d", keyword, &(hdr->max_parloop_workrange)) != 2 ||
      strncmp(PERF_MAXPARLOOPWORKRANGE_KW, keyword,
	      strlen(PERF_MAXPARLOOPWORKRANGE_KW))) {
    (void) fprintf(stderr, "%s: nonexistent/illegal %s field (%s)\n",
	    PERF_ERR_NAME, PERF_MAXPARLOOPWORKRANGE_KW, filename);
    return -1;
  }

  if (getline(buf, fp) == NULL ||
      sscanf(buf, "%s %d", keyword, &(hdr->max_parloop_workincr)) != 2 ||
      strncmp(PERF_MAXPARLOOPWORKINCR_KW, keyword,
	      strlen(PERF_MAXPARLOOPWORKINCR_KW))) {
    (void) fprintf(stderr, "%s: nonexistent/illegal %s field (%s)\n",
	    PERF_ERR_NAME, PERF_MAXPARLOOPWORKINCR_KW, filename);
    return -1;
  }

  /* Return.
   */
  return 0; /* success */
}

/*
 * Find a particular section in the data file. Currently we just use a
 * brute-force search, but if the data files start getting too big, this
 * routine could be changed to use "lseek()" instead (there would have to be
 * more information stored in the header section of the file).
 * 
 * Return value: -1  if error, 0 if found.
 * 
 * The file pointer for the data file (hdr->fp) is left positioned on the first
 * data line in the section (the line just after the section keyword).
 */

static perf_datafile_find_section(Perf_datafilehdr *hdr, char *section)
{
  char buf[PERF_DATA_MAXLINE];
  FILE *fp = hdr->fp;
  int seclen = strlen(section);
  
  /*
   * Start from the beginning of the file and read until we hit the
   * start of the section in question. Return an error if for some
   * reason we can't find what we want.
   */
  (void) fseek(fp, (long) 0, 0); /* set fp to beginning of file */
  while (getline(buf, fp) != NULL)
    if (!strncmp(buf, section, seclen))
      return 0;	/* found */
  
  return -1;
}

/*
 * The following routine reads in the data file section containing parallel
 * loop info.
 */

static perf_datafile_read_parallel_loop_data_subsec(FILE *fp, Perf_datafilehdr *hdr, 
                                                    Perf_data *pdata)
{
  char buf[PERF_DATA_MAXLINE];	/* input buffer */
  Perf_parloopdata *vec;        /* vector to read in a row of data */
  Perf_parloopdata **pd;        /* new parallel loop data subsection */
  int rvals = 1;		/* running check on status of reads/sscanf's */
  int i;			/* loop var */
  int n_rows, pts_per_row;
  int row;
  int sercost;
  double tmp; /* last value in field not used */
  
  /*
   * Allocate memory for the array and zero it.
   */

  n_rows = hdr->max_parloop_iters;
  pts_per_row = hdr->max_parloop_workrange;

  pd = (Perf_parloopdata **)
       get_mem(sizeof(Perf_parloopdata *) * n_rows, PERF_READ_DATA);
  for (row = 0; row < hdr->max_parloop_iters; row++) {
    vec = (Perf_parloopdata *)
      get_mem(sizeof(Perf_parloopdata) * pts_per_row, PERF_READ_DATA);
    for (i = 0; i < pts_per_row; i++) {
      rvals = rvals && getline(buf, fp);
      rvals = rvals && (sscanf(buf, "%d %d %lf", &(vec[i].nprocs),
			       &(sercost), &(vec[i].parloopcost)) == 3);
      vec[i].bodycost = sercost;
    }
    pd[row] = vec;
  }
  
  pdata->comp.parloop = pd;
  return rvals;
}

/*
 * Read in a subsection of computational data. Return nonzero if it went ok,
 * otherwise return 0.
 */

static int perf_datafile_read_compdata_subsec(FILE *fp, Perf_compdata *sec, 
                                              int s_len, int p_len)
/* fp: input file pointer */
/* sec: ptr to data to read in values into */
/*  s_len: # of "simple" ops to read */
/* p_len: # of "parameterized" ops to read in */
{
  char buf[PERF_DATA_MAXLINE];	/* input buffer */
  int rvals = 1;		/* running check on status of reads/sscanf's */
  int i;			/* loop var */
  
  /*
   * Read in the data, first the simple ops and then the parameterized
   * ones. Save status of reads/sscanf's, check for error after loops.
   */
  for (i = 0; i < s_len; i++) {
    rvals = rvals && getline(buf, fp);
    rvals = rvals && (sscanf(buf, "%lf", &(sec[i].val)) == 1);
  }
  for (; i < (s_len + p_len); i++) {
    rvals = rvals && getline(buf, fp);
    rvals = rvals && (sscanf(buf, "%lf %lf", &(sec[i].pval.base),
			     &(sec[i].pval.incr)) == 2);
  }
  
  return rvals;
}

/*
 * Read in the computation data from the data file.
 * 
 * For "simple" computational operations (see perfdata.h) the data will just be
 * a floating point number. For "parameterized" operations, it will be a pair
 * of numbers.
 * 
 * Return value: -1  if error, 0 if found.
 * 
 * The file pointer for the data file (hdr->fp) is left positioned just after
 * the section containing the computation data.
 */

static int perf_datafile_read_compdata(Perf_datafilehdr *hdr, Perf_data *pdata)
{
  Perf_compdata *newsec;     /* block of compdata structures to fill in */
  Perf_compdata *newsecptr;  /* ptr into above */
  Perf_compdata *element;    /* scratch var */
  Perf_compdata *row;        /* scratch var */
  char buf[PERF_DATA_MAXLINE]; /* input buffer */
  FILE *fp = hdr->fp;	     /* file pointer for data file */
  int rvals = 1;	     /* running check on status of reads/sscanf's */
  int max_generics = builtins_numGenerics("read_compdata");
  int max_intrinsics = builtins_numIntrinsics("read_compdata");
  int i;
  int argtype;
  
  /*
   * Find the section with the computational data
   */
  if (perf_datafile_find_section(hdr, PERF_COMPDATA_SECTION_KW) < 0) {
    (void) fprintf(stderr, "%s: data file for %s is missing computation section.\n",
		   PERF_ERR_NAME, hdr->arch_str);
    return -1;
  }

  pdata->comp.untyped = (Perf_compdata *)
    get_mem(sizeof(Perf_compdata) * 
	    (PERF_EST_N_UNTYPED_SIMPLE+PERF_EST_N_UNTYPED_PARAM), "read_compdata");
  for (argtype = TYPE_INTEGER; argtype <= TYPE_COMPLEX; argtype++) {
    pdata->comp.typed[argtype] = (Perf_compdata *)
      get_mem(sizeof(Perf_compdata) *
	      (PERF_EST_N_TYPED_SIMPLE+PERF_EST_N_TYPED_PARAM), "read_compdata");
  }
  pdata->comp.intrinsic = (Perf_compdata *)
    get_mem(sizeof(Perf_compdata) * max_intrinsics, "read_compdata");
  for (argtype = TYPE_INTEGER; argtype <= TYPE_COMPLEX; argtype++) {
    pdata->comp.generic[argtype] = (Perf_compdata *)
      get_mem(sizeof(Perf_compdata) * max_generics, "read_compdata");
  }

  /*
   * Read in each subsection. Start with the regular computational
   * operations and then move on to the intrinsic and generic
   * functions.
   */
  /* typed simple */
  for (i = 0; i < PERF_EST_N_TYPED_SIMPLE; i++)
    for (argtype = TYPE_INTEGER; argtype <= TYPE_COMPLEX; argtype++) {
      row = pdata->comp.typed[argtype];
      element = &(row[i]);
      rvals = rvals &&
	perf_datafile_read_compdata_subsec(fp, element, 1, 0);
    }
  /* untyped simple, then untyped parameterized */
  rvals = rvals &&
    perf_datafile_read_compdata_subsec(fp, pdata->comp.untyped,
				       PERF_EST_N_UNTYPED_SIMPLE,
				       PERF_EST_N_UNTYPED_PARAM);
  /* typed parameterized */
  for (i = 0; i < PERF_EST_N_TYPED_PARAM; i++)
    for (argtype = TYPE_INTEGER; argtype <= TYPE_COMPLEX; argtype++) {
      row = pdata->comp.typed[argtype];
      element = &(row[i+PERF_EST_N_TYPED_SIMPLE]);
      rvals = rvals &&
	perf_datafile_read_compdata_subsec(fp, element, 0, 1);
    }
    
  /* Intrinsics. 
   */
  rvals = rvals &&
    perf_datafile_read_compdata_subsec(fp, pdata->comp.intrinsic,
				       max_intrinsics, 0);
  for (argtype = TYPE_INTEGER; argtype <= TYPE_COMPLEX; argtype++)
    rvals = rvals &&
      perf_datafile_read_compdata_subsec(fp, pdata->comp.generic[argtype],
					 0, max_generics);
  
  /*
   * Read parallel loop data
   */
  rvals = rvals &&
    perf_datafile_read_parallel_loop_data_subsec(fp, hdr, pdata);
  
  /*
   * Check to see if there were any errors during this whole mess.
   */
  if (!rvals) {
    (void) fprintf(stderr, "%s: data file for %s corrupted.\n", PERF_ERR_NAME,
		   hdr->arch_str);
    return -1;
  }
  /*
   * Read end of section, just to be sure the numbers match
   */
  if (getline(buf, fp) == NULL ||
      strncmp(buf, PERF_END_SECTION_KW, strlen(PERF_END_SECTION_KW))) {
    (void) fprintf(stderr, "%s: data file for %s corrupted.\n",
		   PERF_ERR_NAME, hdr->arch_str);
    return -1;
  }
  /*
   * Done.
   */
  return 0;
}

/*
 * Storage management functions. Please update when structures are modified
 * or fields are added.
 */

static void free_Perf_datafilehdr_fields(Perf_datafilehdr *n)
{
  if (n->arch_str)
    free_mem((void*) n->arch_str);
}

static Perf_data *alloc_Perf_data()
{
  Perf_data *n;

  n = (Perf_data *) get_mem(sizeof(Perf_data), PERF_READ_DATA);
  (void) bzero((char *) n, sizeof(*n));
  return n;
}

/*---------------------------------------------------------------*/
/* Public entry points 
 */

/*
 * Public routine to read in performance data for a selected architecture.
 * Allocates a "Perf_data" structure, reads in the data, and returns a
 * pointer to the allocated structure.
 * 
 * Returns: 0 for if error, otherwise returns pointer to filled in data
 * structure.
 * 
 * Side effects: reads file, may print messages to stderr if something goes
 * wrong.
 */

Perf_data *perf_read_data(Perf_archtype arch)
{
  Perf_data *d = alloc_Perf_data();
  Perf_datafilehdr hdr;
  
  /*
   * Open the header, then read in the computation data. Currently this
   * is all we need to do. If more performance data is added to the
   * file (ex: communication data for distributed memory machines) it
   * should be added at this level.
   */
  if (perf_datafile_readhdr(arch, &hdr) < 0) {
    free_Perf_datafilehdr_fields(&hdr);
    return 0;
  }
  d->arch_id = hdr.arch;
  d->max_parloop_iters = hdr.max_parloop_iters;
  d->max_parloop_procs = hdr.max_parloop_procs;
  d->max_parloop_workrange = hdr.max_parloop_workrange;
  d->max_parloop_workincr = hdr.max_parloop_workincr;

  if (perf_datafile_read_compdata(&hdr, d) < 0) {
    free_Perf_datafilehdr_fields(&hdr);
    free_Perf_data(d);
    return 0;
  }

  /*
   * Close the data file; free the fields in the file hdr structure
   */
  (void) fclose(hdr.fp);
  free_Perf_datafilehdr_fields(&hdr);
  
  /*
   * Success.
   */
  return d;
}

/*
 * Function to free Perf_data structure when finished with it. Called by
 * other parts of the performance estimator; must be public.
 */

void free_Perf_data(Perf_data *n)
{
  int t, argtype, i;

  if (n->comp.untyped)
    free_mem((void*) n->comp.untyped);
  for (argtype = TYPE_INTEGER; argtype <= TYPE_COMPLEX; argtype++)
    if (n->comp.typed[argtype])
      free_mem((void*) n->comp.typed[argtype]);
  if (n->comp.intrinsic)
    free_mem((void*) n->comp.intrinsic);
  for (argtype = TYPE_INTEGER; argtype <= TYPE_COMPLEX; argtype++)
    if (n->comp.generic[argtype])
      free_mem((void*) n->comp.generic[argtype]);

  for (i = 0; i < n->max_parloop_iters; i++)
    if (n->comp.parloop[i])
      free_mem((void*) n->comp.parloop[i]);
  if (n->comp.parloop)
    free_mem((void*) n->comp.parloop);

  free_mem((void*) n);
}

#ifdef HUGE
#define MY_HUGE_VAL HUGE
#else
#define MY_HUGE_VAL HUGE_VAL
#endif

void perf_get_parallel_loop_cost(Perf_data *pdata, double sercost, int niters,
				 double *parcostptr, int *nprocptr)
/* pdata: performance data for this architecture */
/* sercost: cost to execute a single iteration */
/* niters: number of iterations ( must be >= 1 ) */
/* parcostptr: cost of loop in parallel (returned) */
/* nprocptr: # of procs used for loop (returned) */
{
  int i, workind;
  Perf_parloopdata *vec;
  double mincost = MY_HUGE_VAL;
  
  /*
   * Use the number of iterations as an index into this table
   */
  if (niters > pdata->max_parloop_iters)
    niters = pdata->max_parloop_iters;
  if (niters <= 1)
    niters = 1;
  niters--;
  vec = pdata->comp.parloop[niters];
  workind = (((int) sercost) / pdata->max_parloop_workincr);
  
  /*
   * If the cost of executing the body is larger than the largest value
   * we keep in our table, then use the largest value as a default.
   */
  if (((int) sercost) > (pdata->max_parloop_workrange *
			 pdata->max_parloop_workincr)) {
    /*
     * At this point we have some guessing to do, since the
     * explicit data kept in the table doesn't extend to loops
     * with this much work in them. If the number of iterations
     * is 1, then just record the serial cost. Otherwise,
     * calculate the parallel time based on the last entry in the
     * table.
     */
    if (niters == 0) {
      *parcostptr = sercost;
      *nprocptr = 1;
      return;
    } else {
      /*
       * Calculate the speedup that you get for the maximum
       * work value entry in the table, then apply that
       * speedup to the current serial cost value.
       */
      int max_sercost = (pdata->max_parloop_workrange *
			 pdata->max_parloop_workincr);
      int max_workind = pdata->max_parloop_workrange - 1;
      double speedup = vec[max_workind].parloopcost /
	(max_sercost * (niters + 1));
      
      *parcostptr = (sercost * (niters + 1)) * speedup;
      *nprocptr = vec[max_workind].nprocs;
      return;
    }
  }

  /* Check to make sure we didn't get a bogus value
  */
  if (workind < 0 || workind > (pdata->max_parloop_workrange - 1)) {
    fprintf(stderr, "%s: bogus value passed to parallel cost routine\n",
	    __FILE__);
    workind = 0;
  }

  /*
   * Otherwise, use the value in the table
   */
  *parcostptr = vec[workind].parloopcost;
  *nprocptr = vec[workind].nprocs;
  return;
}

/* Get the cost of an intrinsic. This is a hideous hack.
 */

double perf_get_intrinsic_cost(Perf_data *pdata, char *i_name)
  /* pdata: performance data for this architecture */
  /* i_name: intrinsic function name */
{
  int i_index;
  int numintr = builtins_numIntrinsics("free_perf_data");
  intrinsic_descriptor *idesc;
  
  for (i_index = 0; i_index < numintr; i_index++) {
    idesc = builtins_intrinsicFunctionInfo_bynumber(i_index);
    if (!strcmp(idesc->name, i_name))
      return pdata->comp.intrinsic[i_index].val;
  }

  /* Should never get here.
  */ 
  fprintf(stderr, "%s:%d - cost requested for non-intrinsic (?)\n",
	  __FILE__, __LINE__);
  return 0.0;
}

/* Get the cost of a generic 
 */

double perf_get_generic_cost(Perf_data *pdata, char *g_name, int typ, int nargs)
  /* pdata: performance data for this architecture */
  /* g_name: generic function name */
  /* typ, nargs: data type and number of arguments */
{
  int g_index;
  int numgener = builtins_numGenerics("free_perf_data");
  generic_descriptor *gdesc;
  
  for (g_index = 0; g_index < numgener; g_index++) {
    gdesc = builtins_genericFunctionInfo_bynumber(g_index);
    if (!strcmp(gdesc->name, g_name))
      return ((pdata->comp.generic[typ])[g_index].pval.base +
	      (nargs * (pdata->comp.generic[typ])[g_index].pval.incr));
  }

  /* Should never get here.
  */ 
  fprintf(stderr, "%s:%d - cost requested for non-intrinsic (?)\n",
	  __FILE__, __LINE__);
  return 0.0;
}

