/* $Id: sigcomments.h,v 1.4 1997/03/11 14:30:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef sigcomments_h
#define sigcomments_h


/**************************************************************************
 sigcomments.h         definitions for significant comment handlers that
                       are used when pretty printing source to a machine 
		       that has indentation restrictions on significant
                       comments

 created: John Mellor-Crummey                        September 1992

 Copyright 1992, Rice University, as part of the ParaScope Programming
 Environment Project.
 **************************************************************************/

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef struct sch_t {
  FUNCTION_POINTER(Boolean, is_significant_comment, (char *comment_string));
  FUNCTION_POINTER(char *, significant_comment_prefix, 
		   (char *comment_string));
  FUNCTION_POINTER(char *, significant_comment_wrap_prefix, 
		   (char *comment_string, char *wrap_point));
} SignificantCommentHandlers;

#define DEFAULT_SIG_COMMENT_HANDLERS  ((SignificantCommentHandlers *) 0)

#endif /* sigcomments_h */
