/* $Id: f2i_options.C,v 1.17 2002/02/20 16:19:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdlib.h>
#include <libs/support/misc/general.h>
#include <assert.h>
#include <string.h>
#include <iostream.h>

#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>

#include <libs/support/optParsing/Options.h>
#include <execs/f2i/f2i_options.h>
#include <libs/f2i/ai.h>

char *f2i_program = NULL;
char *f2i_module = NULL;
char *f2i_module_list = NULL;
Boolean f2i_ComputeInterproceduralInfo = false;

/* these decls are to allow linking with the memory compiler for cache
	analysis */

int  selection;
char *mc_program = NULL;
char *mc_module_list = NULL;
char *mc_module = NULL;
Boolean mc_unroll_cache = false;
Boolean mc_allow_expansion = false;
Boolean mc_extended_cache = false;
Boolean RestrictedUnrolling = false;
Boolean Memoria_LetRocketSchedulePrefetches = false;
Boolean Memoria_IssueDead = false;
Boolean Memoria_ConservativeSelfSpatial = false;
int PartitionUnrollAmount = 0;
int ReplaceLevel = 0;
int DependenceLevel = 3;
int StatsLevel = 0;
int blue_color = 0;
Boolean ReuseModelDebugFlag = false;
Boolean DDGDebugFlag = false;
Boolean MinDistDebugFlag = false;
Boolean CheckRecurrencesForPrefetching = false;
Boolean GeneratePreLoop = true;

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
 int aiParseComments; /* parse comments for directives        */
 int aiSymMap;        /* print out a storage map              */
 int aiSparc; /* compile with SPARC attributes        */
 int aiRocket; /* compile with Rocket naming        */
 int aiRt;    /* generate code for Rt                 */
                        /* so default is for itoc  (cij 8/6/92) */
 int aiOptimizeAddressCode; /* utilize non-zero offsets in address generation */
 int aiCache; /* do cache reuse analysis */
 int aiSpecialCache; /* do cache reuse analysis, self-spatial refs that are
		        leaders of a group-spatial set are given no reuse */
 int aiLongIntegers; /* use 64-bit integers */
 int aiDoubleReals; /* use double-precision reals*/
 int aiAlignDoubles; /* set by various machine preferences    */

 int aiGenClusters;

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

 int  aiCurrentCluster;

 char         *proc_name;
 char *proc_text;
 char         error_buffer[256];

    /* change 6/10/91
     * Added global variable 'root_node' to hold root of ast
     * since aiProcedurePrologue needs it
     */
 AST_INDEX   root_node;
 AST_INDEX    formal_list;



void f2i_options_usage(char *pgm_name)
{
  cerr << "Usage: " << pgm_name << "%s [-aAcdgImprstvEFS] -M <module> [-P <program>]"
       << endl << endl;
  cerr << "\t-a  no comments" << endl;
  cerr << "\t-A  assume no aliases exist" << endl;
  cerr << "\t-c  run type checker on tree" << endl;
  cerr << "\t-d  print out debugging information" << endl;
  cerr << "\t-D  double precision reals only" << endl;
  cerr << "\t-E  enable enregistering of common variables" << endl;
  cerr << "\t-F  continue after fatal error" << endl;
  cerr << "\t-g  don't print out iloc code" << endl;
  cerr << "\t-h  transmit cache analysis information" << endl;
  cerr << "\t-H  transmit cache analysis w/ special self-spatial analysis" << endl;
  cerr << "\t-I  change error message format" << endl;
  cerr << "\t-l  use 64-bit integers" << endl;
  cerr << "\t-m  print out storage map" << endl;
  cerr << "\t-M  Fortran module" << endl;
  cerr << "\t-n  no align doubles" << endl;
  cerr << "\t-o  optimize address arithmetic in loops" << endl;
  cerr << "\t-p  parse comments for directives" << endl;
  cerr << "\t-P  program composition" << endl;
  cerr << "\t-r  generate code for RT" << endl;
  cerr << "\t-s  dump symbol table" << endl;
  cerr << "\t-S  compile with Sparc attributes" << endl;
  cerr << "\t-t  dump ast" << endl;
  cerr << "\t-v  report on virtual register usage" << endl;
  cerr << "\t-X#  set debug flags" << endl;
  cerr << "\t-Z  annote ILOC with cluster information" << endl;

  exit(-1);
}

static void f2i_opt_Annotate(void *state)
{
 aiAnnotate = 0;
}

