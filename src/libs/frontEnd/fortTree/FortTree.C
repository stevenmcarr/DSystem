/* $Id: FortTree.C,v 1.40 1997/03/11 14:29:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************
 *                                                                      *
 *  FortTree -- a Fortran AST with context-sensitive semantics          *
 *                                                                      *
 *  Modification History                                                *
 *    January 7, 1992                    John Mellor-Crummey            *
 *       -- converted this file from C to C++                           *
 *       -- added support for externally visible symbol table           *
 *          abstraction                                                 *
 *       -- added support for assigning unique node numbers to AST      *
 *          nodes (used to for mapping IP information back to the code  *
 *          since the AST_INDEX of a node is not stable across saves)   *
 *       -- initial interprocedural information collection now          *
 *          collected for each scope (procedure, entry, or loop)        *
 *          using new abstractions for external representation          *
 *       -- removed all dependences of the FortTree abstraction on decl *
 *          nodes                                                       *
 *       -- added new type checker error messages                       *
 *                                                                      *
 *   Febuary 10, 1991                   Seema Hiranandani               *
 *      -- removed visit routines that compute mod/ref information      *
 *      -- updated ft_Save to use the new LInfo class to                *
 *         compute MOD/REF information                                  *
 *      -- Redesigned the way local information is collected so that    *
 *         a variety of different information may be collected.         *
 *         The information collected may depend on the problem being    *
 *         solved                                                       *
 ************************************************************************/


#include <string.h>

#include <libs/frontEnd/fortTree/ft.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/tables/cNameValueTable.h>
#include <libs/support/file/Cookie.h>
#include <libs/support/file/File.h>

#include <libs/frontEnd/include/NeedProvSet.h>


/************************/
/*  Initialization info	*/
/************************/

static int ft_InitCount = 0;


/************************/
/*  Saving information	*/
/************************/

char			*ft_SourceAttribute = "source";
static Nodeinfo		fortran_nodeinfo[] = {
#include <include/frontEnd/nodeinfo.h>
};

/************************/
/*  Error information	*/
/************************/


typedef union {
	int			error_code;
	struct {
		short		semantic;
		short		parser;
	}			sub_error;
} ft_Error;


static char		*ft_SemanticErrorTable[] = {
	"No error",				/* must match the set 	*/
	"Cannot parse",				/* of #defines in 	*/
	"Incomplete program",			/* ./FortTree.i		*/
	"VM FORTRAN extension",
	"PARASCOPE extension",
	"Rn extension",
	"Not an intrinsic",
	"Duplicate declaration",
	"Incompatible type declaration",
	"Illegal invocation",
	"Invalid Type-Length combination",
	"Length specifier too complex",
	"Type invalid in this context",
	"Wrong number of arguments to intrinsic function",
	"Invalid complex constant",
	"Wrong number of subscripts",
	"Expression in DO is not INTEGER, REAL, or DOUBLE PRECISION",
	"Statement is out of order",
	"Invalid use of adjustable array dimension",
	"Bad array dimension specifier",
	"Duplicate definition of statement label",
	"Bad specifier in IMPLICIT statement",
	"Illegal EQUIVALENCE between variables in different common blocks",
	"Invalid variable in EQUIVALENCE statement", 
	"Unexpected data type encountered", 
	"EQUIVALENCE inconsistent with earlier equivalences", 
	"Parameter value recursively defined", 
	"Unexpected node type in expression",
	"Non-constant encountered in constant integer expression", 
	"Non-integer constant encountered in constant integer expression", 
	"Common variable name conflicts with prior use", 
	"A function cannot be declared as both INTRINSIC and EXTERNAL",  
	"Name appears in more than one INTRINSIC declaration",
	"More than one IMPLICIT declaration for the same letter",
	"More than one array declarator for the same symbolic name",
	"Declared INTRINSIC used as statement function dummy argument",
	"ENTRY name is also used as dummy argument",
	"ENTRY name cannot be external", 
	"ENTRY can only appear in function or subroutine", 
	"Array declarator exceeds maximum number of dimensions supported", 
	"Array dimension upper bound < lower bound",
	"EQUIVALENCE invalid for dummy argument of an external procedure",
	"Non-constant subscript expression in an EQUIVALENCE",
	"Character length specifier required", 
	"Illegal character constant expression",
	"Constant value specified for entity that is not a local variable",
	"Character argument of ICHAR must have length 1",
	"Function returns character string of unknown length",
	"Multiple dummy arguments have the same name", 
	"EQUIVALENCE multiply defined", 
	"EQUIVALENCE with array variable negatively extends COMMON block",
	"Unexpected AST node type encountered -- type checker problem?",
	"Redefinition of a manifest constant",
	"Statement of this type not permitted in BLOCK DATA subprogram",
	"Invocation references name with conflicting use in local scope",
	"Wrong number of arguments to statement function",
	"Statement function argument type mismatch",
	"Length mismatch of character argument to statement function", 
	"Incompatible lhs and rhs types in ASSIGNMENT statement",
	"Length specifier must be integer constant expression",
	"Private declaration for non-data object",
	"Private declaration for datum that is already private in this scope", 
	"Private declaration in bad context",
	"Illegal private declaration for procedure or entry argument",
	"Illegal private declaration for common variable",
	"Invalid substring expression",
	"Apparent procedure parameter has no EXTERNAL declaration",
	"Actual parameter may not be a statement function",
	"Function or subroutine name cannot appear as a term in an expression",
	"Function invocation of a subroutine",
	"Subroutine invocation of a function",
	"Incompatible argument types for built-in function",
	"Invalid argument type for built-in function",
	"Wrong number of arguments to built-in function",
	"DATA/SAVE specification invalid for dummy argument of procedure",
	"DATA/SAVE specification invalid for non-data identifier",
	"Statement function invokes not yet defined statement function",
	"Statement function has actual arguments that are not identifiers",
	"Referenced statement label not defined",
        "Variable declared constant is used in a COMMON statement",
        "Referenced COMMON block name not defined in a COMMON statement",

	"Statement not in supported Shared Memory Fortran Dialect",
	"Statement not allowed in logical if",
	"Statement not allowed in this context",

	"Definition of procedure conflicts with a prior use in module",
	"Use of procedure conflicts with prior definition in module",
	"Use of procedure conflicts with a prior use in module",

	"ParaScope Restriction: body of statement function may not invoke procedures",
	"ParaScope Restriction: actual to statement function may not invoke procedures",

	/* These last two should never appear - they indicate problems with TC */
	"Invalid error code - type checker internal problem",
	"Logic problem in the type checker"
};



