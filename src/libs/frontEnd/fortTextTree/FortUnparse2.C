/* $Id: FortUnparse2.C,v 1.1 1997/06/24 17:51:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortUnparse2.c				*/
/*									*/
/*	FortUnparse2 -- high-level unparser for FortTrees		*/
/*									*/
/************************************************************************/


#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/FortUnparse2.h>

#include <libs/support/arrays/FlexibleArray.h>


/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/* Sad kludge prompted by initialization -- cf. 'tt_Open' => 'unparse2' */

extern TT_Methods ftt_methods;




/************************/
/*  Static data		*/
/************************/




/* what kind of result is wanted */

typedef enum
  {
    UNPARSE_LINES,
    UNPARSE_MAP_NODE,
    UNPARSE_MAP_LINE
  } UnparseMode;


static UnparseMode unp2_mode;
static int unp2_currentLine;



/* when lines are wanted */

#define UNP2_TABSIZE	2
static Flex * unp2_lines;
static FortTextTree unp2_fortTextTree;
static FortTree unp2_fortTree;
static TextTree unp2_textTree;




/* when node/char mapping is wanted */

static FortTreeNode unp2_node;
static int unp2_firstLine;
static int unp2_lastLine;
static int unp2_indent;

static Generic unp2_cacheOb;
static cacheProcFunc unp2_cacheProc;




/* when expansion is wanted */

static char *unp2_stmtExpansion       = "&stmt&";
static char *unp2_subprExpansion      = "&subprogram&";
static char *unp2_ifExpansion         = "if (&expr&) then";
static char *unp2_elseifExpansion     = "elseif (&expr&) then";
static char *unp2_elseExpansion       = "else";
static char *unp2_parcaseExpansion    = "parallel:";
static char *unp2_parpidcaseExpansion = "parallel: (&expr...&)";
static char *unp2_othercaseExpansion  = "other processes:";
static char *unp2_enddoExpansion      = "enddo";
static char *unp2_endallExpansion     = "endall";
static char *unp2_endloopExpansion    = "endloop";
static char *unp2_continueExpansion   = "continue";
static char *unp2_whereExpansion      = "where (&expr&)";
static char *unp2_elsewhereExpansion  = "elsewhere";




/* bracket kinds */

#define SIMPLE		0
#define OPEN		1
#define CLOSE		2





/************************/
/* Forward declarations	*/
/************************/




STATIC (void,		outputNode,(AST_INDEX node, int indent));
STATIC (void,		output,(AST_INDEX parent, int bracket, int tokenVal, AST_INDEX
                                tree, AST_INDEX part0, AST_INDEX part1, AST_INDEX part2,
                                AST_INDEX part3, int indent));
STATIC (void,		outputPlaceholder,(FortTreeNode node, int token, int indent));
STATIC (void,		addExpansion,(Flex *expansions, FortTreeNode node, int type,
                                      char *title));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void unp2_Init()
{
  /* nothing */
}




void unp2_Fini()
{
  /* nothing */
}






/************************/
/*  Unparsing		*/
/************************/




void unp2_Unparse(FortTextTree ftt, FortTreeNode node, int indent, Flex **lines)
{
  unp2_fortTextTree = ftt;
  unp2_fortTree = ftt_Tree(ftt);
  ftt_getTextTree(ftt, &unp2_textTree);

  /* initialize result variables */
    unp2_mode = UNPARSE_LINES;
    unp2_lines = flex_create(sizeof(TT_Line));

  /* unparse the given tree */
    ft_AstSelect(ftt_Tree(ftt));
    unp2_currentLine = -1;
    outputNode(node, indent);

  /* return result variables */
    *lines = unp2_lines;
}




