/* $Id: data.C,v 1.3 1998/04/29 13:00:23 carr Exp $ */
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

#include <math.h>
/*  move bit_masks[] to data.c and make static  */
static int bit_masks[32] = {	0x00000001, 0x00000002, 0x00000004, 0x00000008,
			0x00000010, 0x00000020, 0x00000040, 0x00000080,
			0x00000100, 0x00000200, 0x00000400, 0x00000800,
			0x00001000, 0x00002000, 0x00004000, 0x00008000,
			0x00010000, 0x00020000, 0x00040000, 0x00080000,
			0x00100000, 0x00200000, 0x00400000, 0x00800000,
			0x01000000, 0x02000000, 0x04000000, 0x08000000,
			0x10000000, 0x20000000, 0x40000000, 0x80000000 };



# define	FALSE	0
# define	TRUE	1



/* structure for data statements  */
static AST_INDEX	DataStmts[MAX_DATA_STMTS];
static int		DataPtr=0;
static int		repeat;
static int		data_type;
static char		*data_elt;
static char		*imag_elt;




/* set symbol table index for the identifier in the */
/* data statement and mark the identifier as STATIC */

void setIndex(AST_INDEX DataName)

  // AST_INDEX	DataName;	/* AST index of identifier */

  {
    int		index;	/* symbol table index of identifier */
    AST_INDEX	list;	/* list of nodes in implied DO */

    /*  set symbol table index from node type and mark identifier as STATIC */
    switch (gen_get_node_type(DataName))
      {
	case GEN_IDENTIFIER:
	  index = getIndex(DataName);
	  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS,
	     fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS) |
	     SC_STATIC);
	break;

	case GEN_SUBSCRIPT:
	  index = getIndex(gen_SUBSCRIPT_get_name(DataName));
	  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS,
	     fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS) |
	     SC_STATIC);
	break;

	case GEN_SUBSTRING:
	  index = getIndex(gen_SUBSTRING_get_substring_name(DataName));
	  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS,
	     fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS) |
	     SC_STATIC);
	break;

	case GEN_IMPLIED_DO:
	  /* walk the implied DO list, setting the symbol table index */
	  /* for each identifier and marking the identifier as STATIC */
	  list = list_first(gen_IMPLIED_DO_get_imp_elt_LIST(DataName));
	  while (list != ast_null_node)
	  {
	    setIndex(list);
	    list = list_next(list);
	  }
	break;

	default:
	  ERROR("setIndex", "Unexpected node type in data statement", FATAL);
      };
  } /* setIndex */




/* returns the length of a character string in bytes */

int string_length(int index)

  //         int index;	/* symbol table index of the character string */
{
        return(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH));

} /* string_length */




/* Returns the the number of bytes required by an item in the symbol table. */
/* For a string, it returns the length of the string in bytes.              */

int SizeOfTypewIndex( int index )
  // int index;
{
  register int value;
  switch(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE))
  {
    case TYPE_CHARACTER:        value = string_length(index);	break;
    case TYPE_LOGICAL:      
    case TYPE_INTEGER:          
    case TYPE_LABEL:           
    case TYPE_REAL:             
    case TYPE_COMPLEX: 
    case TYPE_DOUBLE_PRECISION:
    case TYPE_DOUBLE_COMPLEX: 
       value = GetDataSize(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE));
       break;
    case TYPE_UNKNOWN:     
    default:                    value = DK;     		break;
  };
  return value;
} /* SizeOfTypewIndex */




/* Walks a DATA statement and inserts all identifiers into the symbol */
/* table, marking each identifier as STATIC.  Saves the location of   */
/* the DATA statement in the AST for future use when processing data. */

