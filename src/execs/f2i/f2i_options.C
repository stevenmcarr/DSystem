#include <stdlib.h>
#include <stdio.h>
#include <general.h>
#include <assert.h>
#include <string.h>

#include <context.h>
#include <newdatabase.h>

#include <misc/Options.h>
#include <a2i_options.h>
#include <ai.h>

char *a2i_program = NULL;
char *a2i_module = NULL;
Boolean a2i_ComputeInterproceduralInfo = false;

/* these decls are to allow linking with the memory compiler for cache
	analysis */
char *mc_program = NULL;
char *mc_module_list = NULL;
Boolean mc_unroll_cache = false;
Boolean mc_allow_expansion = false;
Boolean mc_extended_cache = false;
Boolean RestrictedUnrolling = false;
Boolean PartitionUnrollAmount = 0;
int ReplaceLevel = 0;
int DependenceLevel = 3;
int blue_color = 0;

 int aiAnnotate;   /* automatically generate comments      */
 int aiCheapGoto;  /* controls assigned goto generation    */
 int aiConstants;     /* report on Constants?                 */
 int aiDebug; /* level of debugging information       */
 int aiEnregGlobals;  /* enable enregistering of common vars  */
 int aiFatals;        /* continue after Fatal Error?          */
 int aiGenerate;      /* print out iloc code?                 */
 int aiMessageId;     /* controls format of ERROR() messages  */
 int aiSymDump;       /* report on symbol table?              */
 int aiTreeDump;      /* dump the tree?                       */
 int aiTreeCheck;     /* run ft_Check() on the tree?          */
 int aiVirtual;       /* report on virtual register use?      */
 int aiNoAlias;       /* assume No Aliases exist              */
 int aiParseComments;/* parse comments for directives        */
 int aiSymMap;        /* print out a storage map              */
 int aiSparc; /* compile with SPARC attributes        */
 int aiRocket; /* compile with Rocket naming        */
 int aiRt;    /* generate code for Rt                 */
                        /* so default is for itoc  (cij 8/6/92) */
 int aiCache; /* do cache reuse analysis */
 int aiLongIntegers; /* use 64-bit integers */
 int aiDoubleReals; /* use double-precision reals*/
 int aiAlignDoubles; /* set by various machine preferences    */

  /* globally accessed variables */
 int  aiStmtCount;

 int  aiNextRegister; /* vars related to storage mapping      */
 int  aiNextLabel;
 int  aiNextStack;
 int  aiNextStatic;
 int  aiNumParameters;
 int  aiNumInstructions;
 int  aiEpilogue;             /* label of program epilogue
*/

 int  aiMaxVariables; /* related to interprocedural annotations */
 int  aiNextCallSite;

 int  aiStackSize;
 int  aiExpressionStackSpace;
 int  aiNextReg;

 char         *proc_name;
 char *proc_text;
 char         error_buffer[256];

    /* change 6/10/91
     * Added global variable 'root_node' to hold root of ast
     * since aiProcedurePrologue needs it
     */
 AST_INDEX   root_node;
 AST_INDEX    formal_list;



void a2i_options_usage(char *pgm_name)
{
  fprintf(stderr, "Usage: %s [-aAcdgImprstvEFS] -M <module> [-P <program>]",pgm_name);
  puts(" ");
  puts("         -a  no comments");
  puts("         -A  assume no aliases exist");
  puts("         -c  run type checker on tree");
  puts("         -d  print out debugging information");
  puts("         -g  don't print out iloc code");
  puts("         -I  change error message format");
  puts("         -l  use 64-bit integers");
  puts("         -m  print out storage map");
  puts("         -n  no align doubles");
  puts("         -p  parse comments for directives");
  puts("         -r  generate code for RT");
  puts("         -s  dump symbol table");
  puts("         -t  dump ast");
  puts("         -v  report on virtual register usage");
  puts("         -E  enable enregistering of common variables");
  puts("         -F  continue after fatal error");
  puts("         -S  compile with Sparc attributes");
  puts("         -h  transmit cache analysis information");
  puts("         -M  Fortran module");
  puts("         -P  program composition");
  fflush(stdout);
  exit(-1);
}

