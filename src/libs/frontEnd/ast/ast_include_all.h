/* $Id: ast_include_all.h,v 3.6 1997/03/11 14:29:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * -- ast_include.h
 */



#ifndef __ast_include_h__
#define __ast_include_h__


/* AST includes */
#include <libs/support/misc/general.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <libs/frontEnd/ast/ast.h>
#include <include/frontEnd/astcons.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <include/frontEnd/astsel.h>
#include <libs/frontEnd/ast/asttree.h>


/* an AST macro that ought to be */
#define get_AST_type_text(n) gen_node_type_get_text(gen_get_node_type(n))

/* 
 * If a stmt has a close label def, it's the second son.  
 * This is for function, program, do, block-data, subroutine, if, debug, 
 *     parallel,  do-all, and parallelloop nodes only 
 */
#define get_close_label(n) (ast_get_son2(n))

#endif /* ! __ast_include_h__ */
