/* $Id: CfgOptions.C,v 1.1 1998/01/07 14:42:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    CfgOptions.C
 *
 * Original Author
 *    Unknown
 *
 * Creation Date
 *    Unknown
 *
 * Description
 *    Option processing for the cfg program.
 *
 * History
 *    08/94 - Kevin Cureton - rewrite to use the file attribute abstraction.
 *
 ******************************************************************************/


/**************************** System Include Files ****************************/
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************** User Include Files *****************************/
#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>

#include <libs/support/optParsing/Options.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>

#include <execs/cfg/CfgOptions.h>

/**************************** Variable Definitions ****************************/

char *cfgProgram = NULL;
char *cfgModule = NULL;
char *cfgModuleList = NULL;

Boolean dumpCfg	   = false;	/* cfg/ssa */
Boolean dumpSort   = false;	/* cfg/ssa */
Boolean dumpDom    = false;	/* cfg */
Boolean dumpTarj   = false;	/* cfg */
Boolean doDataFlow = false;	/* ssa */
Boolean doGated    = false;	/* ssa */
Boolean doArrays   = false;	/* ssa */
Boolean ipSmush    = false;	/* ssa */
Boolean doDefKill  = false;	/* ssa */
Boolean doVals     = false;	/* val */
Boolean dumpVals   = false;	/* val */
Boolean ipTest     = false;	/* val */
Boolean dumpTree   = false;
Boolean doStats	   = false;
Boolean printTimes = false;


/****************************** Option Processing *****************************/

void cfg_dumpCfg_opt(void *state) {dumpCfg = true;}
void cfg_dumpSort_opt(void *state) {dumpSort = true;}
void cfg_dumpDom_opt(void *state) {dumpDom = true;}
void cfg_dumpTarj_opt(void *state) {dumpTarj = true;}

void ssa_doDataFlow_opt(void *state) { doDataFlow = true;}
void ssa_doGated_opt(void *state) { doDataFlow = doGated = true;}
void ssa_doArrays_opt(void *state) { doDataFlow = doArrays = true;}
void ssa_ipSmush_opt(void *state) { doDataFlow = ipSmush = true;}
void ssa_doDefKill_opt(void *state) { doDataFlow = doDefKill = true;}

void val_doVals_opt(void *state)   { doDataFlow = doVals = true;}
void val_dumpVals_opt(void *state) { doDataFlow = doVals = dumpVals= true;}
void val_ipTest_opt(void *state)   { doDataFlow = doVals = ipTest  = true;}

void cfg_dumpTree_opt(void *state) {dumpTree = true;}
void cfg_doStats_opt(void *state) {doStats = true;}
void cfg_printTimes_opt(void *state) {printTimes = true;}

void cfg_opt_program(void *state, char *str) { cfgProgram = str; }
void cfg_opt_module_list(void *state, char *str) { cfgModuleList = str; }
void cfg_opt_module(void *state, char *str) { cfgModule = str; }

static struct flag_	dumpCfg_f = {
  cfg_dumpCfg_opt,
  "dump cfg",
  "dump cfg (with dataflow if built)",
 };

static struct flag_	dumpSort_f = {
  cfg_dumpSort_opt,
  "dump sorted cfg",
  "dump sorted cfg (with dataflow)",
 };

static struct flag_	dumpDom_f = {
  cfg_dumpDom_opt,
  "dump dom",
  "dump pre/post dominator trees",
 };

static struct flag_	dumpTarj_f = {
  cfg_dumpTarj_opt,
  "dump tarj",
  "dump Tarjan interval tree (loops)",
 };

static struct flag_	doDataFlow_f = {
  ssa_doDataFlow_opt,
  "do dataflow",
  "build dataflow (ssa form)",
 };

static struct flag_	doGated_f = {
  ssa_doGated_opt,
  "do gated",
  "build dataflow (gsa form)",
 };

static struct flag_	doArrays_f = {
  ssa_doArrays_opt,
  "do arrays",
  "build array dataflow",
 };

