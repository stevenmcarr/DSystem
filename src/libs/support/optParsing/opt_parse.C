/* $Id: opt_parse.C,v 1.8 1997/03/27 20:51:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 			      parse.c
 * 									
 *			argument list handling
 * 									
 *   FUNCTION:
 *	given a description of an abstract argument list:
 *	    provide a routine that can parse an arg list
 *		and make callbacks as flags are encountered
 *
 *   USAGE:
 *
 *	
 *	
 *   AUTHOR:
 *	Robert Hood
 *
 *   MODIFICATION HISTORY:
 *      John Mellor-Crummey                                February 1993
 *        ported to C++, replaced struct option_ with Options structure 
 *        that permits incremental extention.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libs/support/msgHandlers/log_msg.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/optParsing/Options.h>
#include <libs/support/strings/StringBuffer.h>

static char *get_optstr(Options *opts); // forward declaration


/*
 * parse an [argc, argv], invoking callback routines registered in t
 *
 * 
 *
 * return a 0 for success
 *	o.w. return index of option found to be in error
 */
int opt_parse_argv (Options *opts, void *handle, int argc, char **argv)
{
  // extern int		 getopt();
  extern char		*optarg;
  extern int		 optind;
  extern int		 opterr;
  
  char			*optstr;	/* dynamically allocated */
  
  char			*extra_arg;
  
  struct choice_	*ch;
  struct string_	*st;
  struct flag_		*fl;
  
  int 			 c = 0;
  int			 i = 0, j = 0;
  
  optstr = get_optstr(opts);
  
  opterr = 0;
  optind = 1; /*index to argc of curent option to parse */
  while ( (c = getopt(argc, argv, optstr)) != -1  ) {

    OptionsIterator oi(opts);
    Option *opt;
    for (; c != '?' && (opt = oi.Current()) && c != opt->arg_char; ++oi);
    
    if ( c != '?' && opt && c == opt->arg_char ) {
      switch (opt->t) {
      case flag:
	fl = (struct flag_ *)opt->f_c;
	fl->callback(handle);
	break;
	
      case choice:
	ch = (struct choice_ *)opt->f_c;
	extra_arg = optarg;
	for (j = 0; j < ch->num_choices &&
	     strcmp(extra_arg, ch->choice[j].arg_str); j++);
	
	if (strcmp(extra_arg, ch->choice[j].arg_str) == 0)
	  ch->callback(handle, ch->choice[j].choice_handle);
	else return optind;
	break;
	
      case string:
	st = (struct string_ *)opt->f_c;
	extra_arg = optarg;
	if (st->callback != 0) st->callback(handle, extra_arg);
	break;
	
      default:
	log_msg(true, "error in options.c:opt_parse_argv():  %s loc=%d\n",
		"unknown arg type in table", i);
	break;
      }
    } else { // not found --> error  
      free(optstr);
      return optind;
    }
  }
  
  free(optstr);
  return 0;
}

//===========================================================================
// scan an [argc, argv] to find a particular string option, return its value,
// and remove the entry from (argc, argv)
// 
// Assume that there are no duplicate arguments, and assume that a string
// actually follows the flag.
//===========================================================================
char *opt_filter_string(Options *, char arg_char, int *argc_p, 
			char ***argv_p)
{
  int    i;
  char   optstr[3];
  Boolean found = false;
  int    next_arg;
  char  *value = 0;

  optstr[0] = '-';
  optstr[1] = arg_char;
  optstr[2] = '\0';

  next_arg = 1;
  for (i = 1; i < *argc_p; i++)
  {
    if (strcmp(optstr, (*argv_p)[i]) == 0) {
      found = true;
      value = ssave((*argv_p)[++i]);
      // don't include argv[i] or argv[i+1] in the new arg list
    } else {
      if (found) { // move argument into previous slot in argv 
	(*argv_p)[next_arg++] = (*argv_p)[i];
      }
    }
  }

  if (found) *argc_p -= 2;

  return value;
}


int opt_get_choice(Options *opts, char arg_char, int argc, char **argv)
{
  int i = 0, j = 0;
  struct choice_  *ch;
  char optstr[3];
  Generic value = -1;

  optstr[0] = '-';
  optstr[1] = arg_char;
  optstr[2] = '\0';

  OptionsIterator oi(opts);
  Option *opt;
  for (; (opt = oi.Current()) && arg_char != opt->arg_char; ++oi);

  if (opt) { // found the option 
    for (i = 1; i < argc; i++) {
      if (strcmp(optstr, argv[i]) == 0) { // found the flag 
	ch = (struct choice_ *)opt->f_c;
	j = 0;
	for (; j < ch->num_choices && 
	     (strcmp(argv[i+1], ch->choice[j].arg_str) != 0); j++);
	
	if (j == ch->num_choices)
	  fprintf(stderr, "opt_get_choice: unknown choice %s specified.\n", 
		  argv[i+1]);
	else value = ch->choice[j].choice_handle;
      }
    }
  }
  return value;
}


static char *get_optstr(Options *opts)
{
  //------------------------------------------------------------------------
  // unlikely more than 100 options, but the StringBuffer will expand if 
  // necessary -- JMC 2/93
  //------------------------------------------------------------------------
  StringBuffer optstring(100); 

  Option *opt;

  for (OptionsIterator oi(opts); opt = oi.Current(); ++oi) {
    optstring.Append(opt->arg_char);
    if (opt->t == choice || opt->t == string) optstring.Append(':');
  }

  return optstring.Finalize();
}

