/* $Id: FortTextTree.C,v 1.2 2000/01/12 23:13:41 mjbedy Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortTextTree.c					*/
/*									*/
/*	FortTextTree -- text-and-structure view of a FortTree		*/
/*									*/
/************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include <include/bstring.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/arrays/FlexibleArray.h>
#include <libs/support/file/Cookie.h>

#include <libs/frontEnd/prettyPrinter/sigcomments.h>

#include <libs/frontEnd/fortTextTree/MapInfo_c.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.i>
#include <libs/frontEnd/fortTextTree/FortUnparse1.h>
#include <libs/frontEnd/fortTextTree/FortUnparse2.h>
#include <libs/frontEnd/fortTextTree/FortParse1.h>
#include <libs/frontEnd/fortTextTree/FortParse2.h>

#include <libs/support/memMgmt/mem.h>

/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




STATIC (TT_TreeNode, getFather, (FortTextTree ftt, FortTreeNode node));
STATIC (void, getExtra, (FortTextTree ftt, FortTreeNode node, int k, int *value));
STATIC (void, setExtra,(FortTextTree ftt, FortTreeNode node, int k, int value));


TT_Methods ftt_methods =
  { 
    (tt_Parse1Func)fp1_Parse,
    (tt_Parse2Func)fp2_Parse,
    (tt_Unparse1Func)unp1_Unparse,
    (tt_Unparse2Func)unp2_Unparse,
    (tt_Maptext1Func)unp1_TextToNode,
    (tt_Maptext2Func)unp2_TextToNode,
    (tt_Mapnode1Func)unp1_NodeToText,
    (tt_Mapnode2Func)unp2_NodeToText,
    (tt_Synch2Func)fp2_Synch,
    (tt_Copy1Func)fp1_Copy,
    (tt_Copy2Func)fp2_Copy,
    (tt_Destroy1Func)fp1_Destroy,
    (tt_Destroy2Func)fp2_Destroy,
    (tt_SetRootFunc)fp2_SetRoot,
    (tt_GetFatherFunc)getFather,
    (tt_GetExtraFunc)getExtra,
    (tt_SetExtraFunc)setExtra
  };






/* communication between FortParse2 and gram2 */

FortTree ftt_fortTree;

TextTree ftt_textTree;

fx_EndingStat UNUSED_ENDVAL =
  {
    true,
    0,
    0,
    0
  };






/************************/
/*  Initialization info	*/
/************************/




static int		ftt_InitCount = 0;




/************************/
/*  Unparse Tables	*/
/************************/


/* '...' char used in expansion names */

#define ELLIPSIS_CHAR	'\206'




/* unparsing state */

unsigned char ftt_style;




/* unparse table structure */

#define OPEN_BRACKET    '{'	
#define CLOSE_BRACKET   '}'


typedef struct
  {
    int    stmtToken;
    char * format;
    char * expansionName;
    
  } unparseTable;




/* unparse table for nodes */

