/* $Id: cfg_labels.h,v 3.5 1997/03/11 14:35:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 *  -- cfg_labels.h
 *
 *          functions for building and maintaining a map of statement labels 
 *          and AST nodes associated with them.
 *
 *
 * LabelMap void cfg_insert_label(oldLblMap, label, node)
 *   LabelMap oldLblMap;
 *   int label;
 *   AST_INDEX node;
 * 
 *     return new LabelMap containing oldLblMap plus entry for (label, node)
 *     if oldLblMap == NULL, returns new map with single entry (label, node)
 *     (duplicate labels are not allowed in same LabelMap)
 *
 *     pass AST_NIL for node to indicate that the label is used in an ASSIGN.
 *
 *
 * AST_INDEX cfg_lookup_label(lblMap, label)
 *   LabelMap lblMap;
 *   int label;
 *
 *     return node associated with label, if label appears in lblMap
 *     if lblMap doesn't contain an entry for label, return null_node
 *
 * Boolean cfg_label_assigned(lblMap, label)
 *   LabelMap lblMap;
 *   int label;
 *
 *     return true if label has occurred in an ASSIGN statement
 *
 * void cfg_kill_label_map(lblMap)
 *   LabelMap lblMap;
 *
 *     free space associated with lblMap
 *
 *
 * void cfg_dump_label_map(lblMap)
 *   LabelMap lblMap;
 *
 *     pretty print label map structure
 *
 */

#ifndef cfg_labels_h
#define cfg_labels_h


#include <libs/frontEnd/ast/ast.h>
#include <libs/support/tables/symtable.h>

typedef SymTable LabelMap;

EXTERN(LabelMap, cfg_insert_label, (LabelMap oldLblMap,
					    int label, AST_INDEX node));
EXTERN(AST_INDEX, cfg_lookup_label, (LabelMap lblMap, int label));
EXTERN(void, cfg_kill_label_map, (LabelMap lblMap));
EXTERN(void, cfg_dump_label_map, (LabelMap lblMap));
EXTERN(Boolean, cfg_label_assigned, (LabelMap lblMap, int label));

#define cfg_run_through_labels(map, func, extra) SymForAll(map, func, extra)
#define NODE_FIELD "astnode"
#define label_node(map, lblId) SymGetFieldByIndex(map, lblId, NODE_FIELD)

#define get_label_int(lbl_ref, i)  sscanf(gen_get_text(lbl_ref), "%d", &(i))

#endif /* !cfg_labels_h */