void HandleData( AST_INDEX	Stmt )

  // AST_INDEX Stmt;	/* AST index of DATA statement */
{
  AST_INDEX 	DataPair;	/* AST index of a DATA pair   */
  AST_INDEX	DataName;	/* AST index of an identifier */


  DataPair = gen_DATA_get_data_elt_LIST(Stmt);
  if (DataPair != ast_null_node)
  {

    /* walk the list of data pairs */
    DataPair = list_first(DataPair);
    while (DataPair != ast_null_node)
    {

      /* walk the list of identifiers */
      DataName = list_first(gen_DATA_ELT_get_data_vars_LIST(DataPair));
      while (DataName != ast_null_node)
      {
	/*  mark each identifier in the data statement as STATIC  */
	setIndex(DataName);

	DataName = list_next(DataName);
      }

      DataPair = list_next(DataPair);
    }
  }

  /*  store the location of the data statement  */
  if (DataPtr < MAX_DATA_STMTS)
    DataStmts[DataPtr++] = Stmt;
  else
    ERROR("HandleData", "Too many data statements", FATAL);
} /* HandleData */




/* Returns the value of an integer expression. */
/* This routine is used throughout a2i.        */

int evalExpr(AST_INDEX	expr)

  // AST_INDEX	expr;	/* AST node associated with expression */
  {
    int i;		/* index used for exponentiation         */
    int value;		/* value of the expression               */
    int answer;		/* result of exponentiation              */
    int product;	/* intermediate result of exponentiation */
    int exponent;	/* exponent used in exponentiation       */
    char name[32];	/* buffer for identifier name            */
    
    /* recursively walk the expression with evalExpr and compute result */
    switch (gen_get_node_type(expr))
    {
      /* return the value of the integer constant */
      case GEN_CONSTANT:
	if (gen_get_real_type(expr) != TYPE_INTEGER)
	{
	  (void) sprintf(error_buffer, 
		  "'%s' is not of type integer in integer expression",
		  gen_get_text(expr));
	  ERROR("EvalExpr", error_buffer, FATAL);
	}
	else
	{
	   (void) get_num(gen_get_text(expr), &value);
	   return value;
	}
	break;


      /* return the current value of the implied do variable from scratch field */
      case GEN_IDENTIFIER:
	name[0] = '\0';
        (void) strcat(name, "_$");
	(void) strcat(name, gen_get_text(expr));
	value = SymInsertSymbol(name, TYPE_INTEGER, OC_IS_DATA, 0, 
				SC_NO_MEMORY, FALSE);

	if (fst_my_GetFieldByIndex(ft_SymTable, value, SYMTAB_temp) != -1)
	{
	  (void) sprintf(error_buffer, "'%s' is not a valid implied do variable",
		  gen_get_text(expr));
	  ERROR("EvalExpr", error_buffer, FATAL);
	}

	if (aiDebug > 0)
	   (void) fprintf(stdout, "\t'%s' evaluates to '%d'.\n", 
		   (char *) fst_my_GetFieldByIndex(ft_SymTable, value, SYMTAB_NAME),
		   fst_my_GetFieldByIndex(ft_SymTable, value, SYMTAB_scratch));

	return fst_my_GetFieldByIndex(ft_SymTable, value, SYMTAB_scratch);


      /* return the sum of the values of the left and right sons */
      case GEN_BINARY_PLUS:
	return	evalExpr(gen_BINARY_PLUS_get_rvalue1(expr)) + 
		evalExpr(gen_BINARY_PLUS_get_rvalue2(expr));


      /* return the difference between the values of the left and right sons */
      case GEN_BINARY_MINUS:
	return	evalExpr(gen_BINARY_MINUS_get_rvalue1(expr)) -
		evalExpr(gen_BINARY_MINUS_get_rvalue2(expr));


      /* return the product of the values of the left and right sons */
      case GEN_BINARY_TIMES:
	return	evalExpr(gen_BINARY_TIMES_get_rvalue1(expr)) *
		evalExpr(gen_BINARY_TIMES_get_rvalue2(expr));


      /* return the result of dividing the value of */
      /* the left son by the value of the right son */
      case GEN_BINARY_DIVIDE:
	return	evalExpr(gen_BINARY_DIVIDE_get_rvalue1(expr)) /
		evalExpr(gen_BINARY_DIVIDE_get_rvalue2(expr));


      /* return the result of raising the value of  */
      /* the left son to the value of the right son */
      case GEN_BINARY_EXPONENT:
	value    = evalExpr(gen_BINARY_EXPONENT_get_rvalue1(expr));
	exponent = evalExpr(gen_BINARY_EXPONENT_get_rvalue2(expr));

        /*  What should be returned for 0**0?  */
        /*  At the moment, we return 0.	       */
	if ((value == 1) || (value == 0))
	  answer = value;

        /* no fractional values allowed */
	else if (exponent < 0)
	  answer = 0;

        /* compute exponent efficiently by using the bit mask of the exponent */
	else
	  {
	    i = 0;
	    answer = 1;
	    product = value;
	    while (bit_masks[i] <= exponent)
	      {
		if (exponent & bit_masks[i++])
		  answer = answer * product;
		product = product * product;
	      }
	  }
	return answer;


      /* return the negated value of the son */
      case GEN_UNARY_MINUS:
	return	- evalExpr(gen_UNARY_MINUS_get_rvalue(expr));


      /* fail due to an invalid node type for an integer expression */
      default:
	ERROR("evalExpr", "Unexpected node type detected in expression", FATAL);
    };
    return 0;
  } /* evalExpr */



