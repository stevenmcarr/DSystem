/* $Id: ai.h,v 1.15 1999/07/22 18:06:37 carr Exp $ */
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
#endif 
#include <stdlib.h>
#include <string.h>
#include <libs/support/database/newdatabase.h>
#include <libs/support/database/context.h>

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/support/lists/list.h>

#include <libs/Memoria/annotate/DirectivesInclude.h>
#include <libs/Memoria/include/memory_menu.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/Memoria/include/ASTToIntMap.h>

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
    extern int aiGeneratePrefetches; /* gen code for fetch and flush */
    extern int aiSymMap;	/* print out a storage map		*/
    extern int aiSparc;	/* compile with SPARC attributes	*/
    extern int aiRocket;	/* compile with Rocket naming	*/
    extern int aiRt;   	/* generate code for Rt                 */
                        /* so default is for itoc  (cij 8/6/92) */
    extern int aiOptimizeAddressCode; /* utilize non-zero offsets in 
					 address generation */
    extern int aiCache; /* do cache reuse analysis */
    extern int aiSpecialCache; /* make self spatial gs leaders cache misses */
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


    extern ASTToIntMap *ASTRegMap;

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

typedef enum {UNDEFINED,NONE,SELF_TEMPORAL,GROUP_TEMPORAL,SELF_SPATIAL,
              GROUP_SPATIAL,SELF_TEMPORAL_CACHE,GROUP_TEMPORAL_CACHE} 
        LocalityType;

typedef struct DepInfoStruct {
  int ReferenceNumber;
  UtilList *DependenceList;
  LocalityType Locality;
  Boolean UsePrefetchingLoad;
  AST_INDEX AddressLeader;
  AST_INDEX FirstInLoop;
  int       Offset;
  int       StmtNumber;
  int       PrefetchDistance;
  AST_INDEX PrefetchOffsetAST;
 } DepInfoType;         /* copy in Memoria/annotate/CacheAnalysis.h */

typedef struct depstruct {
  AST_INDEX Reference;
  int ReferenceNumber;
  char DType;
  int Distance;
 } DepStruct; /* copy in mc_lib/annotate/CacheAnalysis.h */

#define DepInfoPtr(n) \
   ((DepInfoType *)ast_get_scratch(n))

#define CreateDepInfoPtr(n) \
   ast_get_scratch(n,malloc(sizeof(DepInfoType)))

     /* addr.ansi.c */
EXTERN(int, getSubscriptLValue,(AST_INDEX ));
EXTERN(int, getValueInReg,(int ));
EXTERN(int, AddressFromNode,(AST_INDEX ));
EXTERN(char *, BaseAddressName,(int ));
EXTERN(int, BaseIndex,(int ));

     /* ai.ansi.c */
EXTERN(void, ai,(Context  , FortTree  , FortTextTree  , char * ));

     /* assign.ansi.c */
EXTERN(void, HandleAssign,(AST_INDEX ));

     /* assignment.ansi.c */
EXTERN(void, HandleAssignment,(AST_INDEX ));
struct  CharDesc;
EXTERN(void,evalCharExpr,(AST_INDEX  , struct CharDesc [] , int *  , int ));

     /* calls.ansi.c */
EXTERN(void,HandleCall,(AST_INDEX ));
EXTERN(int,HandleInvocation,(AST_INDEX ));
EXTERN(int,InlineStmtFunc,(AST_INDEX ));
EXTERN(void,HandleStatementFunction,(AST_INDEX ));

     /* chars.ansi.c */
EXTERN( void, generate_mvcl,(int  , int  , int ));
EXTERN( void, generate_fill,(int  , int  , int ));
EXTERN( int, generate_char_compare,(int  , int  , int  , int  , int ));
EXTERN( void, generate_index,(int  , int  , int  , int  , int ));

     /* common.ansi.c */
EXTERN(void, CommonDumpTable,(SymDescriptor,fst_index_t,Generic));
EXTERN(char * ,aiGetCommonBlockName,(fst_index_t  , char * ));
EXTERN(void ,aiGenerateCommon,(SymDescriptor  , fst_index_t  , Generic ));
EXTERN(void ,aiGenerateCommons,(void));
EXTERN(void ,MarkGlobalName,(SymDescriptor  , fst_index_t  , Generic ));
EXTERN(int *, aiGlobalNames,(char *  , int  , int ));

     /* data.ansi.c */