/************************/
/* External declarations*/
/************************/

EXTERN(int, system, (const char*));

/************************/
/* Forward declarations	*/
/************************/


STATIC(void, ft_ReadSideArrayNodes,
       (AST_INDEX tree, Generic s_a_id, Generic width, DB_FP *fp, Generic *array));
STATIC(void, ft_WriteSideArrayNodes,
       (AST_INDEX  tree, Generic  s_a_id, Generic width, DB_FP   *fp));
STATIC(void, add_pair_to_node_number_mapping,
       (FortTree ft, FortTreeNode node, unsigned int *counter));
STATIC(void, assign_node_numbering,
       (FortTree ft, FortTreeNode node, unsigned int *counter));
STATIC(Boolean, last_statement_in_scope,
       (FortTreeNode node));
STATIC(void, treeCheckParse,
       (FortTree ft, FortTreeNode node));
STATIC(void, removeSemanticErrors,
       (FortTree ft, FortTreeNode node));



/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/

/************************/
/*  Initialization	*/
/************************/




void ft_Init(void)
{
	if(ft_InitCount++ == 0) { /* initialize submodules */
		fst_Init();
	}
}




void ft_Fini(void)
{
	if(--ft_InitCount == 0) { /* finalize submodules */
		fst_Fini();
	}
}




FortTree ft_Create()
{
  FortTree ft;
  
  // allocate a new instance
  ft = (FortTree)
    get_mem(sizeof(struct FortTree_internal_structure), "FortTree");
  if (ft == (FortTree) 0) return (FortTree) UNUSED;
  
  // initialize parts 
  ft->state = ft_UNINITIALIZED;
  
  // allocate a placholder for an ast
  ft->asttab = ast_create(fortran_nodeinfo, 100, 100);
  ast_select(ft->asttab);
  ft->root   = AST_NIL;

  ft->needs = 0;
  ft->provs = 0;
  
  // open the subparts
  ft->td = fst_Open((Generic) ft);
  
  // node numbering hashtables do not exist until needed
  
  ft->NodeToNumber = 0;
  ft->NumberToNode = 0;
  
  return ft;
}


ft_States ft_GetState(FortTree ft)
{
  return ft->state;
}


void ft_ResetStateToInitialized(FortTree ft)
{
  ft->state = ft_INITIALIZED;
}

