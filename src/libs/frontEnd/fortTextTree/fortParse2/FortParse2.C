/* $Id: FortParse2.C,v 1.1 1997/06/24 17:47:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortParse2/FortParse2.c			        */
/*									*/
/*	FortParse2 -- high-level parser for Fortran source		*/
/*									*/
/************************************************************************/


#include <string.h>


#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/fortParse2/FortParse2.i>

#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Static data		*/
/************************/




/* parser results */

FortTreeNode fp2_root;			/* accessed by gram2.y */

static Boolean fp2_error;






/************************/
/* Forward declarations	*/
/************************/


static void separateTree(FortTextTree,FortTreeNode);




/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void fp2_Init()
{
  lx2_Init();
}




void fp2_Fini()
{
  lx2_Fini();
}




/*ARGSUSED*/

void yy2error(char *s)
  // char * s;
{
  fp2_error = true;
}






/************************/
/*  Parsing		*/
/************************/




Boolean fp2_Parse(FortTextTree ftt, int goal, Flex *lines, int first, int count, 
		  FortTreeNode *node)
//   FortTextTree ftt;
//   int goal;
//   Flex * lines;
//   int first,count;
//   FortTreeNode *node;
{
  ftt_getTextTree(ftt, &ftt_textTree);
  ftt_fortTree = ftt_Tree(ftt);
  ft_AstSelect(ftt_fortTree);
  lx2_SetScan(goal,lines,first,count);
  fp2_error = false;
  fp2_root = nil;

  (void) yy2parse();

  *node = fp2_root;
  return NOT( fp2_error );
}




Boolean fp2_Synch(FortTextTree ftt,FortTreeNode node,int *goal)
//   FortTextTree ftt;
//   FortTreeNode node;
//   int *goal;
{
  FortTree ft = ftt_Tree(ftt);
  Boolean erroneous;
  FortTreeNode This;

  if( node == nil )
    *goal = SGOAL_MODULE;
  else
    switch( NT(node) )
      {
        case GEN_GLOBAL:
          *goal = SGOAL_MODULE;
          break;

        case GEN_PROGRAM:
        case GEN_BLOCK_DATA:
        case GEN_SUBROUTINE:
        case GEN_FUNCTION:
        case GEN_DEBUG:
          *goal = SGOAL_UNIT;
          break;

        case GEN_DO:
          *goal = SGOAL_DO;
          break;

        case GEN_DO_ALL:
          *goal = SGOAL_DOALL;
          break;

        case GEN_IF:
          *goal = SGOAL_IF;
          break;

        case GEN_PARALLEL:
          *goal = SGOAL_PAR;
          break;

        case GEN_PARALLELLOOP:
          *goal = SGOAL_PARALLELLOOP;
          break;

        default:
          if( ftt_nodeIsLevel2(node)  &&  ! is_list(node) )
            *goal = SGOAL_STAT;
          else
            *goal = UNUSED;
      }

  if( *goal == UNUSED )
    return false;
  else
    { /* see if 'node' or any ancestor of 'node' is in error */
        erroneous = false;
        This = node;
        while( ! erroneous  &&  This != nil )
          { erroneous = ft_IsErroneous(ft, This, ft_OPEN);
            if( ! erroneous )
              This = ftt_GetFather(ftt, This);
          }

      return NOT( erroneous );
    }
}




/*ARGSUSED*/

void fp2_Copy(FortTextTree ftt, FortTreeNode oldNode,FortTreeNode *newNode)
//   FortTextTree ftt;
//   FortTreeNode oldNode;
//   FortTreeNode *newNode;
{
  *newNode = tree_copy(oldNode);
}




void fp2_Destroy(FortTextTree ftt,FortTreeNode root)
//   FortTextTree ftt;
//   FortTreeNode root;
{
  ft_AstSelect(ftt_Tree(ftt));
  separateTree(ftt, root);
  tree_free(root);
}




void fp2_SetRoot(FortTextTree ftt,FortTreeNode node)
//   FortTextTree ftt;
//   FortTreeNode node;
{
  ft_SetRoot(ftt_Tree(ftt),node);
}





/************************/
/*  Error reporting	*/
/************************/




/*ARGSUSED*/

