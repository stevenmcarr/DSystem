/* $Id: val_ip.h,v 1.6 1997/03/27 20:47:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 *  Include file for interprocedural symbolic analysis routines that        *
 *  don't handle individual value numbers.                                  *
 *  Author: Paul Havlak                                                     *
 *                                                                          *
 *  Copyright 1993 Rice University, as part of the Rn/ParaScope             *
 *  Programming Environment Project.                                        *
 ****************************************************************************/

#ifndef val_ip_h
#define val_ip_h

#include <libs/support/misc/general.h>

typedef Generic ValNumber;

class FormattedFile;  // minimal external declaration
class ValPassMap;
class ValTable;

class ValIP {
  public:
    ValTable *values;
    ValPassMap *pass;

    ValIP();
    ~ValIP();

    //  ASCII database I/O routines
    Boolean Write(FormattedFile& port);
    Boolean Read(FormattedFile& port);
    void Dump();
};

#endif val_ip_h
