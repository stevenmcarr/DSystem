/* $Id: equiv.C,v 1.3 1998/04/29 13:00:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 *  ignore all the nonsense involving high and low. ... kdc
 *
 *  WARNING: renovation in progress.  This is a hard hat area.   ... kdc
 */

#include <libs/support/misc/general.h>
#include <libs/support/strings/rn_string.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/gen.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>

static void compute_size(AST_INDEX, int*, int*);
static void CheckConflict(void);

#ifndef OSF1
extern "C" void bzero(void *s, size_t n);
#endif

/* An implementation of the equivalence handling algorithm given in
 * "Principles of Compiler Design" by Aho and Ullman
 */

/* A couple of fields to watch throughout the code 
 *
 * fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_EQ_OFFSET)
 *
 *  (1) initialized down in find_leader on 1st appearance
 *  (2) changed whenever a leader becomes a follower 
 *    or
 *  (3) changed when the tree is flattened
 *
 * similarly, each symbol's low and high fields are
 *
 *  (1) initialed in find_leader on 1st appearance
 *  (2) changed when the symbol is a leader and a 
 *	new symbol is added to its set
 *
 *
 */


struct equiv_struct /* info about the two obj's being equiv'ed	*/
{
  AST_INDEX	node;		/* pointer to the equiv'd item		    */
  int		name;		/*  - its symbol table index		    */
  int		leader;		/*  - its leader			    */
  int		offset;		/*  - distance from origin of obj to	    */
				/*    origin of its leader		    */
  int		height;		/*  - # links to its leader		    */
  int		high;		/*  - upper bound of array rel. to start    */
  int		common;		/*  - common table index of set's anchor    */
  int		equiv_pt;	/*  - offset of equivalence from origin	    */
  int		storage_class;	/*  - its s.c. field 			    */
};

static struct equiv_struct objs[2];

/* externals */

struct EQList
{
  AST_INDEX 	 stmt;
  struct EQList *next;
};

static struct EQList *EquivList = (struct EQList *) 0;

void Equivalence(AST_INDEX stmt)
  // AST_INDEX stmt;
{
  struct EQList *p;

  p = (struct EQList *) get_mem(sizeof(struct EQList), "Equivalence EQList");
  
  p->stmt   = stmt;
  p->next   = EquivList;
  EquivList = p;

  SymHaveSeenAnEquivalence++;
}

void ProcessEquivalences()
{
  struct EQList *p;
  struct EQList *q;

 /*   SetCommonLeaders();	*/
  p = EquivList;
  while (p != (struct EQList *) 0)
  {
    AnEquivalence(p->stmt);
    q = p;
    p = p->next;
    free_mem((void*) q);
  }
  EquivList = (struct EQList *) 0;
}

void AnEquivalence(AST_INDEX stmt)
  // AST_INDEX stmt;
{
  register AST_INDEX list;

  if (aiDebug > 0)
     (void) fprintf(stdout, "Equivalence( %d ).\n", stmt);

  list = list_first(gen_EQUIVALENCE_get_equiv_elt_LIST(stmt));
  while (list != ast_null_node)
  {
    EquivEltList( list );
    list = list_next(list);
  }
}

