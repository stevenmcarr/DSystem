/* $Id: FortParse1.C,v 1.1 1997/06/24 17:45:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortParse1/FortParse1.c			        */
/*									*/
/*	FortParse1 -- low-level parser for Fortran source		*/
/*									*/
/************************************************************************/



#include <string.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.i>
#include <libs/frontEnd/fortTextTree/fortParse1/FortParse1.i>
#include <libs/frontEnd/fortTextTree/fortParse1/lex1.h>


#include <libs/support/memMgmt/mem.h>





/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Static data		*/
/************************/




fx_StatToken fp1_token;		/* accessed by gram1.y */
char * fp1_error;		/* accessed by gram1.y */


#define MAX_STACK 100
static FortTreeNode  stack[MAX_STACK];
static int stack_size = 0;




/************************/
/* Forward declarations	*/
/************************/




STATIC (void,		initToken, (fx_StatToken *st));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void fp1_Init()
{
  lx1_Init();
}




void fp1_Fini()
{
  if( fp1_error != nil )
    { free_mem((void *) fp1_error);
      fp1_error = nil;
    }

  lx1_Fini();
}




/************************/
/*  Parsing		*/
/************************/




Boolean fp1_Parse(FortTextTree ftt, TextString text, fx_StatToken *st)
{
  FortTreeNode linetext,commtext;
  char *s, *ptr;
  int i;

  /* initialize the parser */
    ft_AstSelect(ftt_Tree(ftt));
    if( fp1_error != nil )
      { free_mem((void *) fp1_error);
        fp1_error = nil;
      }
    initToken(&fp1_token);

  parse1(ftt, text);

  /* check for parse1 error */
    if( fp1_error != nil  ||  fp1_token.token == SERROR_STAT )
      { /* build the complaint node */
          commtext = gen_TEXT();
          gen_put_text(commtext, (fp1_error)  ?  fp1_error  :  "syntax error",
						STR_COMMENT_TEXT);

        /* build the text node (saved input string) */
          s = (char *) get_mem(text.num_tc * 2 + 1, "makeErrorNode()");
          ptr = s;
          for (i = 0; i < text.num_tc; i++)
            { if( text.tc_ptr[i].style & ftt_BEGIN_PLACEHOLDER_STYLE )
                *ptr++ = 128;

              if( text.tc_ptr[i].style & ftt_PLACEHOLDER_STYLE )
                *ptr++ = text.tc_ptr[i].ch | 128;
              else
                *ptr++ = text.tc_ptr[i].ch;
            }
          *ptr = '\0';
          linetext = gen_TEXT();
          gen_put_text(linetext, s, STR_COMMENT_TEXT);
          free_mem((void *) s);

	/* make the error token */
          fp1_token.token = SERROR_STAT;
          fp1_token.tree  = gen_ERROR(commtext, linetext, AST_NIL, AST_NIL,
						AST_NIL, AST_NIL);
      }

  /* set return values */
    *st = fp1_token;
    return BOOL(fp1_token.token != SERROR_STAT);
}




/*ARGSUSED*/
void fp1_Copy(FortTextTree ftt, fx_StatToken *oldToken, fx_StatToken *newToken)
{
  newToken->token     = oldToken->token;
  newToken->tree      = tree_copy(oldToken->tree);
  newToken->part[0]   = tree_copy(oldToken->part[0]);
  newToken->part[1]   = tree_copy(oldToken->part[1]);
  newToken->part[2]   = tree_copy(oldToken->part[2]);
  newToken->part[3]   = tree_copy(oldToken->part[3]);
}




/* ARGSUSED */
void fp1_Destroy(FortTextTree ftt, fx_StatToken *st)
{
  ft_AstSelect(ftt_Tree(ftt));

  /* separate the token */
    tree_replace(st->tree, AST_NIL);
    tree_replace(st->part[0], AST_NIL);
    tree_replace(st->part[1], AST_NIL);
    tree_replace(st->part[2], AST_NIL);
    tree_replace(st->part[3], AST_NIL);

  /* free the token */
    tree_free(st->tree);
    tree_free(st->part[0]);
    tree_free(st->part[1]);
    tree_free(st->part[2]);
    tree_free(st->part[3]);

  /* zero the token */
    initToken(st);
}






/************************/
/*  Error reporting	*/
/************************/




/*ARGSUSED*/
void fp1_GetError(FortTreeNode errorNode, char *msg, int maxMessageLen)
{
  (void) strncpy(msg,"Not implemented",maxMessageLen);
}






/************************************************************************/
/*	Internal Operations for Subparts				*/
/************************************************************************/




/************************/
/*  Tree Stack		*/
/************************/




void
treeStackPush(FortTreeNode node)
{
  if ( stack_size == MAX_STACK )
    die_with_message("Tree stack size exceeded.");
  stack[stack_size++] = node;
}




void
treeStackDrop(int num)
{
  if ( stack_size < num )
    die_with_message("Tree stack size too small.");
  stack_size -= num;
}




void
treeStackCheck()
{
  while ( stack_size )
    tree_free(stack[--stack_size]);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void initToken(fx_StatToken *st)
{
  /* establish precondition relied upon by FortParse2 */

  st->token   = UNUSED;
  st->tree    = AST_NIL;
  st->part[0] = AST_NIL;
  st->part[1] = AST_NIL;
  st->part[2] = AST_NIL;
  st->part[3] = AST_NIL;
}