// marker to make sure whole file read/written properly
#define MAGIC_COOKIE 0xa5a5a5 

int ft_Read(FortTree ft, DB_FP *file)
{  
  int code = -1;
  // clean up placeholder AST allocated by ft_Create
  ast_destroy(ft->asttab);
  
  // read AST from file, success assumed for ast_import2!
  ft->root = ast_import2(fortran_nodeinfo, file, &ft->asttab);
  
  if (ft->root) {
    if (code = file->Read(&ft->state, sizeof(ft->state)) || 
	ReadCookie(file)) {
      ast_destroy(ft->asttab);
      ft->asttab = ast_create(fortran_nodeinfo, 100, 100);
    }
  } else {
    // if ft->root is NULL, the import failed and we need to
    // replace the non-existent asttab
    ft->asttab = ast_create(fortran_nodeinfo, 100, 100);
  }

  ast_select(ft->asttab);
  return code; 
}




int ft_Write(FortTree ft, File *file)
{  
  // write AST to file
  ast_export2(file, ft->root);

  return file->Write(&ft->state, sizeof(ft->state)) || WriteCookie(file);
}



/*********************************************************
 * With a pointer based AST, node pointers are not stable
 * across saves. Therefore, some stable numbering of nodes
 * is needed to relate interprocedural information written
 * out back to the AST. The functions below implement
 * an efficient bi-directional mapping that is stable across
 * saves.
 *********************************************************/


/* map an AST node to its unique id if any exists */
int
ft_NodeToNumber(FortTree ft, FortTreeNode node)
{
	Generic value;

	if (ft->NodeToNumber == 0) ft_RecomputeNodeNumbers(ft);

	if (NameValueTableQueryPair(ft->NodeToNumber, (Generic) node, &value))
		return (int) value;
	else return -1;
}


/* map the unique id for an AST node back to the node itself */
FortTreeNode
ft_NumberToNode(FortTree ft, int number)
{
	Generic value; 

	if (ft->NumberToNode == 0) ft_RecomputeNodeNumbers(ft);

	if (NameValueTableQueryPair(ft->NumberToNode, (Generic) number, &value))
	  return (FortTreeNode)value;
	else 
          return AST_NIL;
}


/* recompute the AST node to unique id bi-directional mapping. this
 * function should be invoked after all edits before writing any
 * unique ids to the file system
 */
void
ft_RecomputeNodeNumbers(FortTree ft)
{

	int unsigned counter = 0;

	if (ft->NodeToNumber) NameValueTableFree(ft->NodeToNumber);
	ft->NodeToNumber = NameValueTableAlloc(8, 
                                      (NameCompareCallback)NameValueTableIntCompare, 
                                      (NameHashFunctCallback)NameValueTableIntHash);

	if (ft->NumberToNode) NameValueTableFree(ft->NumberToNode);
	ft->NumberToNode = NameValueTableAlloc(8, 
                                      (NameCompareCallback)NameValueTableIntCompare, 
                                      (NameHashFunctCallback)NameValueTableIntHash);

	assign_node_numbering(ft, ft->root, &counter);
}


void ft_Close(FortTree ft)
{
  ast_select(ft->asttab);
  
  /* close the subparts */
  fst_Close(ft->td);

  /* provides and requires */
  if (ft->needs) delete ft->needs;	
  if (ft->provs) delete ft->provs;	
  
  ast_destroy(ft->asttab);
  
  if (ft->NodeToNumber) NameValueTableFree(ft->NodeToNumber);
  if (ft->NumberToNode) NameValueTableFree(ft->NumberToNode);
  
  free_mem((void*)ft);
}


/************************/
/*  AST access		*/
/************************/

void ft_AstSelect(FortTree ft)
{
	ast_select(ft->asttab);
}

FortTreeNode ft_Root(FortTree ft)
{
	return ft->root;
}

void ft_SetRoot(FortTree ft, FortTreeNode node)
{
	/* ASSERT: caller has done any relevant freeing */

	ft->root = node;
	ast_setroot(ft->asttab, (AST_INDEX) node);
}

/************************/
/* Semantics access	*/
/************************/

/* what does this mean? ... */


/************************/
/* Updating semantics	*/
/************************/

/* ditto ... */


/************************/
/*  Side Array		*/
/************************/

FortTreeSideArray ft_AttachSideArray(FortTree ft, int size, Generic *initials)
{
	return ast_attach_side_array(ft->asttab, size, initials);
}

