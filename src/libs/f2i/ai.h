/* $Id: ai.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/* 
 * ai.h -
 *
 *  global definitions for the AST->iloc translator
 *
 */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif general_h
#include <libs/support/database/newdatabase.h>
#include <libs/support/database/context.h>

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/support/lists/list.h>

#include <libs/Memoria/annotate/DirectivesInclude.h>

  /* the various flags - initialized in main.c */
    extern    int aiAnnotate;	/* automatically generate comments	*/
    extern    int aiCheapGoto;	/* controls assigned goto generation	*/
    extern int aiConstants;	/* report on Constants?			*/
    extern int aiDebug;	/* level of debugging information	*/
    extern int aiEnregGlobals;	/* enable enregistering of common vars	*/
    extern int aiFatals;	/* continue after Fatal Error?		*/
    extern int aiGenerate;	/* print out iloc code?			*/
    extern int aiMessageId;	/* controls format of ERROR() messages	*/
    extern int aiSymDump;	/* report on symbol table?		*/
    extern int aiTreeDump;	/* dump the tree?			*/
    extern int aiTreeCheck;	/* run ft_Check() on the tree?		*/
    extern int aiVirtual;	/* report on virtual register use?	*/
    extern int aiNoAlias;	/* assume No Aliases exist		*/
    extern int aiParseComments;/* parse comments for directives        */
    extern int aiSymMap;	/* print out a storage map		*/
    extern int aiSparc;	/* compile with SPARC attributes	*/
    extern int aiRocket;	/* compile with Rocket naming	*/
    extern int aiRt;   	/* generate code for Rt                 */
                        /* so default is for itoc  (cij 8/6/92) */
    extern int aiCache; /* do cache reuse analysis */
    extern int aiLongIntegers; /* use 64-bit integers */
    extern int aiDoubleReals; /* use double precision only*/

    extern int aiAlignDoubles; /* set by various machine preferences	*/

  /* globally accessed variables */
    extern int  aiStmtCount;

    extern int  aiNextRegister;	/* vars related to storage mapping	*/
    extern int  aiNextLabel;
    extern int  aiNextStack;
    extern int  aiNextStatic;
    extern int  aiNumParameters;
    extern int  aiNumInstructions;
    extern int  aiEpilogue;		/* label of program epilogue		*/

    extern int  aiMaxVariables;	/* related to interprocedural annotations */
    extern int  aiNextCallSite;

    extern int  aiStackSize;
    extern int  aiExpressionStackSpace;
    extern int  aiNextReg;

    extern char 	*proc_name;
    extern char	*proc_text;
    extern char 	error_buffer[256];

    /* change 6/10/91
     * Added global variable 'root_node' to hold root of ast 
     * since aiProcedurePrologue needs it
     */
    extern AST_INDEX   root_node;
    extern AST_INDEX	formal_list;

  /* globally used routines */
    void ERROR();

  /* flags */
# define WARNING 1
# define SERIOUS 2
# define FATAL	3

# define TRUE	1
# define FALSE	0

# define NOCOMMENT	""
# define COMMENT	NOP

# define NO_TARGET	    -1
# define LABEL_DEFINITION    0
# define LABEL_USE           1
# define LABEL_USE_IN_ASSIGN 2

# define START_OF_LIST  -9089
# define aiEND_OF_LIST	-9090
# define NOT_ASSIGNED	-9191

# define MIN_REG	0
# define MAX_DATA_STMTS 256
# define MAX_PARMS	256

# define MAX_INITIAL_EXPRESSIONS 256

# define GEN_ALIGNMENT	-1

# define GEN_LABEL	16
# define GEN_NUMBER	17
# define GEN_STRING	18

/* to generate DATA statements */
# define DATA_UNTYPED	0
# define DATA_INTEGER	1
# define DATA_FLOAT	2
# define DATA_DOUBLE	3
# define DATA_COMPLEX	4
# define DATA_LABEL	5
# define DATA_STRING_LABEL 6
# define DATA_CHARACTER 7
# define DATA_LOGICAL   8
# define DATA_BYTE	9
# define DATA_DOUBLE_COMPLEX 10

/* types of variables in interprocedural information  */
#define	IDFA_GLOBAL	1
#define	IDFA_LOCAL	2
#define	IDFA_FORMAL	3

#define MAX_IDFA	64

int getIndex();

#define INDEX_NOT_MODIFIED -999
#define INDEX_MODIFIED      999

#define getIntConstantInRegister( i ) \
	getConstantInRegFromString( i, TYPE_INTEGER, TYPE_INTEGER )

#define GET_DIRECTIVE_INFO(N) \
        ((Directive *)ast_get_scratch(N))

#define PUT_DIRECTIVE_INFO(N,D) \
        ast_put_scratch(N,D)  

EXTERN(void,ai,(Context, FortTree, FortTextTree, char *));

typedef enum {UNDEFINED,NONE,SELF_TEMPORAL,GROUP_TEMPORAL,SELF_SPATIAL,
              GROUP_SPATIAL,SELF_TEMPORAL_CACHE,GROUP_TEMPORAL_CACHE} 
        LocalityType;


EXTERN(void, ut_GetSubscriptText, (AST_INDEX Node, char *Text));

typedef struct DepInfoStruct {
  int ReferenceNumber;
  UtilList *DependenceList;
  LocalityType Locality;
 } DepInfoType;         /* copy in mc_lib/annotate/CacheAnalysis.h */

typedef struct depstruct {
  int ReferenceNumber;
  char DType;
  int Distance;
 } DepStruct; /* copy in mc_lib/annotate/CacheAnalysis.h */

#define DepInfoPtr(n) \
   ((DepInfoType *)ast_get_scratch(n))

#define CreateDepInfoPtr(n) \
   ast_get_scratch(n,malloc(sizeof(DepInfoType)))