static unparseTable nodeInfo[] =
  {
    /* FREED_NODE */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* NULL_NODE */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* LIST_OF_NODES */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* ERROR */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* PLACE_HOLDER */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* GLOBAL */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* FUNCTION */
      { UNUSED,
        (char *) 0,
        "&type-name& function &func-name&(&formal...&)",
      },

    /* PROGRAM */
      { UNUSED,
        (char *) 0,
        "program &name&",
      },

    /* SUBROUTINE */
      { UNUSED,
        (char *) 0,
        "subroutine &subr-name&(&formal...&)",
      },

    /* BLOCK_DATA */
      { UNUSED,
        (char *) 0,
        "block data &name&",
      },

    /* COMMON */
      { SCOMMON_STAT,
        "{exists(%1)%1}	common %2",
        "common /&name&/ &name&(&dim...&) &...&",
      },

    /* COMMON_ELT */
      { UNUSED,
        "{show(%1){placeholder(%1)/}%1{placeholder(%1)/} }%2",
        "/&name&/ &array-def...&",
      },

    /* DIMENSION */
      { SDIMENSION_STAT,
        "{exists(%1)%1}	dimension %2",
        "dimension &name&(&dim...&)&...&",
      },

    /* EQUIVALENCE */
      { SEQUIVALENCE_STAT,
        "{exists(%1)%1}	equivalence %2",
        "equivalence (&var...&) &...&",
      },

    /* EQUIV_ELT */
      { UNUSED,
        "(%1)",
        "&var...&",
      },

    /* TYPE_STATEMENT */
      { STYPE_STATEMENT_STAT,
        "{exists(%1)%1}	%2{comma(%0),} %3",
        "&type-name& &name&(&dim...&) &...&",
      },

    /* ARRAY_DECL_LEN */
      { UNUSED,
        "%1{exists(%3)(%3)}{exists(%2)*%2}{exists(%4) /%4/}",
        (char *) 0,
      },

    /* EXTERNAL */
      { SEXTERNAL_STAT,
        "{exists(%1)%1}	external %2",
        "external &name...&",
      },

    /* IMPLICIT */
      { SIMPLICIT_STAT,
        "{exists(%1)%1}	implicit %2",
        "implicit &type-name& (&letters...&) &...&",
      },

    /* IMPLICIT_ELT */
      { UNUSED,
        "%1 (%2)",
        "&type& (&letters&)",
      },

    /* IMPLICIT_PAIR */
      { UNUSED,
        "%1-%2",
        "&letter& - &letter&",
      },

    /* INTRINSIC */
      { SINTRINSIC_STAT,
        "{exists(%1)%1}	intrinsic %2",
        "intrinsic &name...&",
      },

    /* PARAMETER */
      { SPARAMETER_STAT,
        "{exists(%1)%1}	parameter (%2)",
        "parameter (&name& = &expr ...&)",
      },

    /* PARAM_ELT */
      { UNUSED,
        "%1 = %2",
        "&name& = &expr&",
      },

    /* SAVE */
      { SSAVE_STAT,
        "{exists(%1)%1}	save{exists(%2) %2}",
        "save &name...&",
      },

    /* COMMENT */
      { SCOMMENT_STAT,
        "{exists(%1)C}	{exists(%1)%1}",
        (char *) 0,
      },

    /* ENTRY */
      { SENTRY_STAT,
        "{exists(%1)%1}	entry %2{exists(%3)(%3)}",
        "entry &subr-name&(&formal...&)",
      },

    /* DATA */
      { SDATA_STAT,
        "{exists(%1)%1}	data %2",
        "data &data& /&init&/ &...&",
      },

    /* IMPLIED_DO */
      { UNUSED,
        "(%1, %2 = %3, %4{exists(%5), %5})",
        "(&data&, &var& = &expr&, &expr&)",
      },

    /* DATA_ELT */
      { UNUSED,
        "%1 /%2/",
        "&data& /&init...&/",
      },

    /* STMT_FUNCTION */
      { SSTMT_FUNCTION_STAT,
        "{exists(%1)%1}	%2(%3) = %4",
        "&func-name&(&formal...&) = &expr&",
      },

    /* FORMAT */
      { SFORMAT_STAT,
        "{exists(%1)%1}	format %2",
        "format &text&",
      },

    /* ASSIGN */
      { SASSIGN_STAT,
        "{exists(%1)%1}	assign %2 to %3",
        "assign &lbl& to &name&",
      },

    /* ASSIGNMENT */
      { SASSIGNMENT_STAT,
        "{exists(%1)%1}	%2 = %3",
        "&var& = &expr&",
      },

    /* BACKSPACE_SHORT */
      { SBACKSPACE_SHORT_STAT,
        "{exists(%1)%1}	backspace %2",
        "backspace &unit&",
      },

    /* BACKSPACE_LONG */
      { SBACKSPACE_LONG_STAT,
        "{exists(%1)%1}	backspace (%2)",
        "backspace (&kwd...&)",
      },

    /* CALL */
      { SCALL_STAT,
        "{exists(%1)%1}	call %2",
        "call &subr-name&(&arg...&)",
      },

    /* CLOSE */
      { SCLOSE_STAT,
        "{exists(%1)%1}	close (%2)",
        "close (&kwd...&)",
      },

    /* CONTINUE */
      { SCONTINUE_STAT,
        "{exists(%1)%1}	continue",
        "continue",
      },

    /* DO */
      { UNUSED,
        (char *) 0,
        "do &loop-control&",
      },

    /* ENDFILE_SHORT */
      { SENDFILE_SHORT_STAT,
        "{exists(%1)%1}	endfile %2",
        "endfile &unit&",
      },

    /* ENDFILE_LONG */
      { SENDFILE_LONG_STAT,
        "{exists(%1)%1}	endfile (%2)",
        "endfile (&kwd...&)",
      },

    /* ASSIGNED_GOTO */
      { SASSIGNED_GOTO_STAT,
        "{exists(%1)%1}	goto %2{exists(%3){comma(%0),} (%3)}",
        "goto &name&, (&lbl...&)",
      },

    /* COMPUTED_GOTO */
      { SCOMPUTED_GOTO_STAT,
        "{exists(%1)%1}	goto (%2){comma(%0),} %3",
        "goto (&lbl...&), &expr&",
      },

    /* GOTO */
      { SGOTO_STAT,
        "{exists(%1)%1}	goto %2",
        "goto &lbl&",
      },

    /* ARITHMETIC_IF */
      { SARITHMETIC_IF_STAT,
        "{exists(%1)%1}	if (%2) %3, %4, %5",
        "if (&expr&) &lbl&, &lbl&, &lbl&",
      },

    /* IF */
      { UNUSED,
        (char *) 0,
        "if (&expr&) then",
      },

    /* LOGICAL_IF */
      { SLOGICAL_IF_STAT,
        "{exists(%1)%1}	if (%2) %3",
        "if (&expr&) &stmt&",
      },

    /* GUARD */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* INQUIRE */
      { SINQUIRE_STAT,
        "{exists(%1)%1}	inquire (%2)",
        "inquire (&kwd...&)",
      },

    /* OPEN */
      { SOPEN_STAT,
        "{exists(%1)%1}	open (%2)",
        "open (&kwd...&)",
      },

    /* PAUSE */
      { SPAUSE_STAT,
        "{exists(%1)%1}	pause{exists(%2) %2}",
        "pause &constant&",
      },

    /* PRINT */
      { SPRINT_STAT,
        "{exists(%1)%1}	print %2{exists(%3), %3}",
        "print &format&, &data...&",
      },

    /* READ_SHORT */
      { SREAD_SHORT_STAT,
        "{exists(%1)%1}	read %2{exists(%3), %3}",
        "read &format&, &data...&",
      },

    /* READ_LONG */
      { SREAD_LONG_STAT,
        "{exists(%1)%1}	read (%2){exists(%3) %3}",
        "read (&kwd...&) &data...&",
      },

    /* RETURN */
      { SRETURN_STAT,
        "{exists(%1)%1}	return{exists(%2) %2}",
        "return &expr&",
      },

    /* REWIND_SHORT */
      { SREWIND_SHORT_STAT,
        "{exists(%1)%1}	rewind %2",
        "rewind &unit&",
      },

    /* REWIND_LONG */
      { SREWIND_LONG_STAT,
        "{exists(%1)%1}	rewind (%2)",
        "rewind (&kwd...&)",
      },

    /* STOP */
      { SSTOP_STAT,
        "{exists(%1)%1}	stop{exists(%2) %2}",
        "stop &constant&",
      },

    /* WRITE */
      { SWRITE_STAT,
        "{exists(%1)%1}	write (%2){exists(%3) %3}",
        "write (&kwd...&) &data...&",
      },

    /* AT */
      { SAT_STAT,
        "{exists(%1)%1}	at %2",
        "at &lbl&",
      },

    /* DEBUG */
      { UNUSED,
        (char *) 0,
        "debug &option...&",
      },

    /* TRACEON */
      { STRACEON_STAT,
        "{exists(%1)%1}	trace on",
        "trace on",
      },

    /* TRACEOFF */
      { STRACEOFF_STAT,
        "{exists(%1)%1}	trace off",
        "trace off",
      },

    /* UNIT */
      { UNUSED,
        "	unit(%1)",
        "unit(&constant&)",
      },

    /* SUBCHK */
      { UNUSED,
        "	subchk{exists(%1)(%1)}",
        "subchk(&name...&)",
      },

    /* TRACE */
      { UNUSED,
        "	trace",
        "trace",
      },

    /* INIT */
      { UNUSED,
        "	init{exists(%1)(%1)}",
        "init(&name...&)",
      },

    /* SUBTRACE */
      { UNUSED,
        "	subtrace",
        "subtrace",
      },

    /* TASK */
      { STASK_STAT,
        "{exists(%1)%1}	{show(%0)create }task %2{exists(%3), posting %3}",
        "create task &subr-name&(&arg...&), posting &var&: &post-expr&",
      },

    /* VALUE_PARAMETER */
      { UNUSED,
        "\\%val(%1)",
        "\\%val(&expr&)",
      },

    /* PARALLEL */
      { UNUSED,
        (char *) 0,
        "parbegin (&var& = &expr&)",	/* exists? */
      },

    /* PARALLEL_CASE */
      { UNUSED,
        (char *) 0,
        "parallel: (&expr...&)",	/* exists? -- SKW */
      },

    /* DO_ALL */
      { UNUSED,
        (char *) 0,
        "forall &loop-control&",
      },

    /* TASK_COMMON */
      { STASK_COMMON_STAT,
        "{exists(%1)%1}	task common %2",
        "task common /&name&/ &name&(&dim...&) &...&",
      },

    /* LOCK */
      { SLOCK_STAT,
        "{exists(%1)%1}	lock %2",
        "lock &var&",
      },

    /* UNLOCK */
      { SUNLOCK_STAT,
        "{exists(%1)%1}	unlock %2",
        "unlock &var&",
      },

    /* WAIT */
      { SWAIT_STAT,
        "{exists(%1)%1}	wait %2{exists(%3): until %3}",
        "wait &var&: until &expr&",
      },

    /* POST */
      { SPOST_STAT,
        "{exists(%1)%1}	post %2",
        "post &var&: &post-expr&",
      },

    /* POSTING */
      { UNUSED,
        "%1{exists(%2): %2}",
        "&var&: &post-expr&",
      },

    /* POST_TO */
      { UNUSED,
        "to %1",
        "to &expr&",
      },

    /* POST_INC */
      { UNUSED,
        "inc %1",
        "inc &expr&",
      },

    /* CLEAR */
      { SCLEAR_STAT,
        "{exists(%1)%1}	clear %2",
        "clear &var&",
      },

    /* SET_BARRIER */
      { SSET_BARRIER_STAT,
        "{exists(%1)%1}	set %2: %3",
        "set &var&: &expr&",
      },

    /* BLOCK */
      { SBLOCK_STAT,
        "{exists(%1)%1}	block %2",
        "block &var&",
      },

    /* SEMAPHORE */
      { UNUSED,
        "semaphore",
        "semaphore",
      },

    /* EVENT */
      { UNUSED,
        "event",
        "event",
      },

    /* BARRIER */
      { UNUSED,
        "barrier",
        "barrier",
      },

    /* SUBSCRIPT */
      { UNUSED,
        "{emphasize(%0)\\b}%1(%2){emphasize(%0)\\b}",
        "&name&(&expr...&)",
      },

    /* SUBSTRING */
      { UNUSED,
        "{emphasize(%0)\\b}%1(%2:%3){emphasize(%0)\\b}",
        "&string-var&(&expr&:&expr&)",
      },

    /* INVOCATION */
      { UNUSED,
        "%1{exists(%2)(%2)}",
        "&func-name&(&arg...&)",
      },

    /* BINARY_EXPONENT */
      { UNUSED,
        "{parens(%0)(}%1 ** %2{parens(%0))}",
        "&expr& ** &expr&",
      },

    /* BINARY_TIMES */
      { UNUSED,
        "{parens(%0)(}%1 * %2{parens(%0))}",
        "&expr& * &expr&",
      },

    /* BINARY_DIVIDE */
      { UNUSED,
        "{parens(%0)(}%1 / %2{parens(%0))}",
        "&expr& / &expr&",
      },

    /* BINARY_PLUS */
      { UNUSED,
        "{parens(%0)(}%1 + %2{parens(%0))}",
        "&expr& + &expr&",
      },

    /* BINARY_MINUS */
      { UNUSED,
        "{parens(%0)(}%1 - %2{parens(%0))}",
        "&expr& - &expr&",
      },

    /* BINARY_CONCAT */
      { UNUSED,
        "{parens(%0)(}%1 // %2{parens(%0))}",
        "&expr& // &expr&",
      },

    /* BINARY_AND */
      { UNUSED,
        "{parens(%0)(}%1 .and. %2{parens(%0))}",
        "&expr& .and. &expr&",
      },

    /* BINARY_OR */
      { UNUSED,
        "{parens(%0)(}%1 .or. %2{parens(%0))}",
        "&expr& .or. &expr&",
      },

    /* BINARY_EQ */
      { UNUSED,
        "{parens(%0)(}%1 .eq. %2{parens(%0))}",
        "&expr& .eq. &expr&",
      },

    /* BINARY_NE */
      { UNUSED,
        "{parens(%0)(}%1 .ne. %2{parens(%0))}",
        "&expr& .ne. &expr&",
      },

    /* BINARY_GE */
      { UNUSED,
        "{parens(%0)(}%1 .ge. %2{parens(%0))}",
        "&expr& .ge. &expr&",
      },

    /* BINARY_GT */
      { UNUSED,
        "{parens(%0)(}%1 .gt. %2{parens(%0))}",
        "&expr& .gt. &expr&",
      },

    /* BINARY_LE */
      { UNUSED,
        "{parens(%0)(}%1 .le. %2{parens(%0))}",
        "&expr& .le. &expr&",
      },

    /* BINARY_LT */
      { UNUSED,
        "{parens(%0)(}%1 .lt. %2{parens(%0))}",
        "&expr& .lt. &expr&",
      },

    /* BINARY_EQV */
      { UNUSED,
        "{parens(%0)(}%1 .eqv. %2{parens(%0))}",
        "&expr& .eqv. &expr&",
      },

    /* BINARY_NEQV */
      { UNUSED,
        "{parens(%0)(}%1 .neqv. %2{parens(%0))}",
        "&expr& .neqv &expr&",
      },

    /* UNARY_MINUS */
      { UNUSED,
        "{parens(%0)(}-%1{parens(%0))}",
        "- &expr&",
      },

    /* UNARY_NOT */
      { UNUSED,
        "{parens(%0)(}.not. %1{parens(%0))}",
        ".not. &expr&",
      },

    /* CONDITIONAL */
      { UNUSED,
        "while (%1)",
        "while (&expr&)",
      },

    /* INDUCTIVE */
      { UNUSED,
        "%1 = %2, %3{exists(%4), %4}",
        "&name& = &expr&, &expr&",
      },

    /* REPETITIVE */
      { UNUSED,
        "(%1) times",
        "(&expr&) times",
      },

    /* INTEGER */
      { UNUSED,
        "integer",
        "integer",
      },

    /* REAL */
      { UNUSED,
        "real",
        "real",
      },

    /* CHARACTER */
      { UNUSED,
        "character",
        "character",
      },

    /* DOUBLE_PRECISION */
      { UNUSED,
        "double precision",
        "double precision",
      },

    /* COMPLEX */
      { UNUSED,
        "complex",
        "complex",
      },

    /* LOGICAL */
      { UNUSED,
        "logical",
        "logical",
      },

    /* EXACT */
      { UNUSED,
        "exact precision",
        "exact precision",
      },

    /* EXIST_QUERY */
      { UNUSED,
        "exist = %1",
        "exist = &var&",
      },

    /* OPENED_QUERY */
      { UNUSED,
        "opened = %1",
        "opened = &var&",
      },

    /* NUMBER_QUERY */
      { UNUSED,
        "number = %1",
        "number = &var&",
      },

    /* NAMED_QUERY */
      { UNUSED,
        "named = %1",
        "named = &var&",
      },

    /* NAME_QUERY */
      { UNUSED,
        "name = %1",
        "name = &var&",
      },

    /* ACCESS_QUERY */
      { UNUSED,
        "access = %1",
        "access = &var&",
      },

    /* SEQUENTIAL_QUERY */
      { UNUSED,
        "sequential = %1",
        "sequential = &var&",
      },

    /* DIRECT_QUERY */
      { UNUSED,
        "direct = %1",
        "direct = &var&",
      },

    /* FORM_QUERY */
      { UNUSED,
        "form = %1",
        "form = &var&",
      },

    /* FORMATTED_QUERY */
      { UNUSED,
        "formatted = %1",
        "formatted = &var&",
      },

    /* UNFORMATTED_QUERY */
      { UNUSED,
        "unformatted = %1",
        "unformatted = &var&",
      },

    /* RECL_QUERY */
      { UNUSED,
        "recl = %1",
        "recl = &var&",
      },

    /* BLANK_QUERY */
      { UNUSED,
        "blank = %1",
        "blank = &var&",
      },

    /* NEXTREC_QUERY */
      { UNUSED,
        "nextrec = %1",
        "nextrec = &var&",
      },

    /* IOSTAT_QUERY */
      { UNUSED,
        "iostat = %1",
        "iostat = &var&",
      },

    /* FILE_SPECIFY */
      { UNUSED,
        "file = %1",
        "file = &expr&",
      },

    /* FMT_SPECIFY */
      { UNUSED,
        "{show(%0)fmt = }%1",
        "fmt = &format&",
      },

    /* REC_SPECIFY */
      { UNUSED,
        "rec = %1",
        "rec = &expr&",
      },

    /* END_SPECIFY */
      { UNUSED,
        "end = %1",
        "end = &lbl&",
      },

    /* STATUS_SPECIFY */
      { UNUSED,
        "status = %1",
        "status = &expr&",
      },

    /* ACCESS_SPECIFY */
      { UNUSED,
        "access = %1",
        "access = &expr&",
      },

    /* FORM_SPECIFY */
      { UNUSED,
        "form = %1",
        "form = &expr&",
      },

    /* RECL_SPECIFY */
      { UNUSED,
        "recl = %1",
        "recl = &expr&",
      },

    /* BLANK_SPECIFY */
      { UNUSED,
        "blank = %1",
        "blank = &expr&",
      },

    /* UNIT_SPECIFY */
      { UNUSED,
        "{show(%0)unit = }%1",
        "unit = &unit&",
      },

    /* ERR_SPECIFY */
      { UNUSED,
        "err = %1",
        "err = &lbl&",
      },

    /* LABEL_DEF */
      { UNUSED,
        "%s",
        "&lbl&",
      },

    /* LABEL_REF */
      { UNUSED,
        "%s",
        "&lbl&",
      },

    /* COMPLEX_CONSTANT */
      { UNUSED,
        "(%1, %2)",
        "(&constant&, &constant&)",
      },

    /* CONSTANT */
      { UNUSED,
        "{parens(%0)(}%s{parens(%0))}",
        (char *) 0,
      },

    /* LETTER */
      { UNUSED,
        "%s",
        (char *) 0,
      },

    /* TEXT */
      { UNUSED,
        "%s",
        (char *) 0,
      },

    /* IDENTIFIER */
      { UNUSED,
        "{parens(%0)(}{emphasize(%0)\\b}%s{emphasize(%0)\\b}{parens(%0))}",
        (char *) 0,
      },

    /* STAR */
      { UNUSED,
        "{parens(%0)(}*{parens(%0))}",
        "*",
      },

    /* RETURN_LABEL */
      { UNUSED,
        "*%1",
        "*&lbl&",
      },

    /* TYPE_LEN */
      { UNUSED,
        "%1{exists(%2)*%2}",
        "&type-name& * &len&",
      },

    /* DIM */
      { UNUSED,
        "{exists(%1)%1:}%2",
        "&expr&:&expr&",
      },

    /* REPEAT */
      { UNUSED,
        "%1*%2",
        "&expr& * &constant&",
      },

    /* DECL */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* PARALLELLOOP */
      { UNUSED,
        (char *) 0,
        "parallel loop &loop-control&",	/* exists? -- "label"? -- SKW */
      },

    /* PRIVATE */
      { SPRIVATE_STAT,
        "{exists(%1)%1}	private %2",
        "private &name...&",
      },

    /* STOPLOOP */
      { SSTOP_LOOP_STAT,
        "{exists(%1)%1}	stop loop{exists(%2) %2}",
        "stop loop &lbl&",
      }, 

    /* NONE */
      { UNUSED,
        (char *) 0,
        (char *) 0,
      },

    /* TRIPLET */
      { UNUSED,
        "%1:%2{exists(%3):%3}",
        "&expr&:&expr&:&expr&",
      }
  };




