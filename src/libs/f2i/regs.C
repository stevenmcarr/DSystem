/* $Id: regs.C,v 1.2 1997/06/25 15:21:51 carr Exp $ */
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
#include <libs/f2i/mnemonics.h>

/* forward declarations */

/* some static storage */
static char name_buffer[64];




/* retrieve the register used to hold the address for a variable */
int getAddressRegister( int index )
  // int index;
{
  register int  newRegister;

  if (aiDebug > 1) 
     (void) fprintf(stdout, "getAddressRegister( %d '%s' ).\n", index, 
	     (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));

  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_addressReg) != NO_REGISTER)
     newRegister = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_addressReg);
  else
  {
    (void) sprintf(name_buffer, "@%s",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
    newRegister = SymInsertSymbol(name_buffer, TYPE_INTEGER, OC_IS_DATA, 
				  0, SC_NO_MEMORY, FALSE);
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_addressReg,  newRegister);
  }
  return newRegister;
} /* getAddressRegister */




/* compute address for a variable and put it in a register */
int getAddressInRegister( int index )
  // int index;
{
  register int AReg, BReg, TReg, SC;
  int CTIndex;
  char space[32];
  char space2[32];   /* used for extracting common name block from symbol table */
  int CToffset;
  unsigned int CTsize;

  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS) & OC_IS_FORMAL_PAR)
     TReg = getAddressRegister( index );
  else if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS)
	& OC_IS_EXECUTABLE)
  {
    (void) sprintf(space, "_%s_",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
    if (aiSparc > 0)
       generate(0, iLDI, (Generic) space, index, GEN_STRING, "Code address");
    else
       generate(0, iLDI, (Generic) space, index, GEN_STRING, "Static data area address");
    TReg = index;
  }
  else                 /** the case for the majority **/
  {
    (void) sprintf(error_buffer, "Address for '%s'",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
    generate(0, NOP, 0, 0, 0, error_buffer);

    /* this is a gross, disgusting kludge!! */
    (void) sprintf(space, "%d",
		fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset));
    AReg = getConstantInRegFromString(space, TYPE_INTEGER, TYPE_INTEGER);

    SC = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS);

    if (SC & SC_GLOBAL)                  
    {
      /* get index of common block name by getting the leader */
      fst_Symbol_To_EquivLeader_Offset_Size
			(ft_SymTable, index, &CTIndex, &CToffset, &CTsize);
      
      if ((CTIndex == SYM_INVALID_INDEX) ||
	  (!(fst_GetFieldByIndex(ft_SymTable, CTIndex, SYMTAB_OBJECT_CLASS)
		& OC_IS_COMMON_NAME)))
	{
	  (void) sprintf(error_buffer, "'%s' is GLOBAL but has no COMMON NAME",
			 (char *) fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	  ERROR("GetAddressInRegister", error_buffer, FATAL);
	}

      BReg = fst_GetFieldByIndex(ft_SymTable, CTIndex, SYMTAB_addressReg);

      (void) sprintf(space, "_%s_", aiGetCommonBlockName(CTIndex, space2));

      generate(0, iLDI, (Generic) space, BReg, GEN_STRING, "Load Common base addr");

      TReg = TempReg(AReg, BReg, iADD, TYPE_INTEGER);
      generate(0, iADD, AReg, BReg, TReg, "Common base");

      /* force output of definition */
      fst_PutFieldByIndex(ft_SymTable, CTIndex, SYMTAB_used, 1);
    }
    else if (SC & (SC_CONSTANT | SC_STATIC))
    {
      if (SC & SC_CONSTANT &&
		fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE) == TYPE_INTEGER)
      {
	(void) sprintf(error_buffer, 
		"attempt to take address of integer constant '%s' (%d)",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME), index);
	ERROR("GetAddressInRegister", error_buffer, FATAL);
      }
      else /* it is in the static pool */
      {
	BReg = aiStaticLabel();
	TReg = TempReg(AReg, BReg, iADD, TYPE_INTEGER);
	generate(0, iADD, AReg, BReg, TReg, "Static base");
      }
    }
    else if (SC & SC_STACK)
    {
      BReg = aiStackBase();
      TReg = TempReg(AReg, BReg, iADD, TYPE_INTEGER);
      generate(0, iADD, AReg, BReg, TReg, "Stack base");
    }
     
    else /* an invalid storage class */
    {
      (void) sprintf(error_buffer, 
		"Invalid storage class for '%s'(%d) - '%s'(%d)", 
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		index, decodeStorageClass(index),
		fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS));
      ERROR("GetAddressInRegister", error_buffer, FATAL);
    }
  }
  return TReg;
} /* getAddressInRegister */




/* loads the address into which a character function writes. */
int getFunctionAddressInReg(int index)
  // int index;
{
  int TReg, BReg, AReg;

  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE) == TYPE_CHARACTER &&
      fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS)
		& OC_IS_EXECUTABLE &&
      fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS) & SC_FUNCTION)
    {
      AReg = getConstantInRegFromInt(
		fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset));
      BReg = aiStackBase();
      TReg = TempReg(AReg, BReg, iADD, TYPE_CHARACTER);
      fst_my_PutFieldByIndex(ft_SymTable, TReg, SYMTAB_CHAR_LENGTH,
		fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH));
      fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_addressReg,  TReg);
      generate(0, iADD, AReg, BReg, TReg, "Stack base");
    }
  else {
    (void) sprintf(error_buffer, 
		   "Invalid storage class for '%s'(%d) - '%s'(%d)", 
		   (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		   index, decodeStorageClass(index),
		   fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS));
    ERROR("getFunctionAddressInReg", error_buffer, FATAL);
  }

  return(TReg);
} /* getFunctionAddressInReg */
