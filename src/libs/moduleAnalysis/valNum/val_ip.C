/* $Id: val_ip.C,v 1.6 1997/03/11 14:36:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 *  Support for interprocedural symbolic analysis routines that             *
 *  don't handle individual value numbers.                                  *
 *  Author: Paul Havlak                                                     *
 *                                                                          *
 *  Copyright 1993 Rice University, as part of the Rn/ParaScope             *
 *  Programming Environment Project.                                        *
 ****************************************************************************/

#include <libs/moduleAnalysis/valNum/val.h>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/moduleAnalysis/valNum/val_ht.h>
#include <libs/moduleAnalysis/valNum/val_pass.h>

Boolean ValIP::Read(FormattedFile& port)
{
    Boolean retFlag = true;

    if (values) delete values;
    values = new ValTable;
    retFlag = (Boolean)(retFlag && values->Read(port));

    if (pass) delete pass;
    pass = new ValPassMap;
    retFlag = (Boolean)(retFlag && pass->Read(port));

    return retFlag;
}

Boolean ValIP::Write(FormattedFile& port)
{
    Boolean retFlag = true;

    if (values)
    {
	retFlag = (Boolean)(retFlag && values->Write(port));
	if (pass) retFlag = (Boolean)(retFlag && pass->Write(port));
    }
    return retFlag;
}

void ValIP::Dump()
{
    if (values)
    {
	val_Dump(values);
	if (pass) pass->Dump();
    }
}

ValIP::ValIP()
{
    values   = (ValTable *) 0;
    pass     = (ValPassMap *) 0;
}

ValIP::~ValIP()
{
    if (values)  delete values;
    if (pass)    delete pass;
}
