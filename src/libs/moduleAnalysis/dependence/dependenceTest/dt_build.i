/* $Id: dt_build.i,v 1.5 1997/03/11 14:35:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef dt_build_i
#define dt_build_i

#include <assert.h>
#include <math.h>

#include <libs/support/misc/general.h>

#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/frontEnd/ast/builtins.h>
#include <libs/support/strings/rn_string.h>

/*	For separation of dependence graph	*/

#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>

/*---------*/
/* defines */

#define R_REFS		4000	/* # of variable refs		*/
#define R_STMTS		1000	/* # of statements		*/
#define R_LOOPS		  64	/* # of loops			*/
#define	R_FUNCS		   4	/* # of funcs			*/

#define R_LOOP_VARS	  64	/* # of var classes in a loop	*/
#define R_LOOP_VAR_REFS   32    /* # of refs to a var in a class*/
#define R_LOOP_VAR_NAMES   4    /* # of names in a var class    */

#define EDGE_NUM	2000	/* # of initial DG edges	*/

#define GUARDED      (r->guards ? T_GUARDED : 0) /* reference is guarded */

/*---------------*/

typedef struct dg_func_info
{
  AST_INDEX	func_hdr;	/* AST index of routine		*/
  char         *func_name;	/* char string of function name	*/
  int           func_start;	/* index of first ref in func	*/
  int           func_end;	/* index of ref AFTER last ref in func	*/
  int	        first_loop;	/* index of first loop in this function */
  int	        last_loop;	/* index of loop AFTER last loop in function */
  CfgInstance   cfg_inst;       /* cfg instance for the function */	
} Dg_func_info;			/* func_start <= func_end 	*/

typedef struct dg_loop_ref_info
{
  int  leader;                   /* sym.table index of this name */
  int *var_refs;                 /* list of refs to this class   */
  int  var_ref_num;              /* number of refs to this class */
  int  var_ref_size;             /* size of reference buffer     */
  int *var_names;                /* list of names in this class  */
  int  var_name_num;             /* number of names in this class*/
  int  var_name_size;            /* size of name buffer          */
} Dg_loop_var_info;

typedef struct dg_loop_info
{
  AST_INDEX         loop_hdr;	   /* AST index of DO loop	   */
  AST_INDEX         ivar;	   /* AST index of induction var   */
  char             *ivar_sym;	   /* char string of ivar	   */
  int               ivar_index;    /* symbol table index of ivar   */
  int               loop_level;	   /* level of loop		   */
  int               loop_start;	   /* index of first ref in loop   */
  int               loop_end;	   /* index of ref AFTER last ref in loop */
  Dg_loop_var_info *loop_vars;     /* pointer to loop names info   */
  int               loop_var_num;  /* number of distinct loop refs */
  int               loop_var_size; /* size of loop reference buffer*/
} Dg_loop_info;		 	   /* loop_start <= loop_end 	   */

typedef struct dg_stmt_info
{
  AST_INDEX node;		/* AST of stmt			*/
  int       level;		/* loop level of stmt	        */
} Dg_stmt_info;

/*---------------*/

typedef struct dg_ref_params
{
  DG_Instance	*dg;
  DT_info	*dt;
  LI_Instance	*li;
  SideInfo	*infoPtr;

  Dg_ref_info *refs;		/* pointer to ref info		*/
  int ref_num;			/* number of ref found		*/
  int ref_size;			/* size of ref buffer		*/

  Dg_func_info *funcs;		/* pointer to func info		*/
  int func_num;			/* number of funcs found	*/
  int func_size;		/* size of funcs buffer		*/

  Dg_loop_info *loops;		/* pointer to loop info		*/
  int loop_num;			/* number of loops found	*/
  int loop_size;		/* size of loops buffer		*/

  int loop_cur_level;		/* # of open loops		*/
  int loop_cur[MAXLOOP];	/* index of current loops	*/

  Dg_stmt_info *stmts;		/* pointer to stmt buffer	*/
  int stmt_num;			/* number of stmts found	*/
  int stmt_size;		/* size of stmts buffer		*/

  int guards;			/* depth of structured guards 	*/

  char         *current_proc;	/* name of the current procedure */
  Generic       program_callgraph;
  FortTree      ft;
  SymDescriptor sym_descriptor; /* handle for symbol table       */
  CfgInstance   cfg_inst;	/* handle to current CfgInstance */
} Dg_ref_params;

/*---------------------------*/
/* struct for clear routines */

typedef struct
{
	SideInfo	*infoPtr;
} Clear_lv_parm;

typedef struct dg_clr_params
{
	DG_Edge		*Earray;
	DG_Instance	*dg;
	DT_info		*dt;
	SideInfo	*infoPtr;
} Clear_edge_parm;

EXTERN(void, dg_add_ref, (Dg_ref_params *r, AST_INDEX node, char *sym,
				  int def));

EXTERN(void, dg_invoc_globals, (SymDescriptor d, int callsite_id, 
					Dg_ref_params *r, AST_INDEX invoc));

EXTERN(char *, dg_conservative_rsd_string, (FortTree ft, AST_INDEX node,
						   char *name));
#endif /* dt_build_i */