static struct flag_	ipSmush_f = {
  ssa_ipSmush_opt,
  "ip smush",
  "collapse interprocedural nodes",
 };

static struct flag_	doDefKill_f = {
  ssa_doDefKill_opt,
  "do def-kill",
  "build def-def edges",
 };

static struct flag_	doVals_f = {
  val_doVals_opt,
  "do vals",
  "build value numbers",
 };

static struct flag_	dumpVals_f = {
  val_dumpVals_opt,
  "dump vals",
  "dump value numbers",
 };

static struct flag_	ipTest_f = {
  val_ipTest_opt,
  "ip test",
  "save interprocedural values",
 };

static struct flag_	dumpTree_f = {
  cfg_dumpTree_opt,
  "dump tree",
  "dump Fortran AST",
 };

static struct flag_	doStats_f = {
  cfg_doStats_opt,
  "print statistics",
  "print size statistics",
 };

static struct flag_	printTimes_f = {
  cfg_printTimes_opt,
  "print times",
  "print timing measurements",
 };

static struct string_	program_s = {
  cfg_opt_program,
  "program context     ",
  "program context",
  512, 50,
  ".*"
};

static struct string_	module_s = {
  cfg_opt_module,
  "name of fortran file",
  "name of fortran file",
  512, 50,
  ".*"
};

static struct string_	module_list_s = {
  cfg_opt_module_list,
  "list of fortran files",
  "list of fortran files",
  512, 50,
  ".*"
};

Option cfg_mod_opt = 
{ string, CFG_MOD_OPT,  (Generic) "",   true, (Generic)&module_s };

Option cfg_pgm_opt = 
{ string, CFG_PGM_OPT,  (Generic) "",   true, (Generic)&program_s };

Option cfg_lst_opt = 
{ string, CFG_LST_OPT,  (Generic) "",   true, (Generic)&module_list_s };

Option cfg_dumpCfg_flag = 
{ flag,   CFG_dumpCfg_FLAG, (Generic)false, true, (Generic)&dumpCfg_f };

Option cfg_dumpSort_flag = 
{ flag,   CFG_dumpSort_FLAG, (Generic)false, true, (Generic)&dumpSort_f };

Option cfg_dumpDom_flag = 
{ flag,   CFG_dumpDom_FLAG, (Generic)false, true, (Generic)&dumpDom_f };

Option cfg_dumpTarj_flag = 
{ flag,   CFG_dumpTarj_FLAG, (Generic)false, true, (Generic)&dumpTarj_f };

Option ssa_doDataFlow_flag = 
{ flag,   SSA_doDataFlow_FLAG, (Generic)false, true, (Generic)&doDataFlow_f };

Option ssa_doGated_flag = 
{ flag,   SSA_doGated_FLAG, (Generic)false, true, (Generic)&doGated_f };

Option ssa_doArrays_flag = 
{ flag,   SSA_doArrays_FLAG, (Generic)false, true, (Generic)&doArrays_f };

Option ssa_ipSmush_flag = 
{ flag,   SSA_ipSmush_FLAG, (Generic)false, true, (Generic)&ipSmush_f };

Option ssa_doDefKill_flag = 
{ flag,   SSA_doDefKill_FLAG, (Generic)false, true, (Generic)&doDefKill_f };

Option val_doVals_flag = 
{ flag,   VAL_doVals_FLAG, (Generic)false, true, (Generic)&doVals_f };

Option val_dumpVals_flag = 
{ flag,   VAL_dumpVals_FLAG, (Generic)false, true, (Generic)&dumpVals_f };

Option val_ipTest_flag = 
{ flag,   VAL_ipTest_FLAG, (Generic)false, true, (Generic)&ipTest_f };

Option cfg_dumpTree_flag = 
{ flag,   CFG_dumpTree_FLAG, (Generic)false, true, (Generic)&dumpTree_f };

Option cfg_doStats_flag = 
{ flag,   CFG_doStats_FLAG, (Generic)false, true, (Generic)&doStats_f };

Option cfg_printTimes_flag = 
{ flag,   CFG_printTimes_FLAG, (Generic)false, true, (Generic)&printTimes_f };