/* unparse table for statement tokens */

static unparseTable statInfo[] =
  {
    { SBLOCK_DATA_STAT,
      "{exists(%1)%1}	block data{show(%2) %2}",
      (char *) 0,
    },

    { SDEBUG_STAT,
      "{exists(%1)%1}	debug %2",
      (char *) 0,
    },

    { SDO_STAT,
      "{exists(%1)%1}	do %3",
      (char *) 0,
    },

    { SDO_LABEL_STAT,
      "{exists(%1)%1}	do %2{comma(%2),} %3",
      (char *) 0,
    },

    { SDOALL_STAT,
      "{exists(%1)%1}	forall %3",
      (char *) 0,
    },

    { SDOALL_LABEL_STAT,
      "{exists(%1)%1}	forall %2{comma(%2),} %3",
      (char *) 0,
    },

    { SELSE_IF_STAT,
      "{exists(%1)%1}	elseif (%2) then",
      (char *) 0,
    },

    { SELSE_STAT,
      "{exists(%1)%1}	else",
      (char *) 0,
    },

    { SEND_ALL_STAT,
      "{exists(%1)%1}	endfor",
      (char *) 0,
    },

    { SEND_DO_STAT,
      "{exists(%1)%1}	enddo",
      (char *) 0,
    },

    { SEND_IF_STAT,
      "{exists(%1)%1}	endif",
      (char *) 0,
    },

    { SEND_STAT,
      "{exists(%1)%1}	end",
      (char *) 0,
    },

    { SEND_LOOP_STAT,
      "{exists(%1)%1}	end loop",
      (char *) 0,
    },

    { SFUNCTION_STAT,
      "{exists(%1)%1}	{exists(%2)%2 }function %3(%4)",
      (char *) 0,
    },

    { SIF_STAT,
      "{exists(%1)%1}	if (%2) then",
      (char *) 0,
    },

    { SOTHER_PROCESSES_STAT,
      "{exists(%1)%1}	other processes:",
      (char *) 0,
    },

    { SPARALLELLOOP_STAT,
      "{exists(%1)%1}	parallel loop %3",
      (char *) 0,
    },

    { SPARALLELLOOP_LABEL_STAT,
      "{exists(%1)%1}	parallel loop %2{comma(%2),} %3",
      (char *) 0,
    },

    { SPARALLEL_STAT,
      "{exists(%1)%1}	parallel:",
      (char *) 0,
    },

    { SPARALLEL_PID_STAT,
      "{exists(%1)%1}	parallel:  (%2)",
      (char *) 0,
    },

    { SPARBEGIN_STAT,
      "{exists(%1)%1}	parbegin",
      (char *) 0,
    },

    { SPARBEGIN_PID_STAT,
      "{exists(%1)%1}	parbegin (%2 = %3)",
      (char *) 0,
    },

    { SPAREND_STAT,
      "{exists(%1)%1}	parend",
      (char *) 0,
    },

    { SPROGRAM_STAT,
      "{exists(%1)%1}	program{show(%2) %2}",
      (char *) 0,
    },

    { SPRSCOPE_PH_STAT,
      "	%1",
      (char *) 0,
    },

    { SSTMT_PH_STAT,
      "	%1",
      (char *) 0,
    },

    { SSPECIFICATION_STMT_PH_STAT,
      "	%1",
      (char *) 0,
    },

    { SCONTROL_STMT_PH_STAT,
      "	%1",
      (char *) 0,
    },

    { SIO_STMT_PH_STAT,
      "	%1",
      (char *) 0,
    },

    { SPARASCOPE_STMT_PH_STAT,
      "	%1",
      (char *) 0,
    },

    { SDEBUG_STMT_PH_STAT,
      "	%1",
      (char *) 0,
    },

    { SSUBROUTINE_STAT,
      "{exists(%1)%1}	subroutine %2{exists(%3)(%3)}",
      (char *) 0,
    }

};









/************************/
/* Forward declarations	*/
/************************/




STATIC(TextString, makeLineTextString, (char *s));
STATIC(void, placeholderExpandee, (FortTreeNode node, int type, Flex *expansions));
STATIC(void, addExpansion, (Flex *expansions, FortTreeNode node, int value,
                            int type, TextString title, Boolean first));
STATIC(void, placeholderExpand, (FortTextTree ftt, fx_Expansion ex,
                                 FortTreeNode *New, FortTreeNode *focus));
STATIC(FortTreeNode, firstPlaceholder, (FortTreeNode node));
STATIC(void, subExpand, (FortTextTree ftt, FortTreeNode *root, FortTreeNode *focus));
STATIC(Boolean, is_trivial_ph, (FortTreeNode root, fx_Expansion *ex));
STATIC(void, removeExpansions, (FortTextTree ftt));
STATIC(void, getFunction, (char **fmt, char *funcName, int *selector));
STATIC(Boolean, findStmtExpansion, (Flex *expansions, fx_Expansion *ex));

/************************************************************************/
/*	Text -> FTT and FTT -> Text Operations 				*/
/************************************************************************/
typedef struct node {
  char        *s;
  struct node *next;
} CNode;

typedef struct clist {
   CNode *head, *tail;
} *CList;

typedef struct ostate_t {
  FortTextTree        ftt;
  SignificantCommentHandlers *schandlers;
  char                continuationCharacter;
  int                 currentSourceLine;
  int                 currentOutputLine;
  FILE               *mapf;
  FortTreeSideArray   mapData;  
} ppstate;

typedef struct l_ {
    char	*text;
    AST_INDEX	 node;
    int		 bracket;
} Line;

STATIC(Boolean, isInclude, (char *buf, char **include_name));
STATIC(Boolean, isComment, (char *s));
STATIC(Boolean, isNewExecutable, (char *s));
STATIC(char*, getTabExpandedLine, (FILE *fp));
STATIC(char*, extractFront, (CList cl));
STATIC(void, insertComment, (CList cl, char *s));
STATIC(CList,initComments, (void));
STATIC(Boolean, empty, (CList cl));
STATIC(int, writeLine, (ppstate *state, MapInfoOpaque Map, 
		                ftt_ListingCallBack exportLine, va_list exportLineArgs));

