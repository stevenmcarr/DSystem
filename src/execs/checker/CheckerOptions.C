/* $Id: CheckerOptions.C,v 1.4 1997/03/11 14:27:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    checkerOptions.C
 *
 * Original Author
 *    Unknown
 *
 * Creation Date
 *    Unknown
 *
 * Description
 *    Fortran file type checker.
 *
 * History
 *    08/94 - Kevin Cureton - rewrite to use the file attribute abstraction.
 *
 ******************************************************************************/

/**************************** System Include Files ****************************/

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/***************************** User Include Files *****************************/

#include <libs/support/optParsing/Options.h>
#include <libs/support/msgHandlers/ErrorMsgHandler.h>

#include <execs/checker/CheckerOptions.h>

/************************* Static Function Prototypes *************************/

static void CheckerOptsUsage(char* pgm_name);

/**************************** Variable Definitions ****************************/

char* global_pgm_loc = NULL;
char* global_mod_loc = NULL;
char* global_list_loc = NULL;

FILE* global_dep_ptr = stdout;

Boolean global_dep_opt = false;

/****************************** Option Processing *****************************/

void set_dep_opt(void *state)
{
  global_dep_opt = true;
}

void set_dep_ptr(void *state, char *str)
{
  if (!(global_dep_ptr = fopen(str, "w"))) 
  {
    cerr << "Error opening graph file " << str << "\n"
         << endl;
      
    exit (-1);
  }
}

void set_pgm_loc(void *state, char *str)
{
  global_pgm_loc = str;
}

void set_mod_loc(void *state, char *str)
{
  global_mod_loc = str;
}

void set_list_loc(void *state, char *str)
{
  global_list_loc = str;
}

static struct flag_	dep_f = {
  set_dep_opt,
  "dependence optiion",	
  "build and save the dependence graph for a program, module or list of modules",
};

static struct string_	dep_ptr_s = {
  set_dep_ptr,
  "output file for dependence graph",
  "output file for dependence graph",
  512, 50,
  ".*"
};

static struct string_	pgm_loc_s = {
  set_pgm_loc,
  "program context     ",
  "program context",
  512, 50,
  ".*"
};

static struct string_	mod_loc_s = {
  set_mod_loc,
  "module context     ",
  "module context",
  512, 50,
  ".*"
};

static struct string_	list_loc_s = {
  set_list_loc,
  "list context     ",
  "list context",
  512, 50,
  ".*"
};

Option pgm_loc_opt = 
{ string, 'P',  (Generic) "",   true, (Generic)&pgm_loc_s };

Option mod_loc_opt = 
{ string, 'M',  (Generic) "",   true, (Generic)&mod_loc_s };

Option list_loc_opt = 
{ string, 'L',  (Generic) "",   true, (Generic)&list_loc_s };

Option dep_ptr_opt = 
{ string, 'o',  (Generic) "",   true, (Generic)&dep_ptr_s };

Option dep_opt = 
{ flag,   'd', (Generic)false, true, (Generic)&dep_f };

/****************************** External Functions ****************************/

/******************************************************************************
 *
 *  Function
 *     CheckerOptsProcess (extern)
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
int CheckerOptsProcess(int argc, char **argv)
{
  Options checkerOptions("Checker command line options"); 

  checkerOptions.Add(&pgm_loc_opt); 
  checkerOptions.Add(&mod_loc_opt); 
  checkerOptions.Add(&list_loc_opt);
  checkerOptions.Add(&dep_ptr_opt);
  checkerOptions.Add(&dep_opt);

  if (opt_parse_argv(&checkerOptions, 0, argc, argv)) 
  {
    CheckerOptsUsage(argv[0]);
    return -1;
  }
  
  if ((global_pgm_loc && global_list_loc) ||
      (global_mod_loc && global_list_loc)) 
  {
    errorMsgHandler.HandleMsg ("Do not specify -M or -P with -L.\n");
    CheckerOptsUsage(argv[0]);
    return -1;
  }
  else if (!global_pgm_loc && !global_mod_loc && !global_list_loc) 
  {
    errorMsgHandler.HandleMsg("Specify one of -P, -M and -L (or -P and -M)\n");
    CheckerOptsUsage(argv[0]);
    return -1;
  }
}

/******************************* Static Functions *****************************/

/******************************************************************************
 *
 *  Function
 *     CheckerOptsUsage (static)
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
static void CheckerOptsUsage(char* pgm_name)
{
  cerr << "usage: " << pgm_name
       << " [-d]"
       << " [-o <output file>]]"
       << " [-P <composition>] | [-M <module> | -L <module list>]\n"
       << endl;

  cerr << "d - compute and save dependence graph"
       << endl;

  cerr << "o - output file for dependence graph\n"
       << "    if not specified, output goes in stdout"
       << endl;

  exit (-1);
}