void fp2_GetError(FortTreeNode errorNode,char *msg,int  maxMessageLen)
//   FortTreeNode errorNode;
//   char * msg;
//   int maxMessageLen;
{
  (void) strncpy(msg,"Not implemented",maxMessageLen);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static void separateTree(FortTextTree ftt,FortTreeNode node)
  //FortTextTree ftt;
  //FortTreeNode node;
{
  FortTreeNode son,next;

  switch( NT(node) )
    {
      case GEN_LIST_OF_NODES:	/* list of statements */
        /* must be careful, since we are destroying list elements as we go */
        son = list_first(node);
        while( son != AST_NIL )
          { next = list_next(son);
            separateTree(ftt, son);
            son = next;
          }
        break;

      case GEN_GLOBAL:		/* global scope */
	separateTree(ftt, gen_GLOBAL_get_subprogram_scope_LIST(node));
        break;

      case GEN_FUNCTION:	/* function statement */
        tree_replace(gen_FUNCTION_get_lbl_def(node), AST_NIL);
        tree_replace(gen_FUNCTION_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_FUNCTION_get_type_len(node), AST_NIL);
        tree_replace(gen_FUNCTION_get_name(node), AST_NIL);
        tree_replace(gen_FUNCTION_get_formal_arg_LIST(node), AST_NIL);
        separateTree(ftt, gen_FUNCTION_get_stmt_LIST(node));
        break;

      case GEN_PROGRAM:		/* program statement */
        tree_replace(gen_PROGRAM_get_lbl_def(node), AST_NIL);
        tree_replace(gen_PROGRAM_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_PROGRAM_get_name(node), AST_NIL);
        separateTree(ftt, gen_PROGRAM_get_stmt_LIST(node));
        break;

      case GEN_SUBROUTINE:	/* subroutine statement */
        tree_replace(gen_SUBROUTINE_get_lbl_def(node), AST_NIL);
        tree_replace(gen_SUBROUTINE_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_SUBROUTINE_get_name(node), AST_NIL);
        tree_replace(gen_SUBROUTINE_get_formal_arg_LIST(node), AST_NIL);
        separateTree(ftt, gen_SUBROUTINE_get_stmt_LIST(node));
        break;

      case GEN_BLOCK_DATA:	/* block data statement */
        tree_replace(gen_BLOCK_DATA_get_lbl_def(node), AST_NIL);
        tree_replace(gen_BLOCK_DATA_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_BLOCK_DATA_get_name(node), AST_NIL);
        separateTree(ftt, gen_BLOCK_DATA_get_stmt_LIST(node));
        break;

      case GEN_DEBUG:		/* debug statement */
        tree_replace(gen_DEBUG_get_lbl_def(node), AST_NIL);
        tree_replace(gen_DEBUG_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_DEBUG_get_option_LIST(node), AST_NIL);
        separateTree(ftt, gen_DEBUG_get_stmt_LIST(node));
        break;

      case GEN_DO:		/* do statement */
        tree_replace(gen_DO_get_lbl_def(node), AST_NIL);
        tree_replace(gen_DO_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_DO_get_lbl_ref(node), AST_NIL);
        tree_replace(gen_DO_get_control(node), AST_NIL);
        separateTree(ftt, gen_DO_get_stmt_LIST(node));
        break;

      case GEN_DO_ALL:		/* do all statement */
        tree_replace(gen_DO_ALL_get_lbl_def(node), AST_NIL);
        tree_replace(gen_DO_ALL_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_DO_ALL_get_lbl_ref(node), AST_NIL);
        tree_replace(gen_DO_ALL_get_control(node), AST_NIL);
        separateTree(ftt, gen_DO_ALL_get_stmt_LIST(node));
        break;

      case GEN_IF:		/* if statement */
        tree_replace(gen_IF_get_lbl_def(node), AST_NIL);
        tree_replace(gen_IF_get_close_lbl_def(node), AST_NIL);
	separateTree(ftt, gen_IF_get_guard_LIST(node));
        break;

      case GEN_GUARD:		/* guard */
        tree_replace(gen_GUARD_get_lbl_def(node), AST_NIL);
        tree_replace(gen_GUARD_get_rvalue(node), AST_NIL);
        separateTree(ftt, gen_GUARD_get_stmt_LIST(node));
        break;

      case GEN_PARALLEL:	/* parallel block */
	tree_replace(gen_PARALLEL_get_lbl_def(node), AST_NIL);
	tree_replace(gen_PARALLEL_get_close_lbl_def(node), AST_NIL);
	tree_replace(gen_PARALLEL_get_lvalue(node), AST_NIL);
	tree_replace(gen_PARALLEL_get_rvalue(node), AST_NIL);
        separateTree(ftt, gen_PARALLEL_get_stmt_LIST(node));
        separateTree(ftt, gen_PARALLEL_get_parallel_case_LIST(node));
        break;

      case GEN_PARALLEL_CASE:	/* parallel case statement */
	tree_replace(gen_PARALLEL_CASE_get_lbl_def(node), AST_NIL);
	tree_replace(gen_PARALLEL_CASE_get_rvalue_LIST(node), AST_NIL);
        separateTree(ftt, gen_PARALLEL_CASE_get_stmt_LIST(node));
        break;

      case GEN_PARALLELLOOP:	/* parallel loop statement */
        tree_replace(gen_PARALLELLOOP_get_lbl_def(node), AST_NIL);
        tree_replace(gen_PARALLELLOOP_get_close_lbl_def(node), AST_NIL);
        tree_replace(gen_PARALLELLOOP_get_lbl_ref(node), AST_NIL);
        tree_replace(gen_PARALLELLOOP_get_control(node), AST_NIL);
        separateTree(ftt, gen_PARALLELLOOP_get_stmt_LIST(node));
        break;

      case GEN_PLACE_HOLDER:	/* statement placeholders */
	tree_replace(node, AST_NIL);
        break;

      case GEN_ERROR:		/* misplaced / error statement */
        if( gen_get_error_code(node) == 0 )
	  { /* an error statement */
              tree_replace(node, AST_NIL);
          }
        else
          { /* a misplaced statement */
              tree_replace(gen_ERROR_get_tree(node),  AST_NIL);
              tree_replace(gen_ERROR_get_part0(node), AST_NIL);
              tree_replace(gen_ERROR_get_part1(node), AST_NIL);
              tree_replace(gen_ERROR_get_part2(node), AST_NIL);
              tree_replace(gen_ERROR_get_part3(node), AST_NIL);
          }
        break;

      default:			/* simple statement */
	tree_replace(node, AST_NIL);
        break;
    }
}