void unp2_TextToNode(FortTextTree ftt, int firstLine, FortTreeNode *node)
{
  /* initialize result variables */
    unp2_mode = UNPARSE_MAP_LINE;
    unp2_firstLine = firstLine;

  /* unparse the given tree */
    ft_AstSelect(ftt_Tree(ftt));
    unp2_currentLine = -1;
    outputNode(ftt_Root(ftt), 0);

  /* return result variables */
    *node = unp2_node;
}




void unp2_NodeToText(FortTextTree ftt, FortTreeNode node, int *firstLine, 
                     int *lastLine, int *indent, Generic cacheOb, 
		     cacheProcFunc cacheProc)
{
  /* initialize result variables */
    unp2_mode = UNPARSE_MAP_NODE;
    unp2_node = node;
    unp2_firstLine = UNUSED;
    unp2_lastLine = UNUSED;
    unp2_indent = UNUSED;
    unp2_cacheOb = cacheOb;
    unp2_cacheProc = cacheProc;

  /* unparse the given tree */
    unp2_currentLine = -1;
    ft_AstSelect(ftt_Tree(ftt));
    if ( ftt_nodeIsLevel2(node) )
      outputNode(ftt_Root(ftt), 0);

  /* return result variables */
    *firstLine = unp2_firstLine;
    *lastLine  = unp2_lastLine;
    *indent  = unp2_indent;
}






/************************/
/*  Expanding		*/
/************************/




void unp2_Expandee(FortTextTree ftt, FortTreeNode node, Flex *expansions)
{
  FortTreeNode parent;

  ft_AstSelect(ftt_Tree(ftt));

  parent = tree_out(node);
  switch( gen_get_node_type(parent) )
    { case GEN_GLOBAL:
        addExpansion(expansions, node, GEN_subprogram, unp2_subprExpansion);
        break;

#ifdef later
      case GEN_DO:
      case GEN_DO_ALL:
      case GEN_PARALLELLOOP:
        if( ft_GetParseErrorCode(parent) != ftt_NO_ERROR )
          { if( gen_DO_get_lbl_ref(parent) != AST_NIL )
              addExpansion(expansions, node, UNUSED, unp2_continueExpansion);
            else if( gen_get_node_type(parent) == GEN_DO )
              addExpansion(expansions, node, UNUSED, unp2_enddoExpansion);
            else if( gen_get_node_type(parent) == GEN_DO_ALL )
              addExpansion(expansions, node, UNUSED, unp2_endallExpansion);
            else
              addExpansion(expansions, node, UNUSED, unp2_endloopExpansion);
          }
        addExpansion(expansions, node, GEN_stmt, unp2_stmtExpansion);
        break;
#endif

      case GEN_GUARD:
        if( gen_GUARD_get_rvalue(parent) != AST_NIL )
          { addExpansion(expansions, node, GEN_guard, unp2_elseifExpansion);
            if( list_last(list_head(parent)) == parent )
              addExpansion(expansions, node, GEN_guard, unp2_elseExpansion);
          }
        addExpansion(expansions, node, GEN_stmt, unp2_stmtExpansion);
        break;

      case GEN_PARALLEL_CASE:
        if( gen_PARALLEL_get_lvalue(tree_out(parent)) == AST_NIL )
          addExpansion(
              expansions,
              node,
              GEN_parallel_case,
              unp2_parcaseExpansion
          );
        else
          { addExpansion(
                expansions,
                node,
                GEN_parallel_case,
                unp2_parpidcaseExpansion
            );
            addExpansion(
                expansions,
                node,
                GEN_parallel_case,
                unp2_othercaseExpansion
            );
          }
        addExpansion(expansions, node, GEN_stmt, unp2_stmtExpansion);
        break;

      default:
        addExpansion(expansions, node, GEN_stmt, unp2_stmtExpansion);
        break;
    }
}




