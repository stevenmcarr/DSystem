/* $Id: cfg_labels.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_labels.c
 *
 *        functions for building and maintaining a map of statement labels and
 *        AST nodes associated with them.
 *
 *
 * LabelMap cfg_insert_label(old_lbl_map, label, node)
 *   LabelMap old_lbl_map;
 *   int label;
 *   AST_INDEX node;
 * 
 *     return new label_map containing old_lbl_map plus entry for (label, node)
 *     if old_lbl_map == NULL, returns new map with single entry (label, node)
 *     (duplicate labels are not allowed in same LabelMap)
 *
 *
 * AST_INDEX cfg_lookup_label(lbl_map, label)
 *   LabelMap lbl_map;
 *   int label;
 *
 *     return node associated with label, if label appears in LabelMap
 *     if LabelMap doesn't contain an entry for label, return null_node
 *
 *
 * AST_INDEX cfg_run_through_labels(lbl_map)
 *   LabelMap lbl_map;
 *
 *     return each defined label (in some order), ending list with null_node
 *
 *
 * void cfg_kill_label_map(lbl_map)
 *   LabelMap lbl_map;
 *
 *     free space associated with lbl_map
 *
 *
 * void cfg_dump_label_map(lbl_map)
 *   LabelMap lbl_map;
 *
 *     pretty print label map structure
 *
 */


#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/cfg_labels.h>

#define MAP_SIZE 0x20
#define ERR_REDEF_LABEL 1
#define ERR_LBL_NOT_FOUND 2
#define ASSIGNED_FIELD "Boolean: assigned"

STATIC(void, error, (int err, int msg));

/*
 *  add (label, node) to LabelMap old
 *	if node is AST_NIL, mark label as involved in ASSIGN
 */
LabelMap cfg_insert_label(LabelMap old, int lbl, AST_INDEX node)
{
    int id;
    char lblText[10];	/* actually, labels limited to 5 digits by standard */

    (void) sprintf(lblText, "%d", lbl);

    if (!old) {
	old = SymInit(MAP_SIZE);
	SymInitField(old, NODE_FIELD, AST_NIL,
		     (SymCleanupFunc) 0);
	SymInitField(old, ASSIGNED_FIELD, false,
		     (SymCleanupFunc) 0);
    }

    if (node == AST_NIL)
    {
	/*
	 *  We encountered an ASSIGN of this label to a variable
	 */
	id = SymIndex(old, lblText);
	SymPutFieldByIndex(old, id, ASSIGNED_FIELD, true);
	return old;
    }

    if (SymQueryIndex(old, lblText) != SYM_INVALID_INDEX)
    {
	id = SymIndex(old, lblText);

	if (SymGetFieldByIndex(old, id, NODE_FIELD) == AST_NIL)
	{
	    /*
	     *  Only involved in an ASSIGN, not really re-used
	     */
	    SymPutFieldByIndex(old, id, NODE_FIELD, node);
	}
	else
	{
	    /*
	     *  Reuse of statement label
	     */
	    error(lbl, ERR_REDEF_LABEL);
	}
    }
    else 
    {
	id = SymIndex(old, lblText);
	SymPutFieldByIndex(old, id, NODE_FIELD, node);
    }

    return old;
}

/*
 *  Report whether label has occurred in an ASSIGN statement
 */
Boolean cfg_label_assigned(LabelMap map, int lbl)
{
    char lblText[10];	/* actually, labels limited to 5 digits by standard */
    (void) sprintf(lblText, "%d", lbl);

    return (Boolean) SymGetField(map, lblText, ASSIGNED_FIELD);
}


/*
 *  return ast node for stmt identified by lbl
 */
AST_INDEX cfg_lookup_label(LabelMap map, int lbl)
{
    char lblText[10];	/* actually, labels limited to 5 digits by standard */
    int id;

    (void) sprintf(lblText, "%d", lbl);

    id = SymQueryIndex(map, lblText);

    if (id == SYM_INVALID_INDEX) {
        error(lbl, ERR_LBL_NOT_FOUND);
        return AST_NIL;
    }
    else
	return (AST_INDEX) SymGetFieldByIndex(map, id, NODE_FIELD);
}

void cfg_kill_label_map(LabelMap map)
{
    if (!map)  return;

    SymKill(map);
}

static void error(int err, int msg)
{
    switch (msg) {
        case ERR_REDEF_LABEL:
            fprintf(stderr, "Error: Label %d redefined.\n", err);
            break;
        case ERR_LBL_NOT_FOUND:
            fprintf(stderr, "Error: Label %d not found.\n", err);
            break;
    }
}

static void label_print(LabelMap map, int id, Generic junk)
{
    AST_INDEX t = SymGetFieldByIndex(map, id, NODE_FIELD);

    printf("(%d, %d - %s)\n",
	   SymGetFieldByIndex(map, id, SYM_NAME_FIELD),
	   t, get_AST_type_text(t));
}

void cfg_dump_label_map(LabelMap map)
{
    if (!map) {
        printf("No labels in label map\n");
        return;
    }

    printf("Stmt labels and nodes:  (label, astnode - nodetype)\n");

    SymForAll(map, label_print, 0);
}





