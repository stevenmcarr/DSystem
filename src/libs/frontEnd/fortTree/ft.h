/* $Id: ft.h,v 1.8 1997/03/11 14:29:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ft_h
#define ft_h

#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include <libs/support/tables/cNameValueTable.h>

#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/treeutil.h>

/* local includes */
#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/FortTree.i>
#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/TypeChecker.h>

class NeedProvSet;

struct FortTree_internal_structure {
        Asttab *         asttab;
        FortTreeNode     root;
        ft_States        state;

        TableDescriptor  td;
        NeedProvSet        *needs;
        NeedProvSet        *provs;

        /* hash tables to map nodes to stable numbers that may be saved in the
         * file system
         */
        cNameValueTable NodeToNumber;
        cNameValueTable NumberToNode;
};

#endif
