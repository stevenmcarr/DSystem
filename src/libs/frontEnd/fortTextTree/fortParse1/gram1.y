/* $Id: gram1.y,v 1.12 2001/10/12 19:37:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortParse1/gram1.y					*/
/*									*/
/*	FortParse1/gram1 -- yacc for low-level Fortran parser		*/
/*									*/
/************************************************************************/




%{

#define gram1_h			/* will already have yacc-generated decls */

#include <string.h>
#include <libs/support/misc/general.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/fortParse1/FortParse1.i>
#include <libs/frontEnd/fortTextTree/fortParse1/lex1.h>

%}






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Static data		*/
/************************/




%{

static int		iostmt;
STATIC(int,		yylex, (void));
STATIC(FortTreeNode,	coerceToLabel, (FortTreeNode node));

%}





/************************/
/* Semantic values	*/
/************************/




%union  {
    FortTreeNode astptr;
    fx_StatToken statval;
    KWD_OPT  kwdval;
    Boolean  boolean;
    }






/************************************************************************/
/*	Statement Grammar	 					*/
/************************************************************************/




/************************/
/*  Terminal symbols	*/
/************************/



%token SARITHIF SASGOTO SASSIGN SAT SBACKSPACE SBARRIER SBLOCK SBLOCKDATA
%token SCALL SCHARACTER SCLEAR SCLOSE SCOMMENT SCOMMON SCOMPGOTO SCOMPLEX
%token SCONTINUE SCREATETASK SDATA SDEBUG SDIMENSION SDO SDOALL SDOALLWHILE
%token SDOUBLE SDOWHILE SELSE SELSEIF SEMPTY SEND SENDALL SENDDO SENDFILE
%token SENDIF SENDLOOP SENTRY SEOS SEQUIV SEVENT SEXACT SEXTERNAL SFORMAT
%token SFUNCTION SGOTO SIMPLICIT SIMPLICITNONE
%token SINC SINIT SINQUIRE SINTEGER SINTRINSIC SLET
%token SLOCK SLOGICAL SLOGIF SNAME SNAMEEQ SOPEN SOTHERPROCESSES SPARALLEL
%token SPARALLELLOOP SPARALLELLOOPWHILE SPARAMETER SPARBEGIN SPAREND SPAUSE
%token SPOST SPOSTING SPRINT SPRIVATE SPROGRAM SREAD SREAL SRETURN SREWIND
%token SSAVE SSEMAPHORE SSET SSTOP SSTOPLOOP SSUBCHK SSUBROUTINE SSUBTRACE
%token STASK STASKCOMMON STHEN STIMES STO STRACE STRACEOFF STRACEON SUNIT
%token SUNKNOWN SUNLOCK SUNTIL SVAL SWAIT SWHILE SWRITE

%token SDCON SFORMATSPEC SHEXCON SHOLLERITH SICON SRCON

%token SCOLON SCOMMA SCONCAT SEQUALS SLPAR SMINUS SPERCENT SPLUS SPOWER SRPAR
%token SSLASH SSTAR

%token SAND SEQ SEQV SFALSE SGE SGT SLE SLT SNE SNEQV SNOT SOR STRUE

%token SALLOCATABLE SALLOCATE SDEALLOCATE SWHERE SELSEWHERE SENDWHERE

%token SARG_PH SARRAY_DECL_LEN_PH SBOUND_PH SLEN_PH SCOMMON_ELT_PH
%token SCOMMON_VARS_PH SCONSTANT_PH SLOOP_CONTROL_PH SDATA_INIT_PH SDATA_PH
%token SDIM_PH SEQUIV_PH SFORMAL_PH SFORMAT_PH
%token SIMPLICIT_DEF_PH SLETTERS_PH SINIT_PH SINVOCATION_PH SKWD_PH
%token SSPECIFY_KWD_PH SQUERY_KWD_PH
%token SLABEL_PH SLETTER_PH SVAR_PH SNAME_PH SOPTION_PH SPARAM_DEF_PH
%token SPOSTING_PH SPOST_EXPR_PH SEXPR_PH SSTMT_PH SSUBPROGRAM_PH
%token SARITH_EXPR_PH SSTRING_EXPR_PH SRELATIONAL_EXPR_PH SLOGICAL_EXPR_PH
%token SSTRING_VAR_PH STEXT_PH STYPE_PH STYPENAME_PH SUNIT_PH
%token SSPECIFICATION_STMT_PH SCONTROL_STMT_PH SIO_STMT_PH SPARASCOPE_STMT_PH
%token SDEBUG_STMT_PH






/************************/
/*  Nonterminal symbols	*/
/************************/


/* Semantic types */

%type <statval> statement if_able_statement

%type <astptr>  expr_ph
%type <astptr>  function_args function_arg_list function_arg
%type <astptr>  subroutine_arguments subroutine_arg_list subroutine_arg
%type <astptr>  common_list common_elt common_block common_name common_var
%type <astptr>  data_elt_list data_elt data_lval_list data_lval
%type <astptr>  subscripted_array subscripted_array_list
%type <astptr>  data_implied_do data_rval_list data_rval data_repeat
%type <astptr>  data_const
%type <astptr>  alloc_object alloc_object_list dealloc_object_list
%type <astptr>  dimension_decl_list dimension_decl dimension_list
%type <astptr>  dimension_elt dimension_ubound
%type <astptr>  equivalence_class_list equivalence_class
%type <astptr>  implicit_elt_list implicit_elt implicit_letter_groups
%type <astptr>  implicit_letter_group implicit_letter
%type <astptr>  parameter_list parameter_elt
%type <astptr>  save_list save_elt
%type <astptr>  type_elt_list type_elt type_init_list type_dims type
%type <astptr>  type_name type_lengspec
%type <astptr>  call_invocation call_rval_list call_rval
%type <astptr>  do_control
%type <astptr>  io_bs_rew_end io_inq_open_close
%type <astptr>  io_control io_control_list io_control_elt io_control_value
%type <astptr>  io_input_list io_input_elt
%type <astptr>  io_output_list io_output_unparen_elt io_output_paren_elt
%type <astptr>  io_unparen_rval io_rval
%type <astptr>  debug_option_list debug_option
%type <astptr>  doall_control
%type <astptr>  post_posting post_value
%type <astptr>  task_posting
%type <astptr>	parallelloop_control
%type <astptr>  rval_list optional_rval optional_comma_rval rval unparen_rval
%type <astptr>  lval_list lval optional_const simple_const complex_const
%type <astptr>  substring subscript subscript_elt subscript_elt_list
%type <astptr>  stmt_label label_list label
%type <astptr>  name_list optional_name name

%type <kwdval>  io_control_kwd
%type <boolean> optional_comma
/* none: 	io_read io_write io_print stmt_no_label			*/
/* 		intolyon intonlyoff iocontrolon iocontroloff		*/
/*		needkwdon needkwdoff					*/



/* Precedence and associativity */

%left SCOMMA
%nonassoc SCOLON
%right SEQUALS
%left SEQV SNEQV
%left SOR
%left SAND
%left SNOT
%nonassoc SLT SGT SLE SGE SEQ SNE
%left SCONCAT
%left SPLUS SMINUS
%left SSTAR SSLASH
%right SPOWER

%%




/****************************************/
/*  Top level rule			*/
/****************************************/


line:
    statement SEOS
      {
        fp1_token = $1;
	switch ($1.token)
	{
	  case SFUNCTION_STAT:
	    treeStackDrop(4);
            break;

	  case SSUBROUTINE_STAT:
	  case SDO_STAT:
          case SDO_LABEL_STAT:
	  case SDOALL_STAT:
          case SDOALL_LABEL_STAT:
	  case SPARBEGIN_PID_STAT:
          case SPARALLELLOOP_STAT:
          case SPARALLELLOOP_LABEL_STAT:
	    treeStackDrop(3);
            break;

	  case SBLOCK_DATA_STAT:
	  case SPROGRAM_STAT:
	  case SIF_STAT:
          case SELSE_IF_STAT:
          case SELSE_STAT:
          case SDEBUG_STAT:
	  case SPARALLEL_PID_STAT:
          case SWHERE_BLOCK_STAT:
          case SELSE_WHERE_STAT:
	    treeStackDrop(2);
	    break;

	  default:
	  case SPRSCOPE_PH_STAT:
	  case SSTMT_PH_STAT:
	  case SEND_STAT:
          case SEND_DO_STAT:
          case SEND_IF_STAT:
	  case SPARBEGIN_STAT:
	  case SPARALLEL_STAT:
	  case SOTHER_PROCESSES_STAT:
	  case SPAREND_STAT:
          case SEND_LOOP_STAT:
          case SEND_WHERE_STAT:
	    treeStackDrop(1);
	    break;

	  case SERROR_STAT:
            break;
	}
      };








/********************************/
/*  Subprogram statements	*/
/********************************/




/************************/
/*  Block data statment */
/************************/


statement:
    SBLOCKDATA stmt_label optional_name
      {
        if( $3 == AST_NIL )
          { $3 = gen_IDENTIFIER();
            gen_put_text($3, "DATA", STR_IDENTIFIER);
            ft_SetShow($3, false);		/* shorthand */
          }

        $$.token   = SBLOCK_DATA_STAT;
        $$.part[0] = $2;
        $$.part[1] = $3;
      };




/************************/
/*  End statement	*/
/************************/


statement:
    SEND stmt_label
      {
        $$.token   = SEND_STAT;
        $$.part[0] = $2;
      };




/************************/
/*  Entry statement	*/
/************************/