static void a2i_opt_Annotate(void *state)
{
 aiAnnotate = 0;
}

static void a2i_opt_AlignDoubles(void *state)
{
 aiAlignDoubles = 0;
}

static void a2i_opt_Constants(void *state)
{
 aiConstants++;
}

static void a2i_opt_Debug(void *state)
{
 aiDebug++;
}

static void a2i_opt_EnregGlobals(void *state)
{
 aiEnregGlobals++;
}

static void a2i_opt_Fatals(void *state)
{
 aiFatals = 0;
}

static void a2i_opt_Generate(void *state)
{
 aiGenerate++;
}

static void a2i_opt_MessageId(void *state)
{
 aiMessageId++;
}

static void a2i_opt_NoAlias(void *state)
{
 aiNoAlias++;
}

static void a2i_opt_ParseComments(void *state)
{
 aiParseComments++;
}

static void a2i_opt_SymDump(void *state)
{
 aiSymDump++;
}

static void a2i_opt_TreeCheck(void *state)
{
 aiTreeCheck++;
}

static void a2i_opt_TreeDump(void *state)
{
 aiTreeDump++;
}

static void a2i_opt_Virtual(void *state)
{
 aiVirtual++;
}

static void a2i_opt_SymMap(void *state)
{
 aiSymMap++;
}

static void a2i_opt_Sparc(void *state)
{
 aiSparc++;
}

static void a2i_opt_Rocket(void *state)
{
 aiRocket++;
}

static void a2i_opt_Rt(void *state)
{
 aiRt++;
}

static void a2i_opt_Cache(void *state)
  {
   aiCache++;
  }

static void a2i_opt_LongIntegers(void *state)
  {
   aiLongIntegers++;
  }

static void a2i_opt_DoubleReals(void *state)
  {
   aiDoubleReals++;
  }

static void a2i_opt_program(void *state, char *str)
{
  a2i_program = str;
  a2i_ComputeInterproceduralInfo = true;
}

static void a2i_opt_module(void *state, char *str)
{
  a2i_module = str;
}


static struct flag_	Annotate_f = {
  a2i_opt_Annotate,
  "Annotate flag",	
  " ",
};

static struct flag_	AlignDoubles_f = {
  a2i_opt_AlignDoubles,
  "AlignDoubles flag",	
  " ",
};

static struct flag_	Constants_f = {
  a2i_opt_Constants,
  "Constants flag",	
  " ",
};

static struct flag_	Cache_f = {
  a2i_opt_Cache,
  "Cache analysis flag",	
  " ",
};

static struct flag_	Debug_f = {
  a2i_opt_Debug,
  "Debug flag",	
  " ",
};

static struct flag_	EnregGlobals_f = {
  a2i_opt_EnregGlobals,
  "EnregGlobals flag",	
  " ",
};

static struct flag_	Fatals_f = {
  a2i_opt_Fatals,
  "Fatals flag",	
  " ",
};

static struct flag_	Generate_f = {
  a2i_opt_Generate,
  "Generate flag",	
  " ",
};

static struct flag_	LongIntegers_f = {
  a2i_opt_LongIntegers,
  "64-bit integers flag",	
  " ",
};

static struct flag_	DoubleReals_f = {
  a2i_opt_DoubleReals,
  "all double-precision reals flag",	
  " ",
};

static struct flag_	MessageId_f = {
  a2i_opt_MessageId,
  "MessageId flag",	
  " ",
};

static struct flag_	NoAlias_f = {
  a2i_opt_NoAlias,
  "NoAlias flag",	
  " ",
};

static struct flag_	ParseComments_f = {
  a2i_opt_ParseComments,
  "ParseComments flag",	
  " ",
};
static struct flag_	SymDump_f = {
  a2i_opt_SymDump,
  "SymDump flag",	
  " ",
};