static char convert_buffer[100];  /* holds type converted data */




/* converts data string from given input type to required output type */

char *evalConvert(char *data, int dataType, int type)

//   char	*data;		/* string representing input data      */
//   int	dataType;	/* type associated with the input data */
//   int	type;		/* output type required                */

  {
    int		i		= 0;     /* index into convert_buffer[]               */
    int		not_finished	= TRUE;  /* parsing flag                              */
    int		found		= FALSE; /* parsing flag                              */
    float	real;			 /* temporary for real to integer conversion  */
    double	double_precision;	 /* temporary for double to integer conversion*/
    char	*c;			 /* pointer into the data string              */

    /* no conversion is necessary */
    if (type == dataType) return data;

    /* select based on input type */
    switch (dataType)
      {
	case TYPE_INTEGER:
          /* select based on output type */
	  switch (type)
	    {
	      case TYPE_REAL:
		(void) strcpy(convert_buffer, data);
		(void) strcat(convert_buffer, ".0E0");
		return convert_buffer;

	      case TYPE_DOUBLE_PRECISION:
		(void) strcpy(convert_buffer, data);
		(void) strcat(convert_buffer, ".0D0");
		return convert_buffer;

	      default:
		ERROR ("evalConvert",
		       "Invalid integer type conversion in data statement.", FATAL); 
	    }
	break;

	case TYPE_REAL:
	case TYPE_COMPLEX:
          /* select based on output type */
	  switch (type)
	    {
	      case TYPE_INTEGER:
		(void) sscanf (data, "%f", &real);
		(void) sprintf(convert_buffer, "%d", (int) real);
		return convert_buffer;

	      case TYPE_REAL:
		return data;

	      case TYPE_DOUBLE_PRECISION:
		/* change format from real "E" format to double precision "D" format */
		c = data;
		while (not_finished)
		  {
		    if ((c[0]=='E') || (c[0]=='e'))
		      {
			convert_buffer[i++] = 'D';
			found = TRUE;
		      }
		    else if (c[0]=='\0')
		      {
			convert_buffer[i++] = '\0';
			if (!found) (void) strcat(convert_buffer,"D0");
			not_finished = FALSE;
		      }
		    else
			convert_buffer[i++] = c[0];
		    c++;
		  }
	        return convert_buffer;

	      default:
		ERROR ("evalConvert", "Invalid real type conversion in data statement.",
		        FATAL);
	    }
	break;

	case TYPE_DOUBLE_PRECISION:
	  /* select based on output type */
	  switch (type)
	    {
	      case TYPE_INTEGER:
		/*  convert "d" to "e" so that sscanf will work correctly  */
		c = data;
		while ((*c!='d')&&(*c!='D')&&(*c!='\0')) c++;
		if (*c!='\0') *c = 'e';
		(void) sscanf (data, "%lg", &double_precision);
		(void) sprintf(convert_buffer, "%d", (int) double_precision);

		/*  return string to double precision  */
		*c = 'd';
		return convert_buffer;

	      case TYPE_REAL:
		/* change format from double precision "D" format to real "E" format */
		c = data;
		while (not_finished)
		  {
		    if ((c[0]=='D') || (c[0]=='d'))
		      {
			convert_buffer[i++] = 'E';
			found = TRUE;
		      }
		    else if (c[0]=='\0')
		      {
			convert_buffer[i++] = '\0';
			if (!found) (void) strcat(convert_buffer,"E0");
			not_finished = FALSE;
		      }
		    else
			convert_buffer[i++] = c[0];
		    c++;
		  }
	        return convert_buffer;

	      default:
		ERROR ("evalConvert",
		"Invalid double precision type conversion in data statement.", FATAL);
	    }
	break;

        case TYPE_CHARACTER:
		ERROR ("evalConvert",
		"Character type conversions are not allowed in data statements.", FATAL);
	break;

        case TYPE_LOGICAL:
		ERROR ("evalConvert",
		"Logical type conversions are not allowed in data statements.", FATAL);
	break;

	default:
	  ERROR ("evalConvert", "Invalid type conversion in data statement.", FATAL);
      }
    /* fall through case ... all others return directly */
    convert_buffer[0] = '\0';
    return convert_buffer;
  } /* evalConvert */



