/* $Id: sym.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdio.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/f2i/classes.h>

#define AI_COMMON_TABLE_SIZE    64
#define AI_TABLE_NOT_FOUND   -2
#define AI_TABLE_IS_FULL     -1

#define NO_ALIAS	-1
#define ALIASED		1

#define NO_EQUIVALENCE  0
#define EQUIV_FOLLOWER  1
#define EQUIV_LEADER	2

#define	NO_REGISTER	-1

#define DONT_KNOW	-9090 /* the DK value for insert_symbol() */
#define DK		-9090

#define FORTRAN_PARAMETER -2

#define fst_my_GetFieldByIndex fst_GetFieldByIndex
#define fst_my_PutFieldByIndex fst_PutFieldByIndex
#define fst_my_QueryIndex fst_QueryIndex

#ifndef _AST_H_
#include <libs/frontEnd/ast/ast.h>
#endif

int			SymDebug;
SymDescriptor ft_SymTable;


char			*Sym2ndReg;

int			SymHaveSeenAnEquivalence;

/* exported names */
extern int SymInsertSymbol();
extern int getIndexForlb();
extern int getIndexForub();
extern void SymZeroScratchField();
extern void InitFieldsInSymTab ();
struct common_table_elt
{
  char	*name;
  int	size;
  int   addressReg;
  int   first_name;
  int	last_name;
  int 	used;
};

struct common_table_elt *common_table;
int CommonNextSlot;
int CommonNumberOfSlots;

struct leader_table_elt
{
  int low;
  int high;
  int common_name;
};
struct leader_table_elt *leader_table;
int    leader_table_free_list;

int SymHaveSeenASave;









