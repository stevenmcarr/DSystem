/* $Id: idfa.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdio.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/classes.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/include/ipdfi.h>
#include <libs/ipAnalysis/interface/IPQuery.h>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefQuery.h>
#include <libs/f2i/AliasQuery.h>

extern C_CallGraph   ProgramCallGraph;
extern Boolean       f2i_ComputeInterproceduralInfo;

#define	NO_INFO	-1

/*
 * Add these two lines to this file if you want to always print the
 * interprocedural analysis debugging information:
 *
 * #define aiDebug 1
 * #define stdout  stderr
 *
 */

/* The interface routines to the interprocedural data flow 
 * information kept in the database.  (After all, this is 
 * part of the Rn programming environment.)
 *
 * aiIdfaFini() - closes down the interprocedural data flow
 *		  information interface.
 *
 *
 * aiNameIsMod(site, index)
 *		- site is the AST_INDEX of the INVOCATION node
 *		- index is the variable's integer symbol table index
 *
 *		- predicate for variables by symbol table index
 *		  TRUE  <=> variable may be MODIFIED by call
 *		  FALSE <=> variable is NOT MODIFIED by call
 *
 * aiGlobalMods(site)
 *		- site is the AST_INDEX of the INVOCATION node
 *
 *		- returns an array of integers.  The first element of the
 *		  array contains the number of symbol table indices in the
 *		  array.  The remaining entries contain symbol table indices
 *		  for potentially modified global variables.
 *
 * aiNameIsUse(site, index)
 *		- site is the AST_INDEX of the INVOCATION node
 *		- index is the variable's integer symbol table index
 *
 *		- predicate for variables by symbol table index
 *		  TRUE  <=> variable may be USED by call
 *		  FALSE <=> variable is NOT USED by call
 *
 * aiGlobalUses(site)
 *		- site is the AST_INDEX of the INVOCATION node
 *
 *		- returns an array of integers.  The first element of the
 *		  array contains the number of symbol table indices in the
 *		  array.  The remaining entries contain symbol table indices
 *		  for potentially used global variables.
 *
 * aiAliases(index)
 *		- index is the variable's integer symbol table index
 *
 *		- returns an array of integers.  The first element of the
 *		  array contains the number of symbol table indices in the
 *		  array.  The remaining entries contain the symbol table indices
 *		  for variables potentially aliased to the variable represented
 *		  by the formal parameter "index".
 *
 *              This is not currently implemented, but it is needed.
 *
 * IsAliased(index,IsGlobal)
 *              - index is the variable's integer symbol table index
 *              - IsGlobal is a boolean indicating whether or not the 
 *                variable is global.
 *
 *              - predicate for variables by symbol table index
 *                TRUE  <=> variable is aliased
 *                FALSE <=> varialbe is not aliased
 *
 * aiNameIsConstant(index)
 *              - index is the variable's integer symbol table index
 *
 *              - returns a string containing the constant associated with
 *                the variable indicated by "index".  If NULL is returned,
 *                either there is no constant value or there is no
 *                interprocedural information available.
 *
 * aiNameIsConstantOnReturn(site, index)
 *              - site is the AST_INDEX of the INVOCATION node
 *              - index is the variable's integer symbol table index
 *
 *              - returns a string containing the constant associated with
 *                the variable indicated by "index" after the invocation
 *                associated with "site".  If NULL is returned, either there
 *                is no constant value or there is no interprocedural
 *                information available.
 *
 */


extern FortTree  ft;

static ipdfi  *ip;

static int	  *all_globals   = (int*) 0;
static int	  *global_scratch= (int*) 0;
static int	  *global_mods   = (int*) 0;
static int	  *global_uses   = (int*) 0;

static void aiGetGlobal(SymDescriptor, fst_index_t, Generic);


void aiGetAllGlobals()

  {
   register int i, j;
  
     /*  allocate the data structures for the global variables  */
     all_globals = (int *) get_mem(sizeof(int)*(aiMaxVariables + 1), 
				   "Global variable information");

      /*  fill in global mods with the names of all of the 
	  globals in the symbol table  */
      /*  add in everyone who is equivalenced (or aliased?)    */
      /*  allGlobals[0] = number of globals in the list	   */

     if (aiDebug > 0)
       (void) fprintf(stdout, "aiGetAllGlobals().\n");
     all_globals[0] = 0;
     fst_ForAll (ft_SymTable, aiGetGlobal, j);
  }

