/* $Id: expr2.C,v 1.2 1997/06/25 15:21:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/gen.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/char.h>
#include <include/frontEnd/astnode.h>
#include <libs/f2i/call.h>

#include <libs/f2i/mnemonics.h>

/* externals and statics */
int *MODS;
int *USES;

extern Boolean f2i_ComputeInterproceduralInfo;

static int NULLSET=0;

/* forward declarations */

#define IN_PLACE 0
#define ON_STACK 1
#define SPECIAL  2




/* adds elements to character lists of registers to be */
/* passed with "JSR" opcodes or the "FRAME" opcode     */
char *add_to_regs_list(char *list_ptr, int reg)
//   char *list_ptr;
//   int  reg;
{
  char buffer[32];

  (void) sprintf(buffer, "r%d ", fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_REG));
  (void) strcat(list_ptr, buffer);

  return list_ptr + strlen(buffer); 
} /* add_to_regs_list */




/* adds elements to character lists of tags to be */
/* passed with "JSR" opcodes or the "FRAME" opcode  */
char *add_to_tags_list(char *list_ptr, int reg)
//   char *list_ptr;
//   int reg;
{
  char buffer[32];

 if (fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_NUM_DIMS) == 0)  /*  scalar  */
    (void) sprintf(buffer, "@%s ", fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_NAME));
 else                                                                 /*  array   */
    (void) sprintf(buffer, "@*%s ", fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_NAME));

  (void) strcat(list_ptr, buffer);

  return list_ptr + strlen(buffer);
} /* add_to_tags_list */



