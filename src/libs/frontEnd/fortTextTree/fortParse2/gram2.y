/* $Id: gram2.y,v 1.11 2001/10/12 19:38:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortParse2/gram2.y				        */
/*									*/
/*	FortParse2/gram2 -- yacc for high-level Fortran parser		*/
/*									*/
/************************************************************************/




%{

#define gram2_h			/* already have yacc-generated decls */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/fortParse2/FortParse2.i>

%}






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Static data		*/
/************************/




%{


/* Special Values */

#define MISSING     ((FortTreeNode) -1)
#define IMPLIED     ((FortTreeNode) -2)


/* Forward declarations */

STATIC(FortTreeNode, misplaced,(fx_StatToken st, char *what, char *prev));
STATIC(fx_EndingStat, missing,(char *what));
STATIC(void, setLineTags1, (FortTreeNode node, fx_StatToken st));
STATIC(void, setLineTags2, (FortTreeNode node, fx_StatToken st, fx_EndingStat endval));

%}





/************************/
/* Semantic values	*/
/************************/




%union  {
    FortTreeNode astptr;
    fx_StatToken statval;
    fx_EndingStat endval;
    }






/************************************************************************/
/*	Statement Grammar	 					*/
/************************************************************************/




/************************/
/*  Terminal symbols	*/
/************************/


%token SGOAL_MODULE SGOAL_UNIT
%token SGOAL_DO SGOAL_DOALL SGOAL_IF SGOAL_PAR SGOAL_PARALLELLOOP
%token SGOAL_STAT SGOAL_WHERE

%token SARITHMETIC_IF_STAT SASSIGNED_GOTO_STAT SASSIGNMENT_STAT
%token SASSIGN_STAT SAT_STAT SBACKSPACE_LONG_STAT
%token SBACKSPACE_SHORT_STAT SBLOCK_DATA_STAT SBLOCK_STAT
%token SCALL_STAT SCLEAR_STAT SCLOSE_STAT SCOMMENT_STAT SCOMMON_STAT
%token SCOMPUTED_GOTO_STAT SCONTINUE_STAT SDATA_STAT SDEBUG_STAT
%token SDIMENSION_STAT SDOALL_LABEL_STAT SDOALL_STAT SDO_LABEL_STAT
%token SDO_STAT SELSE_IF_STAT SELSE_STAT SENDFILE_LONG_STAT
%token SENDFILE_SHORT_STAT SEND_ALL_STAT SEND_DO_STAT SEND_DO_IMPLIED
%token SEND_IF_STAT SEND_LOOP_STAT SEND_STAT SENTRY_STAT
%token SEQUIVALENCE_STAT SERROR_STAT SEXTERNAL_STAT SFORMAT_STAT
%token SFUNCTION_STAT SGOTO_STAT SIF_STAT SIMPLICIT_STAT
%token SINQUIRE_STAT SINTRINSIC_STAT SLOCK_STAT SLOGICAL_IF_STAT
%token SOPEN_STAT SOTHER_PROCESSES_STAT SPARALLEL_PID_STAT
%token SPARALLEL_STAT SPARALLELLOOP_STAT SPARALLELLOOP_LABEL_STAT
%token SPARAMETER_STAT SPARBEGIN_PID_STAT SPARBEGIN_STAT SPAREND_STAT
%token SPAUSE_STAT SPOST_STAT SPRINT_STAT SPRIVATE_STAT
%token SPROGRAM_STAT SPRSCOPE_PH_STAT SREAD_LONG_STAT
%token SREAD_SHORT_STAT SRETURN_STAT SREWIND_LONG_STAT
%token SREWIND_SHORT_STAT SSAVE_STAT SSET_BARRIER_STAT
%token SSTMT_FUNCTION_STAT SSTOP_STAT SSTOP_LOOP_STAT
%token SSUBROUTINE_STAT STASK_COMMON_STAT STASK_STAT STRACEOFF_STAT
%token STRACEON_STAT STYPE_STATEMENT_STAT SUNLOCK_STAT SWAIT_STAT
%token SWRITE_STAT
%token SSTMT_PH_STAT SSPECIFICATION_STMT_PH_STAT SCONTROL_STMT_PH_STAT
%token SIO_STMT_PH_STAT SPARASCOPE_STMT_PH_STAT SDEBUG_STMT_PH_STAT
%token SALLOCATABLE_STAT SALLOCATE_STAT SDEALLOCATE_STAT
%token SWHERE_STAT SWHERE_BLOCK_STAT SELSE_WHERE_STAT SEND_WHERE_STAT
%token SENDMARKER