void ft_DetachSideArray(FortTree ft, FortTreeSideArray sideArray)
{
	ast_detach_side_array(ft->asttab, sideArray);
}

Generic ft_GetFromSideArray(FortTreeSideArray sideArray, FortTreeNode node,
int k)
{
	return ast_get_side_array(sideArray, node, k);
}

void ft_PutToSideArray(
	FortTreeSideArray sideArray, 
	FortTreeNode node, 
	int k,
	Generic value)
{
	ast_put_side_array(sideArray, node, k, value);
}

void ft_ReadSideArrayFromFile(FortTree ft, FortTreeSideArray sideArray, DB_FP *fp)
{
	// Ast_side_array * sa = (Ast_side_array *) sideArray;
	int width;
	Generic   *array;

	// ft_AstSelect(ft);
	//size = (sa->width * sizeof(Generic)) * asttab->stats.high_water_mark;

	// (void) db_buffered_read(fp, (char *) sa->array, size);

  ft_AstSelect(ft);

  /* Find the width of this particular side_array */
  if ( ((Generic) sideArray) >= asttab->stats.side_array_width)
    {
      /* They are trying to access a slot that is out of reach */
      fprintf(stderr, "Error in ft_ReadSideArray() -- tried to access slot %d, max width is %d\n", (Generic) sideArray, asttab->stats.side_array_width);
      return;
    }

  width = asttab->stats.side_array_in_use[((Generic) sideArray)];
  array = (Generic *) get_mem(width * sizeof(Generic), "side array");

  /* Call the recursive function that actually reads in the side arrays */
  ft_ReadSideArrayNodes(asttab->root, (Generic)sideArray, width, fp, array);

  free_mem((void*)array);

}

void ft_ReadSideArray(FortTree ft, FortTreeSideArray sideArray, Context, DB_FP *fp)
{
  ft_ReadSideArrayFromFile(ft,sideArray,fp);
}

static void ft_ReadSideArrayNodes(AST_INDEX tree, 
Generic s_a_id, Generic width, DB_FP *fp, Generic *array)
{
  AST_INDEX      node;
  int            num_of_sons, i;
  
  if (tree == AST_NIL)
    return;
  
  db_buffered_read(fp, (char *) array, width * sizeof(Generic));

  if (memcmp((char *) array, 
             (char *) &asttab->stats.side_array_initial_values[s_a_id],
	     width * sizeof(Generic)) != 0)
    {
      /* If we are in here then the side array that was written out differs */
      /* from the initial values and thus we must put the values into */
      /* the side array */
      for (i = 0; i < width; i++)
	ast_put_side_array(s_a_id, tree, i, array[i]);
    }
  /* Now we need to recursively read in the rest of the side arrays */
  if (is_list_node(tree))
    {
      node = list_first(tree);
      while (node != AST_NIL)
	{
	  ft_ReadSideArrayNodes(node, s_a_id, width, fp, array);
	  node = list_next(node);
	}
    }
  else
    {
      num_of_sons = ast_get_son_count(tree);
      for (i = 1; i <= num_of_sons; i++)
	ft_ReadSideArrayNodes(ast_get_son_n(tree, i), s_a_id, width, fp, array);
    }
}



void ft_WriteSideArray(
	FortTree ft,
	FortTreeSideArray sideArray,
	Context,
	DB_FP * fp)
{
  ft_WriteSideArrayToFile(ft, sideArray, fp);
}

void ft_WriteSideArrayToFile(
	FortTree ft,
	FortTreeSideArray sideArray,
	DB_FP * fp)
{
	// Ast_side_array * sa = (Ast_side_array *) sideArray;
	int width;

	ft_AstSelect(ft);
	//size = (sa->width * sizeof(Generic)) * asttab->stats.high_water_mark;

	// (void) db_buffered_write(fp, (char *) sa->array, size);

  if ( ((Generic) sideArray) >= asttab->stats.side_array_width)
    {
      /* They are trying to access a slot that is out of range */
      fprintf(stderr, "Error in ft_WriteSideArray() -- trying to access slot %d, max width is %d",
	      (Generic) sideArray, asttab->stats.side_array_width);
      return;
    }

  /* Find the width of this particular side array */
  width = asttab->stats.side_array_in_use[((Generic) sideArray)];


  /* Call the recursive function that actually write out the side arrays */
  ft_WriteSideArrayNodes(ft->root, (Generic)sideArray, width, fp);

}