/*
 * 7 is a nice number, add 1 for the source file itself..
 */

# define       MAX_INCLUDE_DEPTH (7 + 1)
struct include_stack {
  char   *file_name;
  FILE   *fp;
  char	 *newStmtPrefix;
  char	 *currentLine;
  CList	  comments;
  Boolean firsttime;
} IncStack[MAX_INCLUDE_DEPTH];

#define COMMENTS IncStack[current_include_depth].comments
#define NEW_STMT_PREFIX IncStack[current_include_depth].newStmtPrefix
#define CURRENT_LINE IncStack[current_include_depth].currentLine
#define FIRSTTIME IncStack[current_include_depth].firsttime

static int  current_include_depth;

STATIC(char*, getStmt, (FILE *fp));
STATIC(void, ftt_import, (Context context, FortTextTree ftt));
STATIC(int, ftt_export, (FortTextTree ftt, FILE *outf, int continuationCharacter,
                          SignificantCommentHandlers *schandlers));

/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void ftt_Init()
{
  if( ftt_InitCount++ == 0 )
    { /* initialize submodules */
        tt_Init();
        unp1_Init();
        unp2_Init();
        fp1_Init();
        fp2_Init();
    }
}




void ftt_Fini()
{
  if( --ftt_InitCount == 0)
    { /* finalize submodules */
        fp2_Fini();
        fp1_Fini();
        unp2_Fini();
        unp1_Fini();
        tt_Fini();
    }
}




FortTextTree ftt_Create(FortTree ft)
{
  FortTextTree ftt;
  TextTree tt;
  static Generic zeros[tt_EXTRASIZE];
  
  /* allocate a new instance */
  ftt = (FortTextTree) get_mem(sizeof(ftt_Repr),"FortEditor:FortTextTree");
  if( (Generic) ftt == 0 ) return UNUSED;
  
  /* initialize the parts */
  /* set creation parameters */
  R(ftt)->ft = ft;
  
  /* initialize side array needed by TextTree */
  bzero((char *) zeros, sizeof(zeros));
  R(ftt)->sideArray = ft_AttachSideArray(ft, tt_EXTRASIZE, zeros);
  
  /* open the TextTree */
  tt = tt_Create((TT_Tree) ftt, (TT_TreeNode) ft_Root(ft),
		 &ftt_methods, sizeof(fx_StatToken));
  R(ftt)->tt = tt;
  
  /* create an empty expansion list */
  R(ftt)->expansions = flex_create(sizeof(fx_Expansion));
  
  return ftt;
}




int ftt_Read(FortTextTree ftt, DB_FP *file)
{
  ft_ReadSideArrayFromFile(R(ftt)->ft, R(ftt)->sideArray, file);
  
  /* read the TextTree */
  tt_Read(R(ftt)->tt, file);
  
  return ReadCookie(file);
}




int ftt_Write(FortTextTree ftt, DB_FP *file)
{
  ft_WriteSideArrayToFile(R(ftt)->ft, R(ftt)->sideArray, file);

  /* write the TextTree */
  tt_Write(R(ftt)->tt, file);
  
  return WriteCookie(file);
}



/*
 * Author: Alan Carle, June 25, 1992
 */

static int exportToFILE(int lineno, char *line, va_list args)
{
  FILE *fp = va_arg(args, FILE*);
  return (fprintf(fp, "%s\n", line) == EOF) ? EOF: 0;
}


void ftt_ExportToFile(FortTextTree ftt, FILE *outf, 
		      char continuationCharacter, 
		      SignificantCommentHandlers *schandlers)
{
  ftt_Export(ftt, continuationCharacter, schandlers, exportToFILE, outf);
}



void ftt_Close(FortTextTree ftt)
{
  tt_Close(R(ftt)->tt);
  ft_DetachSideArray(R(ftt)->ft, R(ftt)->sideArray);
  flex_destroy(R(ftt)->expansions);
  free_mem((void *)ftt);
}




/************************/
/*  Contents as tree	*/
/************************/




FortTree ftt_Tree(FortTextTree ftt)
{
  return R(ftt)->ft;
}




FortTreeNode ftt_GetFather(FortTextTree ftt, FortTreeNode node)
{
  return (FortTreeNode) getFather(ftt, node);
}




FortTreeNode ftt_Root(FortTextTree ftt)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  return (FortTreeNode) tt_Root(R(ftt)->tt);
}




void ftt_TreeWillChange(FortTextTree ftt, FortTreeNode node)
{
  ft_AstSelect(R(ftt)->ft);

  tt_TreeWillChange(R(ftt)->tt, (TT_TreeNode) node);
}




void ftt_TreeChanged(FortTextTree ftt, FortTreeNode node)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_TreeChanged(R(ftt)->tt, (TT_TreeNode) node);
}




Boolean ftt_NodeToID(FortTextTree ftt, FortTreeNode node, int *id)
{
  return tt_NodeToID(R(ftt)->tt, node, id);
}




Boolean ftt_IDToNode(FortTextTree ftt, int id, FortTreeNode *node)
{
  return tt_IDToNode(R(ftt)->tt, id, (TT_TreeNode*)node);
}










/************************/
/*  Contents as text	*/
/************************/




Point ftt_GetDocSize(FortTextTree ftt)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  return (tt_GetDocSize(R(ftt)->tt));
}




int ftt_NumLines(FortTextTree ftt)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  return tt_NumLines(R(ftt)->tt);
}




void ftt_GetLine(FortTextTree ftt, int lineNum, TextString *text)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_GetLine(R(ftt)->tt,lineNum,text);
}




char *ftt_GetTextLine(FortTextTree ftt, int lineNum)
{
  TextString ts;
  char *text;
  int i;

  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_GetLine(R(ftt)->tt,lineNum,&ts);

  /* flatten the text-string (losing placeholder information) */
    text = (char *) get_mem(ts.num_tc + 1, "ftt_GetTextLine");
    for( i = 0; i < ts.num_tc; i++ )
      text[i] = ts.tc_ptr[i].ch;
    text[ts.num_tc] = '\0';

  destroyTextString(ts);

  return text;
}



void ftt_SetLine(FortTextTree ftt, int lineNum, TextString text)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_SetLine(R(ftt)->tt,lineNum,text);
}




void ftt_SetTextLine(FortTextTree ftt, int lineNum, char *text)
{
  TextString ts;

  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  ts = makeLineTextString(text);
  tt_SetLine(R(ftt)->tt,lineNum,ts);
  destroyTextString(ts);
}




void ftt_InsertLine(FortTextTree ftt, int lineNum, TextString text)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_InsertLine(R(ftt)->tt,lineNum,text);
}




void ftt_InsertTextLine(FortTextTree ftt, int lineNum, char *text)
{
  TextString ts;

  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  ts = makeLineTextString(text);
  tt_InsertLine(R(ftt)->tt,lineNum,ts);
  destroyTextString(ts);
}




void ftt_DeleteLine(FortTextTree ftt, int lineNum)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_DeleteLine(R(ftt)->tt,lineNum);
}




void ftt_GetLineInfo(FortTextTree ftt, int lineNum, FortTreeNode *node, 
                     int *bracket)
{
  TT_Line info;

  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_GetLineInfo(R(ftt)->tt,lineNum,false,&info);
  *node    = info.lineNode;
  *bracket = info.bracket;
}




int ftt_GetLineIndent(FortTextTree ftt, int lineNum)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  return (tt_GetLineIndent(R(ftt)->tt,lineNum) + FIXED_INDENT);
}




int ftt_GetLineLength(FortTextTree ftt, int lineNum)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  return (tt_GetLineLength(R(ftt)->tt,lineNum));
}




void ftt_GetConceal(FortTextTree ftt, int lineNum, Boolean *conceal)
{
  tt_GetConceal(R(ftt)->tt,lineNum,conceal);
}




void ftt_SetConceal(FortTextTree ftt, int lineNum1, int lineNum2, Boolean conceal)
{
  tt_SetConceal(R(ftt)->tt,lineNum1,lineNum2,conceal);
}




void ftt_SetConcealNone(FortTextTree ftt, int lineNum1, int lineNum2)
{
  tt_SetConcealNone(R(ftt)->tt,lineNum1,lineNum2);
}




void ftt_SetConcealCount(FortTextTree ftt, int lineNum, int iconceal)
{
  tt_SetConcealCount(R(ftt)->tt,lineNum,iconceal);
}




void ftt_GetConcealCount(FortTextTree ftt, int lineNum, int *iconceal)
{
  tt_GetConcealCount(R(ftt)->tt,lineNum,iconceal);
}




Boolean ftt_IsErroneous(FortTextTree ftt, int lineNum)
{
  TT_Line info;
  FortTextTree node, compound;

  tt_GetLineInfo(R(ftt)->tt,lineNum,false,&info);
  node = (FortTreeNode)info.lineNode;
  return ft_IsErroneous(R(ftt)->ft,node,info.bracket);
}




void ftt_GetErrorMessage(FortTextTree ftt, int lineNum, char *Message)
{
  TT_Line info;
  FortTreeNode node;

  tt_GetLineInfo(R(ftt)->tt,lineNum,false,&info);
  node = (FortTreeNode)info.lineNode;
  ft_GetErrorMessage(R(ftt)->ft, node, info.bracket, Message);
}




/*-------------------------------------------------------------------
   ftt_Listing: support for creating listings files, including errors 
  
   pre-condition: ft_Check must have already been called to ensure
                  that errors (if any) are marked in the tree
 --------------------------------------------------------------------*/
void ftt_Listing(FortTextTree ftt, 
		 ftt_ListingCallBack reportGoodLine, 
		 ftt_ListingCallBack reportBadLine, 
		 ftt_ListingCallBack reportErrorLine, ...)
{ 
  va_list args;
  int fttLines =  ftt_NumLines(ftt);
  int currentLine = 0;

  va_start(args, reportErrorLine);
  
  for (; currentLine < fttLines; currentLine++) {
    if (ftt_IsErroneous(ftt, currentLine)) {
      if (reportBadLine != NULL) {
	char *linetext = ftt_GetTextLine(ftt, currentLine);
	reportBadLine(currentLine, linetext, args);
	sfree(linetext);
      }
      if (reportErrorLine != NULL) {
	char error[200];
	ftt_GetErrorMessage(ftt, currentLine, error);
	reportErrorLine(currentLine, error, args);
      }
    } else if (reportGoodLine != NULL) {
      char *linetext = ftt_GetTextLine(ftt, currentLine);
      reportGoodLine(currentLine, linetext, args);
      sfree(linetext);
    }
  }
  
  va_end(args);
}
    




