/* $Id: routine.C,v 1.2 1997/06/25 15:21:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <include/frontEnd/astnode.h>

#include <libs/f2i/mnemonics.h>




/* forward declarations */
static void GenerateLoadForEachStaticOrGlobalVar (SymDescriptor,fst_index_t,Generic);




/* produce the prologue code for a procedure */
void aiProcedurePrologue ( AST_INDEX parm_list )
  // AST_INDEX	parm_list;
{
  AST_INDEX	tmp_list;
  int		parm_ctr,
		Index[MAX_PARMS],
		addrIndex[MAX_PARMS],
		LenReg,
		LenIndex,
		loadIndex;
  char		buffer[200],
		parms[512],
		*parm_ptr;

   /*
    *  We need to generate:
    *
    *    FRAME size => parms
    *
    *  where size  is the stack frame size in words
    *	     parms is the list of parameters
    */

   /* construct the parameter list */
     tmp_list = list_first(parm_list);
     parms[0] = '\0';
     parm_ptr = parms;
     parm_ctr = 0;

     /* Add the frame pointer as the first parameter in the parameter list. */
     parm_ptr = add_to_regs_list(parm_ptr, aiStackBase());

     /* For architectures where required, add the static data area pointer. */
     if (!aiSparc && !aiRt)
       parm_ptr = add_to_regs_list(parm_ptr, aiStaticLabel());
     
     /* if the parameter is a character, need to include both */
     /* the parameter and the character length as a parameter */
     if (gen_get_node_type(root_node) == GEN_FUNCTION &&
	 ATypeToIType(gen_get_real_type(gen_FUNCTION_get_name(root_node)))
	 == TYPE_CHARACTER)
       {
	 /* look up function name in symbol table */
	 Index[parm_ctr] = fst_my_QueryIndex(ft_SymTable,
		gen_get_text(gen_FUNCTION_get_name(root_node)));
	 parm_ptr = add_to_regs_list(parm_ptr, getAddressRegister(Index[parm_ctr]));
	 
	 /* now load the length */
	 LenIndex = StrTempReg("s-len",Index[parm_ctr++],TYPE_INTEGER);
	 fst_my_PutFieldByIndex(ft_SymTable, LenIndex,
		SYMTAB_OBJECT_CLASS,  OC_IS_DATA | OC_IS_FORMAL_PAR);
	 fst_my_PutFieldByIndex(ft_SymTable, LenIndex,
		SYMTAB_STORAGE_CLASS,  SC_NO_MEMORY);
	 parm_ptr = add_to_regs_list(parm_ptr, getAddressRegister(LenIndex));
       }

     while (tmp_list != ast_null_node)
     {
	/* The decision as to what storage class is appropriate for a */
        /* formal parameter is made over in the storage mapping code. */
	Index[parm_ctr] = getIndex(tmp_list);
	fst_my_PutFieldByIndex(ft_SymTable, Index[parm_ctr], SYMTAB_EXPR,
			(Generic) tmp_list);
	addrIndex[parm_ctr] = getAddressRegister(Index[parm_ctr]);
	parm_ptr = add_to_regs_list(parm_ptr, addrIndex[parm_ctr++]);

	tmp_list = list_next(tmp_list);
     }

  /* now go through the parameter list and add */
  /* string lengths to the list of parameters  */
  tmp_list = list_first(parm_list);
  while (tmp_list != ast_null_node)
  {
    LenIndex = getIndex(tmp_list);
    if (fst_my_GetFieldByIndex(ft_SymTable, LenIndex, SYMTAB_TYPE) ==
		TYPE_CHARACTER)
    {
      (void) sprintf(buffer,"s-len[%d]{%d}", LenIndex, TYPE_INTEGER);
      LenReg = SymInsertSymbol(buffer,TYPE_INTEGER,
		OC_IS_FORMAL_PAR | OC_IS_DATA, 0, SC_NO_MEMORY, DK);
      parm_ptr = add_to_regs_list(parm_ptr, getAddressRegister(LenReg));
    }
   tmp_list = list_next(tmp_list);
  }
  
   /* generate frame instruction  */
     generate_string(proc_name, FRAME, aiStackSize, (Generic) parms, 0,
		"Procedure Prologue Code");

   /* load the parameter values */
     tmp_list = list_first(parm_list);
     parm_ctr = 0;

     /* load character address if this is a character        */
     /* function and store the address in a "safe" register  */
     if (gen_get_node_type(root_node) == GEN_FUNCTION &&
	 ATypeToIType(gen_get_real_type(gen_FUNCTION_get_name(root_node)))
	 == TYPE_CHARACTER)
     {
	 generate_move(getAddressRegister(Index[parm_ctr]), Index[parm_ctr],
	 	TYPE_INTEGER);
	 parm_ctr++;
      }

     while (tmp_list != ast_null_node)
     {
	/* The decision as to what storage class is appropriate for a */
        /* formal parameter is made over in the storage mapping code. */

	loadIndex = StrTempReg("!", addrIndex[parm_ctr],
		fst_my_GetFieldByIndex(ft_SymTable, Index[parm_ctr], SYMTAB_TYPE));

	/*   if the data value is classified as living in a register, we    */
	/*   need to generate a typed load from r<addrIndex> into r<Index>. */
	/* 								    */
	/*  *** nyi ***							    */
	/*   if the data value is an interprocedural constant, we want	    */
	/*   to expressly load the constant value into r<Index>		    */

	if (fst_my_GetFieldByIndex(ft_SymTable, Index[parm_ctr],
		SYMTAB_OBJECT_CLASS) & OC_IS_DATA &&
	    fst_my_GetFieldByIndex(ft_SymTable, Index[parm_ctr],
		SYMTAB_STORAGE_CLASS) & SC_NO_MEMORY)
	{
          generate_load( loadIndex, addrIndex[parm_ctr],
		fst_my_GetFieldByIndex(ft_SymTable, Index[parm_ctr], SYMTAB_TYPE),Index[parm_ctr] , "&unknown");
	  generate_move( Index[parm_ctr], loadIndex,
		fst_my_GetFieldByIndex(ft_SymTable, Index[parm_ctr], SYMTAB_TYPE) );
	}
	tmp_list = list_next(tmp_list);
	parm_ctr++;
     }
  
   /* need to generate a load for each STATIC or    */
   /* GLOBAL variable that's also in a register ... */

  fst_ForAll (ft_SymTable, GenerateLoadForEachStaticOrGlobalVar, 0);

   /* and, generate all the initial expressions */

   GenerateInitialExps();
} /* aiProcedurePrologue */