void unp2_Expand(FortTextTree ftt, fx_Expansion ex, FortTreeNode *New, 
                 FortTreeNode *focus)
{
  FortTreeNode stmtList, label, rvalue, otherList, old, parent;

  parent = tree_out(ex.node);
  switch( ex.type )
    {
      case GEN_guard:
        ftt_TreeWillChange(ftt, tree_out(parent));
        otherList = list_create(AST_NIL);
        stmtList = gen_GUARD_get_stmt_LIST(parent);
        old = list_remove_last(stmtList);
        while( old != ex.node )
          { list_insert_first(otherList, old);
            old = list_remove_last(stmtList);
          }
        tree_free(old);
        if( list_empty(otherList) )
           list_insert_first(otherList, gen_COMMENT(AST_NIL));
        if( (char *) ex.value == unp2_ifExpansion )
          { label  = AST_NIL;	/* this label is really in the IF */
            rvalue = ph_from_mtype(GEN_expr);
            *focus  = rvalue;
          }
        else if( (char *) ex.value == unp2_elseifExpansion )
          { label  = AST_NIL;
            rvalue = ph_from_mtype(GEN_expr);
            *focus  = rvalue;
          }
        else if( (char *) ex.value == unp2_whereExpansion )
          { label  = AST_NIL;
            rvalue = ph_from_mtype(GEN_expr);
            *focus  = rvalue;
          }
        else
          { label  = AST_NIL;
            rvalue = AST_NIL;
            *focus  = list_first(otherList);
          }
        *New = gen_GUARD(label, rvalue, otherList);
        (void) list_insert_after(parent, *New);
        ftt_TreeChanged(ftt, tree_out(parent));
        break;

      case GEN_parallel_case:
        ftt_TreeWillChange(ftt, tree_out(parent));
        otherList = list_create(AST_NIL);
        stmtList = gen_PARALLEL_CASE_get_stmt_LIST(parent);
        old = list_remove_last(stmtList);
        while( old != ex.node )
          { list_insert_first(otherList, old);
            old = list_remove_last(stmtList);
          }
        tree_free(old);
        if( list_empty(otherList) )
           list_insert_first(otherList, gen_COMMENT(AST_NIL));
        label  = AST_NIL;
        if( (char *) ex.value == unp2_parcaseExpansion )
          { rvalue = AST_NIL;
            *focus  = list_first(otherList);
          }
        else if( (char *) ex.value == unp2_parpidcaseExpansion )
          { rvalue = list_create(ph_from_mtype(GEN_expr));
            *focus  = rvalue;
          }
        else
          { rvalue = list_create(AST_NIL);
            *focus  = list_first(otherList);
          }
        *New = gen_PARALLEL_CASE(label, rvalue, otherList);
        (void) list_insert_after(parent, *New);
        ftt_TreeChanged(ftt, tree_out(parent));
        break;

#ifdef later
      case UNUSED:  /* "missing" entry */
        switch( gen_get_node_type(parent) )
          {
            case GEN_DO:
              do
                { line = list_remove_last(gen_DO_get_stmt_LIST(parent));
                  (void) list_insert_after(parent, line);
                } while (line != ex.node);
              if( gen_DO_get_lbl_ref(parent) != AST_NIL )
                { 
              if( 
              break;
          }
        break;
#endif

      default:
        ftt_TreeWillChange(ftt, list_head(ex.node));
        *New = ph_from_mtype(ex.type);
        tree_replace(ex.node, *New);
        tree_free(ex.node);
        *focus = *New;
        ftt_TreeChanged(ftt, list_head(*New));
        break;
    }
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void outputNode(AST_INDEX node, int indent)
{
  FortTreeNode son;
  TT_Line line;
  int subIndent = indent + UNP2_TABSIZE;
  int firstLine;

  if(  unp2_mode == UNPARSE_MAP_NODE )
    { /* enter partial info into cache */
        firstLine = unp2_currentLine + 1;
        unp2_cacheProc(unp2_cacheOb, node, firstLine, UNUSED, indent);

      if( node == unp2_node )
        { /* tag the beginning of the line list as being part of the node */
            unp2_firstLine = firstLine;
            unp2_indent = indent;
        }
    }

  switch( gen_get_node_type(node) )
  {
    case GEN_NULL_NODE:		/* empty program */
      break;

    case GEN_LIST_OF_NODES:	/* list of statements */
      for( son = list_first(node); son != AST_NIL; son = list_next(son) )
        outputNode(son, indent);
      break;

    case GEN_GLOBAL:		/* global scope */
      /* output the list of program scopes */
        outputNode(gen_GLOBAL_get_subprogram_scope_LIST(node), indent);
      break;

    case GEN_FUNCTION:		/* function statement */
      /* output the function statement */
        output(
	    node, OPEN,
            SFUNCTION_STAT,
            AST_NIL,
            gen_FUNCTION_get_lbl_def(node),
            gen_FUNCTION_get_type_len(node),
            gen_FUNCTION_get_name(node),
            gen_FUNCTION_get_formal_arg_LIST(node),
	    indent
        );

      /* output the statement list */
        outputNode(gen_FUNCTION_get_stmt_LIST(node), subIndent);

      /* output the end statment (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
	      node, CLOSE,
              SEND_STAT,
              AST_NIL,
              gen_FUNCTION_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
	      indent
          );
      break;

    case GEN_PROGRAM:		/* program statement */
      /* output the program statment (if necessary) */
        if( ft_GetShow(node) )
          output(
              node, OPEN,
              SPROGRAM_STAT,
              AST_NIL,
              gen_PROGRAM_get_lbl_def(node),
              gen_PROGRAM_get_name(node),
              AST_NIL,
              AST_NIL,
              indent
          );

      /* output the statement list */
        outputNode(gen_PROGRAM_get_stmt_LIST(node), subIndent);

      /* output the end statment (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
              node, CLOSE,
              SEND_STAT,
	      AST_NIL,
              gen_PROGRAM_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
	      indent
          );
      break;

    case GEN_SUBROUTINE:	/* subroutine statement */
      /* output the subroutine statment */
        output(
            node, OPEN,
            SSUBROUTINE_STAT,
            AST_NIL,
            gen_SUBROUTINE_get_lbl_def(node),
            gen_SUBROUTINE_get_name(node),
            gen_SUBROUTINE_get_formal_arg_LIST(node),
            AST_NIL,
	    indent
        );

      /* output the statement list */
        outputNode(gen_SUBROUTINE_get_stmt_LIST(node), subIndent);

      /* output the end statment (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
              node, CLOSE,
              SEND_STAT,
	      AST_NIL,
              gen_SUBROUTINE_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
	      indent
          );
      break;

    case GEN_BLOCK_DATA:	/* block data statement */
      /* output the block data statment */
        output(
            node, OPEN,
            SBLOCK_DATA_STAT,
            AST_NIL,
            gen_BLOCK_DATA_get_lbl_def(node),
            gen_BLOCK_DATA_get_name(node),
            AST_NIL,
            AST_NIL,
	    indent
        );

      /* output the statement list */
        outputNode(gen_BLOCK_DATA_get_stmt_LIST(node), subIndent);

      /* output the end statment (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
              node, CLOSE,
              SEND_STAT,
              AST_NIL,
              gen_BLOCK_DATA_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
          );
      break;

    case GEN_DEBUG:		/* debug statement */
      /* output the debug statment */
        output(
            node, OPEN,
            SDEBUG_STAT,
            AST_NIL,
            gen_DEBUG_get_lbl_def(node),
            gen_DEBUG_get_option_LIST(node),
            AST_NIL,
            AST_NIL,
            indent
        );

      /* output the statement list */
        outputNode(gen_DEBUG_get_stmt_LIST(node), subIndent);

      /* output the end statment (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
              node, CLOSE,
              SEND_STAT,
              AST_NIL,
              gen_DEBUG_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
          );
      break;

    case GEN_DO:		/* do statement */
      if( gen_DO_get_lbl_ref(node) == AST_NIL )
        { /* output the do statmement */
            output(
                node, OPEN,
                SDO_STAT,
                AST_NIL,
                gen_DO_get_lbl_def(node),
                gen_DO_get_lbl_ref(node),
                gen_DO_get_control(node),
                AST_NIL,
                indent
            );

          /* output the statement list */
            outputNode(gen_DO_get_stmt_LIST(node), subIndent);

          /* output the enddo statement (if necessary) */
            if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
              output(
                  node, CLOSE,
                  SEND_DO_STAT,
                  AST_NIL,
                  gen_DO_get_close_lbl_def(node),
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  indent
              );
        }
      else
        { /* output the do (labeled) statement */
            output(
                node, OPEN,
                SDO_LABEL_STAT,
                AST_NIL,
                gen_DO_get_lbl_def(node),
                gen_DO_get_lbl_ref(node),
                gen_DO_get_control(node),
                AST_NIL,
                indent
            );

          /* output the statement list */
            outputNode(gen_DO_get_stmt_LIST(node), subIndent);

          /* dedent the last statment (if necessary) */
            if( unp2_mode == UNPARSE_LINES )
              if( ft_GetParseErrorCode(node) == ftt_NO_ERROR  &&
				!list_empty(gen_DO_get_stmt_LIST(node))   )
                {
	           (void) flex_get_buffer(unp2_lines, unp2_currentLine,
							1,   (char *) &line);
                   line.indent = indent;
                   flex_set_buffer(unp2_lines, unp2_currentLine,
							1,   (char *) &line);
                }

          /* output the erroneous enddo statement (if necessary) */
            if( ft_GetParseErrorCode(node) == ftt_WRONG_ENDBRACKET )
              output(
                  node, CLOSE,
                  SEND_DO_STAT,
                  AST_NIL,
                  gen_DO_get_close_lbl_def(node),
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  indent
              );
        }
      break;

    case GEN_DO_ALL:		/* doall statement */
      if( gen_DO_ALL_get_lbl_ref(node) == AST_NIL )
        { /* output the doall statmement */
            output(
                node, OPEN,
                SDOALL_STAT,
                AST_NIL,
                gen_DO_ALL_get_lbl_def(node),
                gen_DO_ALL_get_lbl_ref(node),
                gen_DO_ALL_get_control(node),
                AST_NIL,
                indent
            );

          /* output the statement list */
            outputNode(gen_DO_ALL_get_stmt_LIST(node), subIndent);

          /* output the endall statement (if necessary) */
            if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
              output(
                  node, CLOSE,
                  SEND_ALL_STAT,
                  AST_NIL,
                  gen_DO_ALL_get_close_lbl_def(node),
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  indent
              );
        }
      else
        { /* output the doall (labeled) statement */
            output(
                node, OPEN,
                SDOALL_LABEL_STAT,
                AST_NIL,
                gen_DO_ALL_get_lbl_def(node),
                gen_DO_ALL_get_lbl_ref(node),
                gen_DO_ALL_get_control(node),
                AST_NIL,
                indent
            );

         /* output the statement list */
           outputNode(gen_DO_ALL_get_stmt_LIST(node), subIndent);

         /* dedent the last statment (if necessary) */
            if( unp2_mode == UNPARSE_LINES )
              if( ft_GetParseErrorCode(node) == ftt_NO_ERROR  &&
				!list_empty(gen_DO_ALL_get_stmt_LIST(node)) )
	        { (void) flex_get_buffer(unp2_lines, unp2_currentLine,
							1, (char *) &line);
                  line.indent = indent;
                  flex_set_buffer(unp2_lines, unp2_currentLine,
							1, (char *) &line);
                }

          /* output the erroneous endall statement (if necessary) */
            if( ft_GetParseErrorCode(node) == ftt_WRONG_ENDBRACKET )
              output(
                  node, CLOSE,
                  SEND_ALL_STAT,
                  AST_NIL,
                  gen_DO_ALL_get_close_lbl_def(node),
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  indent
              );
        }
      break;

    case GEN_IF:		/* if statement */
      /* output the list of guards */
	outputNode(gen_IF_get_guard_LIST(node), indent);

      /* output the endif statement (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
              node, CLOSE,
              SEND_IF_STAT,
              AST_NIL,
              gen_IF_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
          );
      break;

    case GEN_GUARD:		/* guard */
      /* output the guard statement list */
        if( is_first_in_list(node) )
          output(
            tree_out(node), OPEN,
            (is_if(tree_out(node))) ? SIF_STAT : SWHERE_BLOCK_STAT,
            AST_NIL,
            (is_if(tree_out(node))) ? gen_IF_get_lbl_def(tree_out(node))
                                    : gen_WHERE_BLOCK_get_lbl_def(tree_out(node)),
            gen_GUARD_get_rvalue(node),
            AST_NIL,
            AST_NIL,
            indent
          );
        else if( gen_GUARD_get_rvalue(node) == AST_NIL )
          output(
            node, OPEN,
            (is_if(tree_out(node))) ? SELSE_STAT : SELSE_WHERE_STAT,
            AST_NIL,
            gen_GUARD_get_lbl_def(node),
            gen_GUARD_get_rvalue(node),
            AST_NIL,
            AST_NIL,
            indent
          );
        else
          output(
            node, OPEN,
            SELSE_IF_STAT,
            AST_NIL,
            gen_GUARD_get_lbl_def(node),
            gen_GUARD_get_rvalue(node),
            AST_NIL,
            AST_NIL,
            indent
          );

      /* output the statement list */
        outputNode(gen_GUARD_get_stmt_LIST(node), subIndent);
      break;

    case GEN_PARALLEL:		/* parallel block */
      /* output the parbegin statement */
        if( gen_PARALLEL_get_lvalue(node) == AST_NIL )
          output(
              node, OPEN,
              SPARBEGIN_STAT,
              AST_NIL,
              gen_PARALLEL_get_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
          );
        else
          output(
              node, OPEN,
              SPARBEGIN_PID_STAT,
              AST_NIL,
              gen_PARALLEL_get_lbl_def(node),
              gen_PARALLEL_get_lvalue(node),
              gen_PARALLEL_get_rvalue(node),
              AST_NIL,
              indent
          );

      /* output the declaration list */
        outputNode(gen_PARALLEL_get_stmt_LIST(node), subIndent);

      /* output the parallel case list */
        outputNode(gen_PARALLEL_get_parallel_case_LIST(node), subIndent);

      /* output the parend statement (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
              node, CLOSE,
              SPAREND_STAT,
              AST_NIL,
              gen_PARALLEL_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
          );
      break;

    case GEN_PARALLEL_CASE:	/* parallel case */
      /* output the case statement */
        if( gen_PARALLEL_CASE_get_rvalue_LIST(node) == AST_NIL )
          output(
              node, OPEN,
              SPARALLEL_STAT,
              AST_NIL,
              gen_PARALLEL_CASE_get_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
          );
        else if( list_empty(gen_PARALLEL_CASE_get_rvalue_LIST(node)) )
          output(
              node, OPEN,
              SOTHER_PROCESSES_STAT,
              AST_NIL,
              gen_PARALLEL_CASE_get_lbl_def(node),
              gen_PARALLEL_CASE_get_rvalue_LIST(node),
              AST_NIL,
              AST_NIL,
              indent
          );
        else
          output(
              node, OPEN,
              SPARALLEL_PID_STAT,
              AST_NIL,
              gen_PARALLEL_CASE_get_lbl_def(node),
              gen_PARALLEL_CASE_get_rvalue_LIST(node),
              AST_NIL,
              AST_NIL,
              indent
          );

      /* output the statement list */
        outputNode(gen_PARALLEL_CASE_get_stmt_LIST(node), subIndent);
      break;


    case GEN_PARALLELLOOP:	/* parallel loop statement */
      if( gen_PARALLELLOOP_get_lbl_ref(node) == AST_NIL )
        { /* output the parallel loop statmement */
            output(
                node, OPEN,
                SPARALLELLOOP_STAT,
                AST_NIL,
                gen_PARALLELLOOP_get_lbl_def(node),
                gen_PARALLELLOOP_get_lbl_ref(node),
                gen_PARALLELLOOP_get_control(node),
                AST_NIL,
                indent
            );

          /* output the statement list */
            outputNode(gen_PARALLELLOOP_get_stmt_LIST(node), subIndent);

          /* output the end loop statement (if necessary) */
            if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
              output(
                  node, CLOSE,
                  SEND_LOOP_STAT,
                  AST_NIL,
                  gen_PARALLELLOOP_get_close_lbl_def(node),
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  indent
              );
        }
      else
        { /* output the parallel loop (labeled) statement */
            output(
                node, OPEN,
                SPARALLELLOOP_LABEL_STAT,
                AST_NIL,
                gen_PARALLELLOOP_get_lbl_def(node),
                gen_PARALLELLOOP_get_lbl_ref(node),
                gen_PARALLELLOOP_get_control(node),
                AST_NIL,
                indent
            );

         /* output the statement list */
           outputNode(gen_PARALLELLOOP_get_stmt_LIST(node), subIndent);

         /* dedent the last statment (if necessary) */
            if( unp2_mode == UNPARSE_LINES )
              if( ft_GetParseErrorCode(node) == ftt_NO_ERROR  &&
			 !list_empty(gen_PARALLELLOOP_get_stmt_LIST(node)) )
	        { (void) flex_get_buffer(unp2_lines, unp2_currentLine,
							1, (char *) &line);
                  line.indent = indent;
                  flex_set_buffer(unp2_lines, unp2_currentLine,
							1, (char *) &line);
                }

          /* output the erroneous end loop statement (if necessary) */
            if( ft_GetParseErrorCode(node) == ftt_WRONG_ENDBRACKET )
              output(
                  node, CLOSE,
                  SEND_LOOP_STAT,
                  AST_NIL,
                  gen_PARALLELLOOP_get_close_lbl_def(node),
                  AST_NIL,
                  AST_NIL,
                  AST_NIL,
                  indent
              );
        }
      break;

    case GEN_PLACE_HOLDER:	/* statement placeholders */
      switch( gen_get_meta_type(node) )
        {
          case GEN_subprogram:
            outputPlaceholder(node,SPRSCOPE_PH_STAT,indent);
            break;

          case GEN_stmt:
            outputPlaceholder(node,SSTMT_PH_STAT,indent);
            break;

          case GEN_specification_stmt:
            outputPlaceholder(node,SSPECIFICATION_STMT_PH_STAT,indent);
            break;

          case GEN_control_stmt:
            outputPlaceholder(node,SCONTROL_STMT_PH_STAT,indent);
            break;

          case GEN_io_stmt:
            outputPlaceholder(node,SIO_STMT_PH_STAT,indent);
            break;

          case GEN_parascope_stmt:
            outputPlaceholder(node,SPARASCOPE_STMT_PH_STAT,indent);
            break;

          case GEN_debug_stmt:
            outputPlaceholder(node,SDEBUG_STMT_PH_STAT,indent);
            break;
        }
      break;

    case GEN_ERROR:		/* misplaced / error statement */
      if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
        { /* output an error statement */
            output(
              node, SIMPLE,
              SERROR_STAT,
              node,
              AST_NIL,
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
            );
        }
      else if( ft_GetParseErrorCode(node) == SEND_DO_IMPLIED )
        { /* not really a statement */
        }
      else
        { /* output a misplaced statement */
            output(
              node, SIMPLE,
              ft_GetParseErrorCode(node),
              gen_ERROR_get_tree(node),
              gen_ERROR_get_part0(node),
              gen_ERROR_get_part1(node),
              gen_ERROR_get_part2(node),
              gen_ERROR_get_part3(node),
              indent
            );
        }
      break;

    case GEN_WHERE_BLOCK:		/* where statement */
      /* output the list of guards */
	outputNode(gen_WHERE_BLOCK_get_guard_LIST(node), indent);

      /* output the endwhere statement (if necessary) */
        if( ft_GetParseErrorCode(node) == ftt_NO_ERROR )
          output(
              node, CLOSE,
              SEND_WHERE_STAT,
              AST_NIL,
              gen_WHERE_BLOCK_get_close_lbl_def(node),
              AST_NIL,
              AST_NIL,
              AST_NIL,
              indent
          );
      break;

    default:			/* simple statements */
      output(
          node, SIMPLE,
	  ftt_nodeTypeToTokenType(gen_get_node_type(node)),
	  node,
	  AST_NIL,
	  AST_NIL,
	  AST_NIL,
	  AST_NIL,
	  indent
      );
      break;
  }

  if(  unp2_mode == UNPARSE_MAP_NODE )
    { /* enter complete information into cache */
        unp2_cacheProc(unp2_cacheOb, node, firstLine, unp2_currentLine, indent);

      if( node == unp2_node )
        { /* tag the end of the line list as being part of the node */
            unp2_lastLine = unp2_currentLine;
        }
    }
}




static
void output(AST_INDEX parent, int bracket, int tokenVal, AST_INDEX tree, 
            AST_INDEX part0, AST_INDEX part1, AST_INDEX part2, AST_INDEX part3, 
            int indent)
{
  fx_StatToken st;
  TT_Line line;

  unp2_currentLine++;
  switch( unp2_mode )
    {
      case UNPARSE_LINES:
        /* set mapping info */
          line.lineNode = (FortTreeNode) parent;
          line.bracket  = bracket;
          line.indent   = indent;
          line.conceal  = ft_GetConceal(unp2_fortTree, (FortTreeNode) parent, line.bracket);
          /*** SIGH! ***/
          /*** tt_getTagNode(unp2_textTree, (TT_TreeNode) parent, &line.tt_tag); ***/
          (ftt_methods.getExtra)(unp2_fortTextTree, (FortTreeNode) parent, 4, &line.tt_tag);	/* 4 ?! */

        /* set text to no-text */
          line.text.num_tc    = 0;
          line.text.tc_ptr    = nil;
          line.text.ephemeral = false;
          line.textValid      = false;

        /* set 'line.token' */
          st.token   = tokenVal;
          st.tree    = (FortTreeNode) tree;
          st.part[0] = (FortTreeNode) part0;
          st.part[1] = (FortTreeNode) part1;
          st.part[2] = (FortTreeNode) part2;
          st.part[3] = (FortTreeNode) part3;
          line.token = *(TT_MaxToken *) &st;

        flex_insert_one(unp2_lines, unp2_currentLine, (char *) &line);
        break;

      case UNPARSE_MAP_LINE:
        if( unp2_currentLine == unp2_firstLine )
           unp2_node = parent;
        break;
    }
}




static
void outputPlaceholder(FortTreeNode node, int token, int indent)
{
  output( node, SIMPLE,
          token,
          AST_NIL,
          node,
          AST_NIL,
          AST_NIL,
          AST_NIL,
          indent
        );
}




static
void addExpansion(Flex *expansions, FortTreeNode node, int type, char *title)
{
  fx_Expansion ex;

  ex.node  = node;
  ex.value = (Generic) title;	/* used in unp2_Expand */
  ex.type  = type;
  ex.title = ftt_makeExpansionName(title, false);
  ex.who   = UNPARSE2_EXPANDER;
  ex.first = true;

  flex_insert_one(expansions, flex_length(expansions), (char *) &ex);
}