static    char 	buffer[128];	   /* temporary space */
static    char  real_buffer[128];  /* temporary space */




/* prints DATA values, output types, and addresses to intermediate file */

void printVal(AST_INDEX	*DataValue, int address, int type, int BI, FILE *fd)

//   AST_INDEX	*DataValue;	/* AST node containing DATA values */
//   int		address;	/* offset of the address	   */
//   int		type;		/* output type of the DATA value   */ 
//   int 		BI;		/* base index for the address      */
//   FILE		*fd;		/* temporary file for DATA values  */

  {

    AST_INDEX	cnst;		/* constant associated with repeat */

    /* if the value is not to be repeated, process more values */
    if (!repeat)
      {
	if (*DataValue == ast_null_node)
	  ERROR("printVal", "Too few data values in data statement", FATAL);


	/* process repeat node */
	else if (gen_get_node_type(*DataValue) == GEN_REPEAT)
	  {
	    /* retrieve the number of times the value is to be repeated */
	    (void) get_num (gen_get_text
			(gen_REPEAT_get_count(*DataValue)), &repeat);
	    repeat--;

	    /* retrieve the value to be repeated and its current type */
	    cnst = gen_REPEAT_get_constant(*DataValue);
	    data_type = gen_get_real_type(cnst);
	    if (data_type != TYPE_COMPLEX)
	      data_elt = NameFromConstantNode(cnst, buffer);
	    else
	      {
		data_elt = NameFromConstantNode
				(gen_COMPLEX_CONSTANT_get_real_const(cnst), real_buffer);
		data_type = gen_get_real_type
				(gen_COMPLEX_CONSTANT_get_real_const(cnst));
		if (data_type != TYPE_REAL)
		  {
		    data_elt = evalConvert(data_elt, data_type, TYPE_REAL);
		    (void) strcpy(real_buffer, data_elt);
		    data_elt = real_buffer;
		  }
		imag_elt = NameFromConstantNode
				(gen_COMPLEX_CONSTANT_get_imag_const(cnst), buffer);
		data_type = gen_get_real_type(gen_COMPLEX_CONSTANT_get_imag_const(cnst));
		if (data_type != TYPE_REAL)
		    imag_elt = evalConvert(imag_elt, data_type, TYPE_REAL);
		data_type = TYPE_COMPLEX;
	      }

	    /* move to the next DATA value on the list */
	    *DataValue = list_next(*DataValue);
	  }


	/* process a positive or negative, non-complex constant */
	else if ((gen_get_node_type(*DataValue) == GEN_CONSTANT)||
		 (gen_get_node_type(*DataValue) == GEN_UNARY_MINUS))
	  {
	    /* retrieve the current type and the value */
	    data_type = gen_get_real_type(*DataValue);
	    data_elt  = NameFromConstantNode(*DataValue, buffer);

	    /* move to the next DATA value on the list */
	    *DataValue = list_next(*DataValue);
	  }
	

	/* process a complex constant */
        else if (gen_get_node_type(*DataValue) == GEN_COMPLEX_CONSTANT)
	  {
	    /* retrieve value and type for the real part */
	    data_elt = NameFromConstantNode
			(gen_COMPLEX_CONSTANT_get_real_const(*DataValue), real_buffer);
	    data_type = gen_get_real_type
			(gen_COMPLEX_CONSTANT_get_real_const(*DataValue));
	    if (data_type != TYPE_REAL)
	      {
	        data_elt = evalConvert(data_elt, data_type, TYPE_REAL);
		(void) strcpy(real_buffer, data_elt);
		data_elt = real_buffer;
	      }

	    /* retrieve value and type for the imaginary part */
	    imag_elt = NameFromConstantNode
			(gen_COMPLEX_CONSTANT_get_imag_const(*DataValue), buffer);
	    data_type = gen_get_real_type
			(gen_COMPLEX_CONSTANT_get_imag_const(*DataValue));
	    if (data_type != TYPE_REAL)
	      imag_elt = evalConvert(imag_elt, data_type, TYPE_REAL);

	    /* set the type to complex */
	    data_type = TYPE_COMPLEX;

	    /* move to the next DATA value on the list */
	    *DataValue = list_next(*DataValue);
	  }


	/* incorrect node type detected */
	else
	  ERROR("PrintVal", "Unexpected node type in data statement", FATAL);
      }

    /* the value is to be repeated, use existing value */
    else repeat--;




    /*  print out the data information (base index, offset,    */
    /*  output type, and data value), converting as necessary  */
    if (type == data_type)
      if (type != TYPE_COMPLEX)
	(void) fprintf(fd, "%4d %10d %4d %s\n", BI, address,
		       type, data_elt);
      else /* complex */
	{
	  (void) fprintf(fd, "%4d %10d %4d %s\n", BI, address, 
			 TYPE_REAL, data_elt);
	  (void) fprintf(fd, "%4d %10d %4d %s\n", BI, address+4, 
			 TYPE_REAL, imag_elt);
	}

    /* input and output type differ, conversion is necessary */
    else
      if (type != TYPE_COMPLEX)
        (void) fprintf(fd, "%4d %10d %4d %s\n", BI, address, type, 
			evalConvert(data_elt, data_type, type));
      else /* complex */
	{
	  (void) fprintf(fd, "%4d %10d %4d %s\n", BI, address, TYPE_REAL,
			evalConvert(data_elt, data_type, TYPE_REAL));
	  (void) fprintf(fd, "%4d %10d %4d %s\n", BI, address+4, 
			TYPE_REAL, "0.0");
	}
  } /* printVal */