statement:
    SENTRY stmt_label name subroutine_arguments
      {
        $$.token = SENTRY_STAT;
        $$.tree  = gen_ENTRY($2, $3, $4);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };




/************************/
/*  Function statement	*/
/************************/


statement:
    SFUNCTION stmt_label name function_args
      {
        $$.token   = SFUNCTION_STAT;
	$$.part[0] = $2;
        $$.part[1] = AST_NIL;
        $$.part[2] = $3;
        $$.part[3] = $4;
	treeStackPush($$.part[1]);
      }
  | type SFUNCTION stmt_label name function_args
      {
        $$.token   = SFUNCTION_STAT;
	$$.part[0] = $3;
        $$.part[1] = $1;
        $$.part[2] = $4;
        $$.part[3] = $5;
      };


function_args:
    SLPAR SRPAR
      {
        $$ = list_create(AST_NIL);
	treeStackPush($$);
      }
  | SLPAR function_arg_list SRPAR
      {
        $$ = $2;
      };


function_arg_list:
    function_arg
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | function_arg_list SCOMMA function_arg
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


function_arg:
    name
  | SFORMAL_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_formal);
	treeStackPush($$);
      };




/************************/
/*  Placeholder stat.	*/
/************************/


statement:
    SSUBPROGRAM_PH stmt_no_label			/* PLACEHOLDER */
      {
        $$.token   = SPRSCOPE_PH_STAT;
        $$.part[0] = ph_from_mtype(GEN_subprogram);
	treeStackPush($$.part[0]);
      };




/************************/
/*  Program statement	*/
/************************/


statement:
    SPROGRAM stmt_label optional_name
      {
        if( $3 == AST_NIL )
          { $3 = gen_IDENTIFIER();
            gen_put_text($3, "MAIN", STR_IDENTIFIER);
            ft_SetShow($3, false);		/* shorthand */
          }

        $$.token   = SPROGRAM_STAT;
        $$.part[0] = $2;
        $$.part[1] = $3;
      };




/************************/
/*  Subroutine stat.    */
/************************/


statement:
    SSUBROUTINE stmt_label name subroutine_arguments
      {
        $$.token   = SSUBROUTINE_STAT;
        $$.part[0] = $2;
        $$.part[1] = $3;
        $$.part[2] = $4;
      };


subroutine_arguments:
    /* empty */
      {
        $$ = AST_NIL;
	treeStackPush($$);
      }
  | SLPAR SRPAR
      {
        $$ = list_create(AST_NIL);
	treeStackPush($$);
      }
  | SLPAR subroutine_arg_list SRPAR
      {
        $$ = $2;
      };


subroutine_arg_list:
    subroutine_arg
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | subroutine_arg_list SCOMMA subroutine_arg
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


subroutine_arg:
    name
  | SSTAR
      {
        $$ = gen_STAR();
	treeStackPush($$);
      }
  | SFORMAL_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_formal);
	treeStackPush($$);
      };






/********************************/
/*  Non-executable statements	*/
/********************************/




/************************/
/*  Comment statement	*/
/************************/


statement:
    SCOMMENT
      {
        FortTreeNode comment_text_node;
        char * line;
        int len;

        line = lx1_GetLine(&len);
        while ((*line == ' ') && (len > 0))
        {
	  line++;
          len--;
        }
        while ((line[len - 1] == ' ') && (len > 0))
        {
          len--;
        }
        line[len] = '\0';

        comment_text_node = gen_TEXT();
        gen_put_text(comment_text_node, line, STR_COMMENT_TEXT);
        $$.token = SCOMMENT_STAT;
        $$.tree  = gen_COMMENT(comment_text_node);
	treeStackPush($$.tree);
      }
  | SEMPTY stmt_no_label
      {
        $$.token = SCOMMENT_STAT;
        $$.tree  = gen_COMMENT(AST_NIL);
	treeStackPush($$.tree);
      };




/************************/
/*  Common statement	*/
/************************/