static void ft_WriteSideArrayNodes(AST_INDEX  tree,
Generic  s_a_id, Generic width, DB_FP   *fp)
{
  AST_INDEX       node;
  int             num_of_sons;
  int             i;
  
  if (tree == AST_NIL)
    return;
  if (((NODE *)tree)->Leafnode->side_array_ptr == 0)
    {
	  /* Use the initial values */
      (void) db_buffered_write(fp, (char *)&asttab->stats.side_array_initial_values[s_a_id],
			       width * sizeof(Generic));
    }
  else
    {
	  /* This node must have its own side array */
      (void) db_buffered_write(fp, (char *) &((NODE *)tree)->Leafnode->side_array_ptr[s_a_id],
		      width *sizeof(Generic));
    }
  if (is_list_node(tree))
    {
      node = list_first(tree);
      while (node != AST_NIL)
	{
	  ft_WriteSideArrayNodes(node, s_a_id, width, fp);
	  node = list_next(node);
	}
    } else {
      num_of_sons = ast_get_son_count(tree);
      for (i = 1; i <= num_of_sons; i++)
	ft_WriteSideArrayNodes(ast_get_son_n(tree, i), s_a_id, width, fp);
    }
}


/************************/
/* Type Checking (temp)	*/
/************************/


ft_States ft_Check(FortTree ft)
{
  assert(ft->state != ft_UNINITIALIZED);

  ft->state = ft_INITIALIZED;

  ast_select(ft->asttab);
  removeSemanticErrors(ft, ft->root);
  
  treeCheckParse(ft, ft->root);	/* should be folded into TC */
  
  if (ft->state != ft_ERRONEOUS) {

    // For this to work, 
    //	fst_Open()	must have been called in FortTreeOpen() 
    //	fst_Close()	must be called in FortTreeClose()
    //  fst_Recompute() re-initializes its world on every 
    //                  invocation; it frees previously allocated tables 
    //                  and such each call.
    
    fst_Recompute(ft->td);
  }
  
  if (ft->state == ft_INITIALIZED) ft->state = ft_CORRECT;
  
  return ft->state;
}


/************************/
/*  Symbol Table Access */
/************************/

ft_States ft_SymRecompute(FortTree ft)
{
  return ft_Check(ft);
}

SymDescriptor
ft_SymGetTable(FortTree ft, char *entry_point)
{
	return fst_GetTable(ft->td, entry_point);
}

unsigned int 
ft_SymNumberOfModuleEntries(FortTree ft)
{
	return fst_NumberOfModuleEntries(ft->td);
}


/************************/
/*  Needs/Provs access  */
/************************/

NeedProvSetPtr ft_GetNeeds(FortTree ft)
{
  if (ft->state == ft_INITIALIZED || ft->needs == 0) ft_Check(ft);
  if (ft->state == ft_ERRONEOUS) return 0;
  return ft->needs;
}

NeedProvSetPtr ft_GetProvs(FortTree ft)
{
  if (ft->state == ft_INITIALIZED || ft->provs == 0) ft_Check(ft);
  if (ft->state == ft_ERRONEOUS) return 0;
  return ft->provs;
}

/************************/
/*  Node Numbering      */
/************************/


static void
add_pair_to_node_number_mapping(FortTree ft, FortTreeNode node, 
	unsigned int *counter)
{
	int dummy;
	unsigned int number = (*counter)++;

	NameValueTableAddPair(ft->NodeToNumber, (Generic)node, (Generic)number, 
		(Generic*)&dummy);
	NameValueTableAddPair(ft->NumberToNode, (Generic)number, (Generic)node, 
		(Generic*)&dummy);
}

static void
assign_node_numbering(FortTree ft, FortTreeNode node, unsigned int *counter)
{

	switch(gen_get_node_type(node)) {

	case GEN_LIST_OF_NODES:
		{
			FortTreeNode elt = list_first(node);
			while(elt != AST_NIL) {
				assign_node_numbering(ft, elt, counter);
				elt = list_next(elt);
			}
		}
		break;

	case GEN_INVOCATION:
	case GEN_FUNCTION:
	case GEN_PROGRAM:
	case GEN_SUBROUTINE:
	case GEN_BLOCK_DATA:
	case GEN_PARALLEL_CASE:
	case GEN_DO_ALL:
	case GEN_PARALLELLOOP:
	case GEN_DO:
	case GEN_COMMENT:
		add_pair_to_node_number_mapping(ft, node, counter);
		/* fall through to label sons as needed */

	default:
		{
			int n = ast_get_node_type_son_count(gen_get_node_type(node));
			for (int i=1; i<=n; i++) {
				FortTreeNode son = ast_get_son_n(node, i);
				assign_node_numbering(ft, son, counter);
			}
		}
		break;
	}
}