void EquivEltList( AST_INDEX node )
  // AST_INDEX node;
{
  AST_INDEX list;
  int PARENT, S_PARENT, CHILD, S_CHILD, P_DIST, C_DIST;

  if (aiDebug > 0)
     (void) fprintf(stdout, "EquivEltList( %d ).\n", node);

  /* We iterate through the list, considering pairs of names.
   * Each pair consists of the first name in the list and one 
   * other name.  
   */
  list = list_first(gen_EQUIV_ELT_get_lvalue_LIST(node));

  if (list != ast_null_node)
  {
    objs[0].node = list;	
    list = list_next(list);

    while (list != ast_null_node)
    {
      find_leader(&objs[0]); /* not loop invariant! */

      objs[1].node   = list;

      find_leader(&objs[1]);

      /* some error checking */
      if (objs[0].common != 0 && objs[1].common != 0)
      {
        if (objs[0].common != objs[1].common)
	   ERROR("Equivalence", "Attempt to equivalence two common blocks",
		 FATAL);
        else /* same common block */
	   CheckConflict();
      }
      else if (objs[0].leader == objs[1].leader) /* same equiv class */
	 CheckConflict();
      else
    { /* indentation is off! */

      /* merge the sets */
      if (objs[0].height >= objs[1].height)
      {
	PARENT = 0; S_PARENT = objs[0].leader; /* changes find_leader(0)*/
	CHILD  = 1; S_CHILD  = objs[1].leader;
      }
      else
      {
	PARENT = 1; S_PARENT = objs[1].leader;
	CHILD  = 0; S_CHILD  = objs[0].leader;
      }
       /* *_DIST is the distance of the equivalence point from the
	* origin of the leader.
	* It is used to compute the equivalence point of the added
	* data item from its new leader.  It is also used to update
	* the low and high fields of the newly aggregated group.
	*/
      P_DIST = objs[PARENT].offset + objs[PARENT].equiv_pt;
      C_DIST = objs[CHILD].offset  + objs[CHILD].equiv_pt;

      /* fix up the parental relationships */
      fst_my_PutFieldByIndex(ft_SymTable, S_CHILD, SYMTAB_PARENT,  S_PARENT);
      fst_my_PutFieldByIndex(ft_SymTable, S_CHILD, SYMTAB_EQ_OFFSET,  P_DIST - C_DIST);

      if (objs[0].common != 0)
      {
	  fst_my_PutFieldByIndex(ft_SymTable, S_PARENT, SYMTAB_common_name,  objs[0].common);
	  fst_my_PutFieldByIndex(ft_SymTable, S_PARENT, SYMTAB_STORAGE_CLASS,  objs[0].storage_class);
	  fst_my_PutFieldByIndex(ft_SymTable, objs[1].name, SYMTAB_common_name,  objs[0].common);
	  fst_my_PutFieldByIndex(ft_SymTable, objs[1].name, SYMTAB_STORAGE_CLASS,  objs[0].storage_class);
      }
      else if (objs[1].common != 0)
      {
	  fst_my_PutFieldByIndex(ft_SymTable, S_PARENT, SYMTAB_common_name,  objs[1].common);
	  fst_my_PutFieldByIndex(ft_SymTable, S_PARENT, SYMTAB_STORAGE_CLASS,  objs[1].storage_class);
	  fst_my_PutFieldByIndex(ft_SymTable, objs[0].name, SYMTAB_common_name,  objs[1].common);
	  fst_my_PutFieldByIndex(ft_SymTable, objs[0].name, SYMTAB_STORAGE_CLASS,  objs[1].storage_class);
      }
    } /* remember that indentation is off by one level */

      if (aiDebug > 0)
      {
         (void) fprintf(stderr, 
	  "'%s' [%d,%d] set %d bytes from '%s' [%d,%d] (leader '%s' [%d,%d]).\n",
	  (char *) fst_my_GetFieldByIndex(ft_SymTable, S_CHILD, SYMTAB_NAME), fst_my_GetFieldByIndex(ft_SymTable, S_CHILD, SYMTAB_low), fst_my_GetFieldByIndex(ft_SymTable, S_CHILD, SYMTAB_high),
	  fst_my_GetFieldByIndex(ft_SymTable, S_CHILD, SYMTAB_EQ_OFFSET), (char *) fst_my_GetFieldByIndex(ft_SymTable, S_PARENT, SYMTAB_NAME), 
	  fst_my_GetFieldByIndex(ft_SymTable, S_PARENT, SYMTAB_low), fst_my_GetFieldByIndex(ft_SymTable, S_PARENT, SYMTAB_high),
	  (char *) fst_my_GetFieldByIndex(ft_SymTable, objs[PARENT].leader, SYMTAB_NAME), fst_my_GetFieldByIndex(ft_SymTable, objs[PARENT].leader, SYMTAB_low),
	  fst_my_GetFieldByIndex(ft_SymTable, objs[PARENT].leader, SYMTAB_high));
      }
      list = list_next(list);
    }
  }
}

