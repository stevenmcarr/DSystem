/* $Id: MapInfo_c.h,v 1.6 1997/03/11 14:29:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************
	MapInfo_c.h -- provide access to the MapInfo object from a C file.

 ****************************************************************************/

#ifndef MapInfo_c_h
#define MapInfo_c_h

typedef void* MapInfoOpaque;  /* C pointer for a MapInfo object */

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif

#ifndef treeutil_h
#include <libs/frontEnd/ast/treeutil.h>
#endif


EXTERN(MapInfoOpaque, CreateMapInfo, (FortTextTree ftt));
EXTERN(void, DestroyMapInfo, (MapInfoOpaque map));

EXTERN(AST_INDEX, MapLineToIndex, (MapInfoOpaque map,int LineNum));
EXTERN(AST_INDEX, MapVarToIndex, (MapInfoOpaque map,int Line,char *Var,int n));
EXTERN(void, ProcessMapLine, (MapInfoOpaque map, FortTextTree ftt, int ftt_line,
			     int output_line));
EXTERN(void, OpaqueConstructInfo, (AST_INDEX node,
				  ReferenceAccessType atype,va_list arg_list));

#endif	/* MapInfo_c_h */

