/* $Id: utility.C,v 1.2 1998/04/29 13:00:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <libs/f2i/mnemonics.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/classes.h>

/* UTILITY Routines
 *
 *	void 	ERROR();
 *	void 	UNIMPL();
 *
 *	int	ATypeToIType();
 *	int	ITypeToAType();
 *	char	*TypeName();
 *	int	SizeOfType();
 *
 *	char	*get_num();
 *
 *	int	TempReg();
 *	int	StrTempReg();
 *
 *	char	*ConstantName();
 *	char	*NameFromConstantNode();
 *
 */

/* forward declarations */
static int TR(char *,int);


/* the standard ai ERROR reporting routines */
void ERROR(char *proc, char *msg, int flag)

//   char *proc,    /* name of procedure detecting the error */
//        *msg;     /* text for the error messge             */
//   int  flag;     /* flag indicating severity of the error */
{
  /* print name of calling procedure if specified by aiMessageId */
  if (aiMessageId > 0)
     (void) fprintf(stderr, "%s: ", proc);

  /* print error message and severity.  Note that aiStmtCount == 0 */
  /* indicates that the error message arose in the prepass         */
  if (flag == WARNING && aiStmtCount == 0)
    (void) fprintf(stderr, "Prepass: %s (WARNING).\n", msg);

  else if (flag == SERIOUS && aiStmtCount == 0)
    (void) fprintf(stderr, "Prepass: %s (SERIOUS).\n", msg);

  else if (flag == FATAL && aiStmtCount == 0)
  {
    (void) fprintf(stderr, "Prepass: %s (FATAL).\n", msg);
    if (aiFatals)
	 exit(1);
  }

  /* print error message and severity for errors not in the prepass */
  else if (flag == WARNING)
    (void) fprintf(stderr, "Stmt %d: %s (WARNING).\n", aiStmtCount, msg);

  else if (flag == SERIOUS)
    (void) fprintf(stderr, "Stmt %d: %s (SERIOUS).\n", aiStmtCount, msg);

  else if (flag == FATAL)
  {
    (void) fprintf(stderr, "Stmt %d: %s (FATAL).\n", aiStmtCount, msg);
    if (aiFatals)
	 exit(1);
  }
} /* ERROR */



/* error message indicating that a FORTRAN feature is not supported in a2i */
void UNIMPL(int type)
//  int type;
{
  (void) fprintf(stderr, "Subtree of type %s encountered.\n", 
	  ast_get_node_type_name(type) );
} /* UNIMPL */


/* a simple routine to pick an integer out of a string */
char *get_num( char *char_ptr, int *num_ptr )
//   register char *char_ptr;   /* pointer to string containing number */
//   register int	*num_ptr;    /* pointer to the value of the integer */
{
  register int sign, num;

  /* skip leading blanks */
  while (*char_ptr == ' ')
     char_ptr++;

  /* record the sign of the integer */
  if (*char_ptr != '-')
     sign = 1;
  else
  {
    sign = -1;
    char_ptr++;
  }

  /* compute the value of the integer */
  num = 0;
  while(*char_ptr >= '0' && *char_ptr <= '9')
       num = num * 10 + (*char_ptr++ - '0');

  /* correct the sign */
  if (sign == -1)
     num = -num;

  *num_ptr = num;
  return char_ptr;

}



/* routine for converting to iloc types.  Routine is largely   */
/* Unnecessary now that AST types and iloc types are the same. */
/* After all code is converted, this routine will be deleted.  */
int ATypeToIType( int ast_type )
//   register int ast_type;
{
  switch(ast_type)
  {
    case TYPE_CHARACTER:
    case TYPE_LOGICAL:
    case TYPE_INTEGER:
    case TYPE_LABEL:
    case TYPE_REAL:
    case TYPE_COMPLEX:
    case TYPE_DOUBLE_PRECISION:
    case TYPE_UNKNOWN:
    case TYPE_NONE:
    break;
    
    default:
	(void) sprintf(error_buffer, "Unknown AST type '%d'", ast_type);
	ERROR("ATypeToIType", error_buffer, FATAL);
	break;
  }
  return ast_type;
} /* ATypeToIType */



/* routine for converting to AST TYPES.  Routine is largely    */
/* unnecessary now that iloc types and AST types are the same. */
int ITypeToAType( int iloc_type )
//   register int iloc_type;
{
  register int value;

  switch(iloc_type)
  {
    case TYPE_CHARACTER:
    case TYPE_LOGICAL:
    case TYPE_INTEGER:
    case TYPE_LABEL:
    case TYPE_REAL:
    case TYPE_COMPLEX:
    case TYPE_DOUBLE_PRECISION:
    case TYPE_UNKNOWN:
	  value = iloc_type;
    break;
    
    case TYPE_DOUBLE_COMPLEX:
      value = TYPE_UNKNOWN;
    break;
    
    default:
	  (void) sprintf(error_buffer, "Unknown iloc type '%d'", iloc_type);
	  ERROR("ITypeToAType", error_buffer, FATAL);
	break;
  }
  return value;
} /* ITypeToAType */



/* return the character string representation of the type name */
static char *TNames[] = {"character", "logical", "integer", "label",
			"real", "complex", "double precision", 
			"double complex", "unknown" };