void find_leader( struct equiv_struct *obj )
  // struct equiv_struct *obj;
{
  int type, leader, temp;
  AST_INDEX node_name;

  type  = gen_get_node_type(obj->node);
  switch(type)
  {
      case GEN_IDENTIFIER: 
		node_name = obj->node;
		break;
      case GEN_SUBSCRIPT:
		node_name = gen_SUBSCRIPT_get_name(obj->node);
		break;
      case GEN_SUBSTRING:
		if (gen_get_node_type(gen_SUBSTRING_get_substring_name(obj->node))
		    == GEN_SUBSCRIPT)
		  node_name = gen_SUBSCRIPT_get_name(gen_SUBSTRING_get_substring_name(obj->node));
		else
		  node_name = gen_SUBSTRING_get_substring_name(obj->node);
		break;
      default:
		ERROR("Equivalence", "Unexpected node type", FATAL);
		break;
  }

  leader      = getIndex(node_name); /* initializes if necessary */
  obj->name   = leader;
  obj->height = 1;
  obj->offset = 0;

  obj->common 	     = fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_common_name);
  obj->storage_class = fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_STORAGE_CLASS);

  compute_size(obj->node, &obj->high, &obj->equiv_pt);

  if (fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_high) == fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_low)) 
  { /* first appearance in an EQUIVALENCE */
    fst_my_PutFieldByIndex(ft_SymTable, leader, SYMTAB_low,  0);
    fst_my_PutFieldByIndex(ft_SymTable, leader, SYMTAB_high,  obj->high);
    fst_my_PutFieldByIndex(ft_SymTable, leader, SYMTAB_EQ_OFFSET,  0); /* initialize eq_offset */
  }

  temp = fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_PARENT);

  /* now run up the tree to a leader */
  while(temp != -1) /* never entered for a new node! */
  {
    obj->offset += fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_EQ_OFFSET);
    obj->height ++;			/* used to pick leader above	*/

    if (fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_EQ_OFFSET) > 0) /* extend the high field */
       obj->high += fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_EQ_OFFSET);

    if (fst_my_GetFieldByIndex(ft_SymTable, temp, SYMTAB_common_name) !=0)	/* adjust for common membership	*/
    {
      if (obj->common != 0 && fst_my_GetFieldByIndex(ft_SymTable, temp, SYMTAB_common_name) != obj->common)
	 ERROR("Equivalence", "Two common blocks equivalenced", FATAL);
      else if (obj->common == 0)
      {
	obj->common = fst_my_GetFieldByIndex(ft_SymTable, temp, SYMTAB_common_name);
	obj->storage_class = fst_my_GetFieldByIndex(ft_SymTable, temp, SYMTAB_STORAGE_CLASS);
      }
    }
    leader = temp;			/* and advance the pointers	*/
    temp   = fst_my_GetFieldByIndex(ft_SymTable, leader, SYMTAB_PARENT);
  }
  obj->leader = leader;
}

