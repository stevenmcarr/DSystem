/* $Id: lex2.C,v 1.1 1997/06/24 17:47:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortParse2/lex2.c				        */
/*									*/
/*	FortParse2/lex2 -- scanner for high-level Fortran parser	*/
/*									*/
/************************************************************************/

#include <string.h>

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/gen.h>


#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/fortParse2/FortParse2.i>

#include <libs/support/arrays/FlexibleArray.h>
#include <libs/support/stacks/xstack.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Static data		*/
/************************/




/* input description */

static int              lx2_goal;
static Flex *           lx2_lines;
static int		lx2_first;
static int		lx2_last;
static int		lx2_next;




/* do loop label stack information */

static Stack		lx2_labelStack;
static Boolean		lx2_popping;
static fx_StatToken	lx2_enddoImpliedToken =
  {
    SEND_DO_IMPLIED,
    0,
    0,
    0,
    0,
    0
  };







/************************/
/* Forward declarations	*/
/************************/




STATIC(void,		clipToken, (fx_StatToken stToken));
STATIC(Boolean,		equalLabels, (FortTreeNode l1, FortTreeNode l2));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void lx2_Init(void)
{
  lx2_labelStack = stack_create(sizeof(FortTreeNode));
}




void lx2_Fini(void)
{
  stack_destroy(lx2_labelStack);
}




void lx2_SetScan(int goal, Flex *lines, int start, int count)
//   int goal;
//   Flex * lines;
//   int start,count;
{
  lx2_goal  = goal;
  lx2_lines = lines;
  lx2_first = start;
  lx2_last  = start + count - 1;
  lx2_next  = lx2_first - 1;  /* goal token will be prepended */
  if (lx2_popping)
    fprintf(stderr, "lx2_popping is tree in lx2_SetScan\n");
  lx2_popping = false;
}




int yy2lex(void)
{
  TT_Line line;
  fx_StatToken stToken;
  FortTreeNode top, current;
# define SEOF	-1

  if( lx2_popping )
    { /* return an implied enddo */
        (void) stack_pop(lx2_labelStack, (Generic *) &current);
        if( stack_get(lx2_labelStack, (Generic *) &top, 1) )
          lx2_popping = equalLabels(current, top);
        else
          lx2_popping = false;

      yy2lval.statval = lx2_enddoImpliedToken;
      return SEND_DO_IMPLIED;
    }
  else if( lx2_next == lx2_first-1 )
    { lx2_next += 1;
      return lx2_goal;
    }
  else if( lx2_next <= lx2_last )
    { /* fetch the next line description */
        (void) flex_get_buffer(lx2_lines, lx2_next, 1, (char *) &line);
        stToken = * ((fx_StatToken *) &line.token);
        stToken.conceal = line.conceal;
        tt_getTagNode(ftt_textTree, line.lineNode, &stToken.tt_tag);
        lx2_next += 1;

      /* prepare the token for gram2 */
        clipToken(stToken);
        yy2lval.statval = stToken;

      switch( stToken.token )
        { case SDO_LABEL_STAT:
          case SDOALL_LABEL_STAT:
          case SPARALLELLOOP_LABEL_STAT:
            /* add a label to the stack */
              stack_push(lx2_labelStack, (Generic *)&stToken.part[1]);
            break;

          case SCOMMENT_STAT:
          case SERROR_STAT:
          case SPRSCOPE_PH_STAT:
          case SSTMT_PH_STAT:
          case SSPECIFICATION_STMT_PH_STAT:
          case SCONTROL_STMT_PH_STAT:
          case SIO_STMT_PH_STAT:
          case SPARASCOPE_STMT_PH_STAT:
          case SDEBUG_STMT_PH_STAT:
            /* these do not have labels */
            break;

	  case SFUNCTION_STAT: /* new */
          case SBLOCK_DATA_STAT:
          case SDEBUG_STAT:
          case SEND_STAT:
          case SPROGRAM_STAT:
          case SSUBROUTINE_STAT:
            /* clear the stack */
              while( stack_pop(lx2_labelStack, (Generic *) &current) )
                ;
            break;

	  case SDO_STAT: /* new */
          case SPARBEGIN_STAT: /* new */
          case SPARBEGIN_PID_STAT: /* new */
	  case SDOALL_STAT: /* new */
          case SPARALLELLOOP_STAT: /* new */
	  case SIF_STAT:
          case SELSE_IF_STAT:
          case SELSE_STAT:
          case SEND_ALL_STAT:
          case SEND_DO_STAT:
          case SEND_IF_STAT:
          case SEND_LOOP_STAT:
          case SOTHER_PROCESSES_STAT:
          case SPARALLEL_PID_STAT:
          case SPARALLEL_STAT:
          case SPAREND_STAT:
          case SWHERE_BLOCK_STAT: /* F90 */
          case SELSE_WHERE_STAT: /* F90 */
          case SEND_WHERE_STAT: /* F90 */
            if( stack_get(lx2_labelStack, (Generic *) &top, 1) )
              lx2_popping = equalLabels(stToken.part[0], top);
            else
              lx2_popping = false;
            break;

          default:
            if( stack_get(lx2_labelStack, (Generic *) &top, 1) )
              lx2_popping = equalLabels(gen_get_label(stToken.tree), top);
            else
              lx2_popping = false;
            break;
        }

      return stToken.token;
    }
  else if( lx2_next == lx2_last+1 )
    { lx2_next += 1;

      /* clear the stack */
        while( stack_pop(lx2_labelStack, (Generic *) &current) )
          {};

      return SENDMARKER;
    }
  else return SEOF;
}





/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void clipToken(fx_StatToken stToken)
  // fx_StatToken stToken;
{
  tree_replace(stToken.tree,    AST_NIL);
  tree_replace(stToken.part[0], AST_NIL);
  tree_replace(stToken.part[1], AST_NIL);
  tree_replace(stToken.part[2], AST_NIL);
  tree_replace(stToken.part[3], AST_NIL);
}




static
Boolean equalLabels(FortTreeNode l1, FortTreeNode l2)
  // FortTreeNode l1, l2;
{
  char *t1 = gen_get_text(l1);
  char *t2 = gen_get_text(l2);

  return BOOL( strcmp(t1, t2) == 0 );
}