static struct flag_	TreeCheck_f = {
  a2i_opt_TreeCheck,
  "TreeCheck flag",	
  " ",
};

static struct flag_	TreeDump_f = {
  a2i_opt_TreeDump,
  "TreeDump flag",	
  " ",
};

static struct flag_	SymMap_f = {
  a2i_opt_SymMap,
  "SymMap flag",	
  " ",
};

static struct flag_	Rocket_f = {
  a2i_opt_Rocket,
  "Rocket flag",	
  " ",
};

static struct flag_	Sparc_f = {
  a2i_opt_Sparc,
  "Sparc flag",	
  " ",
};

static struct flag_	Rt_f = {
  a2i_opt_Rt,
  "Rt flag",	
  " ",
};

static struct flag_	Virtual_f = {
  a2i_opt_Virtual,
  "Virtual flag",	
  " ",
};

static struct string_	program_s = {
  a2i_opt_program,
  "program context     ",
  "program context",
  512, 50,
  ".*"
};

static struct string_	module_s = {
  a2i_opt_module,
  "module context     ",
  "module context",
  512, 50,
  ".*"
};


Option a2i_pgm_opt, 
       a2i_mod_opt, 
       a2i_Annotate_flag, 
       a2i_AlignDoubles_flag, 
       a2i_Constants_flag, 
       a2i_Cache_flag, 
       a2i_Debug_flag, 
       a2i_EnregGlobals_flag, 
       a2i_Fatals_flag, 
       a2i_Generate_flag, 
       a2i_LongIntegers_flag, 
       a2i_DoubleReals_flag, 
       a2i_MessageId_flag, 
       a2i_NoAlias_flag, 
       a2i_ParseComments_flag, 
       a2i_SymDump_flag, 
       a2i_TreeCheck_flag, 
       a2i_TreeDump_flag, 
       a2i_Virtual_flag, 
       a2i_SymMap_flag, 
       a2i_Sparc_flag, 
       a2i_Rocket_flag, 
       a2i_Rt_flag; 