/* compute_size() */
static void compute_size( AST_INDEX node, int *high, int *eq_pt)
//   AST_INDEX 	node;
//   int		*high;  /* its upper bound (#elts * size)		*/
//   int		*eq_pt;/* distance from equiv pt to low pt		*/
{
  int Index, size, dims, current_index, cum_index, i;
  AST_INDEX identifier;
  AST_INDEX subscript;
  AST_INDEX list;
  ArrayBound *bounds;

  int elements = 1;

  /* cheap kludge for fortran - assume number of dimensions <= 7 */
  int ubound[7], lbound[7];

  i = gen_get_node_type(node);
  switch(i)
  {
    case GEN_IDENTIFIER:
	identifier = node;
	subscript  = ast_null_node;
	break;

    case GEN_SUBSCRIPT:
	identifier = gen_SUBSCRIPT_get_name(node);
	subscript  = gen_SUBSCRIPT_get_rvalue_LIST(node);
	break;

    case GEN_SUBSTRING:
	identifier = gen_SUBSTRING_get_substring_name(node);
	subscript  = ast_null_node;
	break;

    default:
	ERROR("Equivalence(CS)", "Invalid variable name", FATAL);
	break;
  }

  Index = getIndex(identifier);
  dims  = fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NUM_DIMS);

  int ftype = fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE);
  switch (ftype)
  {
    case TYPE_CHARACTER:
    case TYPE_LOGICAL:
    case TYPE_INTEGER:	
    case TYPE_LABEL:
    case TYPE_REAL:	
    case TYPE_DOUBLE_PRECISION:
    case TYPE_COMPLEX:	
    case TYPE_DOUBLE_COMPLEX:
      size = GetDataSize(ftype);
      break;
    default:
	ERROR("Equivalence(ComputeSize)", "Unexpected data type encountered",
	      WARNING);
	break;
  }

  *eq_pt = 0;  /* eq_pt starts out as a zero. 			*/
	       /* changes only for subscripted array references	*/
  
  if (dims != 0)
  {
    /* get the raw information about array's dimensions */
    bounds = (ArrayBound *) fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_DIM_BOUNDS);

    for (i = 0; i<dims; i++)
    {
      elements = elements * (bounds[i].ub.value.const_val - bounds[i].lb.value.const_val + 1);
    }

    if (subscript != ast_null_node) /* scope out offset from the front */
    {
      list = list_last(subscript);
      cum_index = 0;
      while(list != ast_null_node)
      {
	if (ai_isConstantExpr(list) == false)
	{
	  ERROR("Equivalence", 
		"Non-constant subscript expression in equivalence statement",
		SERIOUS);
	  current_index = 0;
	}
	else 
	   current_index = evalExpr(list);

	cum_index +=  current_index - 1;
	if (dims > 1)
	   cum_index = cum_index * (ubound[dims-1] - lbound[dims-1] + 1);
	dims--;
	list = list_prev(list);
      }

      if (dims != 0)
	 ERROR("Equivalence", "Array reference doesn't use full dimension", 
		FATAL);

      *eq_pt = cum_index * size; /* and adjust the equivalence point */
    }
  }  
  *high = size * elements;

  if (aiDebug > 1)
     (void) fprintf(stdout, 
	     "ComputeSize(%s [%d]) %d elts of size %d => eq pt %d.\n",
	     (char *) fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NAME), node, elements, size, *eq_pt);
}




/*ARGSUSED*/
/* and the code to invert the equivalence map ...
 * used to interpret the interprocedural information
 * produced by the program compiler
 *
 */
static int *EqNTable = (int *) 0;
static int *LTable   = (int *) 0;
static int *EqNext   = (int *) 0;

static void InvertEqMap(SymDescriptor SymTab, fst_index_t i, Generic dummy)
  // SymDescriptor SymTab;
  // fst_index_t i;
  // Generic dummy;
{
  register int  j;
  
  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_DATA &&
      IsValidName((char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME)) == 1)
    { /* only look at identifiers declared in the FORTRAN program */
      j = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_PARENT);
      if (j > 0)			/* recall, ai_SymTable[0] is the SP */
	{					/* so, parent can't be 0 ...	 */
	  while(fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_PARENT) != -1)
	    j = fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_PARENT);
	  LTable[i] = j;			/* Set it's leader table entry	*/
	  EqNext[i] = EqNext[j];		/* and add it to class' chain	*/
	  EqNext[j] = i;			/* from leader's EqNext slot    */
	  
	  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) != 
	      fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_STORAGE_CLASS))
	    {
	      (void) sprintf(error_buffer, 
	        "TROUBLE - '%s' [%d] and '%s' [%d] have different storage classes (%d, %d)",
		 (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME), i,
		 (char *) fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_NAME), j,
	         fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS), 
		 fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_STORAGE_CLASS));
	      ERROR("InvertEqMap", error_buffer, SERIOUS);
	      ERROR("InvertEqMap", "Indicates a logic problem", FATAL);
	    }
	}
      else if (j < 1)			/* It's a leader or a singleton	*/
	LTable[i] = i;
    }
}