%type <statval> SGOAL_MODULE SGOAL_UNIT
%type <statval> SGOAL_DO SGOAL_DOALL SGOAL_IF SGOAL_PAR SGOAL_PARALLELLOOP
%type <statval> SGOAL_STAT SGOAL_WHERE

%type <statval> SARITHMETIC_IF_STAT SASSIGNED_GOTO_STAT SASSIGNMENT_STAT
%type <statval> SASSIGN_STAT SAT_STAT SBACKSPACE_LONG_STAT
%type <statval> SBACKSPACE_SHORT_STAT SBLOCK_DATA_STAT SBLOCK_STAT
%type <statval> SCALL_STAT SCLEAR_STAT SCLOSE_STAT SCOMMENT_STAT SCOMMON_STAT
%type <statval> SCOMPUTED_GOTO_STAT SCONTINUE_STAT SDATA_STAT SDEBUG_STAT
%type <statval> SDIMENSION_STAT SDOALL_LABEL_STAT SDOALL_STAT SDO_LABEL_STAT
%type <statval> SDO_STAT SELSE_IF_STAT SELSE_STAT SENDFILE_LONG_STAT
%type <statval> SENDFILE_SHORT_STAT SEND_ALL_STAT SEND_DO_STAT SEND_DO_IMPLIED
%type <statval> SEND_IF_STAT SEND_LOOP_STAT SEND_STAT SENTRY_STAT
%type <statval> SEQUIVALENCE_STAT SERROR_STAT SEXTERNAL_STAT SFORMAT_STAT
%type <statval> SFUNCTION_STAT SGOTO_STAT SIF_STAT SIMPLICIT_STAT
%type <statval> SINQUIRE_STAT SINTRINSIC_STAT SLOCK_STAT SLOGICAL_IF_STAT
%type <statval> SOPEN_STAT SOTHER_PROCESSES_STAT SPARALLEL_PID_STAT
%type <statval> SPARALLEL_STAT SPARALLELLOOP_STAT SPARALLELLOOP_LABEL_STAT
%type <statval> SPARAMETER_STAT SPARBEGIN_PID_STAT SPARBEGIN_STAT SPAREND_STAT
%type <statval> SPAUSE_STAT SPOST_STAT SPRINT_STAT SPRIVATE_STAT
%type <statval> SPROGRAM_STAT SPRSCOPE_PH_STAT SREAD_LONG_STAT
%type <statval> SREAD_SHORT_STAT SRETURN_STAT SREWIND_LONG_STAT
%type <statval> SREWIND_SHORT_STAT SSAVE_STAT SSET_BARRIER_STAT
%type <statval> SSTMT_FUNCTION_STAT SSTOP_STAT SSTOP_LOOP_STAT
%type <statval> SSUBROUTINE_STAT STASK_COMMON_STAT STASK_STAT STRACEOFF_STAT
%type <statval> STRACEON_STAT STYPE_STATEMENT_STAT SUNLOCK_STAT SWAIT_STAT
%type <statval> SWRITE_STAT
%type <statval> SSTMT_PH_STAT SSPECIFICATION_STMT_PH_STAT SCONTROL_STMT_PH_STAT
%type <statval> SIO_STMT_PH_STAT SPARASCOPE_STMT_PH_STAT SDEBUG_STMT_PH_STAT
%type <statval> SALLOCATABLE_STAT SALLOCATE_STAT SDEALLOCATE_STAT
%type <statval> SWHERE_STAT SWHERE_BLOCK_STAT SELSE_WHERE_STAT SEND_WHERE_STAT
%type <statval> SENDMARKER