/*ARGSUSED*/
/* generate a load for each STATIC or GLOBAL variable */
static void GenerateLoadForEachStaticOrGlobalVar (SymDescriptor SymTab, 
						  fst_index_t Index, Generic dummy)
//   SymDescriptor SymTab;
//   fst_index_t Index;
//   Generic dummy;
{
  int addrIndex, loadIndex, type, storage_class;

  storage_class = fst_my_GetFieldByIndex(ft_SymTable, Index,
		SYMTAB_STORAGE_CLASS);
  type = fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE);
  
  if ((storage_class == (SC_NO_MEMORY | SC_STATIC) ||
      (!(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS) &
      OC_IS_FORMAL_PAR) && storage_class == (SC_NO_MEMORY | SC_GLOBAL))) &&
      IsValidName((char *) fst_my_GetFieldByIndex(ft_SymTable, Index,
      SYMTAB_NAME)) == 1)
  {
    addrIndex = getAddressInRegister(Index);
    loadIndex = StrTempReg("!", addrIndex, type);
    
    generate_load(loadIndex, addrIndex, type, Index, "&unknown");
    generate_move(Index, loadIndex, type);
  }
} /* GenerateLoadForEachStaticOrGlobalVar */




/*ARGSUSED*/
/* generate a store for each STATIC variable that is in a register */
static void GenerateStoresForEachStaticVarThatIsInAReg(SymDescriptor SymTab, 
						       fst_index_t Index, 
						       Generic dummy)
//   SymDescriptor SymTab;
//   fst_index_t Index;
//   Generic dummy;
{
  int storage_class;

  storage_class = fst_my_GetFieldByIndex(ft_SymTable, Index,
	SYMTAB_STORAGE_CLASS);
	
  if ((storage_class == (SC_NO_MEMORY | SC_STATIC) ||
	(!(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS) &
	OC_IS_FORMAL_PAR) && storage_class == (SC_NO_MEMORY | SC_GLOBAL))) &&
        IsValidName((char *) fst_my_GetFieldByIndex(ft_SymTable, Index,
	SYMTAB_NAME)) == 1)
    generate_store(getAddressInRegister(Index), Index,
	fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE),Index,"&unknown");
} /* GenerateStoresForEachStaticVarThatIsInAReg */




