/* $ID: CfgOptions.h,v 1.5 1994/10/03 19:17:07 curetonk Exp curetonk $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    CfgOptions.h
 *
 * Original Author
 *    Unknown
 *
 * Creation Date
 *    Unknown
 *
 * Description
 *    Header file for CfgOptions.C.
 *
 * History
 *    08/94 - Kevin Cureton - rewrite to use the file attribute abstraction.
 *
 ******************************************************************************/

#ifndef CfgOptions_h
#define CfgOptions_h

/**************************** System Include Files ****************************/

/***************************** User Include Files *****************************/

#include <libs/support/misc/general.h>
#include <libs/support/optParsing/Options.h>


/**************************** Variable Definitions ****************************/

#define CFG_PGM_OPT           'P'
#define CFG_MOD_OPT           'M'
#define CFG_LST_OPT           'L'

#define CFG_dumpCfg_FLAG      'c'
#define CFG_dumpSort_FLAG     'S'
#define CFG_dumpDom_FLAG      'D'
#define CFG_dumpTarj_FLAG     'T'
#define SSA_doDataFlow_FLAG   'd'
#define SSA_doGated_FLAG      'g'
#define SSA_doArrays_FLAG     'a'
#define SSA_ipSmush_FLAG      'h'
#define SSA_doDefKill_FLAG    'k'
#define VAL_doVals_FLAG       'v'
#define VAL_ipTest_FLAG       'i'
#define VAL_dumpVals_FLAG     'V'
#define CFG_dumpTree_FLAG     't'
#define CFG_doStats_FLAG      's'
#define CFG_printTimes_FLAG   'p'

extern char *cfgProgram;
extern char *cfgModule;
extern char *cfgModuleList;

extern Boolean dumpCfg;	  
extern Boolean dumpSort;   
extern Boolean dumpDom;    
extern Boolean dumpTarj;   

extern Boolean doDataFlow; 
extern Boolean doGated;    
extern Boolean doArrays;   
extern Boolean ipSmush;    
extern Boolean ipTest;    
extern Boolean doDefKill;  

extern Boolean doVals;    
extern Boolean dumpVals;  

extern Boolean dumpTree;  
extern Boolean doStats;	 
extern Boolean printTimes;


/************************* Extern Function Prototypes *************************/

int CfgOptsProcess(int argc, char** argv);

#endif