/* determines the address of each item to receive data and */
/* calls printVal to print the value, offset and  base     */
/* index, and output type of the item to a temporary file  */

void evalAddress(AST_INDEX	DataName, AST_INDEX	*DataValue, FILE *fd)

//   AST_INDEX	DataName;	/* name of DATA element  */
//   AST_INDEX	*DataValue;	/* value of DATA element */
//   FILE		*fd;		/* temporary file 	 */

  {
    int		i;		/* loop index 				     */
    int		dims;		/* number of dimensions in an array 	     */
    int		index;		/* symbol table index			     */
    int		term;		/* temporary used to compute offset	     */
    int		val;		/* temporary variable			     */
    int		upper;		/* temporary variable			     */
    int		incr;		/* temporary variable 			     */
    AST_INDEX	list;		/* AST list node 			     */
    char	name[32];	/* used to construct implied DO index name   */
    int		args[8];	/* holds value of each subscript expression  */
    int		lb;		/* lower bound 				     */
    int		ub;		/* upper bound 				     */

    ArrayBound *bounds;

    /*  set symbol table index from node type  */
    switch (gen_get_node_type(DataName))
      {
	/* process identifier */
	case GEN_IDENTIFIER:
	  /* retrieve symbol table index */
	  index = getIndex(DataName);

	  /* compute number of bytes needed for each element of a type */
	  incr  = SizeOfTypewIndex(index);

	  /* compute the number of elements associated with the identifier */
	  upper = VarSize(index)/incr;

	  /* retrieve the offset of the identifier */
	  val   = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset);

	  /* print value of each element to temporary file using printVal */
	  for (i=0; i<upper; i++)
	  {
	    printVal(DataValue, val,
		     fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE),
		     BaseIndex(index), fd);
	    val += incr;
	  }
	break;


	/* process nodes with subscripts */
	case GEN_SUBSCRIPT:
	  /* get the symbol table index of the item */
	  index = getIndex(gen_SUBSCRIPT_get_name(DataName));

	  /* walk list of subscript expressions, computing and */
	  /* storing the value of each subscript expression    */
	  /* and the total number of dimensions for the item   */
	  dims = 0;
	  list = list_first(gen_SUBSCRIPT_get_rvalue_LIST(DataName));
	  
	  bounds = (ArrayBound *)
			fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_DIM_BOUNDS);

	  while (list != ast_null_node)
   	  {
	    lb = bounds[dims].lb.value.const_val;
	    args[dims++] = evalExpr(list) - lb;
	    list = list_next(list);
	  }

	  /*  check to insure that the number of dimensions was correct  */
	  if (dims != fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS))
	  {
	    (void) sprintf(error_buffer, 
		"reference to '%s' in data stmt not fully dimensioned",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));

	    ERROR("EvalAddress", error_buffer, FATAL);
	  }

	  /* begin the computation of the offset */
	  dims--;
	  term = args[dims];
	  i    = dims-1;

	  /* calculate the location of the item in the array by adding in */
	  /* the contribution of each appropriately weighted dimension    */
	  while(i > 0)
	  {
	    ub = bounds[i].ub.value.const_val;
	    lb = bounds[i].lb.value.const_val;

	    dims--;
	    term = term * (ub-lb+1) + args[dims];

	    if (aiDebug > 0)
		(void) fprintf(stdout, "\tdim %d; ub %d; lb %d; term: %d\n",
			dims, ub, lb, term);
	  }

	  /* compute the offset of the item relative to the beginning of the array */
	  term *= SizeOfType(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE));

	  if (aiDebug > 0)
	     (void) fprintf(stdout, "\tsized: %d; offset %d; final %d.\n\n",
		     term, fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset),
		     term+fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset));

	  /* compute the actual offset of the item */
	  term += fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset);

	  if (aiDebug > 0)
	     (void) fprintf(stdout, "Address for '%s'; term: %d\n",
			(char *) fst_my_GetFieldByIndex
			(ft_SymTable, index, SYMTAB_NAME), term);

	  /* print item to a temporary file using printVal */
	  printVal(DataValue, term,
		   fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE),
		   BaseIndex(index),fd);
	break;


	/* substring is not yet implemented */
	case GEN_SUBSTRING:
	  index = getIndex(gen_SUBSTRING_get_substring_name(DataName));
	  ERROR("evalAddress", "Substring not yet implemented", FATAL);
	break;


	/* process the implied DO */
	case GEN_IMPLIED_DO:
	  /* get the initial value, the upper bound, and the increment */
	  val   = evalExpr(gen_IMPLIED_DO_get_rvalue1(DataName));
	  upper = evalExpr(gen_IMPLIED_DO_get_rvalue2(DataName));
	  if (gen_IMPLIED_DO_get_rvalue3(DataName) == ast_null_node)
	    incr = 1;
	  else
	    incr = evalExpr(gen_IMPLIED_DO_get_rvalue3(DataName));

	  /*  get the name from gen_IMPLIED_DO_get_name(DataName)  */
	  name[0] = '\0';
	  (void) strcat(name, "_$");
	  (void) strcat(name, gen_get_text(gen_IMPLIED_DO_get_name(DataName)));

	  /* insert name into the symbol table */
	  index = SymInsertSymbol(name, TYPE_INTEGER, OC_IS_DATA, 0,
				  SC_NO_MEMORY, FALSE);

	  /* initialize the implied DO variable, using the scratch field */
	  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_scratch,  val);

	  /* initialize SYMTAB_temp field to -1 (it is normally 0) to */
	  /* indicate that the implied DO variable has a valid value  */
	  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_temp,  -1);

	  /* simulate the implied DO loop */
	  while (val <= upper)
	  {
	    /* process the list of elements */
	    list = list_first(gen_IMPLIED_DO_get_imp_elt_LIST(DataName));

	    while (list != ast_null_node)
	    {
		evalAddress(list,DataValue,fd);
		list = list_next(list);
            }

	    /* bump the index and store in the scratch field */
	    val += incr;
	    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_scratch,  val);
	  }

	  /* invalidate the value associated with the implied DO variable */
	  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_temp,  0);
	break;

	default:
	  ERROR("EvalAddress", "Unexpected node type in data statement", FATAL);
      };
  } /* evalAddress */

  