void a2i_init_options(Options &opts)
{
  aiAnnotate  		= 1;
  aiAlignDoubles	= 1;
  aiConstants 		= 0;
  aiCache 		= 0;
  aiLongIntegers	= 0;
  aiDoubleReals    	= 0;
  aiDebug		= 0;
  aiEnregGlobals	= 0;
  aiFatals		= 1;
  aiGenerate  		= 0;
  aiMessageId 		= 0;
  aiNoAlias   		= 0;
  aiParseComments 	= 0;
  aiSymDump   		= 0;
  aiTreeCheck 		= 0;
  aiTreeDump		= 0;
  aiVirtual		= 0;
  aiSymMap		= 0;
  aiSparc		= 0;
  aiRocket		= 0;
  aiRt                  = 0;
  a2i_pgm_opt.t =  string; 
  a2i_pgm_opt.arg_char =  A2I_PGM_OPT;
  a2i_pgm_opt.init_value =  (Generic) "";
  a2i_pgm_opt.in_dialog =  true;
  a2i_pgm_opt.f_c =  (Generic)&program_s;
  opts.Add(&a2i_pgm_opt);
  a2i_mod_opt.t =  string;
  a2i_mod_opt.arg_char =  A2I_MOD_OPT;
  a2i_mod_opt.init_value =  (Generic) "";
  a2i_mod_opt.in_dialog =  true;
  a2i_mod_opt.f_c =  (Generic)&module_s;
  opts.Add(&a2i_mod_opt);
  a2i_Annotate_flag.t =  flag;
  a2i_Annotate_flag.arg_char =  A2I_ANNOTATE_FLAG;
  a2i_Annotate_flag.init_value =  (Generic)false;
  a2i_Annotate_flag.in_dialog =  true; 
  a2i_Annotate_flag.f_c =  (Generic)&Annotate_f;
  opts.Add(&a2i_Annotate_flag);
  a2i_AlignDoubles_flag.t =  flag;
  a2i_AlignDoubles_flag.arg_char =  A2I_ALIGNDOUBLES_FLAG;
  a2i_AlignDoubles_flag.init_value =  (Generic)false;
  a2i_AlignDoubles_flag.in_dialog =  true; 
  a2i_AlignDoubles_flag.f_c =  (Generic)&AlignDoubles_f;
  opts.Add(&a2i_AlignDoubles_flag);
  a2i_Constants_flag.t =  flag;
  a2i_Constants_flag.arg_char =  A2I_CONSTANTS_FLAG;
  a2i_Constants_flag.init_value =  (Generic)false;
  a2i_Constants_flag.in_dialog =  true;
  a2i_Constants_flag.f_c =  (Generic)&Constants_f;
  opts.Add(&a2i_Constants_flag);
  a2i_Cache_flag.t =  flag;
  a2i_Cache_flag.arg_char =  A2I_CACHE_FLAG;
  a2i_Cache_flag.init_value =  (Generic)false;
  a2i_Cache_flag.in_dialog =  true;
  a2i_Cache_flag.f_c =  (Generic)&Cache_f;
  opts.Add(&a2i_Cache_flag);
  a2i_Debug_flag.t =  flag;
  a2i_Debug_flag.arg_char =  A2I_DEBUG_FLAG;
  a2i_Debug_flag.init_value =  (Generic)false;
  a2i_Debug_flag.in_dialog =  true;
  a2i_Debug_flag.f_c =  (Generic)&Debug_f;
  opts.Add(&a2i_Debug_flag);
  a2i_EnregGlobals_flag.t =  flag;
  a2i_EnregGlobals_flag.arg_char =  A2I_ENREGGLOBALS_FLAG;
  a2i_EnregGlobals_flag.init_value =  (Generic)false;
  a2i_EnregGlobals_flag.in_dialog =  true;
  a2i_EnregGlobals_flag.f_c =  (Generic)&EnregGlobals_f;
  opts.Add(&a2i_EnregGlobals_flag);
  a2i_Fatals_flag.t =  flag;
  a2i_Fatals_flag.arg_char =  A2I_FATALS_FLAG;
  a2i_Fatals_flag.init_value =  (Generic)false;
  a2i_Fatals_flag.in_dialog =  true;
  a2i_Fatals_flag.f_c =  (Generic)&Fatals_f;
  opts.Add(&a2i_Fatals_flag);
  a2i_Generate_flag.t =  flag;
  a2i_Generate_flag.arg_char =  A2I_GENERATE_FLAG;
  a2i_Generate_flag.init_value =  (Generic)false;
  a2i_Generate_flag.in_dialog =  true;
  a2i_Generate_flag.f_c =  (Generic)&Generate_f;
  opts.Add(&a2i_Generate_flag);
  a2i_LongIntegers_flag.t =  flag;
  a2i_LongIntegers_flag.arg_char =  A2I_LONGINTEGERS_FLAG;
  a2i_LongIntegers_flag.init_value =  (Generic)false;
  a2i_LongIntegers_flag.in_dialog =  true;
  a2i_LongIntegers_flag.f_c =  (Generic)&LongIntegers_f;
  opts.Add(&a2i_LongIntegers_flag);
  a2i_DoubleReals_flag.t =  flag;
  a2i_DoubleReals_flag.arg_char =  A2I_DOUBLEREALS_FLAG;
  a2i_DoubleReals_flag.init_value =  (Generic)false;
  a2i_DoubleReals_flag.in_dialog =  true;
  a2i_DoubleReals_flag.f_c =  (Generic)&DoubleReals_f;
  opts.Add(&a2i_DoubleReals_flag);
  a2i_MessageId_flag.t =  flag;
  a2i_MessageId_flag.arg_char =  A2I_MESSAGEID_FLAG;
  a2i_MessageId_flag.init_value =  (Generic)false;
  a2i_MessageId_flag.in_dialog =  true;
  a2i_MessageId_flag.f_c =  (Generic)&MessageId_f;
  opts.Add(&a2i_MessageId_flag);
  a2i_NoAlias_flag.t =  flag;
  a2i_NoAlias_flag.arg_char =  A2I_NOALIAS_FLAG;
  a2i_NoAlias_flag.init_value =  (Generic)false;
  a2i_NoAlias_flag.in_dialog =  true;
  a2i_NoAlias_flag.f_c =  (Generic)&NoAlias_f;
  opts.Add(&a2i_NoAlias_flag);
  a2i_ParseComments_flag.t =  flag;
  a2i_ParseComments_flag.arg_char =  A2I_PARSECOMMENTS_FLAG;
  a2i_ParseComments_flag.init_value =  (Generic)false;
  a2i_ParseComments_flag.in_dialog =  true;
  a2i_ParseComments_flag.f_c =  (Generic)&ParseComments_f;
  opts.Add(&a2i_ParseComments_flag);
  a2i_SymDump_flag.t =  flag;
  a2i_SymDump_flag.arg_char =  A2I_SYMDUMP_FLAG;
  a2i_SymDump_flag.init_value =  (Generic)false;
  a2i_SymDump_flag.in_dialog =  true;
  a2i_SymDump_flag.f_c =  (Generic)&SymDump_f;
  opts.Add(&a2i_SymDump_flag);
  a2i_TreeCheck_flag.t =  flag;
  a2i_TreeCheck_flag.arg_char =  A2I_TREECHECK_FLAG;
  a2i_TreeCheck_flag.init_value =  (Generic)false;
  a2i_TreeCheck_flag.in_dialog =  true;
  a2i_TreeCheck_flag.f_c =  (Generic)&TreeCheck_f;
  opts.Add(&a2i_TreeCheck_flag);
  a2i_TreeDump_flag.t =  flag;
  a2i_TreeDump_flag.arg_char =  A2I_TREEDUMP_FLAG;
  a2i_TreeDump_flag.init_value =  (Generic)false;
  a2i_TreeDump_flag.in_dialog =  true;
  a2i_TreeDump_flag.f_c =  (Generic)&TreeDump_f;
  opts.Add(&a2i_TreeDump_flag);
  a2i_Virtual_flag.t =  flag;
  a2i_Virtual_flag.arg_char =  A2I_VIRTUAL_FLAG;
  a2i_Virtual_flag.init_value =  (Generic)false;
  a2i_Virtual_flag.in_dialog =  true;
  a2i_Virtual_flag.f_c =  (Generic)&Virtual_f;
  opts.Add(&a2i_Virtual_flag);
  a2i_SymMap_flag.t =  flag;
  a2i_SymMap_flag.arg_char =  A2I_SYMMAP_FLAG;
  a2i_SymMap_flag.init_value =  (Generic)false;
  a2i_SymMap_flag.in_dialog =  true;
  a2i_SymMap_flag.f_c =  (Generic)&SymMap_f;
  opts.Add(&a2i_SymMap_flag);
  a2i_Sparc_flag.t =  flag;
  a2i_Sparc_flag.arg_char =  A2I_SPARC_FLAG;
  a2i_Sparc_flag.init_value =  (Generic)false;
  a2i_Sparc_flag.in_dialog =  true;
  a2i_Sparc_flag.f_c =  (Generic)&Sparc_f;
  opts.Add(&a2i_Sparc_flag);
  a2i_Rocket_flag.t =  flag;
  a2i_Rocket_flag.arg_char =  A2I_ROCKET_FLAG;
  a2i_Rocket_flag.init_value =  (Generic)false;
  a2i_Rocket_flag.in_dialog =  true;
  a2i_Rocket_flag.f_c =  (Generic)&Rocket_f;
  opts.Add(&a2i_Rocket_flag);
  a2i_Rt_flag.t =  flag;
  a2i_Rt_flag.arg_char =  A2I_RT_FLAG;
  a2i_Rt_flag.init_value =  (Generic)false;
  a2i_Rt_flag.in_dialog =  true;
  a2i_Rt_flag.f_c =  (Generic)&Rt_f;
  opts.Add(&a2i_Rt_flag);
}