/************************/
/* Display Bits         */
/************************/

#define CLEAR_BIT(val, bit) (val) &= ~(1 << (bit))
#define SET_BIT(val, bit)   (val) |=  (1 << (bit))
#define GET_BIT(val, bit)   BOOL((val) & (1 << (bit)))

#define COMMA_BIT      		0		/* represented inverted */
#define SHOW_BIT       		1		/* represented inverted */
#define EMPHASIS_BIT   		11		/* represented normally	*/
#define CONCEAL_OPEN_BIT	2
#define CONCEAL_CLOSE_BIT	(2 + 8)		/* 8 bits  =>  MASK == 0xff */
#define MASK			0xff

void ft_SetComma(FortTreeNode node, Boolean value)
{
	int disp = gen_get_display(node);

	if (value)
		CLEAR_BIT(disp, COMMA_BIT);
	else
		SET_BIT(disp, COMMA_BIT);

	gen_put_display(node, disp);
}

Boolean ft_GetComma(FortTreeNode node)
{
	int disp = gen_get_display(node);

	return NOT(GET_BIT(disp, COMMA_BIT));
}

void ft_SetShow(FortTreeNode node, Boolean value)
{
	int disp = gen_get_display(node);

	if (value) CLEAR_BIT(disp, SHOW_BIT);
	else SET_BIT(disp, SHOW_BIT);

	gen_put_display(node, disp);
}

Boolean ft_GetShow(AST_INDEX node)
{
	int disp = gen_get_display(node);

	return NOT(GET_BIT(disp, SHOW_BIT));
}

void ft_SetEmphasis(FortTreeNode node, Boolean value)
{
	int disp = gen_get_display(node);

	if (value) SET_BIT(disp, EMPHASIS_BIT);
	else CLEAR_BIT(disp, EMPHASIS_BIT);

	gen_put_display(node, disp);
}

Boolean ft_GetEmphasis(AST_INDEX node)
{
	int disp = gen_get_display(node);

	return GET_BIT(disp, EMPHASIS_BIT);
}

void ft_SetConceal(FortTree ft, FortTreeNode node, int bracket, int value)
{
	int disp,bit;

	ft_AstSelect(ft);

	disp = gen_get_display(node);
	bit  = (bracket == 2) ? CONCEAL_CLOSE_BIT : CONCEAL_OPEN_BIT;

	disp &= ~(MASK << bit);
	disp |= ((value & MASK) << bit);

	gen_put_display(node, disp);
}

int ft_GetConceal(FortTree ft, AST_INDEX node, int bracket)
{
	int disp, bit;

	ft_AstSelect(ft);

	disp = gen_get_display(node);
	bit  = (bracket == 2 ? CONCEAL_CLOSE_BIT : CONCEAL_OPEN_BIT);

	return (disp & (MASK << bit)) >> bit;
}


/************************/
/* Error access		*/
/************************/

/*ARGSUSED*/
void ft_SetSemanticErrorCode(FortTree ft, FortTreeNode node, short value)
{
  ft->state = ft_ERRONEOUS;
	ft_Error err;

	ast_select(ft->asttab);
	err.error_code = gen_get_error_code(node);
	err.sub_error.semantic = value;
	gen_put_error_code(node, err.error_code);
}

short ft_GetSemanticErrorCode(FortTreeNode node)
{
	ft_Error err;

	err.error_code = gen_get_error_code(node);
	return err.sub_error.semantic;
}

void ft_SetParseErrorCode(FortTreeNode node, short value)
{
	ft_Error err;

	err.error_code = gen_get_error_code(node);
	err.sub_error.parser = value;
	gen_put_error_code(node, err.error_code);
}

short ft_GetParseErrorCode(FortTreeNode node)
{
	ft_Error err;

	err.error_code = gen_get_error_code(node);
	return err.sub_error.parser;
}

static Boolean last_statement_in_scope(FortTreeNode node)
{
  FortTreeNode compound;

  if (node == AST_NIL)
    return false;

  if (list_next(node) != AST_NIL)
    return false;

  compound = out(node);
  if (is_scope(compound))
    return true;
  else
    return last_statement_in_scope(compound);
}