EXTERN(void,setIndex,(AST_INDEX ));
EXTERN(int,string_length,(int ));
EXTERN(int,SizeOfTypewIndex,(int ));
EXTERN(void,HandleData,(AST_INDEX ));
EXTERN(int,evalExpr,(AST_INDEX ));
EXTERN(char *,evalConvert,(char *  , int  , int ));
EXTERN(void,printVal,(AST_INDEX *  , int  , int  , int  , FILE * ));
EXTERN(void,evalAddress,(AST_INDEX  , AST_INDEX *  , FILE * ));
EXTERN(void,ProcessData,(FILE * ));

     /* directives.ansi.c */
EXTERN(void, HandleDirective,(AST_INDEX ));

     /* do.ansi.c */
EXTERN(int,get_increment,(AST_INDEX  , int  , int *  , int * ));
EXTERN(void,HandleInductiveDo,(AST_INDEX ));
EXTERN(int,typed_arithmetic,(int  , int ));
EXTERN(void,HandleRepetitiveDo,(AST_INDEX ));
EXTERN(void,HandleConditionalDo,(AST_INDEX ));

     /* equiv.ansi.c */
struct  equiv_struct;
EXTERN(int *,aiEquivClass,(int ));
EXTERN(void,Equivalence,(AST_INDEX ));
EXTERN(void,ProcessEquivalences,(void));
EXTERN(void,AnEquivalence,(AST_INDEX ));
EXTERN(void,EquivEltList,(AST_INDEX ));
EXTERN(void,find_leader,(struct equiv_struct * ));

     /* expr.ansi.c */
EXTERN(int,getExprInReg,(AST_INDEX ));
EXTERN(int,getConstantRegister,(AST_INDEX ));
EXTERN(int,getIdInRegister,(AST_INDEX ));
EXTERN(int,binaryOp,(AST_INDEX ));
EXTERN(int,getConstantInRegFromString,(char *  , int  , int ));
EXTERN(int,getConversion,(int  , int ));
EXTERN(int,getConstantInRegFromIndex,(int  , int ));
EXTERN(int,getConstantInRegFromInt,(int ));
EXTERN(int,Table2,(int  , int ));
EXTERN(int,ArithOp,(int  , int ));

     /* expr2.ansi.c */
struct  CallTemplate;
EXTERN(char *, add_to_regs_list,(char *  , int ));
EXTERN(char *, add_to_tags_list,(char *  , int ));
EXTERN(void, GenerateCall,(struct CallTemplate * ));

     /* expr3.ansi.c */
EXTERN(int,index_by_type,(int ));
EXTERN(int,relOp,(AST_INDEX  , int ));
EXTERN(int,booleanOp,(AST_INDEX ));
EXTERN(int,HandleUnaryNot,(AST_INDEX ));

     /* exps.ansi.c */
EXTERN(int,HandleExponent,(AST_INDEX ));

     /* gen.ansi.c */
EXTERN(char*, GenDepComment,(AST_INDEX));
EXTERN(char*,GenDepCommentForStmt,(Directive*));
EXTERN(void,generate,(int, int, Generic, Generic, Generic, char*));
EXTERN(void,generate_store,( int, int, int, int, char*));
EXTERN(void,generate_move,( int, int, int));
EXTERN(void, generate_branch,(int, int, int, int, int, int, int, char*));
EXTERN(void,generate_string,(char*, int, Generic, Generic, Generic, char*));
EXTERN(void,generate_load,(int, int, int, int, char*));
EXTERN(void,generate_pfload,(int, int, int, AST_INDEX, int, int, char*));
EXTERN(void, generate_cache_op,(int, int, Directive *));
EXTERN(void,PrintData,(int  , int  , int  , int ));
EXTERN(void,generate_long,(int, int, Generic, Generic, Generic, 
			   Generic,Generic,Generic, Generic, char * ));
EXTERN(int,convert_opcode,(int  , int ));
EXTERN(int,getAlignment,(int ));
EXTERN(char *,getTag,(int ));
EXTERN(void,PrintString,(char *  , int ));
EXTERN(void,PrintFloat,(char *  , int ));
EXTERN(void,PrintDouble,(char *  , int ));
EXTERN(char *,Prefix,(char * ));

     /* get.ansi.c */
EXTERN(int, getIndex,(AST_INDEX));
EXTERN(int, getIntConstantIndex,(int ));
EXTERN(char *,getLocality,(AST_INDEX ));

     /* goto.ansi.c */
EXTERN(void,HandleGoto,(AST_INDEX ));
EXTERN(void,HandleComputedGoto,(AST_INDEX ));
EXTERN(void,HandleAssignedGoto,(AST_INDEX ));

     /* idfa.ansi.c */