static void aiGetGlobal(SymDescriptor SymTab, fst_index_t i, Generic dummy)

//   SymDescriptor SymTab;
//   fst_index_t i;
//   Generic dummy;

  {
    if ((fst_my_GetFieldByIndex(SymTab, i, SYMTAB_OBJECT_CLASS)
	 & OC_IS_DATA)  &&
	(!(fst_my_GetFieldByIndex(SymTab, i, SYMTAB_OBJECT_CLASS)
	 & OC_IS_FORMAL_PAR)) &&
	(fst_my_GetFieldByIndex(SymTab, i, SYMTAB_STORAGE_CLASS)
	 & SC_GLOBAL) &&
	(fst_my_GetFieldByIndex(SymTab, i, SYMTAB_REG) > 1))
      {
       all_globals[0] += 1;             /* assumes equivalences are included */
       all_globals[all_globals[0]] = i;
      }
   }

/* int *aiAliases(index)
 *      int  index;
 * {
 *   int  leader;
 *   int  offset;
 *   int  length;
 */
  /* If this is a global variable, get aliases and add equivalences */
/*   if ((fst_my_GetFieldByIndex(symtab, index, SYMTAB_OBJECT_CLASS)
 *        & OC_IS_DATA) &&
 *       (fst_my_GetFieldByIndex(symtab, index, SYMTAB_STORAGE_CLASS)
 *        & SC_GLOBAL) &&
 *       (fst_my_GetFieldByIndex(symtab, index, SYMTAB_REG) > 1))
 *     {
 *       fst_Symbol_To_EquivLeader_Offset_Size(ft_SymTable, index, &leader, &offset, &length);
 * 
 *       aliases = IPQuery_GetAliases(ProgramCallGraph, proc_text,
 * 				   (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
 *				   length, offset, TRUE, aliases);
 *     }
 * }
 */
void aiIdfaFini()
{
  int i;

  if (aiDebug > 0)
     (void) fprintf(stdout, "aiIdfaFini().\n");

    /*  free the MOD structures  */
    if (global_mods != (int*) 0)
      free_mem(global_mods);

    /*  free the USE structures  */
    if (global_uses != (int*) 0)
      free_mem(global_uses);

    /*  free the global scratch structure  */
    if (global_scratch != (int*) 0)
      free_mem(global_scratch);

    /*  free the ALIAS structures  */

    /*  free the all_globals structure, if necessary  */
    if (all_globals != (int*) 0)
      free_mem(all_globals);

    /*  free the all_aliases structure, if necessary  */
 }


void aiIdfaAdd(int index, int *global_sets)

//   int   index;
//   int   *global_sets;

  {
   int   i;
   int   *equivalences;

      /*  if there are no equivalences, simply add the  */
      /*  symbol table index to the appropriate set     */
     if (!SymHaveSeenAnEquivalence)
       global_sets[++global_sets[0]] = index;

      /*  otherwise, add the element and all of its  */
      /*  equivalences to the appropriate sets       */
     else
       if (!global_scratch[index])
	 {
	  equivalences = aiEquivClass(index);
	  for (i=1; i<=equivalences[0]; i++)
	    {
	     global_scratch[equivalences[i]] = TRUE;
	     global_sets[++global_sets[0]] = index;
	    }
	 }
  }


int aiNameIsUse( SymDescriptor symtab, AST_INDEX site, int index )
//   AST_INDEX 	site;
//   int		index;
//   SymDescriptor symtab;

  {    
   int leader,
       offset,
       length;

     if (NOT(f2i_ComputeInterproceduralInfo))
       return(true);
  
     fst_Symbol_To_EquivLeader_Offset_Size(symtab,index,&leader,&offset,
					   (unsigned int *)&length);

     /* check for functions passed as parameters */
     if (leader < 0)
	 {
	   if (fst_my_GetFieldByIndex(symtab, index, SYMTAB_OBJECT_CLASS) & OC_IS_DATA)
	     ERROR("aiNameIsUse", "Invalid request for REF information", WARNING);
	   return (false);
         }

     return IPQuery_IsScalarRef((Generic)ProgramCallGraph,
				proc_text,
				ft_NodeToNumber(ft, site),
				(char *) fst_my_GetFieldByIndex(symtab,leader,SYMTAB_NAME),
				offset,length);
  }


