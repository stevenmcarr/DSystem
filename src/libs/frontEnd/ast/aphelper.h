/* $Id: aphelper.h,v 1.7 1997/03/11 14:29:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef aphelper_h
#define aphelper_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef astutil_h
#include <libs/frontEnd/ast/astutil.h>
#endif

EXTERN(AST_INDEX, place_holder_for, (AST_INDEX node));
EXTERN(AST_INDEX, in, (AST_INDEX node));
EXTERN(AST_INDEX, in_to_end, (AST_INDEX node));
EXTERN(AST_INDEX, out, (AST_INDEX node));
EXTERN(AST_INDEX, next, (AST_INDEX node_in_list));
EXTERN(AST_INDEX, prev, (AST_INDEX node_in_list));
EXTERN(AST_INDEX, ph_from_mtype, (Generic mtype));
EXTERN(AST_INDEX, ph_ify, (AST_INDEX node, TYPE ph_type, Generic ph_status));
EXTERN(Boolean, is_first_in_list, (AST_INDEX node_in_list));
EXTERN(Boolean, is_last_in_list, (AST_INDEX node_in_list));
EXTERN(Boolean, is_only_in_list, (AST_INDEX node_in_list));

EXTERN(Boolean, is_subpgm, (AST_INDEX node));

EXTERN(AST_INDEX, gen_no_meta_list, (AST_INDEX list, Generic status,
                                     META_TYPE type));
EXTERN(AST_INDEX, gen_no_meta_node, (AST_INDEX node, Generic status,
                                     META_TYPE type));

#endif