/************************* Static Function Prototypes *************************/

static void CfgOptsUsage(char *programName);

/****************************** External Functions ****************************/

/******************************************************************************
 *
 *  Function
 *     CfgOptsProcess (extern)
 *
 *  Parameters
 *
 *  Return Value
 *
 *  Description
 *
 *  Original Author
 *
 *  Creation Date
 *
 *  Change History
 *
 ******************************************************************************/
int CfgOptsProcess(int argc, char **argv)
{
  Options cfgOptions("cfg command line options");

  cfgOptions.Add(&cfg_mod_opt);
  cfgOptions.Add(&cfg_pgm_opt);
  cfgOptions.Add(&cfg_lst_opt);
  cfgOptions.Add(&cfg_dumpCfg_flag);
  cfgOptions.Add(&cfg_dumpSort_flag);
  cfgOptions.Add(&cfg_dumpDom_flag);
  cfgOptions.Add(&cfg_dumpTarj_flag);
  cfgOptions.Add(&ssa_doDataFlow_flag);
  cfgOptions.Add(&ssa_doGated_flag);
  cfgOptions.Add(&ssa_doArrays_flag);
  cfgOptions.Add(&ssa_ipSmush_flag);
  cfgOptions.Add(&ssa_doDefKill_flag);
  cfgOptions.Add(&val_doVals_flag);
  cfgOptions.Add(&val_dumpVals_flag);
  cfgOptions.Add(&val_ipTest_flag);
  cfgOptions.Add(&cfg_dumpTree_flag);
  cfgOptions.Add(&cfg_doStats_flag);
  cfgOptions.Add(&cfg_printTimes_flag);

  if (opt_parse_argv(&cfgOptions, 0, argc, argv)) 
  {
    CfgOptsUsage(argv[0]);
    return -1;
  }
 
  if ((cfgProgram && cfgModule) || (cfgProgram && cfgModuleList) ||
      (cfgModule && cfgModuleList)) 
  {
    errorMsgHandler.HandleMsg("Specify only one of -P, -L and -M\n");
    CfgOptsUsage(argv[0]);
    return -1;
  }
  else if (!cfgProgram && !cfgModule && !cfgModuleList) 
  {
    errorMsgHandler.HandleMsg("Specify one of -P, -L and -M\n");
    CfgOptsUsage(argv[0]);
    return -1;
  }

  return 0;
}

/******************************* Static Functions *****************************/

/******************************************************************************
 *
 *  Function
 *     CfgOptsUsage (static)
 *
 *  Parameters
 *
 *  Return Value
 *
 *  Description
 *
 *  Original Author
 *
 *  Creation Date
 *
 *  Change History
 *
 ******************************************************************************/
static void CfgOptsUsage(char *programName)
{
  cerr << "Usage: " 
       << programName 
       << " [-c] | [-d] | [-p] | [-s] | [-S] | [-t] | [-T] "
       << "{ -P <program> | -M <module> | - L <module list> }\n"
       << endl;

  cerr << "         -c  cfg/ssa: dump cfg (with dataflow if built)\n"
       << "         -S  cfg/ssa: dump sorted cfg (with dataflow)\n"
       << "         -D  cfg: dump pre/post dominator trees\n"
       << "         -T  cfg: dump Tarjan interval tree (loops)\n"
       << "         -d  ssa: build dataflow (ssa form)\n"
       << "         -g  ssa: build dataflow (gsa form)\n"
       << "         -a  ssa: build array dataflow\n"
       << "         -h  ssa: collapse interprocedural nodes\n"
       << "         -k  ssa: build def-def edges\n"
       << "         -v  val: build value numbers\n"
       << "         -V  val: dump value numbers\n"
       << "         -i  val: save local vals for ip analysis \n"
       << "         -t  dump Fortran AST\n"
       << "         -s  print size statistics\n"
       << "         -p  print timing measurements\n"
       << "         -P  program composition\n"
       << "         -M  Fortran module\n"
       << "         -L  file listing Fortran modules\n"
       << endl;
}