int *aiGlobalUses( AST_INDEX site )
  // AST_INDEX	site;

  {
   int	i;

     if (NOT(f2i_ComputeInterproceduralInfo))
       {
	if (all_globals == (int*) 0)
	  aiGetAllGlobals();
	return all_globals;
       }

     if (all_globals == (int*) 0)
       aiGetAllGlobals();
     if (global_uses == (int*)0)
       global_uses = (int *) get_mem(sizeof(int)*(aiMaxVariables + 1), 
				     "USE global information"); 
     if (global_scratch == (int*)0)
       global_scratch = (int *) get_mem(sizeof(int)*aiMaxVariables, 
					"Global scratch area");
     global_uses[0] = 0;
     for (i = 1; i < all_globals[0]; i++)
       if (aiNameIsUse(ft_SymTable,site,all_globals[i]))
	 aiIdfaAdd(all_globals[i],global_uses);
     return global_uses;
  }


int aiNameIsMod(SymDescriptor symtab, AST_INDEX site, int index )
//   SymDescriptor symtab;
//   AST_INDEX 	site;
//   int		index;

  {    
   int leader,
       offset,
       length;

   int answer;
  
     if (NOT(f2i_ComputeInterproceduralInfo))
       return(true);

     fst_Symbol_To_EquivLeader_Offset_Size(symtab,index,&leader,&offset,
					   (unsigned int *)&length);

     /* check for functions passed as parameters */
     if (leader < 0)
       {
         if (fst_my_GetFieldByIndex(symtab, index, SYMTAB_OBJECT_CLASS) & OC_IS_DATA)
	   ERROR("aiNameIsMod", "Invalid request for MOD information", WARNING);
         return(false);
       }
       

     answer = IPQuery_IsScalarMod((Generic)ProgramCallGraph,
				  proc_text, ft_NodeToNumber(ft, site), 
				  (char *)fst_my_GetFieldByIndex(symtab,leader,
								 SYMTAB_NAME),
				  offset,length);
 
     return answer;
  }


int *aiGlobalMods( AST_INDEX site )

  // AST_INDEX	site;

  {
   int	*result,i;

     if (NOT(f2i_ComputeInterproceduralInfo))
       {
	if (all_globals == (int*) 0)
	  aiGetAllGlobals();
	return all_globals;
       }

     if (all_globals == (int*) 0)
       aiGetAllGlobals();
     if (global_mods == (int*)0)
       global_mods = (int *) get_mem(sizeof(int)*(aiMaxVariables + 1), 
				     "MOD global information"); 
     if (global_scratch == (int*)0)
       global_scratch = (int *) get_mem(sizeof(int)*aiMaxVariables, 
					"Global scratch area");
      global_mods[0] = 0;
      for (i = 1; i < all_globals[0]; i++)
        if (aiNameIsMod(ft_SymTable,site,all_globals[i]))
	  aiIdfaAdd(all_globals[i],global_mods);
     return global_mods;
  }




/* using interprocedural alias information, determine if a variable is aliased */
int IsAliased(int i,Boolean IsGlobal)

//   int i;
//   Boolean IsGlobal;

  {
   int leader,
       offset,
       length;

     if (aiNoAlias > 0)
       return 0;
     else
       {
	fst_Symbol_To_EquivLeader_Offset_Size(ft_SymTable,i,&leader,&offset,
					      (unsigned int *)&length);

	/* check for functions passed as parameters */
	if ((leader < 0) &&
	    (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_FUNCTION))
	    return 0;

	return IPQuery_IsAliased((Generic)ProgramCallGraph,proc_text,
				 (char *) fst_my_GetFieldByIndex(ft_SymTable,leader,
						     SYMTAB_NAME),
				 length,offset,IsGlobal);
       }
  }




char *aiNameIsConstant(int index)
  //  int   index;
{
  char  *result;

  result = NULL;

  return result;
}

char *aiNameIsConstantOnReturn(AST_INDEX site, int index)
  // AST_INDEX site;
  // int index;
{
        return NULL;
}