Boolean ft_IsErroneous(FortTree ft, FortTreeNode node, int bracket)
{
	ft_AstSelect(ft);

	if(NT(node) == GEN_ERROR)
	  return true;
	else if(ft_GetParseErrorCode(node) != 0)
	{
	  /* always display missing end at end of scope */
	  if (is_scope(node) && bracket == ft_CLOSE)
	    return true;
	  else
	    return BOOL(bracket == ft_OPEN || bracket == ft_SIMPLE);
	}
	else if(ft_GetSemanticErrorCode(node) != 0)
	  return BOOL(bracket == ft_OPEN || bracket == ft_SIMPLE);
	else if (last_statement_in_scope(node))
	  {
	    FortTreeNode scope;
	    scope = find_scope(node);
	    return BOOL(ft_GetParseErrorCode(scope) != 0);
	  }
	else
	  return false;
}

void ft_GetErrorMessage( FortTree ft, AST_INDEX node, int bracket, char * Message)
{
	FortTreeNode complaint;
	int SECode = 0;

	// bogus statement to stop CC from emitting an unused parameter warning
	bracket = bracket; 

	ft_AstSelect(ft);
	(void) strcpy(Message, ">>> ");

	if(NT(node) == GEN_ERROR) {
	  complaint = gen_ERROR_get_complaint(node);
	  (void) strcat(Message,gen_get_text(complaint));
	}
	else if(ft_GetParseErrorCode(node) != 0)
	{
	  /* always display missing end at end of scope */
	  if (bracket == ft_CLOSE)
	  {
	    (void) strcat(Message,"Missing end statement for ");
   	    (void) strcat(Message,
			  gen_node_type_get_text(gen_get_node_type(node)));
	  }
	  else if (bracket == ft_OPEN)
	  {
	    (void) strcat(Message,"No matching end statement for ");
	    (void) strcat(Message,
			  gen_node_type_get_text(gen_get_node_type(node)));
	  }
	}
	else if ((SECode = ft_GetSemanticErrorCode(node)) != 0)
	{
	      (void) strcat(Message,ft_SemanticErrorTable[SECode]);	
	}
	else if (last_statement_in_scope(node))
	{
	    FortTreeNode scope;
	    scope = find_scope(node);
	    if (ft_GetParseErrorCode(scope) != 0)
	    {
		(void) strcat(Message,"Missing end statement for ");
		(void) strcat(Message,
			      gen_node_type_get_text(gen_get_node_type(scope)));
	    }
	}
	else 
	{
	  fprintf(stderr,"ft_GetErrorMessage: failed to find error.\n");
	  *Message = '\0';
	}
}


/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/


/************************/
/* Checking passes	*/
/************************/

/* ARGSUSED */
static void treeCheckParse(FortTree ft, FortTreeNode node)
{
	FortTreeNode elt,father;
	int i, n, which;

	// this check added by CARLE -- some parse errors are not
	// represented by GEN_ERROR nodes 
	if (ft_GetParseErrorCode(node) != 0)
		ft->state = ft_ERRONEOUS;

	switch(gen_get_node_type(node))
	{
	case GEN_LIST_OF_NODES:
		elt = list_first(node);
		while(elt != AST_NIL) {
			treeCheckParse(ft, elt);
			elt = list_next(elt);
		}
		break;

	case GEN_ERROR:
		ft_SetSemanticErrorForStatement(ft, node, ft_CANNOT_PARSE);
		ft->state = ft_ERRONEOUS;
		break;

	case GEN_PLACE_HOLDER:
		father = ast_get_father(node);
		which  = gen_which_son(father,node);
		if(!is_OPTIONAL(get_son(which,NT(father)))) {
			ft_SetSemanticErrorForStatement(ft, node, ft_INCOMPLETE);
		ft->state = ft_ERRONEOUS;
		}
		break;

	case GEN_AT:
	case GEN_DEBUG:
	case GEN_TRACEON:
	case GEN_TRACEOFF:
		ft_SetSemanticErrorForStatement(ft, node, ft_VM_FORTRAN);
		break;

	case GEN_TASK:
	case GEN_VALUE_PARAMETER:
	case GEN_PARALLEL:
	case GEN_PARALLEL_CASE:
	case GEN_DO_ALL:
	case GEN_TASK_COMMON:
	case GEN_LOCK:
	case GEN_UNLOCK:
	case GEN_WAIT:
	case GEN_POST:
	case GEN_POSTING:	// I have commented all this stuff out 	
	case GEN_POST_TO:	// Users now want access to these 	
	case GEN_POST_INC:	// constructs...			
	case GEN_CLEAR:		// The type checker should handle each	
	case GEN_SET_BARRIER:	// of them by now... 			
	case GEN_BLOCK:		// Users will complain if it does not...
	case GEN_SEMAPHORE:
	case GEN_EVENT:		//			kdc -- 1/15/91	
	case GEN_BARRIER:
	case GEN_PARALLELLOOP:
	case GEN_PRIVATE:
	case GEN_STOPLOOP:
	//	ft_SetSemanticErrorForStatement(ft, node, ft_PARASCOPE); 
	// break; 
	case GEN_CONDITIONAL:
	case GEN_REPETITIVE:
	//	ft_SetSemanticErrorForStatement(ft, node, ft_RN); 
	// break;
	default:
		n = gen_how_many_sons(gen_get_node_type(node));
		for(i = 1; i <= n; i++)
			treeCheckParse(ft, gen_get_son_n(node, i));
		break;
	}
}