/* walk the list of AST DATA statements stored in DataStmts[] */
/* and print <data value, address> pairs for each variable to */
/* the file indicated by the file descriptor fd.              */
void ProcessData(FILE *fd)

  // FILE	*fd;	/* temporary file for <value, address> pairs */

{
  AST_INDEX 	DataPair;	/* list of <names, values> pairs  */
  AST_INDEX	DataName;	/* variable to receive data value */
  AST_INDEX	DataValue;	/* data value to be used          */
  int		i;		/* loop index                     */

  /*  iterate through all of the data statements.  DataPtr, */
  /*  the number of data statements, is set in HandleData   */
  for (i=0; i<DataPtr; i++)
  {

    /* for each data staement, retrieve the list of <names,values> pairs */
    DataPair = gen_DATA_get_data_elt_LIST(DataStmts[i]);
    if (DataPair != ast_null_node)
    {

      /* process the list of data pairs */
      DataPair = list_first(DataPair);
      while (DataPair != ast_null_node)
      {
	/* retrieve the name and value from the AST */
        DataName =  list_first(gen_DATA_ELT_get_data_vars_LIST(DataPair));
        DataValue = list_first(gen_DATA_ELT_get_init_LIST(DataPair));
      
	/* initialize the number of times that a value is to be repeated to 0 */
	repeat = 0;

        /* walk the list of names associated with the data pair */
        while (DataName != ast_null_node)
        {
	  /*  print out data and address  */
	  evalAddress(DataName, &DataValue, fd);

	  DataName = list_next(DataName);
        }

	/* detect when not all of the data values supplied are required */
	if ((DataValue != ast_null_node) || (repeat))
	  ERROR("ProcessData", "Too many data values in data statement", FATAL);

        DataPair = list_next(DataPair);
      }
    }
  }
} /* ProcessData */