static void f2i_opt_AlignDoubles(void *state)
{
 aiAlignDoubles = 0;
}

static void f2i_opt_Constants(void *state)
{
 aiConstants++;
}

static void f2i_opt_Debug(void *state)
{
 aiDebug++;
}

static void f2i_opt_EnregGlobals(void *state)
{
 aiEnregGlobals++;
}

static void f2i_opt_Fatals(void *state)
{
 aiFatals = 0;
}

static void f2i_opt_Generate(void *state)
{
 aiGenerate++;
}

static void f2i_opt_MessageId(void *state)
{
 aiMessageId++;
}

static void f2i_opt_NoAlias(void *state)
{
 aiNoAlias++;
}

static void f2i_opt_ParseComments(void *state)
{
 aiParseComments++;
}

static void f2i_opt_GenClusters(void *state)
{
 aiGenClusters++;
 aiParseComments++;
}

static void f2i_opt_SymDump(void *state)
{
 aiSymDump++;
}

static void f2i_opt_TreeCheck(void *state)
{
 aiTreeCheck++;
}

static void f2i_opt_TreeDump(void *state)
{
 aiTreeDump++;
}

static void f2i_opt_Virtual(void *state)
{
 aiVirtual++;
}

static void f2i_opt_SymMap(void *state)
{
 aiSymMap++;
}

static void f2i_opt_Sparc(void *state)
{
 aiSparc++;
}

static void f2i_opt_Rocket(void *state)
{
 aiRocket++;
}

static void f2i_opt_Rt(void *state)
{
 aiRt++;
}

static void f2i_opt_Address(void *state)
  {
   aiOptimizeAddressCode++;
   aiCache++;
  }

static void f2i_opt_Cache(void *state)
  {
   aiCache++;
  }

static void f2i_opt_SpecialCache(void *state)
  {
   aiSpecialCache++;
   aiCache++;
  }

static void f2i_opt_LongIntegers(void *state)
  {
   aiLongIntegers++;
  }

static void f2i_opt_DoubleReals(void *state)
  {
   aiDoubleReals++;
  }

static void f2i_opt_program(void *state, char *str)
{
  f2i_program = str;
  f2i_ComputeInterproceduralInfo = true;
}

static void f2i_opt_module(void *state, char *str)
{
  f2i_module = str;
}


static void f2i_opt_ConservativeSelfSpatial(void *state)
{
 Memoria_ConservativeSelfSpatial++;
}

static void f2i_opt_debug_choice(void *state,Generic flag)
{
  switch(flag)
    {
     case 0:
       ReuseModelDebugFlag = true;
       break;
     default:
       cerr << "Invalid debug flag" << flag << endl;
       break;
    }
}

static struct flag_	Annotate_f = {
  f2i_opt_Annotate,
  "Annotate flag",	
  " ",
};

static struct flag_	AlignDoubles_f = {
  f2i_opt_AlignDoubles,
  "AlignDoubles flag",	
  " ",
};

static struct flag_	Constants_f = {
  f2i_opt_Constants,
  "Constants flag",	
  " ",
};

static struct flag_	Address_f = {
  f2i_opt_Address,
  "Address optimization flag",	
  " ",
};

static struct flag_	Cache_f = {
  f2i_opt_Cache,
  "Cache analysis flag",	
  " ",
};

static struct flag_	SpecialCache_f = {
  f2i_opt_SpecialCache,
  "Special Cache analysis flag",	
  " ",
};

static struct flag_	Debug_f = {
  f2i_opt_Debug,
  "Debug flag",	
  " ",
};

static struct flag_	EnregGlobals_f = {
  f2i_opt_EnregGlobals,
  "EnregGlobals flag",	
  " ",
};

static struct flag_	Fatals_f = {
  f2i_opt_Fatals,
  "Fatals flag",	
  " ",
};

static struct flag_	Generate_f = {
  f2i_opt_Generate,
  "Generate flag",	
  " ",
};

static struct flag_	LongIntegers_f = {
  f2i_opt_LongIntegers,
  "64-bit integers flag",	
  " ",
};

static struct flag_	DoubleReals_f = {
  f2i_opt_DoubleReals,
  "all double-precision reals flag",	
  " ",
};

static struct flag_	MessageId_f = {
  f2i_opt_MessageId,
  "MessageId flag",	
  " ",
};

static struct flag_	NoAlias_f = {
  f2i_opt_NoAlias,
  "NoAlias flag",	
  " ",
};

