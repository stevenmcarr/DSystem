/* $Id: label.C,v 1.2 1997/06/25 15:21:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************/
/*                                      				*/
/* misc. routines for handling fortran labels           		*/
/*                                      				*/
/************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <strings.h>

static int LabelGetInternal (STR_TEXT,int*,AST_INDEX,int);

#ifndef OSF1
extern "C" void bzero(void *s, size_t n);
#endif

/************************************************************************/
/* local & global variables             				*/
/************************************************************************/
static int assigned_list_header;

/* globals used in ai.c and io.c */
extern   FortTree      ft;
extern   FortTextTree  ftt;



/************************************************************************/
/* creates a label table                				*/
/************************************************************************/
void LabelCreateTable (int size)
  // int	size;		/*  size of the label table  */
{
  /*  if a table already exists, destroy it  */
  if (label_table != NULL)
    LabelDestroyTable ();

  /*  allocate the label table  */
  label_table = (struct label_element *) get_mem
    (sizeof (struct label_element) * size, "label_table");
  bzero((char *)label_table, sizeof (struct label_element) * size );

  /*  save the size of the table  */
  label_table_dimension = size;

  assigned_list_header = aiEND_OF_LIST;
} /* LabelCreateTable */



/************************************************************************/
/* simplified interface used to invoke LabelGetInternal when a label    */
/* definition is required.						*/
/************************************************************************/
int LabelDefine(STR_TEXT label, int *iloc_label, AST_INDEX stmt)
//      STR_TEXT	label;		/* FORTRAN label from AST     */
//      int	*iloc_label;	/* ILOC label		      */
//      AST_INDEX	stmt;		/* statement containing label */
{
  return LabelGetInternal(label, iloc_label, stmt, LABEL_DEFINITION);
} /* LabelDefine */



/************************************************************************/
/* simplified interface used to invoke LabelGetInternal when the	*/
/* ILOC label is required.						*/
/************************************************************************/
void LabelGet (STR_TEXT label, int *iloc_label)
//      STR_TEXT	label;		/*  FORTRAN label from AST  */
//      int  	*iloc_label;	/*  ILOC label  */
{
  ( void) LabelGetInternal(label, iloc_label, AST_NIL, LABEL_USE);
} /* LabelGet */



/************************************************************************/
/* simplified interface used to invoke LabelGetInternal to record that  */
/* a label appears in a FORTRAN ASSIGN statement.			*/
/************************************************************************/
void LabelInAssign(STR_TEXT label, int *iloc_label)
//      STR_TEXT	label;		/* FORTRAN label from AST */
//      int	*iloc_label;	/* iloc label */
{
  (void) LabelGetInternal(label, iloc_label, AST_NIL, LABEL_USE_IN_ASSIGN);
} /* LabelInAssign */



/************************************************************************/
/* static routine that controls inserts and lookups in the label	*/
/* table.  Most other routines in this module call this one.    	*/
/************************************************************************/
static int LabelGetInternal (STR_TEXT label, int *iloc_label, AST_INDEX stmt, 
			     int def_use_flag)
//      STR_TEXT	label;		/*  FORTRAN label from AST  */
//      int       *iloc_label;	/*  ILOC label  */
//      AST_INDEX	stmt;		/*  node index of statement */
//      int 	def_use_flag;   /*  Help scope out duplicates */
{
  int fortran_label;		/*  FORTRAN label  */
  int index;			/*  label table index  */
  int found = DONT_KNOW;	/*  return value of the find function  */
  int initial_index;		/*  initial value of index  */

  if (aiDebug > 0)
    (void) fprintf(stdout, "LabelGetInternal( '%s', %d, %d) => ",
		   label, *iloc_label, def_use_flag);

  *iloc_label = DONT_KNOW;

  /*  check to insure that a table exists  */
  if (label_table == NULL)
    ERROR("LabelTable", "No allocated label table", FATAL);

  /*  find the index in the symbol table  */
  (void) sscanf (label, "%d", &fortran_label);
  index = fortran_label % label_table_dimension;
  initial_index = index;

  /*  search for the label  */
  /*  valid FORTRAN labels must be greater than zero  */
  while ((label_table[index].Fortran != 0) && (found == DONT_KNOW)) {
    /*  if not found, move to the next element  */
    if (fortran_label != label_table[index].Fortran) {
      index = (index + 1) % label_table_dimension;

      /*  if this is true, the table is full  */
      if (index == initial_index)
	ERROR("LabelTable", "Label table is full", FATAL);
    }
    /*  the element has been found  */
    else {
      *iloc_label = label_table[index].iloc;
      found = index;
    }
  }

  /*  if not found, add the label to the table  */
  if (label_table[index].Fortran == 0) {
    label_table[index].Fortran 	= fortran_label;
    *iloc_label 		= LABEL_NEW;
    label_table[index].iloc 	= *iloc_label;
    label_table[index].assigned = NOT_ASSIGNED;
    if (def_use_flag != LABEL_DEFINITION)
      label_table[index].defined = FALSE;
    else {
      label_table[index].defined   = TRUE;
      label_table[index].stmt_node = stmt;
    }
  }
  else if (def_use_flag == LABEL_DEFINITION) {
    if (label_table[index].defined == TRUE) {
      (void) sprintf(error_buffer, "duplicate definition for label '%d'",
		     fortran_label);
      ERROR("LabelTable", error_buffer, WARNING);
    }
    label_table[index].defined = TRUE;
    label_table[index].stmt_node = stmt;
  }

  if ((def_use_flag == LABEL_USE_IN_ASSIGN) &&
      (label_table[index].assigned == NOT_ASSIGNED)) {
    label_table[index].assigned 	= assigned_list_header;
    assigned_list_header		= index;
  }

  if (aiDebug > 0)
    (void) fprintf(stdout, " %d.\n", *iloc_label);

  /*  return whether or not the element was found  */
  return found;
} /* LabelGetInternal */