EXTERN(void,aiGetAllGlobals,(void));
EXTERN(void,aiIdfaFini,(void));
EXTERN(void,aiIdfaAdd,(int  , int * ));
EXTERN(int,aiNameIsUse,(SymDescriptor  , AST_INDEX  , int ));
EXTERN(int *,aiGlobalUses,(AST_INDEX ));
EXTERN(int,aiNameIsMod,(SymDescriptor  , AST_INDEX  , int ));
EXTERN(int *,aiGlobalMods,(AST_INDEX ));
EXTERN(int,IsAliased,(int  , Boolean ));
EXTERN(char *,aiNameIsConstant,(int ));
EXTERN(char *,aiNameIsConstantOnReturn,(AST_INDEX  , int ));

     /* if.ansi.c */
EXTERN(void,HandleIf,(AST_INDEX ));
EXTERN(void,ExtendIf,(AST_INDEX  , int ));
EXTERN(void,HandleLogicalIf,(AST_INDEX ));
EXTERN(void,HandleArithmeticIf,(AST_INDEX ));

     /* initials.ansi.c */
EXTERN(void,RecordInitialExp,(int  , AST_INDEX ));
EXTERN(void,GenerateInitialExps,(void));

     /* intrins.ansi.c */
EXTERN(int,HandleIntrinsic,(AST_INDEX ));
EXTERN(int,generate_len,(AST_INDEX ));

     /* io.ansi.c */
EXTERN(void,DoIO,(int  , AST_INDEX ));
EXTERN(void,GenerateFormat,(AST_INDEX ));
EXTERN(char *,GenerateArrayDecl,(int ));
EXTERN(void,BuildIOL,(AST_INDEX  , AST_INDEX  , AST_INDEX  , int  , int ));
EXTERN(int,getIOLActuals,(void));
EXTERN(void,HandlePause,(AST_INDEX ));
EXTERN(void,HandleStop,(AST_INDEX ));

     /* label.ansi.c */
EXTERN(void, LabelGet,(STR_TEXT,int*));
EXTERN(void,LabelCreateTable,(int ));
EXTERN(void,LabelDestroyTable,(void));
EXTERN(int,LabelDefine,(STR_TEXT  , int *  , AST_INDEX ));
EXTERN(void,LabelInAssign,(STR_TEXT  , int * ));
EXTERN(void,LabelDumpTable,(void));
EXTERN(int,LabelNextAssigned,(int * ));
EXTERN(int,LabelAssigned,(STR_TEXT ));
EXTERN(AST_INDEX,LabelGetNode,(STR_TEXT ));

     /* map.ansi.c */
EXTERN(void,MapStorage,(AST_INDEX ));
EXTERN(int,VarSize,(int ));
EXTERN(int,IsValidName,(char * ));
EXTERN(int,is_double,(int ));
EXTERN(int,PadToAlignment,(int  , int ));
EXTERN(int,Align,(int ));

     /* memos.ansi.c */
EXTERN(int,aiStaticLabel,(void));
EXTERN(void,aiRecordStaticLabel,(char * ));
EXTERN(int,aiStackBase,(void));
EXTERN(int,aiFunctionValueIndex,(void));
EXTERN(void,aiRecordFunctionValueIndex,(int ));

     /* mnemonics.ansi.c */
EXTERN(void,iloc_rep_init,(void));
EXTERN(int,iloc_rep_version,(void));
EXTERN(char *,iloc_mnemonic,(int ));
EXTERN(int,iloc_format,(int ));
EXTERN(int,iloc_opcode,(char * ));
EXTERN(char *,format_name,(int ));
EXTERN(int,iloc_intrinsic,(char * ));
EXTERN(int,iloc_op_commutes,(int ));

     /* params.ansi.c */
EXTERN(void,MarkParameters,(AST_INDEX ));
EXTERN(void,aiParameters,(AST_INDEX ));

     /* postmap.ansi.c */
EXTERN(void,aiPostMap,(AST_INDEX ));

     /* prepass.ansi.c */
EXTERN(void,aiPrePass,(AST_INDEX ));
EXTERN(void,WalkCommon,(AST_INDEX ));
EXTERN(void,convertToReal,(AST_INDEX ));
EXTERN(void,convertToInt,(AST_INDEX ));

     /* procs.ansi.c */
EXTERN(void, aiProcedures,(AST_INDEX,FortTree));
EXTERN(void,aiFormals,(AST_INDEX ));
EXTERN(int,aiFormalName,(int ));
EXTERN(void,aiExit,(int  , char * ));

     /* regs.ansi.c */
EXTERN(int,getAddressRegister,(int ));
EXTERN(int,getAddressInRegister,(int ));
EXTERN(int,getFunctionAddressInReg,(int ));

     /* routine.ansi.c */