/************************/
/*  Mapping		*/
/************************/




Boolean ftt_TextToNode(FortTextTree ftt, int line1, int char1, int line2, 
                       int char2, FortTreeNode *node)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  return tt_TextToNode(R(ftt)->tt, line1, char1, line2, char2,
				(TT_TreeNode *) node);
}




Boolean ftt_NodeToText(FortTextTree ftt, FortTreeNode node, int *line1, 
                       int *char1, int *line2, int *char2)
{
  Boolean isLineRange;

  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  isLineRange = tt_NodeToText(R(ftt)->tt, (TT_TreeNode) node,
                                             line1, char1, line2, char2);
  return BOOL( isLineRange  &&  !is_place_holder(node) );
}



char *
ftt_NodeToStr(FortTextTree ftt, FortTreeNode node)
{
  char buf[512];
  int line1;
  int char1;
  int line2;
  int char2;
  int l;
  char *line;

  (void) ftt_NodeToText(ftt, node, &line1, &char1, &line2, &char2);
  line = (char *) ftt_GetTextLine(ftt, line1);
  l = char2 + 1 - (char1 - 1) + 1;
  (void) strncpy (buf, &line[char1 - 1], l);
  buf[l] = '\0';
  sfree(line);
  return (ssave(buf));
}



void ftt_GetChanges(FortTextTree ftt, int *first, int *last, int *delta, 
                    FortTreeNode *node)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_GetChanges(R(ftt)->tt, first, last, delta, (TT_TreeNode *) node);
}






/************************/
/*  Expanding		*/
/************************/




int ftt_IPExpandee(FortTextTree ftt, int lineNum, int charNum)
{
  Flex * expansions = R(ftt)->expansions;
  TT_Line line;
  FortTreeNode node;
  int n,k;
  fx_Expansion ex,dummy;

  ft_AstSelect(R(ftt)->ft);

  removeExpansions(ftt);

  (void) ftt_TextToNode(ftt, lineNum, 0, lineNum, 100, &node);

  if( is_comment(node)  &&  gen_COMMENT_get_text(node) == AST_NIL )
    { unp2_Expandee(ftt, node, expansions);
      if( findStmtExpansion(expansions, &dummy) )
        /* trick for kb shorthand expansion of blank lines */
          placeholderExpandee(UNUSED, GEN_stmt, expansions);
    }
  else
    { tt_GetLineInfo(R(ftt)->tt, lineNum, false, &line);
      unp1_Expandee(ftt, line, charNum, expansions);
    }

  return flex_length(expansions);
}




int ftt_NodeExpandee(FortTextTree ftt, FortTreeNode node)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  removeExpansions(ftt);
  if( is_place_holder(node) )
    placeholderExpandee(node, UNUSED, R(ftt)->expansions);
  else if( is_list(node)  &&  list_length(node) == 1  &&
				is_place_holder(list_first(node)) )
    placeholderExpandee(list_first(node), UNUSED, R(ftt)->expansions);

  return flex_length(R(ftt)->expansions);
}




int ftt_GetExpansionNames(FortTextTree ftt, Boolean firstOnly, 
                          TextString nameList[], int numList[])
{
  int num = flex_length(R(ftt)->expansions);
  fx_Expansion ex;
  int i,j;

  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  j = 0;
  for( i = 0; i < num; i++ )
    { (void) flex_get_buffer(R(ftt)->expansions, i, 1, (char *) &ex);
      if( (firstOnly && ex.first) || ! firstOnly )
        { if( nameList != nil )  nameList[j] = ex.title;
          if( numList  != nil )  numList [j] = i;
          j += 1;
        }
    }

  return j;
}




void ftt_Expand(FortTextTree ftt, int choice, FortTreeNode *New, 
                FortTreeNode *focus)
{
  fx_Expansion ex,ex0;

  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  (void) flex_get_buffer(R(ftt)->expansions, choice, 1, (char *) &ex);
  switch( ex.who )
    { case PLACEHOLDER_EXPANDER:
        placeholderExpand(ftt, ex, New, focus);
        break;

      case UNPARSE1_EXPANDER:
        unp1_Expand(ftt, ex, New, focus);
        break;

      case UNPARSE2_EXPANDER:
        /* trick for kb shorthand expansion of blank lines */
          if( ex.node == UNUSED )
            { (void) findStmtExpansion(R(ftt)->expansions, &ex0);
              unp2_Expand(ftt, ex0, New, focus);
              ex.node = *New;
              placeholderExpand(ftt, ex, New, focus);
            }
          else
            unp2_Expand(ftt, ex, New, focus);
        break;

      default:
        die_with_message("bogus expansion requested");
	/*NOTREACHED*/
    }
    
  subExpand(ftt, New, focus);
}






/************************/
/*  Viewing		*/
/************************/




short ftt_ViewScreenModuleIndex()
{
  return tt_ViewScreenModuleIndex();
}




Point ftt_ViewSize(Point charSize, short font)
{
  return tt_ViewSize(charSize,font);
}




void ftt_ViewInit(FortTextTree ftt, FortTextTreeView pane, Rectangle viewRect)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_ViewInit(R(ftt)->tt, pane, viewRect);
}




void ftt_ViewGet(FortTextTree ftt, FortTextTreeView pane, Rectangle *viewRect)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_ViewGet(R(ftt)->tt, (TextTreeView) pane, viewRect);
}




void ftt_ViewSet(FortTextTree ftt, FortTextTreeView pane, Rectangle viewRect)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_ViewSet(R(ftt)->tt, (TextTreeView) pane, viewRect);
}




void ftt_ViewScroll(FortTextTree ftt, FortTextTreeView pane, int dx, int dy)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_ViewScroll(R(ftt)->tt, (TextTreeView) pane, dx, dy);
}




void ftt_BeginEdit(FortTextTree ftt)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_BeginEdit(R(ftt)->tt);
}




void ftt_EndEdit(FortTextTree ftt)
{
  /* select the tree */
    ft_AstSelect(R(ftt)->ft);

  tt_EndEdit(R(ftt)->tt);
}






/************************************************************************/
/*	Internal Operations for Subparts 				*/
/************************************************************************/




void ftt_getTextTree(FortTextTree ftt, TextTree *tt)
{
  *tt = R(ftt)->tt;
}





Boolean ftt_nodeIsLevel2(FortTreeNode node)
{
  FortTreeNode parent;
  int sonind;
  NODE_TYPE listType;

  if ( is_statement(node) )
    return NOT( is_logical_if(ast_get_father(node)) );
  else if ( is_list_of_nodes(node) )
    { parent = ast_get_father(node);
      sonind = gen_which_son(parent, node);
      listType = THE_TYPE(get_son(sonind, gen_get_node_type(parent)));
      switch ( listType )
        { case GEN_subprogram:
          case GEN_stmt:
          case GEN_guard:
          case GEN_parallel_case:
            return true;
          default:
            return false;
        }
    }
  else if ( is_place_holder(node) )
    switch ( gen_get_meta_type(node) )
      { case GEN_subprogram:
        case GEN_stmt:
        case GEN_specification_stmt:
        case GEN_control_stmt:
        case GEN_io_stmt:
        case GEN_parascope_stmt:
        case GEN_debug_stmt:
          return true;
        default:
          return false;
      }
  else
    return false;
}




int ftt_nodeTypeToTokenType(NODE_TYPE nt)
{
  return nodeInfo[nt].stmtToken;
}




char * ftt_getTokenFormat(int tok)
{
  int i;

  for ( i = 0; i < sizeof(statInfo) / sizeof(*statInfo); i++ )
    if ( statInfo[i].stmtToken == tok )
      return statInfo[i].format;

  return (char *) 0;
}




char * ftt_getNodeFormat(NODE_TYPE nt)
{
  return nodeInfo[nt].format;
}




void ftt_format(char *fmt, Generic ob, ftt_FormatCustomProcs *custom)
{
  char ch;
  char funcName[64];
  int selector;
  int pending_opens;

  while( ch = *fmt++ )
    switch( ch )
      {
        case '%':
          ch = *fmt++;
          if( ch == 's' )
            custom->outputText(ob);
          else
            custom->outputSon(ob, ch - '0');
          break;

        case '\\':
          ch = *fmt++;
          switch( ch )
            {
              case 'b':
                ftt_toggleBold();
                break;
              case 'i':
                ftt_toggleItalic();
                break;
              case 'h':
                ftt_toggleHalf();
                break;
              case 'u':
                ftt_toggleUnderline();
                break;
              case 'r':
                ftt_toggleReverse();
                break;
              default:
                custom->output(ob, ch);
            }
          break;

        case '\t':
          custom->tab(ob);
          break;

        case OPEN_BRACKET:
          getFunction(&fmt, funcName, &selector);
          custom->missing(ob, funcName, selector, 0);
          if( ! custom->eval(ob, funcName, selector) )
            { /* walk past the omitted part */
                for ( pending_opens = 1; pending_opens; fmt++ )
                  { pending_opens += (*fmt == OPEN_BRACKET);
                    pending_opens -= (*fmt == CLOSE_BRACKET);
                  }
              custom->missing(ob, funcName, selector, 1);
            }
          break;

        case CLOSE_BRACKET:
          custom->missing(ob, (char *) 0, UNUSED, 2);
          break;

        default:
          custom->output(ob, ch);
          break;
      }
}




void ftt_toggleBold(void)
{
  if( (ftt_style & STYLE_MASK) == STYLE_BOLD )
    ftt_style = (ftt_style & ~STYLE_MASK) | STYLE_NORMAL;
  else
    ftt_style = (ftt_style & ~STYLE_MASK) | STYLE_BOLD;
}




void ftt_toggleItalic(void)
{
  if( (ftt_style & STYLE_MASK) == STYLE_ITALIC )
    ftt_style = (ftt_style & ~STYLE_MASK) | STYLE_NORMAL;
  else
    ftt_style = (ftt_style & ~STYLE_MASK) | STYLE_ITALIC;
}




