/* $Id: ipdfi.h,v 1.6 1997/03/11 14:29:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ipdfi_h
#define ipdfi_h

#include <libs/support/database/context.h>
#ifndef newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif 

#define		LINKAGE_DEFAULT		0
#define 	LINKAGE_INLINE_USER	1
#define		LINKAGE_INLINE		2

typedef struct csite_info {
    int		linkage;	/* Inline substitute this call site? */
    int		ast_index;	/* ast index of callsite in this entry point */
    int		n_bound;	/* number of procedures bound to this callsite */
    char	**name;		/* array of names of procedures */
    int		*n_use;		/* array of number of vars used */
    char	***use;         /* use sets for each proc */
    int		*n_mod;		/* array of number of vars mod */
    char	***mod;         /* mod sets for each proc */
} csi;

typedef struct entry_ipdf_info {
    int		n_callers;
    char	**caller;
    int		n_alias;	/* number of alias pairs */
    char	***alias;	/* array of alias pairs */
    int		n_constant;
    char	***constant;	/* array of <constant, value> pairs */
    int		n_csite;	/* number of call sites in entry point */
    csi		*csite;		/* array of call site definitions */
} ipdfi;

/* 
 * ipdf_read_info (context, name):
 * 	Context	context; context of program
 *	char	*name; 		 procedure name in form <context>-<name>
 *
 * Reads file of interprocedural data flow info for a given procedure
 * specified by "name".
 */
EXTERN(ipdfi *, ipdf_read_info, (Context context, char *name));

/* 
 * ipdf_write_info (e, context, name):
 *	ipdfi	*e;	 structure to write out
 * 	Context  context; context of program
 *	char	*name; 		 procedure name in form <context>-<name>
 *
 * Write file of interprocedural data flow info from e for a given procedure
 * specified by "name".
 */
EXTERN(void, ipdf_write_info, (ipdfi *e, Context context, char *name));


/* 
 * ipdf_free (e):
 * 	ipdfi	*e; (pointer to ipdf information)
 *
 * Frees memory for ipdfi entry.
 */
EXTERN(void, ipdf_free, (ipdfi *e));


/* ipdf_get_callers (e, num_callers):
 *	ipdfi			*e;
 *	int			*num_callers;
 *
 * Given an entry data flow info ptr, and a pointer to an
 * integer, returns a pointer to an array of strings giving procedures
 * which invoke this procedure.
 */
EXTERN(char * *,ipdf_get_callers, (ipdfi *e, int *num_callers));

/* ipdf_set_linkage (e, csite, value):
 *	ipdfi			*e;
 *	int			csite;
 *	int			value;  index to type of linkage
 *
 * Given an entry data flow info ptr, a call site and a potential linkage,
 * sets the value of the linkage for this callsite.
 */
EXTERN(void, ipdf_set_linkage, (ipdfi *e, int csite, int value));


/* ipdf_get_linkage (e, csite):
 *	ipdfi			*e;
 *	int			csite;
 *
 * Given an entry data flow info ptr, and a call site, return the current
 * linkage for this callsite.
 */
EXTERN(int, ipdf_get_linkage, (ipdfi *e, int csite));

/* ipdf_get_csites (e, num_csites):
 *	ipdfi			*e;
 *	int			*num_csites;
 *
 * Given an entry data flow info ptr, and a pointer to an
 * integer, returns a pointer to an array of AST indices representing
 * all call sites within the procedure.
 */
EXTERN(int *, ipdf_get_csites, (ipdfi *e, int *num_csites));


/* ipdf_get_procs (e, csite, num_procs):
 *	ipdfi			*e;
 *	int			csite;
 *	int			*num_procs;
 *
 * Given an entry data flow info ptr, callsite index, and a pointer to an
 * integer, returns a pointer to an array of procedure names which may be
 * called at the callsite corresponding to the AST index "csite".  Here,
 * the names returned will be of the form <context>-<name>.
 */
EXTERN(char * *,ipdf_get_procs, (ipdfi *e, int csite, int *num_procs));

/* ipdf_get_aliases (e, num_aliases):
 *	ipdfi	*e;
 *	int	*num_aliases;
 *
 * Given an entry data flow info ptr and a pointer to an integer, returns a
 * pointer to an array of alias pairs for the entry, where alias pairs are
 * represented as an array of two strings.
 */
EXTERN(char ** *,ipdf_get_aliases, (ipdfi *e, int *num_aliases));

/* ipdf_get_constants (e, num_constants):
 *	ipdfi	*e;
 *	int	*num_constants;
 *
 * Given an entry data flow info ptr and a pointer to an integer, returns a
 * pointer to an array of <constant, value> pairs for the entry, where 
 * constants and their values are each represented as string.
 */
EXTERN(char ** *,ipdf_get_constants, (ipdfi *e, int *num_constants));

/* ipdf_get_uses (e, csite, name, num_uses):
 * 	ipdfi 	*e;
 * 	int	csite;
 * 	char	*name;
 *	int	*num_uses;
 *
 * Given an entry data flow info ptr, callsite index, procedure name, and a
 * pointer to an integer, returns a pointer to an array of variable names
 * representing variables used as a result of invoking at the callsite
 * corresponding to the AST index "csite" the procedure referred to by "name".
 * Here, name is of the form <context>-<name>, and the variable names will
 * be of the following form:
 *	$<common_name>.<offset>.<length>.<AST index of decl> for globals
 *	@<var_name>.<offset>.<length>.<AST index of decl> for local variables
 *	-<parameter number>.<AST index of decl>	for formal parameters
 */
EXTERN(char * *,ipdf_get_uses, (ipdfi *e, int csite, char *name, int *num_uses));


/* ipdf_get_mods (e, csite, name, num_mods):
 * 	ipdfi 	*e;
 * 	int	csite;
 * 	char	*name;
 *	int	*num_mods;
 *
 * Given an entry data flow info ptr, callsite index, procedure name, and a
 * pointer to an integer, returns a pointer to an array of variable names
 * representing variables modified as a result of invoking at the callsite
 * corresponding to the AST index "csite" the procedure referred to by "name".
 * Here, name is of the form <context>-<name>, and the variable names will
 * be of the following form:
 *	$<common_name>.<offset>.<length>.<AST index of decl> for globals
 *	$<var_name>.<offset>.<length>.<AST index of decl> for local variables
 *	-<parameter number>.<AST index of decl>	for formal parameters
 */
EXTERN(char * *,ipdf_get_mods, (ipdfi *e, int csite, char *name, int *num_mods));

#endif
