/* $Id: MemoriaOptions.C,v 1.17 2002/02/20 16:20:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <libs/support/misc/general.h>
#include <assert.h>
#include <string.h>

#include <execs/Memoria/MemoriaOptions.h>

#include <libs/Memoria/include/memory_menu.h>

int  selection = NO_SELECT;
int  ReplaceLevel = 8;
int  DependenceLevel = 1;
int  PartitionUnrollAmount = 0;
int  StatsLevel = 0;
char select_char;
char *mc_program = NULL;
char *mc_module = NULL;
char *mc_config = NULL;
char *mc_module_list = NULL;
char *mc_output = NULL;
int aiCache = -1;
int aiSpecialCache = 0;
int aiOptimizeAddressCode = 0;
int aiParseComments = 0;
int aiLongIntegers = 0;
int aiDoubleReals = 0;
int aiGenClusters = 0;
int blue_color = 0;

Boolean Memoria_LetRocketSchedulePrefetches = false;
Boolean Memoria_IssueDead = false;
Boolean Memoria_ConservativeSelfSpatial = false;
Options MemoriaOptions("Memoria Options");

Boolean mc_unroll_cache = false;  
Boolean mc_allow_expansion = false;
Boolean mc_extended_cache = false;
Boolean inputs_needed = false;
Boolean RestrictedUnrolling = false;

Boolean ReuseModelDebugFlag = false;
Boolean DDGDebugFlag = false;
Boolean MinDistDebugFlag = false;
Boolean CheckRecurrencesForPrefetching = false;
Boolean GeneratePreLoop = true;

void MemoriaOptionsUsage(char *pgm_name)
{
   printf("Usage: %s [-s#] [-e] [-i] [-n] [-p] [-q] [-r#] [-d#] [-w#] [-u] [-U] [-D] [-I] [-R] [-X#] {-P <program> | -M <module> | -L <module list>} [-C <configuration file>] [-O <output file>]",pgm_name);
  puts(" ");
  puts("         -c  annontate with calls to cache simulator");
  puts("         -d#  set dependence analysis level at 0 or 1 (default)");
  puts("         -e  use extended cache analysis");
  puts("         -h  prepare code for cache analysis");
  puts("         -i  do interchange");
  puts("         -l  count # of loads and stores");
  puts("         -n  do not generate preloops in scalar replacement");
  puts("         -p  do software prefetching");
  puts("         -q  check recurrences in software prefetching");
  puts("         -r#  set scalar replacement level 0 through 8");
  puts("         -s#  do statistics");
  puts("         -u  do unroll-and-jam without cache model");
  puts("         -w#  do unroll-and-jam for partitioned register files");
  puts("         -D  assume all REALS are 64-bits (for Rocket)");
  puts("         -I  assume 64-bit integers");
  puts("         -R  let Rocket schedule prefetches");
  puts("         -S  do depencence statistics");
  puts("         -U  do unroll-and-jam with cache model");
  puts("         -X#  set debug flags");
  puts("         -P  program composition");
  puts("         -M  Fortran module");
  puts("         -L  list of Fortran modules");
  puts("         -C  target machine configuration file");
  puts("         -O  output file for transformed code");
  fflush(stdout);
  exit(-1);
}

static void mc_opt_dependence_stats(void *state)
{
  select_char = 'S';
  switch(selection) {
  case NO_SELECT:
    selection = DEP_STATS;
    break;
  default:
    MemoriaOptionsUsage("mc");
  }
}

static void mc_set_dependence_level(void *state,Generic level)

  {
   DependenceLevel = level;
  }


static void mc_set_partition_unroll_amount(void *state,char* amount)

  {
    selection = PARTITION_UNROLL;
    PartitionUnrollAmount = atoi(amount); 
    RestrictedUnrolling = true;
  }

static void mc_set_no_pre_loop(void *state)
  {
   GeneratePreLoop = false;
  }


static void mc_set_long_integers(void *state)
  {
   aiLongIntegers = 1;
  }

static void mc_set_double_reals(void *state)
  {
   aiDoubleReals = 1;
  }


static void mc_set_cache_level(void *state)

  {
   aiCache = 2;
   selection = F2I_ANALYSIS;
   select_char = 'h';
  }

static void mc_opt_replacement(void *state,Generic level)
{
  if (level == 0) 
    return;
  inputs_needed = true;
  ReplaceLevel = level;
  switch(selection) {
  case NO_SELECT:
    selection = SCALAR_REP;
    select_char = 'r';
    break;
  case UNROLL_AND_JAM:
    selection = UNROLL_SCALAR;
    select_char = 'u';
    break;
  case INTERCHANGE:
    selection = LI_SCALAR;
    select_char = 'i';
    break;
  case STATS:
    selection = SR_STATS;
    break;
  case MEM_ALL:
    break;
  default:
    MemoriaOptionsUsage("Memoria");
  }
}

static void mc_opt_interchange(void *state)
{
  switch(selection) {
  case NO_SELECT:
    selection = INTERCHANGE;
    select_char = 'i';
    break;
  case UNROLL_AND_JAM:
    selection = LI_UNROLL;
    select_char = 'i';
    break;
  case SCALAR_REP:
    selection = LI_SCALAR;
    select_char = 'i';
    break;
  case STATS:
    selection = LI_STATS;
    mc_allow_expansion = true;
    break;
  default:
    MemoriaOptionsUsage("Memoria");
  }
}

void mc_opt_prefetch(void *state)

  {
   switch(selection)
     {
      case NO_SELECT:
        selection = PREFETCH;
	select_char = 'p';
	break;
      default:
	MemoriaOptionsUsage("Memoria");
     }
  }

void mc_opt_prefetch_recurrences(void *state)

  {
    CheckRecurrencesForPrefetching = true;
  }

void mc_opt_rocket_schedule(void *state)
{
  Memoria_LetRocketSchedulePrefetches = true;
}

// void mc_opt_dead(void *state)

//   {
//    switch(selection)
//      {
//       case NO_SELECT:
//         selection = DEAD;
// 	select_char = 'D';
//         Memoria_IssueDead = true;
// 	break;
//       default:
// 	MemoriaOptionsUsage("Memoria");
//      }
//   }

static void mc_opt_statistics(void *state,Generic level)
{
  select_char = 's';
  StatsLevel = level;
  switch(selection) {
  case NO_SELECT:
    selection = STATS;
    break;
  case UNROLL_AND_JAM:
    selection = UJ_STATS;
    break;
  case SCALAR_REP:
    selection = SR_STATS;
    break;
  case INTERCHANGE:
    selection = LI_STATS;
    mc_allow_expansion = true;
    break;
  case MEM_ALL:
    break;
  default:
    MemoriaOptionsUsage("Memoria");
  }
}


static void mc_opt_annotate(void *state)
{
  if (selection != NO_SELECT)
    MemoriaOptionsUsage("Memoria");
  selection = ANNOTATE;
  select_char = 'c';
}

static void mc_opt_count_ldst(void *state)
{
  if (selection != NO_SELECT)
    MemoriaOptionsUsage("Memoria");
  selection = LDST;
  select_char = 'l';
}

static void mc_opt_unroll(void *state)
{
  switch(selection) {
  case NO_SELECT:
    selection = UNROLL_AND_JAM;
    select_char = 'u';
    break;
  case SCALAR_REP:
    selection = UNROLL_SCALAR;
    select_char = 'u';
    break;
  case INTERCHANGE:
    selection = LI_UNROLL;
    select_char = 'i';
    break;
  case STATS:
    selection = UJ_STATS;
    break;
  case MEM_ALL:
    break;
  default:
    MemoriaOptionsUsage("Memoria");
  }
}

static void mc_opt_debug_choice(void *state,Generic flag)
{
  switch(flag)
    {
     case 0:
       ReuseModelDebugFlag = true;
       break;
     case 1:
       DDGDebugFlag = true;
       break;
     case 2:
       MinDistDebugFlag = true;
       break;
     default:
       cerr << "Invalid debug flag" << flag << endl;
       break;
    }
}

static void mc_opt_unroll_cache(void *state)
{
  mc_unroll_cache = true;
  mc_opt_unroll(state);
  }

static void mc_opt_extended_cache(void *state)
{
  mc_extended_cache = true;
  }

static void mc_opt_RestrictedUnrolling(void *state)
{
  RestrictedUnrolling = true;
  }

static void mc_opt_program(void *state, char *str)
{
  mc_program = str;
}

static void mc_opt_module_list(void *state, char *str)
{
  mc_module_list = str;
}

static void mc_opt_module(void *state, char *str)
{
  mc_module = str;
}

static void mc_opt_config(void *state, char *str)
{
  mc_config = str;
}

void mc_opt_output(void *state,char *str)

  {
   mc_output = str;
  }


static struct flag_	prefetch_f = {
  mc_opt_prefetch,
  "software prefetch", 
  "perform software prefetching",
};

static struct flag_	prefetch_rec_f = {
  mc_opt_prefetch_recurrences,
  "use recurrences in prefetch", 
  "recurrences in software prefetching",
};

// static struct flag_	dead_f = {
//   mc_opt_dead,
//   "dead cache lines", 
//   "insert dead cache line directives",
// };

static struct flag_	rocket_schedule_f = {
  mc_opt_rocket_schedule,
  "let rocket schedule prefetches", 
  "",
};

static struct flag_	interchange_f = {
  mc_opt_interchange,
  "loop interchange", 
  "perform loop interchange for memory performance",
};

static struct choice_entry_ replacement_choices[9] = {
    {0,
     "0",
     "scalar replacement",
     "no scalar replacement"},
    {1,
     "1",
     "scalar replacement",
     "perform scalar replacement, no control flow, LI only"},
    {2,
     "2",
     "scalar replacement",
     "perform scalar replacement, add invariant references"},
    {3,
     "3",
     "scalar replacement",
     "perform scalar replacement, add loop carried distance 1"},
    {4,
     "4",
     "scalar replacement",
     "perform scalar replacement, add all loop carried"},
    {5,
     "5",
     "scalar replacement",
     "perform scalar replacement, LI only, add control flow"},
    {6,
     "6",
     "scalar replacement",
     "perform scalar replacement, level 3 + control flow"},
    {7,
     "7",
     "scalar replacement",
     "perform scalar replacement, level 6 + LI partially available"},
    {8,
     "8",
     "scalar replacement",
     "perform full scalar replacement"}
 };

static struct choice_entry_ debug_choices[3] = {
    {0,
     "0",
     "debug flags",
     "debug data reuse model"},
    {1,
     "1",
     "debug flags",
     "debug DDG"},
    {2,
     "2",
     "debug flags",
     "debug MinDist"}
};

static struct flag_	preloop_f = {
  mc_set_no_pre_loop,
  "no pre-loop for scala replacement",
  "for data collecting purposes"
};

static struct flag_	longint_f = {
  mc_set_long_integers,
  "64-bit integers",
  "assume integers are 64-bits"
};

static struct flag_	doublereal_f = {
  mc_set_double_reals,
  "64-bit reals",
  "assume all reals are 64-bits for Rocket"
};

static struct flag_	cache_f = {
  mc_set_cache_level,
  "cache analysis",
  "perform cache analysis for scheduling, level 1 is unrolling"
};

static struct flag_	ldst_f = {
  mc_opt_count_ldst,
  "load and store analysis",
  "Count the number of loads and stores to arrays"
};

static struct choice_	replacement_c = {
  mc_opt_replacement,
  "scalar replacement",
  "perform scalar replacement, allowing control flow",
  8,replacement_choices
};

static struct choice_	debug_c = {
  mc_opt_debug_choice,
  "debug flags",
  "turn on debug flag for data reuse model",
  2,debug_choices
};

static struct choice_entry_ dependence_choices[2] = {
    {0,
     "0",
     "dependence",
     "Delta only"},
    {1,
     "1",
     "dependence",
     "all tests"}
 };


static struct choice_entry_ statistics_choices[2] = {
    {0,
     "0",
     "statistics",
     "default"},
    {1,
     "1",
     "statistics",
     "add register pressure prediction for scalar replacement"}
};

static struct choice_	dependence_c = {
  mc_set_dependence_level,
  "dependence",
  "level",
  1,dependence_choices
};


static struct choice_	statistics_c = {
  mc_opt_statistics,
  "interchange statistics",
  "report locality and loop permutation statistics",
  1,statistics_choices
};

static struct flag_	dependence_stats_f = {
  mc_opt_dependence_stats,
  "dependence statistics",
  "report dependence statistics",
};

static struct flag_	annotate_f = {
  mc_opt_annotate,
  "cache annotations",
  "annotate code with call to cache simulator",
};

static struct flag_	unroll_f = {
  mc_opt_unroll,
  "unroll-and-jam",
  "perform automatic unroll-and-jam for loop balance",
};

static struct flag_	unroll_cache_f = {
  mc_opt_unroll_cache,
  "unroll-and-jam",
  "perform automatic unroll-and-jam for loop balance with cache",
};

static struct flag_	extended_cache_f = {
  mc_opt_extended_cache,
  "cache analysis",
  "Use Wolfs group-spatial formulation",
};

static struct flag_	RestrictedUnrolling_f = {
  mc_opt_RestrictedUnrolling,
  "less unrolling",
  "Do not unroll pre-loops",
};

static struct string_	program_s = {
  mc_opt_program,
  "program context     ",
  "program context",
  512, 50,
  ".*"
};

static struct string_	module_s = {
  mc_opt_module,
  "name of fortran file",
  "name of fortran file",
  512, 50,
  ".*"
};

static struct string_	partition_unroll_amount_s = {
  mc_set_partition_unroll_amount,
  "amount to unroll a loop for partitioned register files",
  "amount to unroll a loop for partitioned register files",
  512, 50,
  ".*"
};

static struct string_	module_list_s = {
  mc_opt_module_list,
  "list of fortran files",
  "list of fortran files",
  512, 50,
  ".*"
};

static struct string_	config_s = {
  mc_opt_config,
  "configuration file",
  "configuration file",
  512, 50,
  ".*"
};

static struct string_	output_s = {
  mc_opt_output,
  "output file",
  "output file",
  512, 50,
  ".*"
};

static Option *InitOption(option_type t,
		         char arg_char,
		         Generic init_value,
		         Boolean in_dialog,
		         Generic f_c)
{
  Option *option = new Option;

  option->t = t;
  option->arg_char = arg_char;
  option->init_value = init_value;
  option->in_dialog = in_dialog,
  option->f_c = f_c;
  return option;
}

int MemoriaInitOptions(int argc, char **argv)

{
  Option *mc_mod_opt = InitOption(string,MC_MOD_OPT,(Generic)"",true,(Generic)&module_s),
    *mc_pgm_opt = InitOption(string,MC_PGM_OPT,(Generic)"",true,(Generic)&program_s), 
    *mc_lst_opt = InitOption(string,MC_LST_OPT,(Generic)"",true,(Generic)&module_list_s),
    *mc_cfg_opt = InitOption(string,MC_CFG_OPT,(Generic)"",true,(Generic)&config_s),
    *mc_out_opt = InitOption(string,MC_OUT_OPT,(Generic) "",true,(Generic)&output_s),
    *mc_partition_unroll_opt = InitOption(string,MC_PARTITION_UNROLL_OPT,(Generic) "",
					  true,(Generic)&partition_unroll_amount_s),
    *mc_int_flag = InitOption(flag,MC_INTERCHANGE_FLAG,(Generic)false,true, 
			      (Generic)&interchange_f),
    *mc_pre_flag = InitOption(flag,MC_PREFETCH_FLAG,(Generic)false,true, 
			      (Generic)&prefetch_f),
    *mc_pre_rec_flag = InitOption(flag,MC_PREFETCH_REC_FLAG,(Generic)false,true, 
			      (Generic)&prefetch_rec_f),
//     *mc_dead_flag = InitOption(flag,MC_DEAD_FLAG,(Generic)false,true,(Generic)&dead_f),
    *mc_rocket_schedule_flag = InitOption(flag,MC_ROCKET_SCHEDULE_FLAG,(Generic)false,
					  true,(Generic)&rocket_schedule_f),
    *mc_no_pre_loop_flag = InitOption(flag,MC_NO_PRE_LOOP_FLAG,(Generic)false,
				      true, (Generic)&preloop_f),
    *mc_long_int_flag = InitOption(flag,MC_LONG_INT_FLAG,(Generic)false,true,
				     (Generic)&longint_f),
    *mc_double_real_flag = InitOption(flag,MC_DOUBLE_REAL_FLAG,(Generic)false,true,
				     (Generic)&doublereal_f),
    *mc_cache_anal_flag = InitOption(flag,MC_CACHE_ANAL_FLAG,(Generic)false,true,
				     (Generic)&cache_f),
    *mc_ldst_anal_flag = InitOption(flag,MC_LDST_ANAL_FLAG,(Generic)false,true,
				    (Generic)&ldst_f),
    *mc_repl_choice = InitOption(choice,MC_REPLACEMENT_CHOICE,(Generic)false,true, 
				 (Generic)&replacement_c),
    *mc_debug_choice = InitOption(choice,MC_DEBUG_CHOICE,(Generic)false,true, 
				 (Generic)&debug_c),
    *mc_dep_choice = InitOption(choice,MC_DEPENDENCE_CHOICE,(Generic)false,true, 
				(Generic)&dependence_c),
    *mc_stats_choice = InitOption(choice,MC_STATISTICS_CHOICE,(Generic)false,true,
				  (Generic)&statistics_c),
    *mc_annotate_flag = InitOption(flag,MC_ANNOTATE_FLAG,(Generic)false,true,
				   (Generic)&annotate_f),
    *mc_unroll_flag = InitOption(flag,MC_UNROLL_FLAG,(Generic)false,true,
				 (Generic)&unroll_f),
    *mc_unroll_cache_flag = InitOption(flag,MC_UNROLL_CACHE_FLAG,(Generic)false,true,
				       (Generic)&unroll_cache_f),
    *mc_extended_cache_flag = InitOption(flag,MC_EXTENDED_CACHE_FLAG,(Generic)false,true,
					 (Generic)&extended_cache_f),
    *RestrictedUnrolling_flag = InitOption(flag,MC_RESTRICTED_FLAG,(Generic)false,true,
					   (Generic)&RestrictedUnrolling_f),
    *mc_dep_stats_flag = InitOption(flag,MC_DEPENDENCE_STATS_FLAG,(Generic)false,true,
				    (Generic)&dependence_stats_f);
  MemoriaOptions.Add(mc_mod_opt);
  MemoriaOptions.Add(mc_pgm_opt);
  MemoriaOptions.Add(mc_lst_opt);
  MemoriaOptions.Add(mc_cfg_opt);
  MemoriaOptions.Add(mc_out_opt);
  MemoriaOptions.Add(mc_partition_unroll_opt);
  MemoriaOptions.Add(mc_int_flag);
  MemoriaOptions.Add(mc_pre_flag);
  MemoriaOptions.Add(mc_pre_rec_flag);
//   MemoriaOptions.Add(mc_dead_flag);
  MemoriaOptions.Add(mc_rocket_schedule_flag);
  MemoriaOptions.Add(mc_repl_choice);
  MemoriaOptions.Add(mc_no_pre_loop_flag);
  MemoriaOptions.Add(mc_long_int_flag);
  MemoriaOptions.Add(mc_double_real_flag);
  MemoriaOptions.Add(mc_cache_anal_flag);
  MemoriaOptions.Add(mc_ldst_anal_flag);
  MemoriaOptions.Add(mc_dep_choice);
  MemoriaOptions.Add(mc_debug_choice);
  MemoriaOptions.Add(mc_stats_choice);
  MemoriaOptions.Add(mc_dep_stats_flag);
  MemoriaOptions.Add(mc_annotate_flag);
  MemoriaOptions.Add(mc_unroll_flag);
  MemoriaOptions.Add(mc_unroll_cache_flag);
  MemoriaOptions.Add(mc_extended_cache_flag);
  MemoriaOptions.Add(RestrictedUnrolling_flag);

  if (opt_parse_argv(&MemoriaOptions,0,argc,argv)) {
    MemoriaOptionsUsage(argv[0]);
    return -1;
  }
  if ((mc_program && mc_module) || (mc_program && mc_module_list) ||
      (mc_module && mc_module_list)) {
    fprintf(stderr, "Specify only one of -P, -L and -M\n");
    MemoriaOptionsUsage(argv[0]);
  }
  else if (!mc_program && !mc_module && !mc_module_list) {
    fprintf(stderr, "Specify only one of -P, -L and -M\n");
    MemoriaOptionsUsage(argv[0]);
  }
  
  return 0;
}