/************************/
/*  Nonterminal symbols	*/
/************************/


%type <astptr>  goal
%type <astptr>  module
%type <astptr>  program_unit_list
%type <astptr>  program_unit
%type <endval>  program_unit_end_stat
%type <astptr>  stat_list stat
%type <astptr>  if_guard_list if_guard
%type <endval>  if_end_stat
%type <astptr>  if_misplaced
%type <endval>  where_end_stat
%type <astptr>  where_misplaced
%type <endval>  do_end_stat
%type <astptr>  do_misplaced
%type <endval>  do_all_end_stat
%type <astptr>  do_all_misplaced
%type <astptr>  par_case_list par_case
%type <endval>  par_end_stat
%type <astptr>  par_misplaced
%type <astptr>  par_pid_case_list par_pid_case
%type <astptr>  par_pid_misplaced
%type <endval>  parallel_loop_end_stat
%type <astptr>  parallel_loop_misplaced
%type <astptr>  do_compound_stat
%type <astptr>  do_all_compound_stat
%type <astptr>  if_compound_stat
%type <astptr>  par_compound_stat
%type <astptr>  par_pid_compound_stat
%type <astptr>  parallelloop_compound_stat
%type <astptr>  where_block_stat



/* Precedence and associativity */

%left error
%left SPARALLEL_STAT SPARALLEL_PID_STAT SOTHER_PROCESSES_STAT SPAREND_STAT
%left SELSE_IF_STAT SELSE_STAT SEND_IF_STAT
%left SEND_ALL_STAT SEND_LOOP_STAT SEND_DO_STAT SEND_DO_IMPLIED
%left SELSE_WHERE_STAT SEND_WHERE_STAT




%%




/********************************/
/*  Program Statements		*/
/********************************/


startSymbol:
    goal SENDMARKER
      {
        fp2_root = $1;
      };


goal:
    SGOAL_MODULE module
      {
        $$ = $2;
      }
  | SGOAL_UNIT program_unit
      {
        $$ = $2;
      }
  | SGOAL_DO do_compound_stat
      {
        $$ = $2;
      }
  | SGOAL_DOALL do_all_compound_stat
      {
        $$ = $2;
      }
  | SGOAL_IF if_compound_stat
      {
        $$ = $2;
      }
  | SGOAL_PAR par_compound_stat
      {
        $$ = $2;
      }
  | SGOAL_PAR par_pid_compound_stat
      {
        $$ = $2;
      }
  | SGOAL_PARALLELLOOP parallelloop_compound_stat
      {
        $$ = $2;
      }
  | SGOAL_STAT stat
      {
        $$ = $2;
      }
  | SGOAL_WHERE where_block_stat
      {
        $$ = $2;
      };


module:
    program_unit_list
      {
        $$ = gen_GLOBAL($1);
      };


program_unit_list:
    /* empty */
      {
        $$ = list_create(AST_NIL);
      }
  | program_unit_list program_unit
      {
        $$ = list_insert_last($1, $2);
      };


