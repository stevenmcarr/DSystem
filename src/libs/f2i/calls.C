/* $Id: calls.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <include/frontEnd/astnode.h>
#include <libs/f2i/call.h>
#include <libs/f2i/char.h>
#include <libs/f2i/mnemonics.h>
#include <libs/support/strings/rn_string.h>



static void WalkTree(AST_INDEX);


/* complete call site template and call GenerateCall */
/* to produce iloc for a procedure call              */
void HandleCall ( AST_INDEX	invocation )

  // AST_INDEX	invocation;
{
    AST_INDEX	list,
		name,
		parm_list;
    int		maxparms,
		index, 
		ctr,
		i;


    struct CallTemplate ct;

  /*  print debugging error message  */
    if (aiDebug > 0)
      (void) fprintf(stdout, "HandleCall( %d ).\n", invocation);

  /*  some tree maneuvering */
    name      = gen_INVOCATION_get_name(invocation);
    parm_list = gen_INVOCATION_get_actual_arg_LIST(invocation);

  /*  count the parameters in the parameter list  */
    list = list_first(parm_list);
    maxparms = 0;

    while (list != ast_null_node)
      {
	maxparms++;
	list = list_next(list);
      }

  /*  allocate the call template */
     ct.name 	  = ssave(gen_get_text(name));
     ct.CallSite  = invocation;
     ct.InLibrary = 0;
     ct.NumParms  = maxparms;

     ct.Actuals     = (AST_INDEX *) 
		      get_mem(sizeof(AST_INDEX) *maxparms+1, "CT.Actuals");
     ct.ActualTypes = (int *) get_mem(sizeof(int) * maxparms+1, "CT.ActualTypes");
     ct.ActualMods  = (int *) get_mem(sizeof(int) * maxparms+1, "CT.ActualMods");
     ct.ActualUses  = (int *) get_mem(sizeof(int) * maxparms+1, "CT.ActualUses");
     ct.ActualReg   = (int *) get_mem(sizeof(int) * maxparms+1, "CT.ActualReg" );

     index = getIndex(name);
     if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS) & SC_FUNCTION
	 || index == aiFunctionValueIndex())
     {
       ct.ReturnReg = index;
       ct.ActualTypes[0] = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE);
     }
     else 
        ct.ReturnReg = 0;


  /*  evaluate parameters in the parameter list  */
    list = list_first(parm_list);
    ctr  = 1;

    while(list != ast_null_node)
    {
	ct.Actuals[ctr]     = list;
	ct.ActualTypes[ctr] = gen_get_converted_type(list);
	ct.ActualReg[ctr]   = 0;
	switch(gen_get_node_type(list))
	{
	  case GEN_IDENTIFIER:
		i = getIndex(list);
		ct.ActualMods[ctr] = aiNameIsMod(ft_SymTable, invocation, i);
	  	ct.ActualUses[ctr] = aiNameIsUse(ft_SymTable, invocation, i);
		break;

	  case GEN_SUBSCRIPT:
		i = getIndex(gen_SUBSCRIPT_get_name(list));
		ct.ActualMods[ctr] = aiNameIsMod(ft_SymTable, invocation, i);
		ct.ActualUses[ctr] = aiNameIsUse(ft_SymTable, invocation, i);
		break;

	  case GEN_SUBSTRING:
		if (gen_get_node_type(gen_SUBSTRING_get_substring_name(list))
		    == GEN_SUBSCRIPT)
		  {
		    i = getIndex
		      (gen_SUBSCRIPT_get_name
		       (gen_SUBSTRING_get_substring_name(list)));
		  }
		else
		  i = getIndex(gen_SUBSTRING_get_substring_name(list));
		ct.ActualMods[ctr] = aiNameIsMod(ft_SymTable, invocation, i);
		ct.ActualUses[ctr] = aiNameIsUse(ft_SymTable, invocation, i);
		break;

	  default:
		ct.ActualMods[ctr] = FALSE; /* can't ask the question here */
		ct.ActualUses[ctr] = TRUE;  /* since there's no name!	   */ 
		break;			    /* not a real big problem	   */
	}
	ctr++;
	list = list_next(list);
      }
    
    /* and pass on the template... */
    GenerateCall( &ct );
    
    /*  clean up the allocated space  */
    free_mem ((void*) ct.Actuals);
    free_mem ((void*) ct.ActualTypes);
    free_mem ((void*) ct.ActualMods);
    free_mem ((void*) ct.ActualUses);
    free_mem ((void*) ct.ActualReg);
    
    sfree(ct.name);
} /* HandleCall */