char *TypeName( int type )
//  int type;
{
  int value;
  switch(type)
  {
    case TYPE_CHARACTER:	value = 0;	break;
    case TYPE_LOGICAL:		value = 1;	break;
    case TYPE_INTEGER:		value = 2; 	break;
    case TYPE_LABEL:		value = 3;	break;
    case TYPE_REAL:		value = 4; 	break;
    case TYPE_COMPLEX:		value = 5; 	break;
    case TYPE_DOUBLE_PRECISION:	value = 6; 	break;
    case TYPE_DOUBLE_COMPLEX:	value = 7;	break;
    default:			value = 8;	break;
  };
  return TNames[value];
} /* TypeName */


/* returns number of bytes used by a given type */
int SizeOfType( int type )
  //  int type;
{
  register int value;
  switch(type)
  {
    case TYPE_CHARACTER:	value = 4;	break; /* pointer to string */
    case TYPE_LOGICAL:	
    case TYPE_INTEGER:	
    case TYPE_LABEL:	
    case TYPE_REAL:
    case TYPE_COMPLEX:	
    case TYPE_DOUBLE_PRECISION:
    case TYPE_DOUBLE_COMPLEX:
	value = GetDataSize(type);
	break;
    case TYPE_UNKNOWN:	
	value = 16;	break; /* worst case */
	break;
    default:			value = DK;	break;
  };
  return value;
} /* SizeOfType */


/* Allocating Registers */
static   char s[64];

/* internal routine for creating a temporary register from a name and type */
static int TR( char *name, int type )

//   char *name;    /* name of the register      */
//   int type;      /* type of register required */
{
  register int index;

  index = SymInsertSymbol(name, type, OC_IS_DATA, 0, DK, DK);

  return index;
}



/* create a temporary register to hold the expression r1 op r2 */
int TempReg( int r1, int r2, int op, int type)

//   int r1,      /* first register  */
//       r2,      /* second register */
//       op,      /* operand         */
//       type;    /* result type     */
{
  register int index;

  /* put expression in canonical order */
  if ((r1 > r2) && iloc_op_commutes(op))
  {
    index = r1; r1 = r2; r2 = index;
  }

  /* create string representation of the typed expression */
  (void) sprintf(s, "%d<%d>%d{%d}", r1, r2, op, type);

  /* create temporary register for the expression */
  index = TR(s, type);

  if (aiDebug > 0)
     (void) fprintf(stdout, "TempReg(%d, %d, %d, %s) => %s (%d).\n",
	     r1, r2, op, TypeName(type), s, index);
  return index;
} /* TempReg */


/* create a temporary register for an item represented by a */
/* character string, str, and a number, num.                */
int StrTempReg( char *str, int num, int type )

//   char *str;      /* descriptive character string */
//   int	num,      /* representative number        */
//         type;     /* type of the item             */
{
  register int index;

  /* create name for the temporary register */
  (void) sprintf(s, "%s[%d]{%d}", str, num, type);

  /* create temporary register for the item */
  index = TR(s, type);

  if (aiDebug > 0)
     (void) fprintf(stdout, "StrTempReg(%s, %d, %s) => %s (%d).\n",
	     str, num, TypeName(type), s, index);

  return index;
} /* StrTempReg */


/* returns internal name used for the constant */
char *ConstantName(char *name, int type, char *buffer)

//   char *name;      /* value of the constant */
//   int  type;       /* type of constant      */
//   char *buffer;    /* buffer to hold name   */
{
  (void) sprintf(buffer, "%s[%d]", name, type);
  return buffer;
} /* ConstantName */




/* returns text for constant given a symbol table index */
char *ConstantNameFromIndex(int index)
  // int index;
{
  char name[32];
  int  type;

  (void) sscanf((char*)fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME), "%s[%d]", name, &type);

  return name;
} /* ConstantNameFromIndex */


/* retrieve the name of the constant from an AST node */
char *NameFromConstantNode( AST_INDEX node, char *buffer )

//   AST_INDEX node;    /* AST node containing constant */
//   char *buffer;      /* buffer to hold constant name */
{
  int type = gen_get_node_type(node);
  char realPart[32], imagPart[32];

  switch(type)
  {
    case GEN_CONSTANT:
	(void) strcpy(buffer, gen_get_text(node));
	break;
	
    case GEN_COMPLEX_CONSTANT:
	(void) NameFromConstantNode(gen_COMPLEX_CONSTANT_get_real_const(node), 
				realPart);
	(void) NameFromConstantNode(gen_COMPLEX_CONSTANT_get_imag_const(node),
				imagPart);
	(void) sprintf(buffer, "(%s,%s)", realPart, imagPart);
	break;

    case GEN_UNARY_MINUS:
	(void) strcpy(buffer, "-");
	node = gen_UNARY_MINUS_get_rvalue(node);
	if (gen_get_node_type(node) != GEN_CONSTANT)
	   ERROR("NameFromConstantNode",
		 "UNARY MINUS has non-CONSTANT son where CONSTANT expected",
		 FATAL);

	(void) strcat(buffer, gen_get_text(node));
	break;

    default:
	ERROR("NameFromConstantNode", "Unexpected node type", FATAL);
	break;
  }
  return buffer;
} /* NameFromConstantNode */