void ftt_toggleReverse(void)
{
  ftt_style ^= ATTR_INVERSE;
}




void ftt_toggleHalf(void)
{
  ftt_style ^= ATTR_HALF;
}




void ftt_toggleUnderline(void)
{
  ftt_style ^= ATTR_UNDERLINE;
}




TextString ftt_expansionName(int type)
{
  char * name;
  Boolean ph;

  if( is_meta(type) )
    { name = gen_meta_type_get_text(THE_TYPE(type));
      ph = true;
    }
  else
    if( nodeInfo[type].expansionName != (char *) 0 )
      { name = nodeInfo[type].expansionName;
        ph   = false;
      }
    else
      { name = gen_node_type_get_text(type);
        ph   = true;
      }

  return ftt_makeExpansionName(name, ph);
}




TextString ftt_makeExpansionName(char *name, Boolean ph_default)
{
  int len = (int)strlen(name);
  int k;
  TextString ts;
  char ch;
  Boolean ph;

# define insert(ch)	ts.tc_ptr[ts.num_tc++] = makeTextChar(ch,style)
# define style    (ph ? ftt_PLACEHOLDER_STYLE : STYLE_NORMAL) | STYLE_BOLD | 0xC0

  /* allocate a sufficiently large TextString, initially 0-length */
    ts = createTextString(len, "ftt_expansionName");
    ts.num_tc = 0;

  /* for each char of 'name', either insert in 'ts' or obey special meaning */
    ph = ph_default;

    for( k = 0;  k < len;  k++ )
      { ch = name[k];
        switch( ch )
          {
            case '&':
              ph = NOT( ph );
              break;

            case '.':
              if( len - k >= 3  &&  strncmp(&name[k], "...", 3) == 0 )
                { k += 3-1;
                  insert(ELLIPSIS_CHAR);
                }
              else
                insert(ch);
              break;

            case '_':
              insert('-');
              break;

            default:
              if( isupper(ch) )
                insert(tolower(ch));
              else
                insert(ch);
              break;
          }
      }

  return ts;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*ARGSUSED*/
static
TT_TreeNode getFather(FortTextTree ftt, FortTreeNode node)
{
  ft_AstSelect(R(ftt)->ft);

    if( in_list(node) )
      return list_head(node);
    else
      return tree_out(node);
}




static
void getExtra(FortTextTree ftt, FortTreeNode node, int k, int *value)
{
  ft_AstSelect(R(ftt)->ft);

  *value = ft_GetFromSideArray(R(ftt)->sideArray, node, k);
}




static
void setExtra(FortTextTree ftt, FortTreeNode node, int k, int value)
{
  ft_AstSelect(R(ftt)->ft);

  ft_PutToSideArray(R(ftt)->sideArray, node, k, value);
}




static
TextString makeLineTextString(char *s)
{
  TextString ts;

  ts = createTextString(strlen(s) + FIXED_INDENT, "makeLineTextString");
  ts.num_tc = 0;
  while( *s )
    { switch( *s )
        { case '\n':
	    break;

          case '\t':
	    do
            { ts.tc_ptr[ts.num_tc++] = makeTextChar(' ', STYLE_NORMAL);
            } while ( ts.num_tc < FIXED_INDENT );
            break;

          default:
            ts.tc_ptr[ts.num_tc++] = makeTextChar(*s, STYLE_NORMAL);
            break;
        }
      s++;
    }
  return ts;
}




static
void placeholderExpandee(FortTreeNode node, int type, Flex *expansions)
{
  Boolean first = BOOL( type == UNUSED );
  int numChoices;
  int exptype, k;

  if( type == UNUSED )  type = gen_get_meta_type(node);
  numChoices = metamap[type].count;

  for( k = 0;  k < numChoices;  k++ )
    { exptype = metamap[type].exp[k];
      if( is_meta(exptype) )
        { addExpansion(
              expansions,
              node,
              0,
              THE_TYPE(exptype),
              ftt_expansionName(exptype),
              first
              );
          placeholderExpandee(node, THE_TYPE(exptype), expansions);
        }
      else
        addExpansion(
            expansions,
            node,
            1,
            exptype,
            ftt_expansionName(exptype),
            first
            );
    }
}




static
void addExpansion(Flex *expansions, FortTreeNode node, int value, 
                  int type, TextString title, Boolean first)
{
  fx_Expansion ex;
  int This = flex_length(expansions);
  short who = (node == UNUSED ? UNPARSE2_EXPANDER : PLACEHOLDER_EXPANDER);
                  /* trick for kb shorthand expansion of blank lines */

  ex.node   = node;
  ex.value  = value;
  ex.type   = type;
  ex.title  = title;
  ex.who    = who;
  ex.first  = first;

  flex_insert_one(expansions, This, (char *) &ex);
}




static
void placeholderExpand(FortTextTree ftt, fx_Expansion ex, FortTreeNode *New, 
                       FortTreeNode *focus)
{
  FortTreeNode part, parent;
   
  parent = tree_out(ex.node);
  switch( ex.value )
    {
      case 0:
        ftt_TreeWillChange(ftt, ex.node);
        gen_put_meta_type(ex.node, ex.type);
        ftt_TreeChanged(ftt, ex.node);
        *New = *focus = ex.node;
        break;

      case 1:
        switch( ex.type )
          { case GEN_FUNCTION:
              *New = gen_FUNCTION(
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_type), 
                  ph_from_mtype(GEN_name), 
                  list_create(ph_from_mtype(GEN_formal)), 
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_PROGRAM:
              *New = gen_PROGRAM(
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_name), 
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_SUBROUTINE:
              *New = gen_SUBROUTINE(
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_name), 
                  list_create(ph_from_mtype(GEN_formal)), 
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_BLOCK_DATA:
              *New = gen_BLOCK_DATA(
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_name), 
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_DEBUG:
              *New = gen_DEBUG(
                  AST_NIL,
                  AST_NIL,
                  *focus = list_create(ph_from_mtype(GEN_option)), 
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_DO:
              *New = gen_DO(
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_loop_control), 
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_IF:
              *New = gen_IF(
                  AST_NIL,
                  AST_NIL,
                  list_create(gen_GUARD(AST_NIL, *focus = ph_from_mtype(GEN_expr),
                                     list_create(gen_COMMENT(AST_NIL))))
              );
              break;

            case GEN_LOGICAL_IF:
              *New = gen_LOGICAL_IF(
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_expr),
                  list_create(ph_from_mtype(GEN_stmt))
              );
              break;

            case GEN_PARALLEL:
              *New = gen_PARALLEL(
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_var),
                  ph_from_mtype(GEN_expr),
                  list_create(gen_COMMENT(AST_NIL)),
                  list_create(gen_PARALLEL_CASE(
                      AST_NIL,
                      list_create(ph_from_mtype(GEN_expr)),
                      list_create(gen_COMMENT(AST_NIL))
                  ))
              );
              break;

            case GEN_DO_ALL:
              *New = gen_DO_ALL(
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_loop_control), 
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_COMMENT:
              *New = gen_COMMENT(*focus = gen_TEXT());
              gen_put_text(*focus, "", STR_COMMENT_TEXT);
              break;

            case GEN_ARRAY_DECL_LEN:
              *New = gen_new_node(ex.type);
              if( is_dimension(parent) || is_common_elt(parent) )
                { /* take out extra parts */
                    part = gen_ARRAY_DECL_LEN_get_len(*New);
                    tree_replace(part, AST_NIL);
                    tree_free(part);
                    part = gen_ARRAY_DECL_LEN_get_init_LIST(*New);
                    tree_replace(part, AST_NIL);
                    tree_free(part);
                }
              *focus = gen_ARRAY_DECL_LEN_get_name(*New);
              break;


            case GEN_PARALLELLOOP:
              *New = gen_PARALLELLOOP(
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  *focus = ph_from_mtype(GEN_loop_control),
                  list_create(gen_COMMENT(AST_NIL))
              );
              break;

            case GEN_TYPE_STATEMENT:
              *New = gen_new_node(ex.type);
              *focus = firstPlaceholder(*New);
              ft_SetComma(*New, false);
              break;

            default:
              *New = gen_new_node(ex.type);
              *focus = firstPlaceholder(*New);
              break;
          }

        ftt_TreeWillChange(ftt, parent);
        tree_replace(ex.node, *New);
        tree_free(ex.node);
        if( is_type_len(parent)  &&  gen_TYPE_LEN_get_length(parent) == *New )
          gen_put_parens(*New, 1);
        ftt_TreeChanged(ftt, parent);
        break;
    }
}




static
FortTreeNode firstPlaceholder(FortTreeNode node)
{
  int numSons = gen_how_many_sons(NT(node));
  int k;
  FortTreeNode son;

  for( k = 1; k <= numSons; k++ )
    { son = gen_get_son_n(node, k);
      if( is_list(son) )
        return list_first(son);
      else if( is_place_holder(son) )
        return son;
    }
  return node;
}



static
void subExpand(FortTextTree ftt, FortTreeNode *root, FortTreeNode *focus)
{
  FortTreeNode node,next,dummy;
  int k,numSons;
  fx_Expansion ex;

  if( *root == AST_NIL )
    return;

  /* put this check first, so that lists won't be checked for
     being placeholders */
  else if( is_list(*root) )
    { node = list_first(*root);
      while( node != AST_NIL )
        { next = list_next(node);
          subExpand(ftt, &node, focus);
          node = next;
        }
    }

  else if( is_trivial_ph(*root, &ex) )
    { placeholderExpand(ftt, ex, root,
                        (*root == *focus ? focus : &dummy));
      subExpand(ftt, root, focus);
    }

  else
    { numSons = gen_how_many_sons(NT(*root));
      for( k = 1;  k <= numSons;  k++ )
        { node = gen_get_son_n(*root, k);
          subExpand(ftt, &node, focus);
        }
    }
}




static
Boolean is_trivial_ph(FortTreeNode root, fx_Expansion *ex)
{
  int root_type = THE_TYPE(gen_get_meta_type(root));
  int exp_type;
  Boolean trivial;

  if( NT(root) == GEN_PLACE_HOLDER  &&  metamap[root_type].count == 1 )
    { trivial = true;
      exp_type = metamap[root_type].exp[0];

      ex->node  = root;
      ex->value = (int) NOT( is_meta(exp_type) );
      ex->type  = exp_type;
      ex->title = emptyTextString;    /* not used */
      ex->who   = PLACEHOLDER_EXPANDER;
    }
  else
    trivial = false;

  return trivial;
}




