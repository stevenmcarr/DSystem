/* $Id: invoke_main.C,v 1.1 1997/06/25 15:18:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*==========================================================================*/
/* 			      invoke_main.c                                 */
/* 									    */
/* This routine invokes a main function (main_function_name).               */
/* The options passed to the main function are given in optsList.           */
/* optsList is converted to the standard argc, argv format.                 */
/*                                                                          */
/*==========================================================================*/
#include <stdio.h>

#include <libs/support/optParsing/invoke_main.h>

STATIC(void, StringToArgcAndArgv, (char* optsList, int* argc, char** argv));

int 
invoke_main (InvokeMain_FunctPtr mainFunctionName, char* optsList)
{
  int    argc;
  char*  argv[64];

  StringToArgcAndArgv(optsList, &argc, argv);

  return mainFunctionName(argc, argv);
}

/*==========================================================================*/
/* This function counts the substrings within optsList delimited by spaces. */
/* The elements of argv[] are set to point to the successive substrings.    */
/* The spaces in optsList are replaced with '\0'.                           */
/*==========================================================================*/
static void 
StringToArgcAndArgv(char* optsList, int* argc, char** argv)
{
  Boolean lastOpt = false;
  int     k = 0, count = 0;

  argv[0] = &optsList[0];

  while (lastOpt == false) 
  {
     while (optsList[k] != '\0' && optsList[k] != ' ')
     {
        k++;
     }

     count++;

     if (optsList[k] == '\0') 
     {
        lastOpt = true;
     }
     else 
     {
        optsList[k] = '\0';
        k++; 
        while (optsList[k] == ' ') k++;
        argv[count] = &optsList[k];
     }

  }

  *argc = count;

  return;
}
