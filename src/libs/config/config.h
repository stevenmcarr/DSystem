/* $Id: config.h,v 1.4 1997/03/11 14:27:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    config.h
 *
 * Author
 *    Kevin Cureton - major revision
 *
 * Date
 *    Aug 1994
 *
 * Description
 *    Resolve the configuration from the configuration file.  Prints the
 *    copyright message, etc.
 *
 ******************************************************************************/

#ifndef config_h
#define config_h

   /* read the configuration file and set the global variables of import */
EXTERN(void, resolveConfiguration, (void));

   /* process and delete standard arguments, and return number of */
   /* arguments remaining                                         */
EXTERN(int,  filterStandardArgs, (int argc, char **argv));

#endif
