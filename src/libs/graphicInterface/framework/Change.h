/* $Id: Change.h,v 1.2 1997/03/11 14:32:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Change.h						*/
/*									*/
/*	Change -- Well-known change kinds				*/
/*	Last edited: August 29, 1993 at 5:57 pm				*/
/*									*/
/************************************************************************/




#ifndef Change_h
#define Change_h




#define CHANGE_UNKNOWN			   0
#define CHANGE_DOCUMENT			   1
#define CHANGE_FILTER			   2
#define CHANGE_SELECTION		   3

#define CHANGE_TREE_WILL_CHANGE		1001
#define CHANGE_TREE_CHANGED		1002

#define CHANGE_LOOP			2001
#define CHANGE_DEPENDENCE		2002
#define CHANGE_VARIABLE			2003

#define CHANGE_HEADING			4001

#define CHANGE_MARKING_PREDICATE	5001

#define CHANGE_CURRENT_DEPENDENCE	6001
#define CHANGE_SRC_SELECTION		6002
#define CHANGE_NAVIGATE_TO 		6003






#endif /* not Change_h */