static struct flag_	ParseComments_f = {
  f2i_opt_ParseComments,
  "ParseComments flag",	
  " ",
};

static struct flag_	GenClusters_f = {
  f2i_opt_GenClusters,
  "GenClusters flag",	
  " ",
};

static struct flag_	SymDump_f = {
  f2i_opt_SymDump,
  "SymDump flag",	
  " ",
};

static struct flag_	TreeCheck_f = {
  f2i_opt_TreeCheck,
  "TreeCheck flag",	
  " ",
};

static struct flag_	TreeDump_f = {
  f2i_opt_TreeDump,
  "TreeDump flag",	
  " ",
};

static struct flag_	SymMap_f = {
  f2i_opt_SymMap,
  "SymMap flag",	
  " ",
};

static struct flag_	Rocket_f = {
  f2i_opt_Rocket,
  "Rocket flag",	
  " ",
};

static struct flag_	Sparc_f = {
  f2i_opt_Sparc,
  "Sparc flag",	
  " ",
};

static struct flag_	Rt_f = {
  f2i_opt_Rt,
  "Rt flag",	
  " ",
};

static struct flag_	Virtual_f = {
  f2i_opt_Virtual,
  "Virtual flag",	
  " ",
};

static struct string_	program_s = {
  f2i_opt_program,
  "program context     ",
  "program context",
  512, 50,
  ".*"
};

static struct string_	module_s = {
  f2i_opt_module,
  "module context     ",
  "module context",
  512, 50,
  ".*"
};

static struct flag_	ConservativeSelfSpatial_f = {
  f2i_opt_ConservativeSelfSpatial,
  "Conservative Self-Spatial flag",	
  " ",
};

static struct choice_entry_ debug_choices[1] = {
    {0,
     "0",
     "debug flags",
     "debug data reuse model"}
};

static struct choice_	debug_c = {
  f2i_opt_debug_choice,
  "debug flags",
  "turn on debug flag for data reuse model",
  0,debug_choices
};

Option f2i_pgm_opt = {string, F2I_PGM_OPT,(Generic) "", true,(Generic)&program_s},
       f2i_mod_opt = {string, F2I_MOD_OPT,(Generic) "", true,(Generic)&module_s},
       f2i_Annotate_flag = {flag, F2I_ANNOTATE_FLAG, (Generic)false, true, 
			    (Generic)&Annotate_f},
       f2i_AlignDoubles_flag = {flag,F2I_ALIGNDOUBLES_FLAG, (Generic)false, true, 
				(Generic)&AlignDoubles_f},
       f2i_Constants_flag ={flag, F2I_CONSTANTS_FLAG, (Generic)false,true, 
			    (Generic)&Constants_f},
       f2i_Address_flag = {flag, F2I_ADDRESSOPT_FLAG, (Generic)false, true, 
			   (Generic)&Address_f},
       f2i_Cache_flag = {flag, F2I_CACHE_FLAG, (Generic)false, true, (Generic)&Cache_f},
       f2i_SpecialCache_flag = {flag, F2I_SPECIALCACHE_FLAG, (Generic)false, true, 
				(Generic)&SpecialCache_f},
       f2i_Debug_flag = {flag, F2I_DEBUG_FLAG, (Generic)false, true, (Generic)&Debug_f},
       f2i_EnregGlobals_flag = {flag, F2I_ENREGGLOBALS_FLAG, (Generic)false, true,
				(Generic)&EnregGlobals_f},
       f2i_Fatals_flag = {flag, F2I_FATALS_FLAG, (Generic)false, true, 
			  (Generic)&Fatals_f},
       f2i_Generate_flag = {flag, F2I_GENERATE_FLAG, (Generic)false, true,
			    (Generic)&Generate_f},
       f2i_LongIntegers_flag = {flag, F2I_LONGINTEGERS_FLAG, (Generic)false, true, 
				(Generic)&LongIntegers_f},
       f2i_DoubleReals_flag = {flag, F2I_DOUBLEREALS_FLAG, (Generic)false, true,
			       (Generic)&DoubleReals_f},
       f2i_MessageId_flag = {flag, F2I_MESSAGEID_FLAG, (Generic)false, true, 
			     (Generic)&MessageId_f},
       f2i_NoAlias_flag = {flag, F2I_NOALIAS_FLAG, (Generic)false, true, 
			   (Generic)&NoAlias_f},
       f2i_ParseComments_flag = {flag, F2I_PARSECOMMENTS_FLAG, (Generic)false,
				 true,(Generic)&ParseComments_f},
       f2i_GenClusters_flag = {flag, F2I_GENCLUSTERS_FLAG, (Generic)false,
				 true,(Generic)&GenClusters_f},
       f2i_SymDump_flag = {flag, F2I_SYMDUMP_FLAG, (Generic)false, true, 
			   (Generic)&SymDump_f},
       f2i_TreeCheck_flag = {flag, F2I_TREECHECK_FLAG, (Generic)false, true,
			     (Generic)&TreeCheck_f},
       f2i_TreeDump_flag = {flag, F2I_TREEDUMP_FLAG, (Generic)false, true, 
			    (Generic)&TreeDump_f},
       f2i_Virtual_flag = {flag, F2I_VIRTUAL_FLAG, (Generic)false, true,
			   (Generic)&Virtual_f},
       f2i_SymMap_flag = {flag, F2I_SYMMAP_FLAG, (Generic)false, true, 
			  (Generic)&SymMap_f},
       f2i_Sparc_flag = {flag, F2I_SPARC_FLAG, (Generic)false, true, (Generic)&Sparc_f},
       f2i_Rocket_flag = {flag, F2I_ROCKET_FLAG, (Generic)false, true, 
			  (Generic)&Rocket_f},
       f2i_Rt_flag = {flag, F2I_RT_FLAG, (Generic)false, true, (Generic)&Rt_f},
       f2i_ConservativeSelfSpatial_flag = {flag, F2I_CONSERVATIVE_SPATIAL_FLAG,
					   (Generic)false, true, 
					  (Generic)&ConservativeSelfSpatial_f},
       f2i_debug_choice = {choice,F2I_DEBUG_CHOICE,(Generic)false,true, 
			      (Generic)&debug_c};