/* produces iloc for a function invocation.  HandleCall */
/* processes everything except the return value.        */
int HandleInvocation ( AST_INDEX	invocation )

  // AST_INDEX	invocation;
  {
    int		index;
    int		result;
    int		type;
    AST_INDEX   Name;
    char	name[32];

  /*  print debugging error message  */
    if (aiDebug > 0)
      (void) fprintf(stdout, "HandleInvocation( %d ).\n", invocation);

  /*  is this a statment function?  */
    if (fst_my_GetFieldByIndex(ft_SymTable, getIndex
	(gen_INVOCATION_get_name(invocation)), SYMTAB_STORAGE_CLASS)
	& SC_STMT_FUNC)
      return InlineStmtFunc(invocation);


  /*  process the procedure call  */
      HandleCall(invocation);

  /*  insert the function in the table to get the index  */
      Name = gen_INVOCATION_get_name(invocation);
      index = fst_my_QueryIndex(ft_SymTable, gen_get_text(Name));
      if (index == SYM_INVALID_INDEX)
      {
	(void) sprintf(error_buffer, "'%s' not entered in Symbol Table", 
			gen_get_text(Name));
	ERROR("HandleInvocation", error_buffer, FATAL);
      }

  /*  We need to place the result in a unique register.  */
  /*  Otherwise, multiple calls in one expression will   */
  /*  set the same register.				 */
    (void) sprintf (name, "&%s",
	(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
    type = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE);
    result = StrTempReg(name, (int) invocation, type);
    generate_move(result, index, type);

    return result;

  } /* HandleInvocation */




/* process a statement function definition */
void HandleStatementFunction(AST_INDEX	stmt)

  // AST_INDEX	stmt;

{
  AST_INDEX	node;
  int 		function_index;
  int		index;
  int		counter = 0;
  int		scratch;
  char		name[32];

  /*  print debugging error message  */
  if (aiDebug > 0)
    (void) fprintf(stdout, "HandleStatementFunction( %d ).\n", stmt);

  /*  add the statement function to the symbol table  */
  node = gen_STMT_FUNCTION_get_name(stmt);

  function_index = SymInsertSymbol(gen_get_text(node), gen_get_real_type(node), 
			  OC_IS_EXECUTABLE,
			  0, SC_STMT_FUNC, FALSE);

  fst_my_PutFieldByIndex(ft_SymTable, function_index, SYMTAB_offset,  
		   (int) gen_STMT_FUNCTION_get_rvalue(stmt));

  /*  zero the scratch field in the symbol table  */
  fst_ForAll (ft_SymTable, SymZeroScratchField, 0);

  /*  walk the parameter list and rename the parameters  */
  node = list_first(gen_STMT_FUNCTION_get_formal_arg_LIST(stmt));

  while (node != ast_null_node)
    {
      (void) sprintf(name, "$%s[%d]",
	(char *) fst_my_GetFieldByIndex(ft_SymTable, function_index, SYMTAB_NAME),
	counter++); 
      index = getIndex(node);
      fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_scratch,  
		SymInsertSymbol(name, gen_get_real_type(node), 
				OC_IS_DATA, 0, SC_NO_MEMORY, FALSE));
				
      /* copy string lengths and mark it as       */
      /* "parameter" to fool getAddressInRegister */
      if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE)
		& TYPE_CHARACTER)
      {
        scratch = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_scratch);
	fst_my_PutFieldByIndex(ft_SymTable, scratch, SYMTAB_CHAR_LENGTH,  
            fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH));

        fst_my_PutFieldByIndex(ft_SymTable, scratch, SYMTAB_OBJECT_CLASS,
            fst_my_GetFieldByIndex(ft_SymTable, scratch, SYMTAB_OBJECT_CLASS)
	    | OC_IS_FORMAL_PAR); 

	fst_my_PutFieldByIndex(ft_SymTable, scratch, SYMTAB_addressReg,
	    scratch);
      }
      node = list_next(node);
    }

  /*  save the number of parameters  */
  fst_my_PutFieldByIndex(ft_SymTable, function_index, SYMTAB_addressReg,  -counter);

  /*  change the names in the expression  */
  WalkTree(gen_STMT_FUNCTION_get_rvalue(stmt));
  
  return;
} /* HandleStatementFunction */