int *aiEquivClass(int i)	/* map a ai_SymTable index into a NameTable that	*/
  //  int i;		/* contains any equivalence'd names		*/
{
  register int j, k, lb, ub;

  if (SymHaveSeenAnEquivalence == 0)
  { /* set up a one slot name table */
    if (EqNTable == (int *) 0)
       EqNTable = (int *) get_mem(sizeof(int)+sizeof(int), "EqNTable");

    EqNTable[0] = 1;	/* without EQUIVALENCE statements, each name	*/
    EqNTable[1] = i;	/* is alone in its class			*/

    return EqNTable;
  }

  /* only reach here if we've seen EQUIVALENCE statements */
  if (EqNTable == (int *) 0)
  {
    j        = sizeof(int) * fst_MaxIndex(ft_SymTable);
    EqNTable = (int *) get_mem(j, "EqNTable");
    LTable   = (int *) get_mem(j, "LTable");
    EqNext   = (int *) get_mem(j, "EqNext");

    bzero((char *) LTable, j);
    bzero((char *) EqNext, j);

    fst_ForAll (ft_SymTable, InvertEqMap, 0);
  }

  j = 0;
  k = LTable[i];

  if (k == 0)
  {
    (void) sprintf(error_buffer,
		   "TROUBLE - leader table for '%s' [%d] is zero!", 
		   (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME),i);
    ERROR("aiEquivClass", error_buffer, SERIOUS);
    ERROR("aiEquivClass", "Indicates a fatal logic problem", FATAL);
  }

  lb = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_EQ_OFFSET);
  ub = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE) + lb;

  while (k != 0) 	/* should always get one trip */
  {
    /* test for overlap ... i's lb < k's hb && k's lb < i's hb	*/
    /* 		assumes non-zero length for i and k		*/
    if (fst_my_GetFieldByIndex(ft_SymTable, k, SYMTAB_EQ_OFFSET) < ub && 
	lb < fst_my_GetFieldByIndex(ft_SymTable, k, SYMTAB_EQ_OFFSET) + fst_my_GetFieldByIndex(ft_SymTable, k, SYMTAB_SIZE))
    { /* They overlap, so add it to the list */
      j++;
      EqNTable[j] = k;
    }
    k = EqNext[k];
  }
  EqNTable[0] = j;

  if (j == 0)
  {
    (void) sprintf(error_buffer, 
	           "TROUBLE - equivalence class fpr '%s' [%d] is empty", 
		   (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME), i);
    ERROR("aiEquivClass", error_buffer, SERIOUS);
    ERROR("aiEquivClass", "Indicates a fatal logic problem", FATAL);
  }
  return EqNTable;
}

static void CheckConflict()
{
  if (objs[0].offset + objs[0].equiv_pt == 
      objs[1].offset + objs[1].equiv_pt)
  {
    (void) sprintf(error_buffer, 
	    "Equivalence between '%s' and '%s' is redundant",
	    (char *) fst_my_GetFieldByIndex(ft_SymTable, objs[0].name, SYMTAB_NAME), (char *) fst_my_GetFieldByIndex(ft_SymTable, objs[1].name, SYMTAB_NAME));
    ERROR("CheckConflict", error_buffer, WARNING);
  }
  else
  {
    (void) sprintf(error_buffer,
	    "Inconsistent equivalences between '%s' and '%s'",
	     (char *) fst_my_GetFieldByIndex(ft_SymTable, objs[0].name, SYMTAB_NAME), (char *) fst_my_GetFieldByIndex(ft_SymTable, objs[1].name, SYMTAB_NAME));
    ERROR("CheckConflict", error_buffer, SERIOUS);
  }
}