EXTERN(void,aiProcedurePrologue,(AST_INDEX ));
EXTERN(void,aiProcedureEpilogue,(AST_INDEX  , AST_INDEX  , int ));
EXTERN(void,aiLoadUpStuff,(void));

     /* runtime.ansi.c */
EXTERN(void,aiRunTimeError,(char *  , int ));
EXTERN(void,aiPause,(int  , int ));

     /* static.ansi.c */
EXTERN(void,aiGenerateStaticArea,(void));
EXTERN(void,OutputData,(FILE * ));
EXTERN(void,PadAndPut,(int  , FILE * ));
EXTERN(int,DataFlagFromIType,(int ));

     /* stmts.ansi.c */
EXTERN(Boolean, ai_isConstantExpr,(AST_INDEX));
EXTERN(Boolean, isParameterExpr,(AST_INDEX));
EXTERN(Boolean, aiDirectiveIsInComment,(AST_INDEX));
EXTERN(Boolean, aiStatementIsPrefetch,(AST_INDEX));
EXTERN(Boolean, aiStatementIsFlush,(AST_INDEX));
EXTERN(void,aiStmtList,(AST_INDEX ));
EXTERN(int,ai_isExecutable,(int ));
EXTERN(void,ArrayDeclLenList,(AST_INDEX  , int  , int ));
EXTERN(void,HandleSave,(AST_INDEX ));
EXTERN(int,ArrayDeclLen,(AST_INDEX  , int  , int  , int ));
EXTERN(int,IntConstant,(char * ));

     /* string2.ansi.c */
struct  CharDesc;
EXTERN(void,generate_move_string,(struct CharDesc *  , struct CharDesc * ));
EXTERN(int,getStringLengthIntoReg,(AST_INDEX ));
EXTERN(int,getSubstringAddress,(AST_INDEX ));
EXTERN(int,getSubstringLength,(AST_INDEX ));
EXTERN(void,AddStringLengthRegister,(int  , int ));
EXTERN(int,NewStringLength,(AST_INDEX ));
EXTERN(void,createStackTarget,(struct CharDesc [2] , int ));

     /* strings.ansi.c */
EXTERN(int,StringLength,(int ));
EXTERN(int,QuotedLength,(char * ));

     /* sym.ansi.c */
EXTERN(void, InitSymTab,(SymDescriptor,fst_index_t,Generic));
EXTERN(int, SymInsertSymbol,(char *, int, int, int, int, int));
EXTERN(int,getIndexForlb,(ArrayBound *  , int  , fst_index_t ));
EXTERN(int,getIndexForub,(ArrayBound *  , int  , fst_index_t ));
EXTERN(void,SymZeroScratchField,(SymDescriptor  , fst_index_t  , Generic ));
EXTERN(void,InitFieldsInSymTab,(SymDescriptor  , fst_index_t  , Generic ));
EXTERN(char *,decodeObjectClass,(register int ));
EXTERN(char *,decodeStorageClass,(register int ));
EXTERN(char *,decodeType,(register int ));
EXTERN(void,SymPromoteStorageClass,(int  , int ));
EXTERN(void,SymInsertData,(int  , int  , int  , int  , int  , int ));
EXTERN(void,Sym_Dim_Dump,(SymDescriptor  , fst_index_t  , Generic ));
EXTERN(void,Short_Dump,(SymDescriptor  , fst_index_t  , Generic ));
EXTERN(void,Sym_Dump_Equivalences,(SymDescriptor  , fst_index_t  , Generic ));
EXTERN(void,fst_Short_Dump,(void));

     /* utility.ansi.c */
EXTERN(void, ERROR,(char *, char *, int));
EXTERN(char*,NameFromConstantNode,(AST_INDEX,char*));
EXTERN(char*,ConstantName, (char *, int, char *));
EXTERN(char *,get_num,(char*,int*));
EXTERN(void, UNIMPL,(int ));
EXTERN(int,ATypeToIType,(register int ));
EXTERN(int,ITypeToAType,(register int ));
EXTERN(char *,TypeName,(int ));
EXTERN(int,SizeOfType,(int ));
EXTERN(int,TempReg,(int  , int  , int  , int ));
EXTERN(int,StrTempReg,(char *  , int  , int ));
EXTERN(char *,ConstantNameFromIndex,(int ));
EXTERN(int ,GetDataSize,(int));

     /* from libs/Memoria/dr/mh_walk.C  */
EXTERN(void, ApplyMemoryCompiler,(int,PedInfo,AST_INDEX,FortTree,Context,char *));

     /* from libs/Memoria/ut/mem_util.C  */
EXTERN(void, ut_GetSubscriptText,(AST_INDEX,char*,SymDescriptor symtab = NULL));
