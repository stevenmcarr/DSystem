/* $Id: Dependence.h,v 1.2 1997/03/11 14:32:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Dependence.h						*/
/*									*/
/*	Dependence -- Type definitions for dependences at the UI	*/
/*	Last edited: October 16, 1993 at 11:21 pm			*/
/*									*/
/************************************************************************/




#ifndef Dependence_h
#define Dependence_h




#include <libs/moduleAnalysis/dependence/interface/depType.h>
#include <libs/frontEnd/fortTree/FortTree.h>




typedef struct
  {
    void *		edge;
    
    DependenceType	type;
    FortTreeNode	src;
    FortTreeNode	sink;
    int			level;

    Boolean		loopCarried;
    Boolean		loopIndependent;
    Boolean		control;
    Boolean		xprivate;

  } Dependence;




#endif /* not Dependence_h */