static
void removeExpansions(FortTextTree ftt)
{
  int num = flex_length(R(ftt)->expansions);
  int k;
  fx_Expansion ex;

  for( k = 0;  k < num;  k++ )
    { flex_get_buffer(R(ftt)->expansions, k, 1, (char *) &ex);
      destroyTextString(ex.title);
    }
  flex_delete(R(ftt)->expansions, 0, num);
}




static
void getFunction(char **fmt, char *funcName, int *selector)
{
  char ch;
  char selectorChars[3];

  /* get the function name */
    while( (ch = *(*fmt)++) != '(' )  *funcName++ = ch;
    *funcName = '\0';

  /* get the selector argument */
    /* already skipped */	/* '(' */
    (*fmt)++;				/* '%' */
    *selector = (*(*fmt)++) - '0';	/* 0-9 */
    (*fmt)++; 			/* ')' */
}




static
Boolean findStmtExpansion(Flex *expansions, fx_Expansion *ex)
{
  int n,k;

  n = flex_length(expansions);
  for( k = 0;  k < n;  k++ )
    { (void) flex_get_buffer(expansions, k, 1, (char *) ex);
      if( THE_TYPE(ex->type) == GEN_stmt )
        return true;
    }

  return false;
}

static int  import_line_count;
static int  import_nesting_depth;

static void enter(char *line, FortTextTree ftt)
{
  ftt_InsertTextLine(ftt, import_line_count++, line);
}

static void push(char *include_file, FortTextTree ftt)
{
  char *start_line;
  
  import_nesting_depth++;

#if 0
  if (import_nesting_depth == 1)
    fprintf(stderr, "Parsing %s.\n", include_file);
  else
#else
    if (import_nesting_depth > 1)
#endif
  {
#if 0
    fprintf(stderr, "Inlining contents of included file %s.\n", include_file);
#endif
    start_line = nssave(2, "C       Start of include file ", include_file);
    ftt_InsertTextLine(ftt, import_line_count++, start_line);
    sfree(start_line);
  }
}

static void pop(char *include_file, FortTextTree ftt)
{
  char *end_line;

  import_nesting_depth--;

  if (import_nesting_depth > 1)
  {
    end_line = nssave(2, "C       End of include file ", include_file);
    ftt_InsertTextLine(ftt, import_line_count++, end_line);
    sfree(end_line);
  }
}

void ftt_ImportFromTextFile(char *loc, FortTextTree ftt)
{
  import_line_count = 0;
  import_nesting_depth = 0;

  /* parse the source */
  ftt_BeginEdit(ftt);
  ftt_TraverseText(loc, push, pop, enter, (Generic) ftt);
  ftt_EndEdit(ftt);
}



void ftt_TraverseText(char *loc, PushFunc push, PopFunc pop, EnterFunc enter, Generic other)
{
#define LINE_SIZE 5000

  static char    line[LINE_SIZE];
  int            lineCount;

  char	        *s,*t;
  FILE          *in_fp;
  char          *include_name;

  in_fp = fopen(loc, "r");
  if (in_fp == NULL)
  {
    fprintf(stderr, "Fortran source file %s cannot be opened.\n", loc);
    return;
  }

  if (push) push(loc, other);

  current_include_depth = 0;
  IncStack[current_include_depth].fp = in_fp;
  IncStack[current_include_depth].file_name = loc;
  FIRSTTIME = true;

  while (((s = getStmt(in_fp)) != NULL) || (current_include_depth > 0))
  {
    /* received EOF on reading of include file */
    if (s == NULL)
    {
      if (pop) pop(IncStack[current_include_depth].file_name, other);
      free_mem(IncStack[current_include_depth].comments);
      sfree(IncStack[current_include_depth].file_name);
      fclose(IncStack[current_include_depth].fp);
      current_include_depth--;

      /* update the cached file pointer */
      in_fp = IncStack[current_include_depth].fp;
    }
    else
    {
      /* tacking on this newline seems to be necessary, although painful */
      t = nssave(2,s,"\n");

      if (isInclude(t, &include_name))
      {
	FILE *fp;

	if (push) push(include_name, other);

	if ((current_include_depth + 1) == MAX_INCLUDE_DEPTH)
	{/* nesting depth is too great -- probably an error in the user's program */
	  fprintf(stderr, "Include nesting depth exceeds ParaScope maximum of %d.\n", MAX_INCLUDE_DEPTH);
	  if (enter) enter(t, other);
	  if (pop)   pop(include_name, other);	  
	}
	else
	{
	  fp = fopen(include_name, "r");

	  if (fp == NULL)
	  {
	    fprintf(stderr, "Could not open included file %s.\n", include_name);
	    sfree(include_name);

	    if (enter) enter(t, other);
	    if (pop)   pop(include_name, other);
	  }
	  else
	  {
	    current_include_depth++;
	    IncStack[current_include_depth].file_name = include_name;
	    IncStack[current_include_depth].fp        = fp;
	    FIRSTTIME = true;

	    /* update the cached file pointer */
	    in_fp = fp;
	  }
	}
      }
      else
      {
	if (enter) enter(t, other);
      }
      sfree(t);
      sfree(s);
    }
  }  

  if (pop) pop(loc, other);
  free_mem(IncStack[current_include_depth].comments);
  fclose(in_fp);
}

#if defined(OSF1) || defined(LINUX_ALPHA)

void ftt_TraverseTextV(char *loc, PushFuncV push, PopFuncV pop, EnterFuncV enter, 
		       va_list other)
{
#define LINE_SIZE 5000

  static char    line[LINE_SIZE];
  int            lineCount;

  char	        *s,*t;
  FILE          *in_fp;
  char          *include_name;

  in_fp = fopen(loc, "r");
  if (in_fp == NULL)
  {
    fprintf(stderr, "Fortran source file %s cannot be opened.\n", loc);
    return;
  }

  if (push) push(loc, other);

  current_include_depth = 0;
  IncStack[current_include_depth].fp = in_fp;
  IncStack[current_include_depth].file_name = loc;
  FIRSTTIME = true;

  while (((s = getStmt(in_fp)) != NULL) || (current_include_depth > 0))
  {
    /* received EOF on reading of include file */
    if (s == NULL)
    {
      if (pop) pop(IncStack[current_include_depth].file_name, other);
      free_mem(IncStack[current_include_depth].comments);
      sfree(IncStack[current_include_depth].file_name);
      fclose(IncStack[current_include_depth].fp);
      current_include_depth--;

      /* update the cached file pointer */
      in_fp = IncStack[current_include_depth].fp;
    }
    else
    {
      /* tacking on this newline seems to be necessary, although painful */
      t = nssave(2,s,"\n");

      if (isInclude(t, &include_name))
      {
	FILE *fp;

	if (push) push(include_name, other);

	if ((current_include_depth + 1) == MAX_INCLUDE_DEPTH)
	{/* nesting depth is too great -- probably an error in the user's program */
	  fprintf(stderr, "Include nesting depth exceeds ParaScope maximum of %d.\n", MAX_INCLUDE_DEPTH);
	  if (enter) enter(t, other);
	  if (pop)   pop(include_name, other);	  
	}
	else
	{
	  fp = fopen(include_name, "r");

	  if (fp == NULL)
	  {
	    fprintf(stderr, "Could not open included file %s.\n", include_name);
	    sfree(include_name);

	    if (enter) enter(t, other);
	    if (pop)   pop(include_name, other);
	  }
	  else
	  {
	    current_include_depth++;
	    IncStack[current_include_depth].file_name = include_name;
	    IncStack[current_include_depth].fp        = fp;
	    FIRSTTIME = true;

	    /* update the cached file pointer */
	    in_fp = fp;
	  }
	}
      }
      else
      {
	if (enter) enter(t, other);
      }
      sfree(t);
      sfree(s);
    }
  }  

  if (pop) pop(loc, other);
  free_mem(IncStack[current_include_depth].comments);
  fclose(in_fp);
}

#endif

/*
 * Return an sfree-able buffer containing the next Fortran stmt on file f
 *	- expand tabs in the process
 *	- join multi-line stmts (via continuation cards) into single lines
 *	  with comments embedded inside of lines getting returned after the line
 *
 * Warning: This code must only be used to read in lines from a single file at a time,
 * since it uses a global variable ``firsttime'' to maintain some of its state.
 *
 * Author: Bob Hood
 *         Alan Carle (modified) June 25, 1992
 */

/* arbitrary, should work with any length */
# define	LEN	80

static char *getStmt (FILE *fp)
{
  char		*returnStmt;
  char		*temp;
  char          *dummy;

  if ( FIRSTTIME ) {
    CURRENT_LINE = getTabExpandedLine(fp);
    if ( isComment(CURRENT_LINE) ) {
      return CURRENT_LINE;
    }
    NEW_STMT_PREFIX = CURRENT_LINE;
    FIRSTTIME     = false;
    COMMENTS = initComments();
  }

  if ( NEW_STMT_PREFIX == NULL  &&  empty(COMMENTS) )
    return NULL;

  if ( !empty(COMMENTS) )
    return extractFront(COMMENTS);
  
  CURRENT_LINE = getTabExpandedLine(fp);
  while ( CURRENT_LINE && !isNewExecutable(CURRENT_LINE) && !isInclude(CURRENT_LINE, &dummy))
  {
    if (dummy != NULL)
      sfree(dummy);

    if ( isComment(CURRENT_LINE) ) {
      insertComment(COMMENTS, CURRENT_LINE);
    } else {
      temp = NEW_STMT_PREFIX;
      NEW_STMT_PREFIX = nssave(2, temp, CURRENT_LINE+6);
      sfree(temp);
      sfree(CURRENT_LINE);
    }
    CURRENT_LINE = getTabExpandedLine(fp);
  }

  returnStmt       = NEW_STMT_PREFIX;
  NEW_STMT_PREFIX  = CURRENT_LINE;

  return returnStmt;
}


static Boolean isComment (char *s)
{
  /* NULL indicates blank line -> comment */
  return BOOL(s && (s[0] == '\0' || s[0] == 'c' || s[0] == 'C' || s[0] == '*'));
}

/* parse a line of the form 
     INCLUDE 'foo.h'
*/