/************************/
/* Semantic errors	*/
/************************/

static void removeSemanticErrors(FortTree ft, FortTreeNode node)
{
	ft_Error err;
	int i, n;
	FortTreeNode elt;

	err.error_code = gen_get_error_code(node);
	err.sub_error.semantic = ft_NO_ERROR;
	gen_put_error_code(node, err.error_code);

	if(is_list(node)) {
		elt = list_first(node);
		while(elt != AST_NIL) {
			removeSemanticErrors(ft, elt);
			elt = list_next(elt);
		}
	}
	else {
		n = gen_how_many_sons(gen_get_node_type(node));
		for(i = 1; i <= n; i++)
			removeSemanticErrors(ft, gen_get_son_n(node, i));
	}
}


/************************/
/* Misc.		*/
/************************/

Boolean isFunctionInvocation(FortTreeNode node)
{
	FortTreeNode parent;

	if(is_invocation(node)) {
		parent = tree_out(node);
		return NOT(is_call(parent) || is_task(parent));
	}
	else return false;
}

void
ft_SetSemanticErrorForStatement(FortTree ft, FortTreeNode node, int value)
{
	FortTreeNode	stmt;
	FortTreeNode    parent_stmt;

	/* first, set the Error field in the node itself	*/
	ft_SetSemanticErrorCode(ft, node, value);

	/* move out to the enclosing statement */
	if (is_statement(node))
	  stmt = node;	  
	else
	{
	  stmt = out(node);
	  if (is_statement(stmt) &&
	      ft_GetSemanticErrorCode(stmt) == ft_NO_ERROR)
	    /* do not overwrite another error */
	    ft_SetSemanticErrorCode(ft, stmt, value);
	}

	/* Now, one more time, in case we are under the control */
	/* of a LOGICAL_IF or in the first guard of a block IF */
        /* (NED display only checks the if!)                   */
	parent_stmt = out(stmt);
	if (is_logical_if(parent_stmt)  /* easy case */
	    ||
	    (is_guard(stmt)     &&
	     is_if(parent_stmt) &&      /* stmt is first guard in block if */
	     (list_first(gen_IF_get_guard_LIST(parent_stmt)) == stmt)))
	{
	  if (ft_GetSemanticErrorCode(parent_stmt) == ft_NO_ERROR)
	    /* do not overwrite another error */
	    ft_SetSemanticErrorCode(ft, parent_stmt, value);
	}
}

char* ft_SemanticErrorMessage(int code)
{
	if (code <= ft_NO_ERROR || ft_LAST_SEMANTIC_ERROR <= code)
		code = ft_NO_ERROR;
	return ft_SemanticErrorTable[code];
}

AST_INDEX get_name_astindex_from_ref(AST_INDEX evar)
{
    AST_INDEX       tmp;

    /* evar can be a GEN_IDENTIFIER, GEN_SUBSCRIPT, GEN_SUBSTRING */
    switch (ast_get_node_type(evar))
    {
    case GEN_IDENTIFIER:
        return evar;

    case GEN_SUBSCRIPT:
        return (gen_SUBSCRIPT_get_name(evar));

    case GEN_SUBSTRING:
        tmp = gen_SUBSTRING_get_substring_name(evar);
        if (ast_get_node_type(tmp) == GEN_SUBSCRIPT)
        return (gen_SUBSCRIPT_get_name(tmp));
        else
        return tmp;
    default:
        return AST_NIL;
    }
}