/************************************************************************/
/* Destroy the label table              				*/
/************************************************************************/
void LabelDestroyTable ()
{
  /*  check to insure that a label_table exists  */
  if (label_table == NULL)
    return;

  /*  free the table  */
  (void) free_mem ((void*) label_table);

  /*  reinitialize the label_table pointer to NULL  */
  label_table = NULL;
} /* LabelDestroyTable */



/************************************************************************/
/* print the label table                				*/
/************************************************************************/
void LabelDumpTable ()
{
  int index;				/*  index into the label table  */
  int need_header = TRUE;		/*  

  if (aiSymDump < 2) return;

  /*  check to insure that a label_table exists  */
  if (label_table == NULL)
    return;

  /*  traverse the table and print the contents of the elements  */
  for ( index = 0; index < label_table_dimension; index++)
    if (label_table[index].Fortran != 0) {
      if (need_header) {
	need_header = FALSE;
	(void) fprintf(stdout,
	       "Label Table\n\tFortran\tIloc\nindex\tLabel\tLabel\tNext\n");
      }

      (void) fprintf (stdout, "%d\t%d\t%d\t%d\n", index,
		      label_table[index].Fortran, label_table[index].iloc,
		      label_table[index].assigned );
    }
  if (need_header == FALSE)
    (void) fprintf(stdout, "\nAssigned list header:\t%d.\n",
		   assigned_list_header);
} /* LabelDumpTable */



/************************************************************************/
/*                                      				*/
/*  This routine takes a single argument, a pointer to an integer, and  */
/*  returns an integer.  To use it correctly, the first call should     */
/*  pass it a pointer to an integer already initialized to the manifest */
/*  constant START_OF_LIST (ai.h).  Successive calls should pass        */
/*  it the same pointer, since it stores its internal state in the      */
/*  integer.                                                            */
/*                                                                      */
/*  It returns, on successive calls, the complete list of iloc labels   */
/*  corresponding to statement labels that have appeared in assign      */
/*  statements in the current program sub-unit.  It denotes the end     */
/*  of this list by returning the manifest constant END_OF_LIST.        */
/*                                      				*/
/************************************************************************/
int LabelNextAssigned( int *state )
  // int *state;
{
  int result;
  
  if (*state == START_OF_LIST)
    *state = assigned_list_header;
  
  if (*state == aiEND_OF_LIST)
    result = aiEND_OF_LIST;
  else {
    result = label_table[*state].iloc;
    *state = label_table[*state].assigned;
  }
  return result;
} /* LabelNextAssigned */



/************************************************************************/
/* determines whether or not the label appeared in an ASSIGN statement  */
/* and returns the answer or indicates that the label table is full.    */
/************************************************************************/
int LabelAssigned (STR_TEXT label)
  // STR_TEXT	label;		/*  FORTRAN label from AST  */
{
  int fortran_label;		/*  FORTRAN label  */
  int index;			/*  label table index  */
  int found = FALSE;		/*  return value of the find function  */
  int initial_index;		/*  initial value of index  */
  int result;
  
  /*  check to insure that a table exists  */
  if (label_table == NULL)
    return LABEL_TABLE_IS_FULL;
  
  /*  find the index in the symbol table  */
  (void) sscanf (label, "%d", &fortran_label);
  index = fortran_label % label_table_dimension;
  initial_index = index;
  
  /*  search for the label  */
  /*  valid FORTRAN labels must be greater than zero  */
  while ((label_table[index].Fortran != 0) & !found)
    
    /*  if not found, move to the next element  */
    if (fortran_label != label_table[index].Fortran) {
      index = index ++ % label_table_dimension;
      
      /*  if this is true, the table is full  */
      if (index == initial_index) {
	found = LABEL_TABLE_IS_FULL;
      }
    }
  /*  the element has been found  */
    else
      found = TRUE;

  /*  if not found, add the label to the table  */
  if (!found)
    result = FALSE;
  else {
    if (label_table[index].assigned == NOT_ASSIGNED)
      result = FALSE;
    else
      result = TRUE;
  }
  
  if (aiDebug > 0)
    (void) fprintf(stdout, "LabelAssigned(%s) returns %d", label, result);
  
  return (result);
} /* LabelAssigned */



/************************************************************************/
/* simplified interface used to invoke LabelGetInternal to retrieve the */
/* AST node for the statement associated with the label.		*/
/************************************************************************/
AST_INDEX LabelGetNode( STR_TEXT label )

  // STR_TEXT label;	/* FORTRAN label from AST */
{
  int index, ilabel;
  
  index = LabelGetInternal(label, &ilabel, AST_NIL, LABEL_USE);
  
  return label_table[index].stmt_node;
} /* LabelGetNode */
