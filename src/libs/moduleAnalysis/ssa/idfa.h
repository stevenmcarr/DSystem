/* $Id: idfa.h,v 3.5 1997/03/11 14:36:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * -- idfa.h
 *
 *       The interface routines to the interprocedural data flow 
 *       information kept in the database.  (After all, this is 
 *       part of the Rn programming environment.)
 *
 */

#ifndef _idfa_h_
#define _idfa_h_

#define	NO_INFO	-1



/*
 *  In all of the below, "cfg" is the CfgInstance, "site" is an AST_INDEX...
 *
 *	If "site" is INVOCATION, answer regards access by the given 
 *		call site or function invocation to an external subprogram.
 *
 *	If "site" is PROGRAM, FUNCTION, PROCEDURE, or ENTRY, answer regards 
 *		access by call to this subprogram via the given entry point.
 *
 *	If "site" is NULL, answer regards access via any entry point to 
 *		this subprogram (denoted by "cfg").
 *
 * Boolean idfaNameIsMod(cfg, site, index)
 *		- index is the variable's Fortran symbol table index
 *
 *		returns true  <=> variable may be MODIFIED by call
 *
 * Boolean idfaNameIsRef(cfg, site, index)
 *		- index is the variable's Fortran symbol table index
 *
 *		returns true  <=> variable may be REFERENCED (read) by call
 *
 * Boolean idfaNameIsAcc(cfg, site, index)
 *		- index is the variable's Fortran symbol table index
 *
 *		returns true  <=> variable may be ACCESSED (ref or mod)
 *
 * Boolean idfaNameIsConst(cfg, site, index)
 * int idfaNameGetConst(cfg, site, index)
 *		- index is the variable's integer symbol table index
 *
 *		idfaNameIsConst returns true if constant on entry or
 *		return (as appropriate to "site"); idfaNameGetConst returns 
 *		the constant value -- only if the corresponding IsConst
 *		function returned true.
 */

EXTERN( Boolean, idfaInit, (CfgInstance cfg));

EXTERN( void, idfaFini, (CfgInstance cfg) );

EXTERN( Boolean, idfaIsPure, (CfgInstance cfg, AST_INDEX site) );

EXTERN( Boolean, idfaNameIsMod, (CfgInstance cfg, 
				 AST_INDEX site, fst_index_t id) );

EXTERN( Boolean, idfaNameIsRef, (CfgInstance cfg,
				 AST_INDEX site, fst_index_t id) );

EXTERN( Boolean, idfaNameIsAcc, (CfgInstance cfg, 
				 AST_INDEX site, fst_index_t id) );

EXTERN( Boolean, idfaArgIsMod, (CfgInstance cfg, 
				AST_INDEX site, AST_INDEX arg) );

EXTERN( Boolean, idfaArgIsRef, (CfgInstance cfg, 
				AST_INDEX site, AST_INDEX arg) );

EXTERN( Boolean, idfaArgIsAcc, (CfgInstance cfg, 
				AST_INDEX site, AST_INDEX arg) );

EXTERN( Boolean, idfaHiddenMods, (CfgInstance cfg, AST_INDEX site) );
EXTERN( Boolean, idfaHiddenRefs, (CfgInstance cfg, AST_INDEX site) );
EXTERN( Boolean, idfaHiddenIo,   (CfgInstance cfg, AST_INDEX site) );

EXTERN( Boolean, idfaNameIsConst,	(CfgInstance cfg, 
					 AST_INDEX site, 
					 fst_index_t id) );

EXTERN( int, idfaNameGetConst,	        (CfgInstance cfg, 
					 AST_INDEX site, 
					 fst_index_t id) );
#endif /* ! _idfa_h_ */