/* walk espression in a statement function */
/* and change the names in the expression  */
static void WalkTree(AST_INDEX	node)
  //   AST_INDEX node;
{
   register int i, n, type, index;

   /*  print debugging error message  */
   if (aiDebug > 0)
     (void) fprintf(stdout, "WalkTree( %d ).\n", node);


   if (node == ast_null_node) return;

   type = gen_get_node_type(node);

   if (is_list(node))
   {
     node = list_first(node);
     while(node != ast_null_node)
     {
       WalkTree(node);
       node = list_next(node);
     }
   }
   else
   {
     if (type == GEN_IDENTIFIER)
       {
	 index = fst_my_GetFieldByIndex(ft_SymTable, getIndex(node), SYMTAB_scratch);
	 if (index != 0)
	   gen_put_text(node,
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		STR_IDENTIFIER);
       }
     else
       {
	 n = ast_get_node_type_son_count(type);
	 for (i=1; i<=n; i++)
	  WalkTree( ast_get_son_n(node, i) );
       }
   }
} /* WalkTree */




/* generate inline iloc for a statement function */
int InlineStmtFunc(AST_INDEX	node)

  // AST_INDEX	node;

{
  AST_INDEX	list;
  int		counter = 0;
  int		index;
  int		function;
  int		Reg, Reg2;
  int		type;
  char		*symtab_name;
  char 		name[32];
  int           dummy;
  struct CharDesc target[2],source[MAX_CONCATS];

  /*  print debugging error message  */
  if (aiDebug > 0)
    (void) fprintf(stdout, "InlineStmtFunc( %d ).\n", node);
  
  /*  get the function information  */
  function = getIndex(gen_INVOCATION_get_name(node));
  symtab_name = (char *) fst_my_GetFieldByIndex
		(ft_SymTable, function, SYMTAB_NAME);
  
  /*  evaluate parameters in the parameter list  */
  list = list_first
    (gen_INVOCATION_get_actual_arg_LIST(node));
  
  /* merged with InlineCharStmtFunc to handle characters */
  while (list != ast_null_node)
    {
      (void) sprintf(name, "$%s[%d]", symtab_name, counter++);
      index = fst_my_QueryIndex(ft_SymTable, name);
      if (index == SYM_INVALID_INDEX)
	{
	  (void) sprintf(error_buffer, "%s called with too many parameters",
			symtab_name);
	  ERROR("InlineStmtFunc", error_buffer, FATAL);
	}
      if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE) == TYPE_CHARACTER)
      {
	if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH) == STAR_LEN)
	{
	  if (gen_get_node_type(list) == GEN_BINARY_CONCAT)
	  {
	    ERROR("InlineStmtFunc","Character concat has * length",FATAL);
	  }
	  else
	  {
	    /* store address and length */
	    Reg = AddressFromNode(list);
	    generate_move(index, Reg, TYPE_INTEGER);
	    Reg2 = getStringLengthIntoReg(list);
	    Reg = StrTempReg("s-len", index, TYPE_INTEGER);
	    generate_move(Reg, Reg2, TYPE_INTEGER);
	  }
	}
	else
	{
	  createStackTarget(target, fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH));
	  dummy = 0;
	  evalCharExpr(list, source, &dummy, MAX_CONCATS);
	  generate_move_string(target, source);
	  generate_move(index, target[0].addr, TYPE_INTEGER);
	}
      }
      else
      {
	Reg = getExprInReg(list);
	type = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE);
	Reg = getConversion(Reg, type);
	generate_move(index, Reg, type);
      }
      list = list_next(list);
    }
  
  /*  verify that all of the parameters were set  */
  if (-fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_addressReg) > counter)
    {
      (void) sprintf(error_buffer, "%s called with too few parameters", 
		     symtab_name);
      ERROR("InlineStmtFunc", error_buffer, FATAL);
    }
  
  type = fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_TYPE);
  if (type == TYPE_CHARACTER)
  {
    createStackTarget(target,
	fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_CHAR_LENGTH));
    dummy = 0;
    evalCharExpr((AST_INDEX) fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_offset),
	source, &dummy, MAX_CONCATS);
    generate_move_string(target, source);
    index = target[0].addr;
    
    /*  We need to place the result in a unique register.  */
    /*  Otherwise, multiple calls in one expression will   */
    /*  set the same register.				 */
    (void) sprintf (name, "&%s", symtab_name);
    Reg = StrTempReg(name, (int)node, TYPE_INTEGER);
    generate_move(Reg, index, TYPE_INTEGER);
  }
  else
  {
    index = getExprInReg((AST_INDEX)
	(fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_offset)));

    /* place result in unique register */
    (void) sprintf (name, "&%s", symtab_name);
    Reg = StrTempReg(name, (int) node, type);
    generate_move(Reg, index, type);
  }
  return Reg;
} /* InlineStmtFunction */
