/* $Id: private_li.h,v 1.6 1997/03/11 14:36:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef	private_li_h
#define	private_li_h
/***************************************************************************

   Changed SH abstraction to the more useful LI (Loop Info) abstraction.
   Added several new routines to support the manipulation of shared and
   private variables and statement insertion/deletion.
   - Vas, May 1988.

   NOTE: As long as the dependence info used by ParaScope is that generated 
   by PSERVE, remember to convert variable names into upper case before
   making any comparisons using strcmp(). This will be unnecessary when
   ParaScope becomes completely independent of PSERVE. 
   -Vas, May 1988.
 ****************************************************************************/


#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>


/* Slist is a list of SHARED variables or PRIVATE variables in the loop. */

/* Loop_info is a linked list, with one link for each loop in the program.
   Each link has info about the loop, and a pointer to an shvar_list */

struct loop_info {
    AST_INDEX	loop_hdr_index;
    int		loop_level;
    int 	cflow;		/* STRUCT - structured control flow
				   UNSTRUCT - forward control flow branches
				   BACK_BRANCH - backward goto
				   NOFLOW  - no control flow  */

    Boolean     parallelized;	/* used by the code generator */
    int         ndeps;
    Slist	*shvar_list;
    Slist	*pvar_list;
    Loop_info   *next;
    Loop_info	*prev;
};

/* LI_Instance is the abstraction that maintains a handle on all info 
   associated with each loop as a whole. */

struct Loops {
    Loop_info   *Linfo;     /* ptr to head of Loop_info list */
    Loop_info	*cur_loop;  /* ptr to Loop_info node that corresponds to current loop */
    int         num_loops;
};

#endif