program_unit:
    SPROGRAM_STAT stat_list program_unit_end_stat
      {
        FortTreeNode label;

        label = $3.label;
        $$ = gen_PROGRAM($1.part[0], label, $1.part[1], $2);
        if( $3.missing ) 
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        setLineTags2($$, $1, $3);
      }
  | stat_list program_unit_end_stat
      {
        FortTreeNode label, name;

        label = $2.label;
        name = gen_IDENTIFIER();
        gen_put_text(name, "main", STR_IDENTIFIER);
        $$ = gen_PROGRAM(AST_NIL, label, name, $1);
        if( $2.missing ) 
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
	ft_SetShow($$,false);		/* no program statement */
      }
  | SBLOCK_DATA_STAT stat_list program_unit_end_stat
      {
        FortTreeNode label;

        label = $3.label;
        $$ = gen_BLOCK_DATA($1.part[0], label, $1.part[1], $2);
        if( $3.missing )
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        setLineTags2($$, $1, $3);
      }
  | SSUBROUTINE_STAT stat_list program_unit_end_stat
      {
        FortTreeNode label;

        label = $3.label;
	$$ = gen_SUBROUTINE($1.part[0], label, $1.part[1], $1.part[2], $2);
        if( $3.missing )
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        setLineTags2($$, $1, $3);
      }
  | SFUNCTION_STAT stat_list program_unit_end_stat
      {
        FortTreeNode label;

        label = $3.label;
        $$ = gen_FUNCTION($1.part[0], label, $1.part[1], $1.part[2],
                          $1.part[3], $2);
        if( $3.missing )
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        setLineTags2($$, $1, $3);
      }
  | SDEBUG_STAT stat_list program_unit_end_stat		/* VS FORTRAN */
      {
        FortTreeNode label;

        label = $3.label;
        $$ = gen_DEBUG($1.part[0], label, $1.part[1], $2);
        if( $3.missing )
        {
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        setLineTags2($$, $1, $3);
      }
  | SCOMMENT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SPRSCOPE_PH_STAT					/* PLACEHOLDER */
      {
        $$ = $1.part[0];
        setLineTags1($$, $1);
      };


program_unit_end_stat:
    SEND_STAT
      {
        $$.missing = false;
        $$.label   = $1.part[0];
        $$.conceal = $1.conceal;
      };


stat_list:
    /* empty */
      {
        $$ = list_create(AST_NIL);
      }
  | stat_list stat
      {
        $$ = list_insert_last($1, $2);
      };






/********************************/
/*  Simple Statements		*/
/********************************/



stat:
    SALLOCATABLE_STAT					/* Fortran 90 */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SALLOCATE_STAT					/* Fortran 90 */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SARITHMETIC_IF_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SASSIGNED_GOTO_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SASSIGNMENT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SASSIGN_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SAT_STAT						/* VS FORTRAN */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SBACKSPACE_LONG_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SBACKSPACE_SHORT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SBLOCK_STAT						/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SCALL_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SCLEAR_STAT						/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SCLOSE_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SCOMMENT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SCOMMON_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SCOMPUTED_GOTO_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SCONTINUE_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SDATA_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SDEALLOCATE_STAT					/* Fortran 90 */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      };
  | SDIMENSION_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SENDFILE_LONG_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SENDFILE_SHORT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SENTRY_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SEQUIVALENCE_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SEXTERNAL_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SFORMAT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SGOTO_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SIMPLICIT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SINQUIRE_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SINTRINSIC_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SLOCK_STAT						/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SLOGICAL_IF_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SOPEN_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SPARAMETER_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SPAUSE_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SPOST_STAT						/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SPRINT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SPRIVATE_STAT					/* PARALLEL */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SREAD_LONG_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SREAD_SHORT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SRETURN_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SREWIND_LONG_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SREWIND_SHORT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SSAVE_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SSET_BARRIER_STAT					/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SSTMT_FUNCTION_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SSTOP_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SSTOP_LOOP_STAT					/* PARALLEL */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | STASK_COMMON_STAT					/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | STASK_STAT						/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | STRACEOFF_STAT					/* VS FORTRAN */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | STRACEON_STAT					/* VS FORTRAN */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | STYPE_STATEMENT_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SUNLOCK_STAT					/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SWAIT_STAT						/* PARASCOPE */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SWHERE_STAT						/* Fortran 90 */
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SWRITE_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | SSTMT_PH_STAT					/* PLACEHOLDER */
      {
        $$ = $1.part[0];
        setLineTags1($$, $1);
      }
  | SSPECIFICATION_STMT_PH_STAT				/* PLACEHOLDER */
      {
        $$ = $1.part[0];
        setLineTags1($$, $1);
      }
  | SCONTROL_STMT_PH_STAT				/* PLACEHOLDER */
      {
        $$ = $1.part[0];
        setLineTags1($$, $1);
      }
  | SIO_STMT_PH_STAT					/* PLACEHOLDER */
      {
        $$ = $1.part[0];
        setLineTags1($$, $1);
      }
  | SPARASCOPE_STMT_PH_STAT				/* PLACEHOLDER */
      {
        $$ = $1.part[0];
        setLineTags1($$, $1);
      }
  | SDEBUG_STMT_PH_STAT					/* PLACEHOLDER */
      {
        $$ = $1.part[0];
        setLineTags1($$, $1);
      };