/* generate the epilogue code for a procedure */
void aiProcedureEpilogue ( AST_INDEX routine, AST_INDEX parm_list, int type )
//   AST_INDEX 	routine;
//   AST_INDEX	parm_list;
//   int		type;
{
   int		Index;

   /* note that we're at the epilogue */
     generate(aiEpilogue, NOP, 0, 0, 0, "Epilogue");

   /* retrieve the parameter values */
     parm_list = list_first(parm_list);
     while (parm_list != ast_null_node)
     {
	Index = getIndex(parm_list);
	if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_STORAGE_CLASS) & SC_NO_MEMORY &&
	    fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NUM_DIMS) == 0)
	   generate_store(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_addressReg), Index, 
		fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE),Index,"&unknown");

	parm_list = list_next(parm_list);
     }

   /* need to generate stores for each STATIC variable	*/
   /* that's also in a register ...			*/

   fst_ForAll (ft_SymTable, GenerateStoresForEachStaticVarThatIsInAReg, 0);

   /* generate a return value, if necessary */
     if (type == GEN_FUNCTION)
     {
	Index   = getIndex(gen_FUNCTION_get_name(routine));
	switch(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE))
	{
	  case TYPE_CHARACTER:
	  case TYPE_LOGICAL:
	  case TYPE_INTEGER:
	    generate(0, iRTN, aiStackBase(), Index, 0, "return integer");
	    break;

	  case TYPE_REAL:
	    generate(0, fRTN, aiStackBase(), Index, 0, "returned real");
	    break;

	  case TYPE_DOUBLE_PRECISION:
	    generate(0, dRTN, aiStackBase(), Index, 0, "returned double precision");
	    break;

	  case TYPE_COMPLEX:
	    generate(0, cRTN, aiStackBase(), Index, 0, "returned complex");
	    break;

	  case TYPE_DOUBLE_COMPLEX:
	    generate(0, qRTN, aiStackBase(), Index, 0, "returned double complex");
	    break;

	  default:
	    ERROR("aiProcedureEpilogue", "unknown function type", FATAL);
	    break;
	}
     }
     
     else if (type == GEN_SUBROUTINE)
       generate(0, RTN, aiStackBase(), 0, 0, "return from subroutine");

} /* aiProcedureEpilogue */




/*ARGSUSED*/
/* generates a load for each STATIC or GLOBAL variable that is in a register */
static void GenerateLoadForEachStaticOrGlobalVarThatIsInAReg(SymDescriptor SymTab, 
							     fst_index_t Index, 
							     Generic dummy)
//   SymDescriptor SymTab;
//   fst_index_t Index;
//   Generic dummy;
{
  int addrIndex, loadIndex, storage_class, type; 

  storage_class = fst_my_GetFieldByIndex(ft_SymTable, Index,
	SYMTAB_STORAGE_CLASS);
  type = fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE);

  if ((storage_class == (SC_NO_MEMORY | SC_STATIC)  ||
       storage_class == (SC_NO_MEMORY | SC_GLOBAL)) &&
       IsValidName((char *) fst_my_GetFieldByIndex(ft_SymTable,
       Index, SYMTAB_NAME)) == 1)
    {
      addrIndex = getAddressInRegister(Index);
      loadIndex = StrTempReg("!", addrIndex, type);
      
      generate_load(loadIndex, addrIndex, type, Index, "&unknown");
      generate_move(Index, loadIndex, type);
    }
} /* GenerateLoadForEachStaticOrGlobalVarThatIsInAReg */




/* generates load for each STATIC or GLOBAL variable that is also a register */
void aiLoadUpStuff()
{

   /* need to generate a load for each STATIC or    */
   /* GLOBAL variable that's also in a register ... */
  
  fst_ForAll (ft_SymTable, GenerateLoadForEachStaticOrGlobalVarThatIsInAReg, 0);

   /* and, generate all the initial expressions  */

   GenerateInitialExps();
} /* aiLoadUpStuff */