/* Given a call template, this routine     */
/* generates a subroutine or function call */
void GenerateCall( struct CallTemplate *ct )
  // struct CallTemplate *ct;
{
  struct CallTemplate t;
  int parm, i, j, k, l, type;
  int i_type, i_object_class, i_storage_class;
  int reg;
  int ModAvailable=1, UseAvailable=1;
  char code_name[64];
  char parms[512], refs[4096], mods[4096];
  char *parms_ptr, *refs_ptr, *mods_ptr;

  int  ARegs[MAXPARMS];
  int  State[MAXPARMS];
  int  DRegs[MAXPARMS];
  int  addrIndex;
  int  loadIndex;

  t = *ct;

  if (aiDebug > 0)
     (void) fprintf(stdout, "GenerateCall( %d ).\n", t);

  if (t.NumParms > MAXPARMS)
  {
    ERROR("GenerateCall", "Defined constant MAXPARMS exceeded", WARNING);
    ERROR("GenerateCall", "Must change definition in 'call.h'", WARNING);
    ERROR("GenerateCall", "And then recompile all of 'ai'",     FATAL);
  }
  
  /* as the first step, we need to ensure that any register-bound global */
  /* variables that are either USEd (REF'ed) or MODified are restored to */
  /* their real memory locations.  We also need to create the "refs" and */
  /* "mods" list to pass to the optimizer.				 */
  if (t.CallSite != AST_NIL)
  {
    MODS = (int*) aiGlobalMods(t.CallSite);
    USES = (int*) aiGlobalUses(t.CallSite);

    ModAvailable = f2i_ComputeInterproceduralInfo;
    UseAvailable = f2i_ComputeInterproceduralInfo;
  }
  else
  {
    MODS = &NULLSET;
    USES = &NULLSET;
  }

  /* interprocedural MOD information */
  mods[0] = '\0';
  mods_ptr = mods;
  for (i=1; i<=MODS[0]; i++)
  {
    j = MODS[i];
    if(ModAvailable) 
      mods_ptr = add_to_tags_list(mods_ptr, j);
    if (!(fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_OBJECT_CLASS) & OC_IS_FORMAL_PAR)
	&& fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_STORAGE_CLASS) == 
                                                (SC_GLOBAL | SC_NO_MEMORY))
    {
      fst_my_PutFieldByIndex(ft_SymTable, j, SYMTAB_saved,  (int) t.CallSite);
      generate_store(getAddressInRegister(j), j, fst_my_GetFieldByIndex(ft_SymTable, 
									j, SYMTAB_TYPE),j,"&unknown");
    }
  }

  /* add modified parameters to the list of interprocedural mods */
  for (parm=1; parm<=t.NumParms; parm++)
  {
    if (ModAvailable && (t.ActualMods[parm]) &&
	(gen_get_node_type(t.Actuals[parm]) == GEN_IDENTIFIER))
    mods_ptr = add_to_tags_list(mods_ptr, getIndex(t.Actuals[parm]));
  }
  
  /* complete the list of interprocedural mods */
  if ((mods_ptr == mods) && (!ModAvailable))
  {
    mods[0] = '?';
    mods[1] = '\0';
  }
  
  /* interprocedural USE (REF) information */
  refs[0] = '\0';
  refs_ptr = refs;
  for (i=1; i<=USES[0]; i++)
  {
    j = USES[i];
    if (UseAvailable)
      refs_ptr = add_to_tags_list(refs_ptr, j);
    if (fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_saved) != (int) t.CallSite &&
	(!fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_OBJECT_CLASS) & OC_IS_FORMAL_PAR)
	&& fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_STORAGE_CLASS) == 
	                                          (SC_GLOBAL | SC_NO_MEMORY))
    {
      fst_my_PutFieldByIndex(ft_SymTable, j, SYMTAB_saved,  (int) t.CallSite);
      generate_store(getAddressInRegister(j), j, fst_my_GetFieldByIndex(ft_SymTable,
									j, SYMTAB_TYPE),j,"&unknown");
    }
  }
  
  /* add referenced parameters to the list of interprocedural refs */
  for (parm=1; parm<=t.NumParms; parm++)
  {
    if (UseAvailable && (t.ActualUses[parm]) &&
	(gen_get_node_type(t.Actuals[parm]) == GEN_IDENTIFIER))
    refs_ptr = add_to_tags_list(refs_ptr, getIndex(t.Actuals[parm]));
  }

  /* complete the list of interprocedural refs */
  if ((refs_ptr == refs) && (!UseAvailable))
  {
    refs[0] = '?';
    refs[1] = '\0';
  }
  
  /* first, find all the address registers, load them, and fill in ARegs */
  for (parm=1; parm<=t.NumParms; parm++)
  {
    type = gen_get_node_type(t.Actuals[parm]);
    switch(type)
    {
      case GEN_IDENTIFIER:
	i = getIndex(t.Actuals[parm]);
	i_type = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE);
	i_object_class = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS);
	i_storage_class = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS);
	if (i_object_class & OC_IS_FORMAL_PAR)
	{
	  if ((i_storage_class & SC_NO_MEMORY) && (i_object_class & OC_IS_DATA) &&
	      (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NUM_DIMS) == 0))
	  { /* must generate a store to its original home */
	    /* We know it has a live addressReg, cuz its a parameter! */
	    generate_store(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_addressReg),
			   i, i_type,i,"&unknown");
	  }
	  State[parm] = IN_PLACE;
	}
	else if (i_storage_class & SC_NO_MEMORY && i_storage_class != SC_NO_MEMORY)
	{ /* a multivalued variable - put it back in place */
	  j = getAddressInRegister(i);
	  generate_store(j, i, i_type,i,"&unknown");
	  State[parm] = IN_PLACE;
	}

	else if (i_storage_class & SC_NO_MEMORY || t.ActualTypes[parm] != i_type)
	   State[parm] = ON_STACK;
	else
	   State[parm] = IN_PLACE;
	break;

      case GEN_CONSTANT:
	i = getIndex(t.Actuals[parm]);
	i_type = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE);
	if (i_type == TYPE_INTEGER     ||         /* have no space in ram */
           (i_type == TYPE_LOGICAL)    ||
	    t.ActualMods[parm] == TRUE ||         /* or, need a copy,     */ 
	    t.ActualTypes[parm] != i_type)        /* a conversion	   */
	   State[parm] = ON_STACK;
	else
	   State[parm] = IN_PLACE;	   
	break;

      case GEN_COMPLEX_CONSTANT:	/* this is really conservative	*/
	State[parm] = ON_STACK;		/* but, it avoids manipulating 	*/
	break;				/* the whole subtree ...	*/

      case GEN_SUBSCRIPT:
	ARegs[parm] = getSubscriptLValue( t.Actuals[parm] ); 
	State[parm] = SPECIAL;
	break;

      case GEN_SUBSTRING:
	ARegs[parm] = getSubstringAddress( t.Actuals[parm] );
	State[parm] = SPECIAL;
	break;

      case GEN_BINARY_CONCAT:
	/* copy the entire expression onto the stack    */
	/* nest a block so I can create some local vars */
	{
	  struct CharDesc Target[2],Source[MAX_CONCATS];
	  int size = NewStringLength(t.Actuals[parm]);
	  int dummy = 0;

	  /* fill in target by hand */
	  createStackTarget(Target,size);

	  evalCharExpr(t.Actuals[parm],Source,&dummy,MAX_CONCATS);
	  generate_move_string(Target,Source);

	  ARegs[parm] = Target[0].addr;
	  State[parm] = SPECIAL;
	}
	break;

      case AST_NULL_NODE:
	if (t.ActualReg[parm] != 0) /* an arg without an AST */
	  DRegs[parm] = t.ActualReg[parm];
	else /* a dummy argument */
	  DRegs[parm] = StrTempReg(t.name, parm, t.ActualTypes[parm]);
	  State[parm] = ON_STACK;
	break;

      default:	/* case for a general expression */
	State[parm] = ON_STACK;
	break;
    }

    if (State[parm] == IN_PLACE)	/* leave it where it is! */
    { 
      ARegs[parm] = getAddressInRegister(i);
    }
    else if (State[parm] == ON_STACK)
    {
      /* need to get the expression into a register, allocate	*/
      /* it some space on the run-time stack, and copy it 	*/
      /* down there 						*/

      if (t.ActualReg[parm])
	 i = t.ActualReg[parm];
      else if (t.Actuals[parm] != ast_null_node)
	 i = getExprInReg(t.Actuals[parm]);
      else
	 i = DRegs[parm];

      if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != t.ActualTypes[parm])
	 i = getConversion(i, t.ActualTypes[parm]);

      switch(type)
      {
	case GEN_IDENTIFIER:	/* must be SC_NO_MEMORY => scalar	*/
	  j = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS);
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_STACK);
	  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_offset) == DONT_KNOW)
	  {
	    if (aiAlignDoubles && is_double(fst_my_GetFieldByIndex(ft_SymTable, i,
								   SYMTAB_TYPE)))
	      aiNextStack = PadToAlignment(aiNextStack,8);
	    fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset, aiNextStack);
	    aiNextStack += fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE);
	  }
	  ARegs[parm] = getAddressInRegister(i);
	  generate_store(ARegs[parm], i, fst_my_GetFieldByIndex(ft_SymTable, i,
							        SYMTAB_TYPE),i,"&unknown");
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  j);
	  break;

	case GEN_CONSTANT:	/* either integer or Modified */
	case GEN_COMPLEX_CONSTANT:
	  j = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS);
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_STACK);
	  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != TYPE_INTEGER)
	  {
	    k = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_offset);
	  }

	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset, aiNextStack);
	  aiNextStack += fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE);

	  ARegs[parm] = getAddressInRegister(i);
	  if (t.ActualUses[parm] == TRUE)
	    generate_store(ARegs[parm], i, 
			  fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE),i,"&unknown");
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  j);
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  k);
	  break;

	case AST_NULL_NODE:
	  j = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS);
	  /* if it's a parameter, then we don't do anything */
	  /* this probably means its a character length     */
	  if (!(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) &
	                                                      OC_IS_FORMAL_PAR))
	    {
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS, SC_STACK);
	      if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_offset) == DONT_KNOW)
	        {
	  	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset, aiNextStack);
		  aiNextStack += SizeOfType(fst_my_GetFieldByIndex(ft_SymTable,
		  						   i, SYMTAB_TYPE));
		}
		ARegs[parm] = getAddressInRegister(i);
		 fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  j);
	     }
	  break;	/* dummy parameter, so no STORE needed */
	  
	default: /* INVARIANT: expression is already in R[i] */
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset, aiNextStack);
	  aiNextStack += VarSize(i);
	  j = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS);
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_STACK);
	  ARegs[parm] = getAddressInRegister( i );
	  if (t.ActualUses[parm] == TRUE)
	     generate_store(ARegs[parm], i, fst_my_GetFieldByIndex(ft_SymTable, i,
								   SYMTAB_TYPE),i,"&unknown");
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  j);
	  break;
      }
    }
  }

  /* create a list of parameters */
  parms[0] = '\0';
  parms_ptr = parms;

  /* if this is a character function, the character address */
  /* and the desired return length are parameters           */
  if (t.ReturnReg != 0 && t.ActualTypes[0] == TYPE_CHARACTER)
  {
    /* allocate stack space and push address */
    i = fst_my_QueryIndex(ft_SymTable, t.name);
    fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset, aiNextStack);
    aiNextStack += Align(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_CHAR_LENGTH));
    parms_ptr = add_to_regs_list(parms_ptr, getFunctionAddressInReg(i));
    parms_ptr = add_to_regs_list(parms_ptr, getStringLengthIntoReg(t.CallSite));
  }

  /* now, add the appropriate parameters to the parameter list */
  for (parm=1; parm<=t.NumParms; parm++)
      parms_ptr = add_to_regs_list(parms_ptr, ARegs[parm]);

  /* push all character lengths onto stack */
  for (parm=1; parm<=t.NumParms; parm++)
  {
    if (t.ActualTypes[parm] == TYPE_CHARACTER)
      parms_ptr = add_to_regs_list(parms_ptr,
		getStringLengthIntoReg(t.Actuals[parm]));
  }

  /* complete the list of parameters */
  if (parms_ptr == parms) parms[0] = '\0';

  /* load up the code and data pointers, and go! */
  i = fst_my_QueryIndex(ft_SymTable, t.name);		/* is it a parameter?	*/
  if (i != SYM_INVALID_INDEX &&
      fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_FORMAL_PAR)
  {
    if (aiRt > 0)	/* function-valued parameters are c-b-r */
    { 
      l = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_addressReg);
      j = StrTempReg("!", l, 
		     fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE));
      generate_load(j, l, TYPE_INTEGER, i, "&unknown");
      reg = j;
    }
    else                /* function-valued parameters are c-b-v */
      reg = getAddressRegister(i);

    /* generate the branch to the function or subroutine */
    if (t.ReturnReg != 0)
     switch(fst_my_GetFieldByIndex(ft_SymTable, t.ReturnReg, SYMTAB_TYPE))
     {
      case TYPE_CHARACTER:
        /* ignore the "return" of a character function */
        generate_long(0, JSRr, reg, aiStackBase(), (Generic) parms,
		(Generic) refs, (Generic) mods, 0, 0, "jump to character function");
	break;

      case TYPE_LOGICAL:
      case TYPE_INTEGER:
	generate_long(0, iJSRr, reg, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, 0, "jump to logical/integer function");
	break;

      case TYPE_REAL:
	generate_long(0, fJSRr, reg, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, 0,  "jump to real function");
	break;

      case TYPE_DOUBLE_PRECISION:
	generate_long(0, dJSRr, reg, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, 0, "jump to double precision function");
	break;

      case TYPE_COMPLEX:
	generate_long(0, cJSRr, reg, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, 0, "jump to complex function");
	break;

      case TYPE_DOUBLE_COMPLEX:
	generate_long(0, qJSRr, reg, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, 0, "jump to double complex function");
	break;

      default:
        ERROR("GenerateCall", "unknown function type", FATAL);
	break;
     }
    else
      generate_long(0, JSRr, reg, aiStackBase(), (Generic) parms,
		(Generic) refs, (Generic) mods, 0, 0, "jump to subroutine");

  }
  else	/* the normal case */
  {
     if (t.InLibrary == TRUE)
     {
       if (aiSparc > 0) /* naming conventions differ */
	 (void) sprintf(code_name, "_%s",  t.name);
       else if (aiRt > 0)
	 (void) sprintf(code_name, "_.%s",   t.name);
       else
	 (void) strcpy(code_name, t.name);
     }
     else
     {
       if (aiSparc > 0) /* naming conventions differ */
	 (void) sprintf(code_name, "_%s_", t.name);
       else if (aiRt > 0)
	 (void) sprintf(code_name, "_.%s_",  t.name);
       else
	 (void) strcpy(code_name, t.name);
     }
     
    /* generate the branch to the function or subroutine */
    if (t.ReturnReg != 0)
     switch(fst_my_GetFieldByIndex(ft_SymTable, t.ReturnReg, SYMTAB_TYPE))
     {
      case TYPE_CHARACTER:
        /* ignore the "return" of a character function */
        generate_long(0, JSRl, (Generic) code_name, aiStackBase(), (Generic) parms,
		(Generic) refs, (Generic) mods, 0, GEN_STRING, "jump to character function");
	break;

      case TYPE_LOGICAL:
      case TYPE_INTEGER:
	generate_long(0, iJSRl, (Generic) code_name, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, GEN_STRING, "jump to logical/integer function");
	break;

      case TYPE_REAL:
	generate_long(0, fJSRl, (Generic) code_name, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, GEN_STRING,  "jump to real function");
	break;

      case TYPE_DOUBLE_PRECISION:
	generate_long(0, dJSRl, (Generic) code_name, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, GEN_STRING, "jump to double precision function");
	break;

      case TYPE_COMPLEX:
	generate_long(0, cJSRl, (Generic) code_name, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, GEN_STRING, "jump to complex function");
	break;

      case TYPE_DOUBLE_COMPLEX:
	generate_long(0, qJSRl, (Generic) code_name, aiStackBase(), (Generic) parms, t.ReturnReg,
		(Generic) refs, (Generic) mods, GEN_STRING, "jump to double complex function");
	break;

      default:
        ERROR("GenerateCall", "unknown function type", FATAL);
	break;
     }
    else
      generate_long(0, JSRl, (Generic) code_name, aiStackBase(), (Generic) parms,
		(Generic) refs, (Generic) mods, 0, GEN_STRING, "jump to subroutine");

   }

  /* and now, restore any registers that require it! 	*/
  /* First, parameters that are SC_NO_MEMORY 		*/     
  for (parm=1; parm <= t.NumParms; parm ++)
  {
      if (t.Actuals[parm] == ast_null_node) 
      {
	if (t.ActualMods[parm] == TRUE)
	{
	  type =  fst_my_GetFieldByIndex(ft_SymTable, DRegs[parm], SYMTAB_TYPE);
	  j = StrTempReg("!", ARegs[parm], type);
	  generate_load(j, ARegs[parm], type, DRegs[parm], "&unknown"); /* not sure about this tag */
	  generate_move(DRegs[parm], j, type);	  
	  t.Actuals[parm] = (AST_INDEX) DRegs[parm];
	}
      }

      else if (gen_get_node_type(t.Actuals[parm]) == GEN_IDENTIFIER)
      { /* we have an identifier node to look at */
	i = getIndex(t.Actuals[parm]);
        if ((State[parm] == ON_STACK || 
	     (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & 
		(OC_IS_FORMAL_PAR | OC_IS_DATA) &&
	     fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NUM_DIMS) == 0)) &&
	  t.ActualMods[parm] == TRUE)
        {
	  i = getIndex(t.Actuals[parm]);
	  i_type = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE);
 	  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS)
		& SC_NO_MEMORY)
	  { /* need to generate a restore! */
	    j = StrTempReg("!", ARegs[parm], i_type);
	    generate_load(j, ARegs[parm], i_type, i, "&unknown"); 
	    if (i_type != t.ActualTypes[parm])
	    {
	      j = getConversion(j, i_type);
	    }
	    generate_move(i, j, i_type);	  
	  }
        }
      }
  }
  
 /* Next, restore any global variables that may have been modified  */
 /* as a side effect of the call.  Note that these values were all  */
 /* saved prior to the call, since the pre-call value makes it here */
 /* if the side effect doesn't really happen.  (Recall that MOD is  */
 /* flow-insensitive!)						    */
  for (i=1; i<=MODS[0]; i++)
  {
    j = MODS[i];
    if (!(fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_OBJECT_CLASS) & OC_IS_FORMAL_PAR)
	&& fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_STORAGE_CLASS) == 
	                                                 (SC_GLOBAL | SC_NO_MEMORY))
    {
      type = fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_TYPE);
      addrIndex = getAddressInRegister(j);
      loadIndex = StrTempReg("!", addrIndex, type);
      generate_load(loadIndex, addrIndex, type, j, "&unknown");
      generate_move(j, loadIndex, type);
    }
  }

  return;
} /* GenerateCall */