int f2i_init_options(int argc, char **argv)
{
  Options f2iOptions("f2i options");

  aiAnnotate  		= 1;
  aiAlignDoubles	= 1;
  aiConstants 		= 0;
  aiOptimizeAddressCode	= 0;
  aiCache 		= 0;
  aiGenClusters         = 0;
  aiSpecialCache	= 0;
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

  f2iOptions.Add(&f2i_pgm_opt);
  f2iOptions.Add(&f2i_mod_opt);
  f2iOptions.Add(&f2i_Annotate_flag);
  f2iOptions.Add(&f2i_AlignDoubles_flag);
  f2iOptions.Add(&f2i_Constants_flag);
  f2iOptions.Add(&f2i_Address_flag);
  f2iOptions.Add(&f2i_Cache_flag);
  f2iOptions.Add(&f2i_SpecialCache_flag);
  f2iOptions.Add(&f2i_Debug_flag);
  f2iOptions.Add(&f2i_EnregGlobals_flag);
  f2iOptions.Add(&f2i_Fatals_flag);
  f2iOptions.Add(&f2i_Generate_flag);
  f2iOptions.Add(&f2i_LongIntegers_flag);
  f2iOptions.Add(&f2i_DoubleReals_flag);
  f2iOptions.Add(&f2i_MessageId_flag);
  f2iOptions.Add(&f2i_NoAlias_flag);
  f2iOptions.Add(&f2i_ParseComments_flag);
  f2iOptions.Add(&f2i_GenClusters_flag);
  f2iOptions.Add(&f2i_SymDump_flag);
  f2iOptions.Add(&f2i_TreeCheck_flag);
  f2iOptions.Add(&f2i_TreeDump_flag);
  f2iOptions.Add(&f2i_Virtual_flag);
  f2iOptions.Add(&f2i_SymMap_flag);
  f2iOptions.Add(&f2i_Sparc_flag);
  f2iOptions.Add(&f2i_Rocket_flag);
  f2iOptions.Add(&f2i_Rt_flag);
  f2iOptions.Add(&f2i_ConservativeSelfSpatial_flag);
  f2iOptions.Add(&f2i_debug_choice);

  if (opt_parse_argv(&f2iOptions,0,argc,argv)) 
    {
      f2i_options_usage(argv[0]);
      return -1;
    }

  if (!f2i_module & !f2i_program & !f2i_module_list)
    {
      cerr << "Must specify one of -M, -L and -P" << endl;
      return -1;
    }
  else if ((f2i_module && f2i_program) ||
	   (f2i_module && f2i_module_list) ||
	   (f2i_program && f2i_module_list))
    {
      cerr << "Must specify only one of -M, -L and -P" << endl;
      return -1;
    }
    
  return 0;
}