#define is_a_quote(c) ((c == '\'') || (c == '"'))

static Boolean isInclude(char *buf, char **include_name)
{
  /* grab the line contents, skipping the label */
  char *temp_buf;
  char *first_blank;
  char *token;
  char *p;
  char *file_name;
  Boolean not_an_include = false;

  *include_name = NULL;
  temp_buf = NULL;

  /* is line way too short? */
  if (strlen(buf) < (size_t)7) not_an_include = true;

  /* ignore comments */
  if (!not_an_include && isComment(buf)) not_an_include = true;

  if (!not_an_include)
  {
    /* we know that the line is this long */
    temp_buf = ssave(buf + 6);

    p = temp_buf;

    /* scan for first token */
    while (*p != '\0' && *p == ' ')
      p++;

    if (*p == '\0')
      not_an_include = true;
  }

  if (!not_an_include)
  {/* find the end of the token */
    token = p;

    while (*p != '\0' && *p != ' ')
      p++;

    if (*p == '\0')
      not_an_include = true;
  }

  if (!not_an_include)
  {/* lowercase the token, and compare */
    *p = '\0';
    token = strlower(token);

    if (strcmp(token, "include") != 0)
      not_an_include = true;
  }

  if (!not_an_include)
  { /* scan the start of the next token -- should start with a quote character */
    p++; /* skip the space we found last */
    while (*p != '\0' && *p == ' ')
      p++;

    if (*p == '\0' || !is_a_quote(*p))
      not_an_include = true;
    /* we found an open quote */
  }

  if (!not_an_include)
  { 
    p++; /* skip the quote */
    file_name = p;
    while (*p != '\0' && !is_a_quote(*p))
      p++;

    if (*p == '\0')
      not_an_include = true;
    /* we found a close quote */
  }

  if (!not_an_include)
  {
    /* chop off the quote */
    *p = '\0';

    if (file_name[0] != '/')
    {/* relative path name -- make it relative to the including file */
      char *prev_loc;
      char *last_slash;

      prev_loc = ssave(IncStack[current_include_depth].file_name);
      last_slash = strrchr(prev_loc, '/');
      if (last_slash != (char*)NULL)
      {
	*last_slash = '\0';
	*include_name = nssave(3, prev_loc, "/", file_name);	
      }
      else
	*include_name = ssave(file_name);

      sfree(prev_loc);
    }
    else
      *include_name = ssave(file_name);
  }
  
  if (temp_buf) 
    sfree(temp_buf);

  return NOT(not_an_include);
}

static Boolean isContinuation (char *s)
{ 
  return BOOL((strlen(s) > (size_t)4) && s[5] != ' ');
}

static Boolean isNewExecutable (char *s)
{ 
  return BOOL( s && !isComment(s) && !isContinuation(s));
}


static char *getTabExpandedLine (FILE *fp)
{
  static char	*buf    = 0;
  static int	 buflen;
  static int	 column = 0;
  char		*result, *concat;
  int		 c;

  if ( buf == (char *) 0 ) {
    buflen = LEN;
    buf    = (char *)get_mem(buflen, "getLongLine()");
  }

  c = getc(fp);
  column = 0;
  while ( c != EOF  &&  c != '\n' )
  {
    if ( column + 10 > buflen )
    {
      buflen += LEN;
      buf     = (char *)reget_mem(buf, buflen, "getLongLine()");
    }

    if ( c == '\t' )
    {
      /* expand tab character */
        buf[column++] = ' ';
        while ( column % 8 != 0 )
	  buf[column++] = ' ';
    }
    else 
    {
      buf[column++] = c;
    }

    c = getc(fp);
  }

  buf[column] = '\0';

  if ( column == 0 && c == EOF )
    return NULL;
  else 
    return ssave(buf);
}

static char *extractFront (CList cl)
{
  char        *s;
  CNode *n;

  if ( cl->head ) {
    n        = cl->head;
    cl->head = cl->head->next;
    s        = n->s;

    free_mem(n);
    if ( cl->head == NULL )
      cl->tail = NULL;

    return s;
  } else {
    return NULL;
  }
}


static void insertComment (CList cl, char *s)
{
  CNode *n;

  n = (CNode *)get_mem(sizeof(CNode), "getLongLine:comment node");
  if ( n == NULL ) {
    fprintf(stderr, "Out of memory in getstmt.c:insertComment()\n");
    return;
  }

  n->s    = s;
  n->next = NULL;

  if ( cl->tail == NULL ) {
    cl->head = n;
    cl->tail = n;
  } else {
    cl->tail->next = n;
    cl->tail       = n;
  }
}


static Boolean empty (CList cl)
{
  return BOOL( cl->head == NULL );
}


static CList initComments ( )
{
  CList cl;

  cl = (CList)get_mem(sizeof(struct clist), "getstmt.c: clist");

  if ( cl ) {
    cl->head = (CNode *)0;
    cl->tail = (CNode *)0;
  }

  return cl;
}



Boolean sig_comment_false(char *string)
{
  return false;
}

char *sig_comment_prefix_abort(char *string)
{
  assert(0);
  return 0;
}

char *sig_comment_wrap_prefix_abort(char *string, char *wrap_point)
{
  assert(0);
  return 0;
}

SignificantCommentHandlers no_sig_comments = { 
  sig_comment_false,
  sig_comment_prefix_abort,
  sig_comment_wrap_prefix_abort
  };


void
ftt_MapWalk(FortTextTree ftt, MapInfoOpaque Map)
{
  ppstate  state;
  int numSourceLines = ftt_NumLines(ftt);

  state.ftt = ftt;
  state.schandlers = &no_sig_comments;

  state.continuationCharacter = '*';
  state.currentOutputLine = 0;
  state.currentSourceLine = 0;

  /* pretty-print the tree line by line */
  for (state.currentSourceLine = 0; state.currentSourceLine < numSourceLines; 
       state.currentSourceLine++) {
    va_list dummyArgs;
    writeLine(&state, Map, (ftt_ListingCallBack) NULL, dummyArgs);
  }
}


/*
 * Pretty-print fortran ast (an ftt)
 */

int ftt_Export(FortTextTree ftt, int continuationCharacter, 
	       SignificantCommentHandlers *schandlers, 
	       ftt_ListingCallBack exportLine, ...)
{
  ppstate  state;
  int numSourceLines = ftt_NumLines(ftt);
  va_list exportLineArgs;


  va_start(exportLineArgs, exportLine);
  state.ftt = ftt;
  state.schandlers = 
    ((schandlers == DEFAULT_SIG_COMMENT_HANDLERS) ? 
     &no_sig_comments :
     schandlers);

  state.continuationCharacter = continuationCharacter;
  state.currentOutputLine = 0;
  state.currentSourceLine = 0;

  /* pretty-print the tree line by line */
  for (state.currentSourceLine = 0; state.currentSourceLine < numSourceLines; 
       state.currentSourceLine++) {
    int code = writeLine(&state, NULL, exportLine, exportLineArgs);
    if (code) return code;
  }

  va_end(exportLineArgs);

  return 0;
}


#define STANDARD_COMMENT_WRAP_PREFIX    "C"
#define STANDARD_LINE_LENGTH            72
#define STANDARD_COMMENT_LENGTH         80
#define MAX_LINE_LENGTH	                1024

static int writeLine(ppstate *state, MapInfoOpaque Map, 
		      ftt_ListingCallBack exportLine, va_list exportLineArgs)
{
  int retcode;
  Line line;
  int numCharsLeft, numToCopy, offset, maxlen;
  char outLine[MAX_LINE_LENGTH], *restOfLine;
  Boolean isComment, isSignificantComment = false;
  int lineNumOfStmt = 0;
  
  
  ftt_GetLineInfo(state->ftt, state->currentSourceLine, &line.node, &line.bracket);
  
  /* identify significant comments using a callback */
  
  isComment = BOOL(is_comment(line.node));
  if (isComment) 
    isSignificantComment = 
      (*state->schandlers->is_significant_comment)
	(gen_get_text(gen_COMMENT_get_text(line.node)));
  

  /* have ftt format all lines for output except significant comments which
   * will have their formatting determined by callbacks
   */
  if (isSignificantComment) 
    line.text = gen_get_text(gen_COMMENT_get_text(line.node));
  else line.text = ftt_GetTextLine(state->ftt, state->currentSourceLine);
  
  maxlen = ((state->continuationCharacter != ' ') ? 
	    (isComment ? STANDARD_COMMENT_LENGTH : STANDARD_LINE_LENGTH) : 
	    MAX_LINE_LENGTH);
  numCharsLeft = strlen(line.text);
  restOfLine = line.text;
  
  /* if significant comment, set proper prefix; else prefix is empty */
  if (isSignificantComment) {
    strcpy(outLine, (*state->schandlers->significant_comment_prefix)(restOfLine));
    offset = (int)strlen(outLine);
  } else offset = 0;
  
  int count = 0;
  for(;;) {
    va_list exportLineCopy;
    va_copy(exportLineCopy,exportLineArgs);
    numToCopy = min(numCharsLeft, maxlen - offset);
    (void) strncpy(outLine + offset, restOfLine, numToCopy);

    restOfLine += numToCopy;
    numCharsLeft -= numToCopy;
    
    outLine[numToCopy + offset] = '\0';

    if (exportLine)
      retcode = exportLine(state->currentOutputLine, outLine, exportLineCopy);

    state->currentOutputLine++;
    
    lineNumOfStmt++;

    if ( Map != NULL )
      ProcessMapLine(Map, state->ftt, state->currentSourceLine,
		     state->currentOutputLine);
    
    if (numCharsLeft == 0 || retcode) break;

    /* characters remaining ... prepare to wrap line */
    if (isComment) {
      strcpy(outLine, isSignificantComment ?
	     (*state->schandlers->significant_comment_wrap_prefix)(line.text, 
								   restOfLine) : 
	     STANDARD_COMMENT_WRAP_PREFIX);
    } else {
      (void)sprintf(outLine, "     %c", state->continuationCharacter == 'n' ? 
		    (lineNumOfStmt % 10) + '0' : 
		    state->continuationCharacter);
    }
    offset = strlen(outLine);
  }
    
  /* free up the dynamically allocated text line present for all but 
   * significant comments
   */
  if (isSignificantComment == false) sfree(line.text);

  return retcode;
}




