/* $Id: initials.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/mnemonics.h>

struct InitNode
{
  int Reg;
  int Node;
  struct InitNode *next;
};

static struct InitNode *InitList = (struct InitNode *) 0;




void RecordInitialExp( int reg, AST_INDEX node )
  // int reg;
  // AST_INDEX node;
{
  struct InitNode *New;

  New = (struct InitNode *) 
	get_mem(sizeof(struct InitNode), "Initial expression node");

  if (New == (struct InitNode *) 0)
     ERROR("RecordInitialExp", "Memory allocation failure", FATAL);

  New->Reg  = reg;
  New->Node = node;
  New->next = InitList;

  InitList  = New;

} /* RecordInitialExps */




void GenerateInitialExps()
{

  struct InitNode *ptr;
  struct InitNode *old;
  int reg;

  if (InitList != (struct InitNode *) 0)
  {
    generate(0, COMMENT, 0, 0, 0, "Initial expressions");

    ptr		= InitList;			                /* get the list header	*/
    InitList	= (struct InitNode *) 0;	                /* and zero it out	*/

    while (ptr != (struct InitNode *) 0)	                /* while there's more	*/
    {
      reg = getExprInReg((AST_INDEX) ptr->Node);		/* generate the code	*/
      generate_move(ptr->Reg, reg, fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_TYPE));

      old = ptr;				                /* free the node and 	*/
      ptr = ptr->next;				                /* move on ...		*/
      free_mem((void *) old);
    }
    generate(0, COMMENT, 0, 0, 0, "End of initial expressions");
  }
} /* GenerateInitialExps */
