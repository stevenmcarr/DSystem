/* $Id: common.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>
#include <stdio.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>

#include <libs/f2i/mnemonics.h>


/* forward declarations */
/*char *strcpy();*/

/* Since variable names can be re-used as common names, 
   /common_name/ denotes the common block name */




/*ARGSUSED*/
/* print out the contents of the COMMON table */
void CommonDumpTable(SymDescriptor SymTab,fst_index_t i, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic dummy;

{
  register int j;

  int last_name;


  if (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_COMMON_NAME)
  {
    (void) fprintf(stdout, "\n\tCommon Table:\nIndex\tName\tSize\tFirst\tLast\n\n");

    if (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_COMMON_NAME)
      {
	j = fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_FIRST_NAME),
	last_name = fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_LAST_NAME),
	(void) fprintf(stdout, "%d\t%-7.7s\t%d\t%d\t%d\n", i, 
		       fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME),
		       fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE),j,last_name);

	while(j != last_name)
	  {
	    (void) fprintf(stdout, "\t  %s (%d)\n",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_NAME), j);
	    j = fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_NEXT_COMMON);
	  }
	(void) fprintf(stdout, "\t%s (%d)\n",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_NAME), j);
    }
    (void) fprintf(stdout, "\n");
  }
} /* CommonDumpTable */


static int inStatic;
char BlankName[] = "_BLNK_";



/* extract COMMON name from symbol table */
char *aiGetCommonBlockName(fst_index_t i, char *name)
//   fst_index_t i;
//   char *name;
{
  if (strcmp((char *) fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME), "//") == 0)
      (void) strcpy (name, BlankName);
  else 
      {
	(void) strcpy (name, (char *) (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME)+1));
	name[strlen(name)-1]='\0';
      }
   return name;

} /* aiGetCommonBlockName */




/*ARGSUSED*/
/* generate iloc to create space for a particular COMMON block */
void aiGenerateCommon(SymDescriptor SymTab, fst_index_t i, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic dummy;

{
  char buffer[64];
  char space[32];  /* used for extracting common name from symbol table */

  if (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_COMMON_NAME)  
  {
    if (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_used) != 0)
    {
      if (inStatic == 0)
      {
	inStatic = 1;
	generate(0, COMMENT, 0, 0, 0, "Common block definitions");
      }
      (void) sprintf(buffer, "_%s_", aiGetCommonBlockName(i, space));

      generate(0, NAME, (int) buffer, 0, 0, "Common block name");
      generate_string(buffer, BYTES,
		Align(fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE)), 0, 0,
		"and its space");
    }
  }
} /* aiGenerateCommon */




/* generate all of the COMMON blocks */
void aiGenerateCommons()
{
  inStatic = 0;
  fst_ForAll (ft_SymTable, aiGenerateCommon, 0);
} /* aiGenerateCommons */



static int *GNTable = (int*)0;
static char Char_Name[64];
static int CTI;



/*ARGSUSED*/
/* Find the location of the common block name */
void MarkGlobalName(SymDescriptor SymTab, fst_index_t i, Generic marked)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic marked;
{
  
  if (!(fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) &
        OC_IS_COMMON_NAME) || marked)
    return;

  if (strcmp(Char_Name, (char *) fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME)) == 0)
    {
      CTI = i;                  /* mark it */
      /* unfortunately, we can't stop looping through sym table once we find
         the name (cij-7/23/92) */
      *((Generic *)marked) = 1;
    }
} /* MarkGlobalName */




/* Get the symbol table indices for a global variable.  This is    */
/* used in the routines for interprocedural data-flow information. */
int *aiGlobalNames(char *CName, int Offset, int Size)
//   char *CName;
//   int  Offset;
//   int  Size;
{
  register int i, where, ub, lb;
  int marked = 0;

  if (GNTable == (int*)0)
     GNTable = (int *) get_mem(sizeof(int)*(fst_MaxIndex(ft_SymTable)), "GNTable");

  if (CName[0] == '\0')
     CName = BlankName;

  CTI = 0;

  (void) strcpy (Char_Name, CName);
  fst_ForAll(ft_SymTable, (fst_ForAllCallback)MarkGlobalName, (Generic) &marked);

  if (CTI == 0)		/* a pass-through common block, not referenced	*/
  {			/* in the current procedure 			*/
    GNTable[0] = 0;
    return GNTable;
  }

  GNTable[0] = 0;			/* initialize the NameTable 	*/
  where = 0;				/* and walk the common block,	*/
  lb = Offset;				/* filling it in		*/
  ub = Offset + Size - 1;
  i  = fst_GetFieldByIndex(ft_SymTable, CTI, SYMTAB_FIRST_NAME);
  while(i != -1)
  {
    if (where < ub && lb < where + fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE))
    {
      GNTable[0]++;
      GNTable[GNTable[0]] = i; 
    }
    where += fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE);

    if (ub <= where)
      i = -1;
    else 
      i = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NEXT_COMMON);
  }

  if (GNTable[0] == 0)
  {
    (void) sprintf(error_buffer, 
	"Global name descriptor <'%s',%d,%d> doesn't map onto '%s' (length %d)",
	CName, Offset, Size, fst_GetFieldByIndex(ft_SymTable, CTI, SYMTAB_NAME),
        fst_GetFieldByIndex(ft_SymTable, CTI, SYMTAB_SIZE));
    ERROR("aiGlobalNames", error_buffer, SERIOUS);
    ERROR("aiGlobalNames", 
	  "This indicates a problem with the interprocedural information",
	  SERIOUS);
  }
  return GNTable;
  
} /* aiGlobalNames */