statement:
    SCOMMON stmt_label common_list
      {
        $$.token = SCOMMON_STAT;
        $$.tree  = gen_COMMON($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


/* These are not the straight-forward grammar productions because of 	*/
/* the optional comma before common block names.  Allowing the comma in	*/
/* the obvious way creates an ambiguous grammar (two levels of comma	*/
/* separated lists).  To get around this problem, we 'flatten' the list	*/
/* and treat common_elt as anything that can appear between two commas.	*/

common_list:
    common_elt
  | common_list SCOMMA common_elt
      {
        FortTreeNode dead, new_vars, list;
        Boolean comma;

	if( NOT(ft_GetShow(gen_COMMON_ELT_get_name(list_first($3)))) )
          {
            /* The first COMMON_ELT of $3 contains a partial variable list. */
	    /* Remove the COMMON_ELT (dead) from $3 and delete it except    */
	    /* for its variable list (var_list).  Append var_list to the    */
            /* variable list of the last COMMON_ELT of $1 (list).	    */
              dead = list_remove_first($3);
	      comma = ft_GetComma(dead);
	      new_vars = gen_COMMON_ELT_get_common_vars_LIST(dead);
	      tree_replace(new_vars, AST_NIL);
	      tree_free(dead);
              list = gen_COMMON_ELT_get_common_vars_LIST(list_last($1));
	      list = list_append(list, new_vars);
              gen_COMMON_ELT_put_common_vars_LIST(list_last($1), list);
	      ft_SetComma(list_last($1), comma);
          }
        $$ = list_append($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


common_elt:
    common_var
      {
        FortTreeNode block;

        block = gen_IDENTIFIER();
	gen_put_text(block, "//", STR_COMMON_NAME);
        ft_SetShow(block, false);
	$$ = list_create(gen_COMMON_ELT(block, list_create($1)));
	treeStackDrop(1);
	treeStackPush($$);
      }
  | common_block common_var
      {
	$$ = list_create(gen_COMMON_ELT($1, list_create($2)));
	treeStackDrop(2);
	treeStackPush($$);
      }
  | common_var common_block common_var
      {
        FortTreeNode block, first, list;

        block = gen_IDENTIFIER();
	gen_put_text(block, "//", STR_COMMON_NAME);
        ft_SetShow(block, false);
        first = gen_COMMON_ELT(block, list_create($1));
	ft_SetComma(first, false);	/* no comma */
	list = list_create(gen_COMMON_ELT($2, list_create($3)));
	$$ = list_insert_first(list, first);
	treeStackDrop(3);
	treeStackPush($$);
      }
  | SCOMMON_ELT_PH					/* PLACEHOLDER */
      {
	$$ = list_create(ph_from_mtype(GEN_common_elt));
	treeStackPush($$);
      };


common_block:
    SCONCAT
      {
        $$ = gen_IDENTIFIER();
	gen_put_text($$, "//", STR_COMMON_NAME);
	treeStackPush($$);
      }
  | SSLASH common_name SSLASH
      {
        $$ = $2;
      };


common_name:
    SNAME
      {
        char *name;

	name = (char *) get_mem(lx1_TokenLen + 3, "gram1.y (common name)");
        (void) strcpy(name + 1, lx1_Token);
	name[0] = name[lx1_TokenLen + 1] = '/';
	name[lx1_TokenLen + 2] = '\0';

        $$ = gen_IDENTIFIER();
	gen_put_text($$, name, STR_COMMON_NAME);
	free_mem((void *) name);
	treeStackPush($$);
      }
  | SNAME_PH						/* PLACEHOLDER */
      {
	$$ = ph_from_mtype(GEN_name);
	treeStackPush($$);
      };


common_var:
    name
      {
        $$ = gen_ARRAY_DECL_LEN($1, AST_NIL, AST_NIL, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | name SLPAR dimension_list SRPAR
      {
        $$ = gen_ARRAY_DECL_LEN($1, AST_NIL, $3, AST_NIL);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SCOMMON_VARS_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_common_vars);
	treeStackPush($$);
      };




/************************/
/*  Data statement	*/
/************************/


statement:
    SDATA stmt_label data_elt_list
      {
        $$.token = SDATA_STAT;
        $$.tree  = gen_DATA($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


data_elt_list:
    data_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | data_elt_list optional_comma data_elt
      {
        ft_SetComma(list_last($1), $2);
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


data_elt:
    data_lval_list SSLASH data_rval_list SSLASH
      {
        $$ = gen_DATA_ELT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SDATA_INIT_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_data_init);
	treeStackPush($$);
      };


data_lval_list:
    data_lval
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | data_lval_list SCOMMA data_lval
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


data_lval:
    lval
  | data_implied_do
  | SDATA_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_data);
	treeStackPush($$);
      }
  | expr_ph;						/* PLACEHOLDER */


data_implied_do:
    SLPAR data_lval_list SCOMMA name SEQUALS
			rval SCOMMA rval optional_comma_rval SRPAR
      {
        $$ = gen_IMPLIED_DO($2, $4, $6, $8, $9);
	treeStackDrop(5);
	treeStackPush($$);
      };


data_rval_list:
    data_rval
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | data_rval_list SCOMMA data_rval
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


data_rval:
    data_repeat SSTAR data_const
      {
        $$ = gen_REPEAT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | data_const
  | SINIT_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_init);
	treeStackPush($$);
      };


data_repeat:
    SICON
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, lx1_Token, STR_CONSTANT_INTEGER);
	treeStackPush($$);
      }
  | name
  | expr_ph;						/* PLACEHOLDER */
    

data_const:
    SMINUS data_const
      {
        $$ = gen_UNARY_MINUS($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SPLUS data_const
      {
        $$ = $2;
      }
  | simple_const
  | complex_const
  | name;


/************************/
/*  Dimension statement	*/
/************************/


statement:
    SDIMENSION stmt_label dimension_decl_list
      {
        $$.token = SDIMENSION_STAT;
        $$.tree  = gen_DIMENSION($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


dimension_decl_list:
    dimension_decl
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | dimension_decl_list SCOMMA dimension_decl
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


dimension_decl:
    name SLPAR dimension_list SRPAR
      {
        $$ = gen_ARRAY_DECL_LEN($1, AST_NIL, $3, AST_NIL);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SARRAY_DECL_LEN_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_array_decl_len);
	treeStackPush($$);
      };
    

dimension_list:
    dimension_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | dimension_list SCOMMA dimension_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


dimension_elt:
    dimension_ubound
      {
        $$ = gen_DIM(AST_NIL, $1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | rval SCOLON dimension_ubound
      {
        $$ = gen_DIM($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SCOLON 
      {
        $$ = gen_DIM(AST_NIL, AST_NIL);
	treeStackPush($$);
      }
  | SDIM_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_dim);
	treeStackPush($$);
      };


dimension_ubound:
    SSTAR
      {
        $$ = gen_STAR();
	treeStackPush($$);
      }
  | rval
  | SBOUND_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_bound);
	treeStackPush($$);
      };




/**************************/
/*  Equivalence statement */
/**************************/


statement:
    SEQUIV stmt_label equivalence_class_list
      {
        $$.token = SEQUIVALENCE_STAT;
        $$.tree  = gen_EQUIVALENCE($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


equivalence_class_list:
    equivalence_class
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | equivalence_class_list SCOMMA equivalence_class
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


equivalence_class:
    SLPAR lval_list SRPAR
      {
        $$ = gen_EQUIV_ELT($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SEQUIV_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_equiv);
	treeStackPush($$);
      };




/************************/
/*  External statement	*/
/************************/


statement:
    SEXTERNAL stmt_label name_list
      {
        $$.token = SEXTERNAL_STAT;
        $$.tree  = gen_EXTERNAL($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Format statement	*/
/************************/


statement:
    SFORMAT stmt_label SFORMATSPEC
      {
        FortTreeNode text;

        if ($2 == AST_NIL)
          { yyerror("Format statments must have statment labels.");
            YYERROR;
          }

	text = gen_TEXT();
	gen_put_text(text, lx1_Token, STR_FORMAT_STRING);
        $$.token = SFORMAT_STAT;
        $$.tree  = gen_FORMAT($2, text);
	treeStackDrop(1);
	treeStackPush($$.tree);
      }
  | SFORMAT stmt_label STEXT_PH				/* PLACEHOLDER */
      {
        $$.token = SFORMAT_STAT;
        $$.tree  = gen_FORMAT($2, ph_from_mtype(GEN_text));
	treeStackDrop(1);
	treeStackPush($$.tree);
      };




/************************/
/*  Implicit statement	*/
/************************/


statement:
    SIMPLICITNONE stmt_label
      {
        $$.token = SIMPLICIT_STAT;
        $$.tree  = gen_IMPLICIT($2, list_create(gen_NONE()));
	treeStackDrop(1);
	treeStackPush($$.tree);
      }
  |  SIMPLICIT stmt_label implicit_elt_list
      {
        $$.token = SIMPLICIT_STAT;
        $$.tree  = gen_IMPLICIT($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


implicit_elt_list:
    needkwdon implicit_elt needkwdoff
      {
        $$ = list_create($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | implicit_elt_list SCOMMA needkwdon implicit_elt needkwdoff
      {
        $$ = list_insert_last($1, $4);
	treeStackDrop(2);
	treeStackPush($$);
      };


implicit_elt:
    type SLPAR implicit_letter_groups SRPAR
      {
        $$ = gen_IMPLICIT_ELT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SIMPLICIT_DEF_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_implicit_def);
	treeStackPush($$);
      };


implicit_letter_groups:
    implicit_letter_group
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | implicit_letter_groups SCOMMA implicit_letter_group
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


implicit_letter_group:
    implicit_letter
  | implicit_letter SMINUS implicit_letter
      {
        char *start_range, *end_range;

        if( ! is_place_holder($1)  &&  ! is_place_holder($3) )
          { start_range = gen_get_text($1);
            end_range   = gen_get_text($3);
            if( *start_range == '$'  &&  *end_range != '$' )
              { yyerror("Backwards implicit letter range.");
	        YYERROR;
                /* NOTREACHED */
              }
            else if( *end_range != '$'  &&  *start_range > *end_range )
              { yyerror("Backwards implicit letter range.");
	        YYERROR;
                /* NOTREACHED */
	      }
          }

	$$ = gen_IMPLICIT_PAIR($1, $3);
        treeStackDrop(2);
	treeStackPush($$);
      }
  | SLETTERS_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_letters);
	treeStackPush($$);
      };


implicit_letter:
    SNAME
      {
	char c = lx1_Token[0];

        if ( lx1_TokenLen != 1 || ((c < 'a' || c > 'z') && c != '$') )
        { yyerror("Implicits must be single letters or ranges of letters.");
	  YYERROR;
        }

        $$ = gen_LETTER();
	gen_put_text($$, lx1_Token, STR_CONSTANT_LETTER);
	treeStackPush($$);
      }
  | SLETTER_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_letter);
	treeStackPush($$);
      };




/************************/
/*  Intrinsic statement	*/
/************************/


statement:
    SINTRINSIC stmt_label name_list
      {
        $$.token = SINTRINSIC_STAT;
        $$.tree  = gen_INTRINSIC($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Parameter statement	*/
/************************/


statement:
    SPARAMETER stmt_label SLPAR parameter_list SRPAR
      {
        $$.token = SPARAMETER_STAT;
        $$.tree  = gen_PARAMETER($2, $4);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


parameter_list:
    parameter_elt
      {
	$$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | parameter_list SCOMMA parameter_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


parameter_elt:
    name SEQUALS rval
      {
        $$ = gen_PARAM_ELT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SPARAM_DEF_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_param_def);
	treeStackPush($$);
      };




/************************/
/*  Save statement	*/
/************************/


statement:
    SSAVE stmt_label
      {
        $$.token = SSAVE_STAT;
        $$.tree  = gen_SAVE($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$.tree);
      }
  | SSAVE stmt_label save_list
      {
        $$.token = SSAVE_STAT;
        $$.tree  = gen_SAVE($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


save_list:
    save_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | save_list SCOMMA save_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


save_elt:
    name
  | common_block;




/************************/
/*  Type statement	*/
/************************/


statement:
    type stmt_label optional_comma type_elt_list
      {
        $$.token = STYPE_STATEMENT_STAT;
        $$.tree  = gen_TYPE_STATEMENT($2, $1, $4);
	ft_SetComma($$.tree, $3);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };


type_elt_list:
    type_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | type_elt_list SCOMMA type_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


type_elt:
    name type_dims type_lengspec type_init_list
      {
        $$ = gen_ARRAY_DECL_LEN($1, $3, $2, $4);
	treeStackDrop(4);
	treeStackPush($$);
      }
  | SARRAY_DECL_LEN_PH					/* PLACEHOLDER */
      {
	$$ = ph_from_mtype(GEN_array_decl_len);
	treeStackPush($$);
      };


type_init_list:						/* VS FORTRAN */
    /* empty */
      {
	$$ = AST_NIL;
	treeStackPush($$);
      }
  | SSLASH data_rval_list SSLASH
      {
	$$ = $2;
      }
  | SINIT_PH						/* PLACEHOLDER */
      {
        $$ = list_create(ph_from_mtype(GEN_init));
	treeStackPush($$);
      };


type_dims:
    /* empty */
      {
        $$ = AST_NIL;
	treeStackPush($$);
      }
  | SLPAR dimension_list SRPAR
      {
        $$ = $2;
      };

type:							/* VS FORTRAN */
    type_name type_lengspec
      {
        $$ = gen_TYPE_LEN($1, $2);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | STYPE_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_type);
	treeStackPush($$);
      };


type_name:
    SINTEGER
      {
        $$ = gen_INTEGER();
	treeStackPush($$);
      }
  | SREAL
      {
        $$ = gen_REAL();
	treeStackPush($$);
      }
  | SCHARACTER
      {
        $$ = gen_CHARACTER();
	treeStackPush($$);
      }
  | SCOMPLEX
      {
        $$ = gen_COMPLEX();
	treeStackPush($$);
      }
  | SDOUBLE
      {
        $$ = gen_DOUBLE_PRECISION();
	treeStackPush($$);
      }
  | SLOGICAL
      {
        $$ = gen_LOGICAL();
	treeStackPush($$);
      }
  | SEXACT						/* RN */
      {
        $$ = gen_EXACT();
	treeStackPush($$);
      }
  | SSEMAPHORE						/* PARASCOPE */
      {
        $$ = gen_SEMAPHORE();
	treeStackPush($$);
      }
  | SEVENT						/* PARASCOPE */
      {
        $$ = gen_EVENT();
	treeStackPush($$);
      }
  | SBARRIER						/* PARASCOPE */
      {
        $$ = gen_BARRIER();
	treeStackPush($$);
      }
  | STYPENAME_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_type_name);
	treeStackPush($$);
      };


type_lengspec:
    /* empty */
      {
        $$ = AST_NIL;
	treeStackPush($$);
      }
  | SSTAR intonlyon simple_const intonlyoff
      {
        $$ = $3;
      }
  | SSTAR intonlyon SLPAR rval SRPAR intonlyoff
      {
        $$ = $4;
        gen_put_parens($$, 1);
      }
  | SSTAR intonlyon SLPAR SSTAR SRPAR intonlyoff
      {
        $$ = gen_STAR();
        gen_put_parens($$, 1);
	treeStackPush($$);
      }
  | SSTAR intonlyon SLEN_PH intonlyoff			/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_len);
	treeStackPush($$);
      };






/********************************/
/*  Executable statements	*/
/********************************/




/************************/
/*  Assign statement	*/
/************************/


if_able_statement:
    SASSIGN stmt_label label STO name
      {
        $$.token = SASSIGN_STAT;
        $$.tree  = gen_ASSIGN($2, $3, $5);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };




/************************/
/*  Assignment stat.    */
/************************/


if_able_statement:
    SLET stmt_label lval SEQUALS rval
      {
        $$.token = SASSIGNMENT_STAT;
        $$.tree  = gen_ASSIGNMENT($2, $3, $5);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };




/************************/
/*  Call statement	*/
/************************/


if_able_statement:
    SCALL stmt_label call_invocation
      {
        $$.token = SCALL_STAT;
        $$.tree  = gen_CALL($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


call_invocation:
    name
      {
        $$ = gen_INVOCATION($1, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | name SLPAR SRPAR
      {
        $$ = gen_INVOCATION($1, list_create(AST_NIL));
	treeStackDrop(1);
	treeStackPush($$);
      }
  | name SLPAR call_rval_list SRPAR
      {
        $$ = gen_INVOCATION($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SINVOCATION_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_invocation);
	treeStackPush($$);
      };


call_rval_list:
    call_rval
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | call_rval_list SCOMMA call_rval
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


call_rval:
    rval
  | SSTAR label
      {
        $$ = gen_RETURN_LABEL($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SPERCENT needkwdon SVAL needkwdoff SLPAR rval SRPAR	/* PARASCOPE */
      {
        $$ = gen_VALUE_PARAMETER($6);
        treeStackDrop(1);
	treeStackPush($$);
      }
  | SARG_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_arg);
	treeStackPush($$);
      };




/************************/
/*  Continue statment	*/
/************************/


if_able_statement:
    SCONTINUE stmt_label
      {
        $$.token = SCONTINUE_STAT;
        $$.tree  = gen_CONTINUE($2);
	treeStackDrop(1);
	treeStackPush($$.tree);
      };




/************************/
/*  Do statements	*/
/************************/


statement:
    SDO stmt_label do_control				/* RN */
      {
        $$.token = SDO_STAT;
	$$.part[0] = $2;
	$$.part[1] = AST_NIL;
        $$.part[2] = $3;
	treeStackPush($$.part[1]);
      }
  | SDO stmt_label label optional_comma do_control
      {
        $$.token = SDO_LABEL_STAT;
	$$.part[0] = $2;
	$$.part[1] = $3;
        $$.part[2] = $5;
        ft_SetComma($$.part[1], $4);
      }
  | SENDDO stmt_label					/* RN */
      {
        $$.token = SEND_DO_STAT;
	$$.part[0] = $2;
      };


do_control:
    SLPAR rval SRPAR needkwdon STIMES needkwdoff	/* RN */
      {
        $$ = gen_REPETITIVE($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SWHILE SLPAR rval SRPAR				/* RN */
      {
        $$ = gen_CONDITIONAL($3);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | name SEQUALS rval SCOMMA rval optional_comma_rval
      {
        $$ = gen_INDUCTIVE($1, $3, $5, $6);
	treeStackDrop(4);
	treeStackPush($$);
      }
  | SLOOP_CONTROL_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_loop_control);
	treeStackPush($$);
      };




/************************/
/*  Goto statements	*/
/************************/


if_able_statement:
    SGOTO stmt_label label
      {
        $$.token = SGOTO_STAT;
        $$.tree  = gen_GOTO($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | SGOTO stmt_label name	/* really 'SASGOTO' but scanner gets it wrong */
      {
        $$.token = SASSIGNED_GOTO_STAT;
        $$.tree  = gen_ASSIGNED_GOTO($2, $3, AST_NIL);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | SASGOTO stmt_label name optional_comma SLPAR label_list SRPAR
      {
        $$.token = SASSIGNED_GOTO_STAT;
        $$.tree  = gen_ASSIGNED_GOTO($2, $3, $6);
        ft_SetComma($$.tree, $4);
	treeStackDrop(3);
	treeStackPush($$.tree);
      }
  | SCOMPGOTO stmt_label SLPAR label_list SRPAR optional_comma rval
      {
        $$.token = SCOMPUTED_GOTO_STAT;
        $$.tree  = gen_COMPUTED_GOTO($2, $4, $7);
        ft_SetComma($$.tree, $6);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };




/************************/
/*  If statements	*/
/************************/


statement:
    if_able_statement
  | SLOGIF stmt_label SLPAR rval SRPAR if_able_statement
      {
        $$.token   = SLOGICAL_IF_STAT;
        if( $6.token == SSTMT_PH_STAT )
          $$.tree    = gen_LOGICAL_IF($2, $4, list_create($6.part[0]));
        else
          $$.tree    = gen_LOGICAL_IF($2, $4, list_create($6.tree));
	treeStackDrop(3);
	treeStackPush($$.tree);
      }
  | SLOGIF stmt_label SLPAR rval SRPAR STHEN
      {
        $$.token   = SIF_STAT;
        $$.part[0] = $2;
        $$.part[1] = $4;
      }
  | SELSEIF stmt_label SLPAR rval SRPAR STHEN
      {
        $$.token   = SELSE_IF_STAT;
	$$.part[0] = $2;
        $$.part[1] = $4;
      }
  | SELSE stmt_label
      {
        $$.token   = SELSE_STAT;
	$$.part[0] = $2;
	$$.part[1] = AST_NIL;
	treeStackPush($$.part[1]);
      }
  | SENDIF stmt_label
      {
        $$.token   = SEND_IF_STAT;
	$$.part[0] = $2;
      };


if_able_statement:
    SARITHIF stmt_label SLPAR rval SRPAR label SCOMMA label SCOMMA label
      {
        $$.token = SARITHMETIC_IF_STAT;
        $$.tree  = gen_ARITHMETIC_IF($2, $4, $6, $8, $10);
	treeStackDrop(5);
	treeStackPush($$.tree);
      };




/************************/
/*  I/O statements	*/
/************************/


if_able_statement:
    io_bs_rew_end io_control iocontroloff
      {
        /* ASSERT kwd list of $1 is AST_NIL */
          switch(iostmt)
          {
            case SBACKSPACE_LONG_STAT:
              gen_BACKSPACE_LONG_put_kwd_LIST($1, $2);
	      break;
            case SREWIND_LONG_STAT:
              gen_REWIND_LONG_put_kwd_LIST($1, $2);
	      break;
            case SENDFILE_LONG_STAT:
              gen_ENDFILE_LONG_put_kwd_LIST($1, $2);
	      break;
          }

	$$.token = iostmt;
        $$.tree  = $1;
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | io_bs_rew_end io_unparen_rval iocontroloff
      {
        FortTreeNode label;

	switch(iostmt)
        {
          case SBACKSPACE_LONG_STAT:
            label = gen_BACKSPACE_LONG_get_lbl_def($1);
	    if( label != AST_NIL )
              tree_replace(label, AST_NIL);
	    tree_free($1);
	    $$.token = SBACKSPACE_SHORT_STAT;
            $$.tree  = gen_BACKSPACE_SHORT(label, $2);
            break;
	  case SREWIND_LONG_STAT:
            label = gen_REWIND_LONG_get_lbl_def($1);
	    if( label != AST_NIL )
              tree_replace(label, AST_NIL);
	    tree_free($1);
	    $$.token = SREWIND_SHORT_STAT;
            $$.tree  = gen_REWIND_SHORT(label, $2);
            break;
	  case SENDFILE_LONG_STAT:
            label = gen_ENDFILE_LONG_get_lbl_def($1);
	    if( label != AST_NIL )
              tree_replace(label, AST_NIL);
	    tree_free($1);
	    $$.token = SENDFILE_SHORT_STAT;
            $$.tree  = gen_ENDFILE_SHORT(label, $2);
            break;
        }
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | io_inq_open_close io_control iocontroloff
      {
        /* ASSERT kwd list of $1 is AST_NIL */
          switch(iostmt)
          {
            case SINQUIRE_STAT:
              gen_INQUIRE_put_kwd_LIST($1, $2);
	      break;
            case SOPEN_STAT:
              gen_OPEN_put_kwd_LIST($1, $2);
	      break;
            case SCLOSE_STAT:
              gen_CLOSE_put_kwd_LIST($1, $2);
	      break;
          }

	$$.token = iostmt;
        $$.tree  = $1;
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | io_read stmt_label io_control iocontroloff
      {
	$$.token = SREAD_LONG_STAT;
        $$.tree  = gen_READ_LONG($2, $3, AST_NIL);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | io_read stmt_label io_control io_input_list iocontroloff
      {
	$$.token = SREAD_LONG_STAT;
        $$.tree  = gen_READ_LONG($2, $3, $4);
	treeStackDrop(3);
	treeStackPush($$.tree);
      }
  | io_read stmt_label io_unparen_rval iocontroloff
      {
	$$.token = SREAD_SHORT_STAT;
        $$.tree  = gen_READ_SHORT($2, coerceToLabel($3), AST_NIL);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | io_read stmt_label io_unparen_rval iocontroloff SCOMMA io_input_list
      {
	$$.token = SREAD_SHORT_STAT;
	$$.tree  = gen_READ_SHORT($2, coerceToLabel($3), $6);
	treeStackDrop(3);
	treeStackPush($$.tree);
      }
  | io_write stmt_label io_control iocontroloff
      {
	$$.token = SWRITE_STAT;
        $$.tree  = gen_WRITE($2, $3, AST_NIL);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | io_write stmt_label io_control io_output_list iocontroloff
      {
	$$.token = SWRITE_STAT;
        $$.tree  = gen_WRITE($2, $3, $4);
	treeStackDrop(3);
	treeStackPush($$.tree);
      }
  | io_print stmt_label io_rval iocontroloff
      {
	$$.token = SPRINT_STAT;
        $$.tree  = gen_PRINT($2, coerceToLabel($3), AST_NIL);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | io_print stmt_label io_rval SCOMMA io_output_list iocontroloff
      {
	$$.token = SPRINT_STAT;
        $$.tree  = gen_PRINT($2, coerceToLabel($3), $5);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };


io_bs_rew_end:
    SBACKSPACE stmt_label iocontrolon
      {
        iostmt = SBACKSPACE_LONG_STAT;
        $$ = gen_BACKSPACE_LONG($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SREWIND stmt_label iocontrolon
      {
        iostmt = SREWIND_LONG_STAT;
        $$ = gen_REWIND_LONG($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SENDFILE stmt_label iocontrolon
      {
        iostmt = SENDFILE_LONG_STAT;
        $$ = gen_ENDFILE_LONG($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      };


io_inq_open_close:
    SINQUIRE stmt_label iocontrolon
      {
        iostmt = SINQUIRE_STAT;
        $$ = gen_INQUIRE($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SOPEN stmt_label iocontrolon
      {
        iostmt = SOPEN_STAT;
        $$ = gen_OPEN($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SCLOSE stmt_label iocontrolon
      {
        iostmt = SCLOSE_STAT;
        $$ = gen_CLOSE($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$);
      };


io_read:
    SREAD iocontrolon
      {
        iostmt = SREAD_LONG_STAT;
      };


io_write:
    SWRITE iocontrolon
      {
        iostmt = SWRITE_STAT;
      };


io_print:
    SPRINT iocontrolon
      {
        iostmt = SPRINT_STAT;
      };


io_control:
    SLPAR io_control_list SRPAR
      {
        $$ = $2;
      }
  | SLPAR io_rval SRPAR
      {
        FortTreeNode unit;

	unit = gen_UNIT_SPECIFY($2);
	ft_SetShow(unit, false);		/* shorthand */
        $$ = list_create(unit);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SLPAR io_rval SCOMMA io_rval SRPAR
      {
        FortTreeNode unit, fmt;

	unit = gen_UNIT_SPECIFY($2);
	ft_SetShow(unit, false);		/* shorthand */
	fmt  = gen_FMT_SPECIFY(coerceToLabel($4));
	ft_SetShow(fmt, false);			/* shorthand */
        $$ = list_insert_last(list_create(unit), fmt);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SLPAR io_rval SCOMMA io_control_list SRPAR
      {
        FortTreeNode unit;

	unit = gen_UNIT_SPECIFY($2);
	ft_SetShow(unit, false);		/* shorthand */
        $$ = list_insert_first($4, unit);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SLPAR io_rval SCOMMA io_rval SCOMMA io_control_list SRPAR
      {
        FortTreeNode unit, fmt;

	unit = gen_UNIT_SPECIFY($2);
	ft_SetShow(unit, false);		/* shorthand */
	fmt  = gen_FMT_SPECIFY(coerceToLabel($4));
	ft_SetShow(fmt, false);			/* shorthand */
        $$ = list_insert_first(list_insert_first($6, fmt), unit);
	treeStackDrop(3);
	treeStackPush($$);
      };


io_control_list:
    io_control_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | io_control_list SCOMMA io_control_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


io_control_elt:
    SPOWER
      {
        $$ = gen_STAR();
	treeStackPush($$);
      }
  | io_control_kwd io_control_value
      {
        switch ($1)
        { case IOUNIT:
            $$ = gen_UNIT_SPECIFY($2);
            break;
          case IOFMT:
            $$ = gen_FMT_SPECIFY(coerceToLabel($2));
            break;
          case IOERR:
            $$ = gen_ERR_SPECIFY(coerceToLabel($2));
            break;
          case IOEND:
            $$ = gen_END_SPECIFY(coerceToLabel($2));
            break;
          case IOREC:
            $$ = gen_REC_SPECIFY($2);
            break;
          case IORECL:
            if (iostmt == SINQUIRE_STAT)
              $$ = gen_RECL_QUERY($2);
            else
              $$ = gen_RECL_SPECIFY($2);
            break;
          case IOFILE:
            $$ = gen_FILE_SPECIFY($2);
            break;
          case IOSTATUS:
            $$ = gen_STATUS_SPECIFY($2);
            break;
          case IOACCESS:
            if (iostmt == SINQUIRE_STAT)
              $$ = gen_ACCESS_QUERY($2);
            else
              $$ = gen_ACCESS_SPECIFY($2);
            break;
          case IOFORM:
            if (iostmt == SINQUIRE_STAT)
    	      $$ = gen_FORM_QUERY($2);
            else
              $$ = gen_FORM_SPECIFY($2);
            break;
          case IOBLANK:
            if (iostmt == SINQUIRE_STAT)
              $$ = gen_BLANK_QUERY($2);
            else
              $$ = gen_BLANK_SPECIFY($2);
            break;
          case IOEXIST:
            $$ = gen_EXIST_QUERY($2);
            break;
          case IOOPENED:
            $$ = gen_OPENED_QUERY($2);
            break;
          case IONUMBER:
            $$ = gen_NUMBER_QUERY($2);
            break;
          case IONAMED:
            $$ = gen_NAMED_QUERY($2);
            break;
          case IONAME:
            $$ = gen_NAME_QUERY($2);
            break;
          case IOSEQUENTIAL:
            $$ = gen_SEQUENTIAL_QUERY($2);
            break;
          case IODIRECT:
            $$ = gen_DIRECT_QUERY($2);
            break;
          case IOFORMATTED:
            $$ = gen_FORMATTED_QUERY($2);
            break;
          case IOUNFORMATTED:
            $$ = gen_UNFORMATTED_QUERY($2);
            break;
          case IONEXTREC:
            $$ = gen_NEXTREC_QUERY($2);
            break;
          case IOIOSTAT: 
            $$ = gen_IOSTAT_QUERY($2);
            break;
        }
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SKWD_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_kwd);
	treeStackPush($$);
      }
  | SSPECIFY_KWD_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_specify_kwd);
	treeStackPush($$);
      }
  | SQUERY_KWD_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_query_kwd);
	treeStackPush($$);
      };


io_control_kwd:
    SNAMEEQ
      {
        $$ = lx1_IOKwd;
      };


io_control_value:
    rval
  | SSTAR
      {
        $$ = gen_STAR();
	treeStackPush($$);
      }
  | SLABEL_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_lbl_ref);
	treeStackPush($$);
      }
  | SUNIT_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_unit);
	treeStackPush($$);
      }
  | SFORMAT_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_format);
	treeStackPush($$);
      };


io_input_list:
    io_input_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | io_input_list SCOMMA io_input_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


io_input_elt:
    lval
      {
        $$ = $1;
      }
  | SLPAR io_input_list SCOMMA name SEQUALS rval SCOMMA rval
						optional_comma_rval SRPAR
      {
        $$ = gen_IMPLIED_DO($2, $4, $6, $8, $9);
	treeStackDrop(5);
	treeStackPush($$);
      }
  | SDATA_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_data);
	treeStackPush($$);
      };


io_output_list:
    unparen_rval
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | io_output_paren_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | io_output_unparen_elt;


io_output_unparen_elt:
    unparen_rval SCOMMA unparen_rval
      {
        $$ = list_insert_last(list_create($1), $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | unparen_rval SCOMMA io_output_paren_elt
      {
        $$ = list_insert_last(list_create($1), $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_output_paren_elt SCOMMA unparen_rval
      {
        $$ = list_insert_last(list_create($1), $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_output_paren_elt SCOMMA io_output_paren_elt
      {
        $$ = list_insert_last(list_create($1), $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_output_unparen_elt SCOMMA unparen_rval
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_output_unparen_elt SCOMMA io_output_paren_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SDATA_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_data);
	treeStackPush($$);
      };


io_output_paren_elt:
    complex_const
  | SLPAR unparen_rval SCOMMA name SEQUALS rval SCOMMA rval
						optional_comma_rval SRPAR
      {
        $$ = gen_IMPLIED_DO(list_create($2), $4, $6, $8, $9);
	treeStackDrop(5);
	treeStackPush($$);
      }
  | SLPAR io_output_paren_elt SCOMMA name SEQUALS rval SCOMMA rval
						optional_comma_rval SRPAR
      {
        $$ = gen_IMPLIED_DO(list_create($2), $4, $6, $8, $9);
	treeStackDrop(5);
	treeStackPush($$);
      }
  | SLPAR io_output_unparen_elt  SCOMMA name SEQUALS rval SCOMMA rval
						optional_comma_rval SRPAR
      {
        $$ = gen_IMPLIED_DO($2, $4, $6, $8, $9);
	treeStackDrop(5);
	treeStackPush($$);
      };


io_rval:
    io_unparen_rval
  | SLPAR io_rval SRPAR
      {
        $$ = $2;
        gen_put_parens($$,1);
      };


io_unparen_rval:
    lval
  | simple_const
  | SSTAR
      {
        $$ = gen_STAR();
	treeStackPush($$);
      }
  | io_rval SPLUS io_rval
      {
        $$ = gen_BINARY_PLUS($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_rval SMINUS io_rval
      {
        $$ = gen_BINARY_PLUS($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_rval SSTAR io_rval
      {
        $$ = gen_BINARY_TIMES($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_rval SSLASH io_rval
      {
        $$ = gen_BINARY_DIVIDE($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | io_rval SPOWER io_rval
      {
        $$ = gen_BINARY_EXPONENT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SPLUS io_rval  %prec SSTAR
      {
        $$ = $2;
      }
  | SMINUS io_rval %prec SSTAR
      {
        $$ = gen_UNARY_MINUS($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | io_rval SCONCAT io_rval
      {
        $$ = gen_BINARY_CONCAT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SUNIT_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_unit);
	treeStackPush($$);
      }
  | SFORMAT_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_format);
	treeStackPush($$);
      };




/************************/
/*  Pause statement	*/
/************************/


if_able_statement:
    SPAUSE stmt_label optional_const
      {
        $$.token = SPAUSE_STAT;
        $$.tree  = gen_PAUSE($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Return statement	*/
/************************/


if_able_statement:
    SRETURN stmt_label optional_rval
      {
        $$.token = SRETURN_STAT;
        $$.tree  = gen_RETURN($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Stop statement	*/
/************************/


if_able_statement:
    SSTOP stmt_label optional_const
      {
        $$.token = SSTOP_STAT;
        $$.tree  = gen_STOP($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };






/********************************/
/*  Special statements		*/
/********************************/




/************************/
/*  Unknown statement	*/
/************************/


statement:
    SUNKNOWN
      {
        $$.token = SERROR_STAT;
        lx1_Flush();
      };




/************************/
/*  Placeholder stats	*/
/************************/


if_able_statement:
    SSTMT_PH stmt_no_label				/* PLACEHOLDER */
      {
        $$.token   = SSTMT_PH_STAT;
        $$.part[0] = ph_from_mtype(GEN_stmt);
	treeStackPush($$.part[0]);
      }
 |  SSPECIFICATION_STMT_PH stmt_no_label		/* PLACEHOLDER */
      {
        $$.token   = SSPECIFICATION_STMT_PH_STAT;
        $$.part[0] = ph_from_mtype(GEN_specification_stmt);
	treeStackPush($$.part[0]);
      }
 |  SCONTROL_STMT_PH stmt_no_label			/* PLACEHOLDER */
      {
        $$.token   = SCONTROL_STMT_PH_STAT;
        $$.part[0] = ph_from_mtype(GEN_control_stmt);
	treeStackPush($$.part[0]);
      }
 |  SIO_STMT_PH stmt_no_label				/* PLACEHOLDER */
      {
        $$.token   = SIO_STMT_PH_STAT;
        $$.part[0] = ph_from_mtype(GEN_io_stmt);
	treeStackPush($$.part[0]);
      }
 |  SPARASCOPE_STMT_PH stmt_no_label			/* PLACEHOLDER */
      {
        $$.token   = SPARASCOPE_STMT_PH_STAT;
        $$.part[0] = ph_from_mtype(GEN_parascope_stmt);
	treeStackPush($$.part[0]);
      }
 |  SDEBUG_STMT_PH stmt_no_label			/* PLACEHOLDER */
      {
        $$.token   = SDEBUG_STMT_PH_STAT;
        $$.part[0] = ph_from_mtype(GEN_debug_stmt);
	treeStackPush($$.part[0]);
      };






/********************************/
/*  Debugging statements	*/
/********************************/




/************************/
/*  At statement	*/
/************************/


statement:
    SAT stmt_label label				/* VS FORTRAN */
      {
        $$.token = SAT_STAT;
        $$.tree  = gen_AT($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Debug statement	*/
/************************/


statement:
    SDEBUG stmt_label debug_option_list			/* VS FORTRAN */
      {
        $$.token   = SDEBUG_STAT;
        $$.part[0] = $2;
        $$.part[1] = $3;
      };


debug_option_list:					/* VS FORTRAN */
    needkwdon debug_option needkwdoff
      {
        $$ = list_create($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | debug_option_list SCOMMA needkwdon debug_option needkwdoff
      {
        $$ = list_insert_last($1, $4);
	treeStackDrop(2);
	treeStackPush($$);
      };


debug_option:						/* VS FORTRAN */
    SUNIT SLPAR simple_const SRPAR
      {
        $$ = gen_UNIT($3);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SSUBCHK
      {
	$$ = gen_SUBCHK(AST_NIL);
	treeStackPush($$);
      }
  | SSUBCHK SLPAR name_list SRPAR
      {
	$$ = gen_SUBCHK($3);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | STRACE
      {
        $$ = gen_TRACE();
	treeStackPush($$);
      }
  | SINIT
      {
        $$ = gen_INIT(AST_NIL);
	treeStackPush($$);
      }
  | SINIT SLPAR name_list SRPAR
      {
        $$ = gen_INIT($3);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SSUBTRACE
      {
        $$ = gen_SUBTRACE();
	treeStackPush($$);
      }
  | SOPTION_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_option);
	treeStackPush($$);
      };




/************************/
/*  Trace off statement	*/
/************************/


if_able_statement:
    STRACEOFF stmt_label				/* VS FORTRAN */
      {
	$$.token = STRACEOFF_STAT;
	$$.tree  = gen_TRACEOFF($2);
	treeStackDrop(1);
	treeStackPush($$.tree);
      };




/************************/
/*  Trace on statement	*/
/************************/


if_able_statement:					/* VS FORTRAN */
    STRACEON stmt_label
      {
	$$.token = STRACEON_STAT;
	$$.tree  = gen_TRACEON($2);
	treeStackDrop(1);
	treeStackPush($$.tree);
      };






/********************************/
/*  Parascope statements	*/
/********************************/




/************************/
/*  Clear statement	*/
/************************/


if_able_statement:
    SCLEAR stmt_label lval				/* PARASCOPE */
      {
	$$.token = SCLEAR_STAT;
	$$.tree  = gen_CLEAR($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };





/************************/
/*  Barrier set stat.	*/
/************************/


if_able_statement:
    SSET stmt_label lval SCOLON rval			/* PARASCOPE */
      {
        $$.token = SSET_BARRIER_STAT;
        $$.tree  = gen_SET_BARRIER($2, $3, $5);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };




/************************/
/*  Block statement	*/
/************************/


if_able_statement:
    SBLOCK stmt_label lval				/* PARASCOPE */
      {
        $$.token = SBLOCK_STAT;
        $$.tree  = gen_BLOCK($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Doall statement	*/
/************************/


statement:
    SDOALL stmt_label doall_control			/* PARASCOPE */
      {
        $$.token = SDOALL_STAT;
	$$.part[0] = $2;
	$$.part[1] = AST_NIL;
        $$.part[2] = $3;
	treeStackPush($$.part[1]);
      }
  | SDOALL stmt_label label optional_comma doall_control /* PARASCOPE */
      {
        $$.token = SDOALL_LABEL_STAT;
	$$.part[0] = $2;
	$$.part[1] = $3;
        $$.part[2] = $5;
        ft_SetComma($$.part[1], $4);
      }
  | SENDALL stmt_label					/* PARASCOPE */
      {
        $$.token = SEND_ALL_STAT;
	$$.part[0] = $2;
      };


doall_control:						/* PARASCOPE */
    name SEQUALS rval SCOMMA rval optional_comma_rval
      {
        $$ = gen_INDUCTIVE($1, $3, $5, $6);
	treeStackDrop(4);
	treeStackPush($$);
      }
  | SLOOP_CONTROL_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_loop_control);
	treeStackPush($$);
      };




/************************/
/*  Lock statement	*/
/************************/


if_able_statement:
    SLOCK stmt_label lval				/* PARASCOPE */
      {
	$$.token = SLOCK_STAT;
	$$.tree  = gen_LOCK($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Parallel statement	*/
/************************/


statement:
    SPARBEGIN stmt_label				/* PARASCOPE */
      {
        $$.token = SPARBEGIN_STAT;
        $$.part[0] = $2;
      }
  | SPARALLEL stmt_label SCOLON				/* PARASCOPE */
      {
        $$.token = SPARALLEL_STAT;
        $$.part[0] = $2;
      }
  | SPARBEGIN stmt_label SLPAR lval SEQUALS rval SRPAR	/* PARASCOPE */
      {
        $$.token = SPARBEGIN_PID_STAT;
        $$.part[0] = $2;
        $$.part[1] = $4;
        $$.part[2] = $6;
      }
  | SPARALLEL stmt_label SCOLON SLPAR rval_list SRPAR	/* PARASCOPE */
      {
        $$.token = SPARALLEL_PID_STAT;
        $$.part[0] = $2;
        $$.part[1] = $5;
      }
  | SOTHERPROCESSES stmt_label SCOLON			/* PARASCOPE */
      {
        $$.token = SOTHER_PROCESSES_STAT;
        $$.part[0] = $2;
      }
  | SPAREND stmt_label					/* PARASCOPE */
      {
        $$.token = SPAREND_STAT;
        $$.part[0] = $2;
      };




/************************/
/*  Post statement	*/
/************************/


if_able_statement:
    SPOST stmt_label post_posting			/* PARASCOPE */
      {
	$$.token = SPOST_STAT;
	$$.tree  = gen_POST($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };


post_posting:						/* PARASCOPE */
    lval needkwdon post_value needkwdoff
      {
        $$ = gen_POSTING($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SPOSTING_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_posting);
        treeStackPush($$);
      };


post_value:						/* PARASCOPE */
    /* empty */
      {
        $$ = AST_NIL;
	treeStackPush($$);
      }
  | SCOLON STO rval
      {
        $$ = gen_POST_TO($3);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SCOLON SINC rval
      {
        $$ = gen_POST_INC($3);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SCOLON SPOST_EXPR_PH				/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_post_expr);
        treeStackPush($$);
      };




/************************/
/*  Task statement	*/
/************************/


if_able_statement:
    STASK stmt_label call_invocation task_posting	/* PARASCOPE */
      {
        $$.token = STASK_STAT;
        $$.tree  = gen_TASK($2, $3, $4);
	ft_SetShow($$.tree, false);		/* shorthand */
	treeStackDrop(3);
	treeStackPush($$.tree);
      }
  | SCREATETASK stmt_label call_invocation task_posting	/* PARASCOPE */
      {
        $$.token = STASK_STAT;
        $$.tree  = gen_TASK($2, $3, $4);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };


task_posting:						/* PARASCOPE */
    /* empty */
      {
        $$ = AST_NIL;
        treeStackPush($$);
      }
  | SCOMMA needkwdon SPOSTING needkwdoff post_posting
      {
        $$ = $5;
      };




/************************/
/*  Task common stat.	*/
/************************/


statement:
    STASKCOMMON stmt_label common_list			/* PARASCOPE */
      {
	$$.token = STASK_COMMON_STAT;
	$$.tree  = gen_TASK_COMMON($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Unock statement	*/
/************************/


if_able_statement:
    SUNLOCK stmt_label lval				/* PARASCOPE */
      {
	$$.token = SUNLOCK_STAT;
	$$.tree  = gen_UNLOCK($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Wait statement	*/
/************************/


if_able_statement:
    SWAIT stmt_label lval				/* PARASCOPE */
      {
	$$.token = SWAIT_STAT;
	$$.tree  = gen_WAIT($2, $3, AST_NIL);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | SWAIT stmt_label lval SCOLON needkwdon SUNTIL
				needkwdoff rval		/* PARASCOPE */
      {
	$$.token = SWAIT_STAT;
	$$.tree  = gen_WAIT($2, $3, $8);
	treeStackDrop(3);
	treeStackPush($$.tree);
      };






/********************************/
/*  Parallel Fortran Extensions	*/
/********************************/




/************************/
/*  Parallel Loop stmt.	*/
/************************/


statement:
    SPARALLELLOOP stmt_label parallelloop_control	/* PARALLEL */
      {
        $$.token = SPARALLELLOOP_STAT;
	$$.part[0] = $2;
	$$.part[1] = AST_NIL;
        $$.part[2] = $3;
	treeStackPush($$.part[1]);
      }
  | SPARALLELLOOP stmt_label label optional_comma
				parallelloop_control	/* PARALLEL */
      {
        $$.token = SPARALLELLOOP_LABEL_STAT;
	$$.part[0] = $2;
	$$.part[1] = $3;
        $$.part[2] = $5;
        ft_SetComma($$.part[1], $4);
      }
  | SENDLOOP stmt_label					/* PARALLEL */
      {
        $$.token = SEND_LOOP_STAT;
	$$.part[0] = $2;
      };


parallelloop_control:					/* PARALLEL */
    name SEQUALS rval SCOMMA rval optional_comma_rval
      {
        $$ = gen_INDUCTIVE($1, $3, $5, $6);
	treeStackDrop(4);
	treeStackPush($$);
      }
  | SLOOP_CONTROL_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_loop_control);
	treeStackPush($$);
      };




/************************/
/*  Private statement	*/
/************************/


statement:
    SPRIVATE stmt_label name_list			/* PARALLEL */
      {
        $$.token = SPRIVATE_STAT;
        $$.tree  = gen_PRIVATE($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Stop Loop statement	*/
/************************/


statement:
    SSTOPLOOP stmt_label label				/* PARALLEL */
      {
        $$.token = SSTOP_LOOP_STAT;
        $$.tree  = gen_STOPLOOP($2, $3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      }
  | SSTOPLOOP stmt_label				/* PARALLEL */
      {
        $$.token = SSTOP_LOOP_STAT;
        $$.tree  = gen_STOPLOOP($2, AST_NIL);
	treeStackDrop(1);
	treeStackPush($$.tree);
      };







/********************************/
/*  Fortran 90 Extensions	*/
/********************************/



/**************************/
/*  Allocatable statement */
/**************************/

alloc_object:						/* FORTRAN 90 */
    name
     {
       $$ = gen_ARRAY_DECL_LEN($1, AST_NIL, AST_NIL, AST_NIL);
       treeStackDrop(1);
       treeStackPush($$);
     }
  | dimension_decl;

alloc_object_list:     					/* FORTRAN 90 */
    alloc_object
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | alloc_object_list SCOMMA alloc_object		/* FORTRAN 90 */
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };

statement:						/* FORTRAN 90 */
    SALLOCATABLE stmt_label alloc_object_list
      {
	$$.token = SALLOCATABLE_STAT;
	$$.tree = gen_ALLOCATABLE($2,$3);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };



/**************************/
/*  Allocate statement    */
/**************************/

subscripted_array:					/* FORTRAN 90 */
    name subscript
      {
	/* ASSERT name of $2 is AST_NIL */
          gen_SUBSCRIPT_put_name($2, $1);

        $$ = $2;
	treeStackDrop(2);
	treeStackPush($$);
      };

subscripted_array_list:					/* FORTRAN 90 */
    subscripted_array
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
    | subscripted_array_list SCOMMA subscripted_array
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);	
      };

statement:						/* FORTRAN 90 */
    SALLOCATE stmt_label SLPAR subscripted_array_list SRPAR
      {
	$$.token = SALLOCATE_STAT;
	$$.tree = gen_ALLOCATE($2,$4);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/**************************/
/*  Deallocate statement  */
/**************************/

dealloc_object_list:					/* FORTRAN 90 */
    name
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | dealloc_object_list SCOMMA name			/* FORTRAN 90 */
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };

statement:						/* FORTRAN 90 */
    SDEALLOCATE stmt_label SLPAR dealloc_object_list SRPAR
      {
	$$.token = SDEALLOCATE_STAT;
	$$.tree = gen_DEALLOCATE($2,$4);
	treeStackDrop(2);
	treeStackPush($$.tree);
      };




/************************/
/*  Where statements	*/
/************************/


if_able_statement:	       				/* FORTRAN 90 */
    SWHERE stmt_label SLPAR rval SRPAR if_able_statement
      {
        $$.token   = SWHERE_STAT;
        if( $6.token == SSTMT_PH_STAT )
          $$.tree    = gen_WHERE($2, $4, list_create($6.part[0]));
        else
          $$.tree    = gen_WHERE($2, $4, list_create($6.tree));
	treeStackDrop(3);
	treeStackPush($$.tree);
      }
  | SWHERE stmt_label SLPAR rval SRPAR 			/* FORTRAN 90 */
      {
        $$.token   = SWHERE_BLOCK_STAT;
        $$.part[0] = $2;
        $$.part[1] = $4;
      }
  | SELSEWHERE stmt_label				/* FORTRAN 90 */
      {
        $$.token   = SELSE_WHERE_STAT;
	$$.part[0] = $2;
	$$.part[1] = AST_NIL;
	treeStackPush($$.part[1]);
      }
  | SENDWHERE stmt_label				/* FORTRAN 90 */
      {
        $$.token   = SEND_WHERE_STAT;
	$$.part[0] = $2;
      };







/********************************/
/*  Expressions			*/
/********************************/


rval_list:
    rval
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | rval_list SCOMMA rval
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


optional_rval:
    /* empty */
      {
        $$ = AST_NIL;
	treeStackPush($$);
      }
  | rval;


optional_comma_rval:
    /* empty */
      {
        $$ = AST_NIL;
        treeStackPush($$);
      }
  | SCOMMA rval
      {
        $$ = $2;
      };


rval:
    unparen_rval
  | SLPAR rval SRPAR
      {
        $$ = $2;
        gen_put_parens($$, 1);
      }
  | complex_const;


unparen_rval:
    simple_const
  | lval
  | rval SPLUS rval
      {
        $$ = gen_BINARY_PLUS($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SMINUS rval
      {
        $$ = gen_BINARY_MINUS($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SSTAR rval
      {
        $$ = gen_BINARY_TIMES($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SSLASH rval
      {
        $$ = gen_BINARY_DIVIDE($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SPOWER rval
      {
        $$ = gen_BINARY_EXPONENT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SPLUS rval  %prec SSTAR
      {
        $$ = $2;
      }
  | SMINUS rval  %prec SSTAR
      {
        $$ = gen_UNARY_MINUS($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | rval SCONCAT rval
      {
        $$ = gen_BINARY_CONCAT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SEQ rval
      {
        $$ = gen_BINARY_EQ($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SGT rval
      {
        $$ = gen_BINARY_GT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SLT rval
      {
        $$ = gen_BINARY_LT($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SGE rval
      {
        $$ = gen_BINARY_GE($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SLE rval
      {
        $$ = gen_BINARY_LE($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SNE rval
      {
        $$ = gen_BINARY_NE($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SEQV rval
      {
        $$ = gen_BINARY_EQV($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SNEQV rval
      {
        $$ = gen_BINARY_NEQV($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SOR rval
      {
        $$ = gen_BINARY_OR($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | rval SAND rval
      {
        $$ = gen_BINARY_AND($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      }
  | SNOT rval
      {
        $$ = gen_UNARY_NOT($2);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SFORMAL_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_formal);
	treeStackPush($$);
      }
  | expr_ph;						/* PLACEHOLDER */


expr_ph:
    SEXPR_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_expr);
	treeStackPush($$);
      }
  | SARITH_EXPR_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_arith_expr);
	treeStackPush($$);
      }
  | SSTRING_EXPR_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_string_expr);
	treeStackPush($$);
      }
  | SRELATIONAL_EXPR_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_relational_expr);
	treeStackPush($$);
      }
  | SLOGICAL_EXPR_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_logical_expr);
	treeStackPush($$);
      };


lval_list:
    lval
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | lval_list SCOMMA lval
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


lval:
    name
/*  | name substring                                  */
/*      {                                             */
/*	    ASSERT name of $2 is AST_NIL              */
/*          gen_SUBSTRING_put_substring_name($2, $1); */
/*                                                    */
/*        $$ = $2;                                    */
/*	treeStackDrop(2);                             */
/*	treeStackPush($$);                            */
/*      }                                             */
  | name subscript
      {
	/* ASSERT name of $2 is AST_NIL */
          gen_SUBSCRIPT_put_name($2, $1);

        $$ = $2;
	treeStackDrop(2);
	treeStackPush($$);
      }
  | name subscript substring
      {
	/* ASSERT name of $2 and $3 are AST_NIL */
          gen_SUBSCRIPT_put_name($2, $1);
          gen_SUBSTRING_put_substring_name($3, $2);

        $$ = $3;
	treeStackDrop(3);
	treeStackPush($$);
      }
  | SSTRING_VAR_PH substring				/* PLACEHOLDER */
      {
        FortTreeNode name;

	name = ph_from_mtype(GEN_string_var);

	/* ASSERT name of $2 is AST_NIL */
          gen_SUBSTRING_put_substring_name($2, name);

	$$ = $2;
	treeStackDrop(1);
	treeStackPush($$);
      }
  | SVAR_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_var);
	treeStackPush($$);
      }
  | SINVOCATION_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_invocation);
	treeStackPush($$);
      };


substring:
    SLPAR optional_rval SCOLON optional_rval SRPAR
      {
        $$ = gen_SUBSTRING(AST_NIL, $2, $4);
	treeStackDrop(2);
	treeStackPush($$);
      };


subscript:
    SLPAR SRPAR
      {
        $$ = gen_SUBSCRIPT(AST_NIL, list_create(AST_NIL));
	treeStackPush($$);
      }
  | SLPAR subscript_elt_list SRPAR
      {
        $$ = gen_SUBSCRIPT(AST_NIL, $2);
	treeStackDrop(1);
	treeStackPush($$);
      };


subscript_elt:
    rval
   | rval SCOLON
     {
       $$ = gen_TRIPLET(AST_NIL, $1, AST_NIL);
       treeStackDrop(1);
       treeStackPush($$);       
     }
  | rval SCOLON rval
     {
       $$ = gen_TRIPLET($1, $3, AST_NIL);
       treeStackDrop(2);
       treeStackPush($$);       
     }
  | rval SCOLON rval SCOLON rval
     {
       $$ = gen_TRIPLET($1, $3, $5);
       treeStackDrop(3);
       treeStackPush($$);       
     }
  | SCOLON rval SCOLON rval   
     {
       $$ = gen_TRIPLET(AST_NIL, $2, $4);
       treeStackDrop(2);
       treeStackPush($$);       
     }
  | SCOLON SCOLON rval
     {
       $$ = gen_TRIPLET(AST_NIL, AST_NIL, $3);
       treeStackDrop(1);
       treeStackPush($$);       
     }
   | SCOLON rval
     {
       $$ = gen_TRIPLET(AST_NIL, $2, AST_NIL);
       treeStackDrop(1);
       treeStackPush($$);       
     }
   | SCOLON
     {
       $$ = gen_TRIPLET(AST_NIL, AST_NIL, AST_NIL);
       treeStackPush($$);       
     };

subscript_elt_list:
    subscript_elt
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      } 
  | subscript_elt_list SCOMMA subscript_elt
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };  

optional_const:
    /* empty */
      {
        $$ = AST_NIL;
	treeStackPush($$);
      }
  | simple_const;


simple_const:
    STRUE
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, ".true.", STR_CONSTANT_LOGICAL);
	treeStackPush($$);
      }
  | SFALSE
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, ".false.", STR_CONSTANT_LOGICAL);
	treeStackPush($$);
      }
  | SHOLLERITH
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, lx1_Token, STR_CONSTANT_CHARACTER);
	treeStackPush($$);
      }
  | SICON
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, lx1_Token, STR_CONSTANT_INTEGER);
	treeStackPush($$);
      }
  | SRCON
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, lx1_Token, STR_CONSTANT_REAL);
	treeStackPush($$);
      }
  | SDCON
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, lx1_Token, STR_CONSTANT_DOUBLE_PRECISION);
	treeStackPush($$);
      }
  | SHEXCON						/* VS FORTRAN */
      {
	$$ = gen_CONSTANT();
	gen_put_text($$, lx1_Token, STR_CONSTANT_HEX);
	treeStackPush($$);
      }
  | SCONSTANT_PH					/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_constant);
	treeStackPush($$);
      };


complex_const:
    SLPAR unparen_rval SCOMMA unparen_rval SRPAR
      {
        $$ = gen_COMPLEX_CONSTANT($2, $4);
	treeStackDrop(2);
	treeStackPush($$);
      };






/********************************/
/*  Basic components		*/
/********************************/


stmt_label:
    /* empty */
      {
        /* Dummy nonterminal, packages statement number into label node */
        char buf[7];

	if( lx1_PlaceholderStatLabel )
          { $$ = ph_from_mtype(GEN_lbl_def);
            lx1_PlaceholderStatLabel = false;
          }
        else if( lx1_StatNumber == 0 )
          { $$ = AST_NIL;
          }
	else
          { $$ = gen_LABEL_DEF();
	    (void) sprintf (buf, "%ld", lx1_StatNumber);
            gen_put_text($$, buf, STR_LABEL_DEF);
          }
        lx1_StatNumber = 0;
        treeStackPush($$);
      };


stmt_no_label:
    /* empty */
      {
        if( lx1_StatNumber != 0 )
          { yyerror("Statement number not allowed");
	    YYERROR;
          }
      };


label_list:
    label
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | label_list SCOMMA label
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


label:
    SICON
      {
	$$ = gen_LABEL_REF();
        gen_put_text($$, lx1_Token, STR_LABEL_REF);
	treeStackPush($$);
      }
  | SLABEL_PH						/* PLACEHOLDER */
      {
        $$ = ph_from_mtype(GEN_lbl_ref);
	treeStackPush($$);
      };


name_list:
    name
      {
        $$ = list_create($1);
	treeStackDrop(1);
	treeStackPush($$);
      }
  | name_list SCOMMA name
      {
        $$ = list_insert_last($1, $3);
	treeStackDrop(2);
	treeStackPush($$);
      };


optional_name:
    /* empty */
      {
        $$ = AST_NIL;
	treeStackPush($$);
      }
  | name;


name:
    SNAME
      {
        $$ = gen_IDENTIFIER();
        gen_put_text($$, lx1_Token, STR_IDENTIFIER);
	treeStackPush($$);
      }
  | SNAME_PH						/* PLACEHOLDER */
      {
	$$ = ph_from_mtype(GEN_name);
	treeStackPush($$);
      };


optional_comma:
    /* empty */
      {
        $$ = false;
      }
  | SCOMMA
      {
        $$ = true;
      };






/********************************/
/*  Scanner flags		*/
/********************************/


intonlyon:
    /* empty */
      {
        lx1_IntOnly = true;
      };


intonlyoff:
    /* empty */
      {
        lx1_IntOnly = false;
      };


iocontrolon:
    /* empty */
      {
        lx1_InIOControl = true;
      };


iocontroloff:
    /* empty */
      {
        lx1_InIOControl = false;
      };


needkwdon:
    /* empty */
      {
        lx1_NeedKwd = true;
      };


needkwdoff:
    /* empty */
      {
        lx1_NeedKwd = false;
      };






%%




/*ARGSUSED*/
void parse1(FortTextTree ftt, TextString text)
{
  lx1_SetScan(text);
  lx1_NeedKwd = false;
  lx1_IntOnly = false;
  lx1_InIOControl = false;
  lx1_StatNumber = 0;
  (void) yyparse();
  treeStackCheck();
}




static
int yylex()
{
  int token;    /* simplifies debugging */

  token = lx1_NextToken();
  return token;
}




void yyerror(const char *s)
{
  /* save just the first error message to occur */
  if( fp1_error == nil )  fp1_error = ssave(s);
}




static
FortTreeNode coerceToLabel(FortTreeNode node)
{
  FortTreeNode New;

  if( is_constant(node)  &&
              str_get_type(gen_get_symbol(node)) == STR_CONSTANT_INTEGER )
    { /* this integer constant should be a label--coerce it */
        New = gen_LABEL_REF();
        gen_put_text(New, gen_get_text(node), STR_LABEL_REF);
        tree_free(node);
        return New;
    }
  else
    return node;
}