/********************************/
/*  Compound statements		*/
/********************************/




/************************/
/*  Do statement	*/
/************************/


stat:
    do_compound_stat;


do_compound_stat:
    SDO_STAT stat_list do_end_stat
      {
        FortTreeNode label;

	label = ( $3.label == IMPLIED  ?  AST_NIL  :  $3.label );
        $$ = gen_DO($1.part[0], label, $1.part[1], $1.part[2], $2);
        if( $3.missing )
	{
          ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        else if( $3.label == IMPLIED )
          ft_SetParseErrorCode($$, ftt_WRONG_ENDBRACKET);

        setLineTags2($$, $1, $3);
      }
  | SDO_LABEL_STAT stat_list do_end_stat
      {
        FortTreeNode label;

	label = ( $3.label == IMPLIED  ?  AST_NIL  :  $3.label );
        $$ = gen_DO($1.part[0], label, $1.part[1], $1.part[2], $2);
        if( $3.missing )
	{
          ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        else if( $3.label != IMPLIED )
          ft_SetParseErrorCode($$, ftt_WRONG_ENDBRACKET);

        setLineTags2($$, $1, $3);
      };


do_end_stat:
    SEND_DO_STAT
      {
        $$.missing = false;
        $$.label   = $1.part[0];
        $$.conceal = $1.conceal;
      }
  | SEND_DO_IMPLIED
      {
        $$.missing = false;
        $$.label   = IMPLIED;
        $$.conceal = $1.conceal;
      };




/************************/
/*  Doall statement	*/
/************************/


stat:
    do_all_compound_stat;


do_all_compound_stat:
    SDOALL_STAT stat_list do_all_end_stat
      {
        FortTreeNode label;

        label = $3.label;
        $$ = gen_DO_ALL($1.part[0], label, $1.part[1], $1.part[2], $2);
        if( $3.missing )  
        {
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        setLineTags2($$, $1, $3);
      }
  | SDOALL_LABEL_STAT stat_list do_all_end_stat
      {
        FortTreeNode label;

        label = ( $3.missing  ||  $3.label != IMPLIED )  ?  $3.label  :  AST_NIL;
        $$ = gen_DO_ALL($1.part[0], label, $1.part[1], $1.part[2], $2);
        if( $3.missing )
	{
          ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        else if( $3.label != IMPLIED )
          ft_SetParseErrorCode($$, ftt_WRONG_ENDBRACKET);

        setLineTags2($$, $1, $3);
      };


do_all_end_stat:
    SEND_ALL_STAT
      {
        $$.missing = false;
        $$.label   = $1.part[0];
        $$.conceal = $1.conceal;
      }
  | SEND_DO_IMPLIED
      {
        $$.missing = false;
        $$.label   = IMPLIED;
        $$.conceal = $1.conceal;
      };




/************************/
/*  If statement	*/
/************************/


stat:
    if_compound_stat;


if_compound_stat:
    SIF_STAT stat_list if_guard_list if_end_stat
      {
        FortTreeNode label, first_guard, guard_list;

        label = $4.label;
        first_guard = gen_GUARD(AST_NIL, $1.part[1], $2);
        guard_list = list_insert_first($3, first_guard);

        $$ = gen_IF($1.part[0], label, guard_list);
        if( $4.missing )
          { 
	    ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }

        setLineTags2($$, $1, $4);
      }
  | SIF_STAT stat_list if_guard_list SELSE_STAT stat_list if_end_stat
      {
        FortTreeNode label, first_guard, last_guard, guard_list;

        label = $6.label;
	guard_list = $3;

        first_guard = gen_GUARD(AST_NIL, $1.part[1], $2);
        guard_list  = list_insert_first(guard_list, first_guard);
        setLineTags2(first_guard, $1, $6);	/* $6 unused */

	last_guard  = gen_GUARD($4.part[0], $4.part[1], $5);
        setLineTags2(last_guard, $4, $6);  /* $6 unused */
        guard_list  = list_insert_last(guard_list, last_guard);

        $$ = gen_IF($1.part[0], label, guard_list);
        if( $6.missing )
          { 
	    ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }
        /* awful kludge -- see 'setConceal2' */
          ft_SetConceal(ftt_fortTree, $$, 2, $6.conceal);

      };


if_guard_list:
    /* empty */					%prec SELSE_IF_STAT
      {
        $$ = list_create(AST_NIL);
      }
  | if_guard_list if_guard
      {
        $$ = list_insert_last($1, $2);
      };

if_guard:
    SELSE_IF_STAT stat_list
      {
        $$ = gen_GUARD($1.part[0], $1.part[1], $2);
        setLineTags2($$, $1, UNUSED_ENDVAL);
      };


if_end_stat:
    SEND_IF_STAT
      {
        $$.missing = false;
        $$.label   = $1.part[0];
        $$.conceal = $1.conceal;
      };




/************************/
/*  Parallel statement	*/
/************************/


stat:
    par_compound_stat;


par_compound_stat:
    SPARBEGIN_STAT stat_list par_case_list par_end_stat
      {
        FortTreeNode label;

        label = $4.label;
        $$ = gen_PARALLEL($1.part[0], label, AST_NIL, AST_NIL, $2, $3);
        if( $4.missing )  
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}

        setLineTags2($$, $1, $4);
      };


par_case_list:
    /* empty */					%prec SPARALLEL_STAT
      {
        $$ = list_create(AST_NIL);
      }
  | par_case_list par_case
      {
        $$ = list_insert_last($1, $2);
      };


par_case:
    SPARALLEL_STAT stat_list
      {
        $$ = gen_PARALLEL_CASE($1.part[0], AST_NIL, $2);
        setLineTags2($$, $1, UNUSED_ENDVAL);
      };


par_end_stat:
    SPAREND_STAT
      {
        $$.missing = false;
        $$.label   = $1.part[0];
        $$.conceal = $1.conceal;
      };




/************************/
/*  Parallel pid stat.	*/
/************************/


stat:
    par_pid_compound_stat;


par_pid_compound_stat:
    SPARBEGIN_PID_STAT stat_list par_pid_case_list par_end_stat
      {
        FortTreeNode label;

        label = $4.label;
        $$ = gen_PARALLEL($1.part[0], label, $1.part[1], $1.part[2], $2, $3);
        if( $4.missing ) 
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        setLineTags2($$, $1, $4);
      }
  | SPARBEGIN_PID_STAT stat_list par_pid_case_list
			SOTHER_PROCESSES_STAT stat_list par_end_stat
      {
        FortTreeNode label, last_case;

        label = $6.label;
        last_case = gen_PARALLEL_CASE($4.part[0], list_create(AST_NIL), $5);
        setLineTags2(last_case, $4, UNUSED_ENDVAL);
        $3 = list_insert_last($3, last_case);

        $$ = gen_PARALLEL($1.part[0], label, $1.part[1], $1.part[2], $2, $3);
        if( $6.missing ) 
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}

        setLineTags2($$, $1, $6);
      };


par_pid_case_list:
    /* empty */					%prec SPARALLEL_PID_STAT
      {
        $$ = list_create(AST_NIL);
      }
  | par_pid_case_list par_pid_case
      {
        $$ = list_insert_last($1, $2);
      };


par_pid_case:
    SPARALLEL_PID_STAT stat_list
      {
        $$ = gen_PARALLEL_CASE($1.part[0], $1.part[1], $2);
        setLineTags1($$, $1);
      };




/************************/
/*  Parallel loop stmt	*/
/************************/


stat:
    parallelloop_compound_stat;


parallelloop_compound_stat:
    SPARALLELLOOP_STAT stat_list parallel_loop_end_stat
      {
        FortTreeNode label;

        label = $3.label;
        $$ = gen_PARALLELLOOP($1.part[0], label, $1.part[1], $1.part[2], $2);
        if( $3.missing ) 
	{
	  ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}

        setLineTags2($$, $1, $3);
      }
  | SPARALLELLOOP_LABEL_STAT stat_list parallel_loop_end_stat
      {
        FortTreeNode label;

        label = ($3.missing || $3.label != IMPLIED)  ?  $3.label  :  AST_NIL;
        $$ = gen_PARALLELLOOP($1.part[0], label, $1.part[1], $1.part[2],
                                 $2);
        if( $3.missing )
	{
          ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
	}
        else if( $3.label != IMPLIED )
	{
          ft_SetParseErrorCode($$, ftt_WRONG_ENDBRACKET);
	}
        setLineTags2($$, $1, $3);
      };


parallel_loop_end_stat:
    SEND_LOOP_STAT
      {
        $$.missing = false;
        $$.label   = $1.part[0];
        $$.conceal = $1.conceal;
      }
  | SEND_DO_IMPLIED
      {
        $$.missing = false;
        $$.label   = IMPLIED;
        $$.conceal = $1.conceal;
      };




/************************/
/*  Where statement	*/				/* Fortran 90 */
/************************/
/* mimics if_compound_stat */

stat:
    where_block_stat;


where_block_stat:
    SWHERE_BLOCK_STAT stat_list if_guard_list where_end_stat
      {
        FortTreeNode label, first_guard, guard_list;

        label = $4.label;
        first_guard = gen_GUARD(AST_NIL, $1.part[1], $2);
        guard_list = list_insert_first($3, first_guard);

        $$ = gen_WHERE_BLOCK($1.part[0], label, guard_list);
        if( $4.missing )
          { 
	    ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }

        setLineTags2($$, $1, $4);
      }
  | SWHERE_BLOCK_STAT stat_list if_guard_list SELSE_WHERE_STAT stat_list where_end_stat
      {
        FortTreeNode label, first_guard, last_guard, guard_list;

        label = $6.label;
	guard_list = $3;

        first_guard = gen_GUARD(AST_NIL, $1.part[1], $2);
        guard_list  = list_insert_first(guard_list, first_guard);
        setLineTags2(first_guard, $1, $6);	/* $6 unused */

	last_guard  = gen_GUARD($4.part[0], $4.part[1], $5);
        setLineTags2(last_guard, $4, $6);  /* $6 unused */
        guard_list  = list_insert_last(guard_list, last_guard);

        $$ = gen_WHERE_BLOCK($1.part[0], label, guard_list);
        if( $6.missing )
          { 
	    ft_SetParseErrorCode($$, ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }
        /* awful kludge -- see 'setConceal2' */
          ft_SetConceal(ftt_fortTree, $$, 2, $6.conceal);

      };


where_end_stat:
    SEND_WHERE_STAT
      {
        $$.missing = false;
        $$.label   = $1.part[0];
        $$.conceal = $1.conceal;
      };






/********************************/
/*  Error productions		*/
/********************************/


stat:
    SERROR_STAT
      {
        $$ = $1.tree;
        setLineTags1($$, $1);
      }
  | do_misplaced
  | do_all_misplaced
  | if_misplaced
  | par_misplaced
  | par_pid_misplaced
  | parallel_loop_misplaced
  | where_misplaced;


do_misplaced:
    error SEND_DO_STAT
      {
        $$ = misplaced($2, "ENDDO", "DO");
      }
  | error SEND_DO_IMPLIED
      {
        $$ = misplaced($2, "labeled statement", "DO");
      };


do_end_stat:
    error /* empty */
      {
        $$ = missing("ENDDO");
      };


do_all_misplaced:
    error SEND_ALL_STAT
      {
        $$ = misplaced($2, "ENDALL", "DOALL");
      };
  /*** SEND_DO_IMPLIED not duplicated ***/


do_all_end_stat:
    error /* empty */
      {
        $$ = missing("ENDALL");
      };


if_misplaced:
    error SELSE_IF_STAT
      {
        $$ = misplaced($2, "ELSEIF", "IF");
      }
  | error SELSE_STAT
      {
        $$ = misplaced($2, "ELSE", "IF");
      }
  | error SEND_IF_STAT
      {
        $$ = misplaced($2, "ENDIF", "IF");
      };


if_end_stat:
    error /* empty */
      {
        $$ = missing("ENDIF");
      };


par_misplaced:
    error SPARALLEL_STAT
      {
        $$ = misplaced($2, "PARALLEL", "PARBEGIN");
      }
  | error SPAREND_STAT
      {
        $$ = misplaced($2, "PAREND", "PARBEGIN");
      };


par_pid_misplaced:
    error SPARALLEL_PID_STAT
      {
        $$ = misplaced($2, "PARALLEL", "PARBEGIN");
      }
  | error SOTHER_PROCESSES_STAT
      {
        $$ = misplaced($2, "OTHER PROCESSES:", "PARBEGIN");
      };
  /*** SPAREND_STAT not duplicated ***/


par_end_stat:
    error /* empty */
      {
        $$ = missing("PAREND");
      };


parallel_loop_misplaced:
    error SEND_LOOP_STAT
      {
        $$ = misplaced($2, "END LOOP", "PARALLEL LOOP");
      };
  /*** SEND_DO_IMPLIED not duplicated ***/


parallel_loop_end_stat:
    error /* empty */
      {
        $$ = missing("END LOOP");
      };


program_unit_end_stat:
    error /* empty */
      {
        $$ = missing("END");
      };

where_misplaced:
    error SELSE_WHERE_STAT
      {
        $$ = misplaced($2, "ELSEWHERE", "WHERE");
      }
  | error SEND_WHERE_STAT
      {
        $$ = misplaced($2, "ENDWHERE", "WHERE");
      };


where_end_stat:
    error /* empty */
      {
        $$ = missing("ENDWHERE");
      };







%%






/********************************/
/*  Local subroutines		*/
/********************************/




static
FortTreeNode misplaced(fx_StatToken st, char *what, char *prev)
  //fx_StatToken st;
  //char *what;
  //char *prev;
{
  char complaint[100];
  FortTreeNode commtext, err;

  yyerrok;
  commtext = gen_TEXT();
  (void) sprintf(complaint, "%s not matched with preceeding %s.", what, prev);
  gen_put_text(commtext, complaint, STR_COMMENT_TEXT);
  err = gen_ERROR(commtext, st.tree, st.part[0], st.part[1], st.part[2],
								st.part[3]);
  ft_SetParseErrorCode(err, (short) st.token);
  setLineTags1(err,st);
  return err;
}

/*ARGSUSED*/

static
fx_EndingStat missing(char *what)
  //char *what;
{
  fx_EndingStat endval;

  yyerrok;
  endval.missing = true;
  endval.label = AST_NIL;
  endval.conceal = 0;

  return endval;
}




static void
setLineTags1(FortTreeNode node, fx_StatToken st)
  // FortTreeNode node;
  // fx_StatToken st;
{
  ft_SetConceal(ftt_fortTree, node, 0, st.conceal);
  tt_setTagNode(ftt_textTree, node, st.tt_tag);
}




static void
setLineTags2(FortTreeNode node, fx_StatToken st, fx_EndingStat endval)
  // FortTreeNode node;
  // fx_StatToken st;
  // fx_EndingStat endval;
{
  ft_SetConceal(ftt_fortTree, node, 1, st.conceal);
  tt_setTagNode(ftt_textTree, node, st.tt_tag );

  ft_SetConceal(ftt_fortTree, node, 2, endval.conceal);
}

void yyerror(const char *s)
{
}
