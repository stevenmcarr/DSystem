/* $Id: ped_dialogs.h,v 1.6 1997/03/11 14:31:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/ped_include/ped_dialogs.h				*/
/*									*/
/*	ped_dialogs.h -- include file for all ped dialogs		*/
/*									*/
/*									*/
/************************************************************************/

#ifndef ped_dialogs_h
#define ped_dialogs_h

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>
#include <libs/graphicInterface/oldMonitor/include/items/radio_btns.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_structs.h>

/* number of variables wide the dialog allows */
#define		DEF_VARS	10

	/* dialog selections */
#define		NO_SELECTION		-1

#define		TEXT_TYPE		1
#define 	TEXT_SRC		2
#define		TEXT_SINK		3
#define		TEXT_DIMS		4
#define		TEXT_BLOCK		5
#define		TEXT_NAME		56

#define		HIDE			6   /* Used in the dependence display filter */
#define		SHOW			7
#define		SHOWALL			8
#define		POP			9
#define		PUSH	       		10

#define		SORT_BY_TYPE   		11
#define		SORT_BY_SRC    		12
#define		SORT_BY_SINK   		13
#define		SORT_BY_BLOCK  		14
#define		SORT_BY_DIMS   		15

#define		NAME			16
#define		INFO			17

#define		DISTRIBUTE_BUTTON	18
#define		INTERCHANGE_BUTTON	19
#define		FUSION_BUTTON		20
#define		SCALAR_BUTTON		21
#define		DELETE_STMT_BUTTON	22

#define		TCHOICE			23

#define		DISTRIBUTE_DO		24
#define		INTERCHANGE_DO		25
#define		SCALAR_DO		26
#define		DELETE_DO		27
#define		SINTER_DO		28
#define		PEEL_FIRST		29
#define		PEEL_LAST		30
#define		DO_STRIP		31
#define		STEP_SIZE		32 
#define		RENAMING_DO		33

#define		PLIST			49
#define		SLIST			50
#define 	CLEAR			51 /* Used in the dependence display filter */

#define		RIGHT_ARROW		52
#define		LEFT_ARROW		53
#define		CLASSIFY		54

#define		REMOVE			55
#define		DELETE			55  /* Used in the dependence display filter */

#define		SHAR_INFO		57 /* Used in the shared var display */
#define		PRIV_INFO		58
#define		ALL_PRIV		59 

#define		SKEW_ONE		60
#define		SKEW_DEGREE		61
#define		ITERS			62

#define		REVERSE_DO		70
#define		REVERSE_ARG		71

#define		ADJUST_DO		72
#define		ADJUST_ARG		73

#define		ALIGN_DO		74
#define		ALIGN_ARG		75

#define		REPLACE_S_DO	76
#define		REPLACE_S_ARG	77

#define		UNROLL_DO		78
#define		UNROLL_ARG		79

#define		UNROLL_JAM_DO	80
#define		UNROLL_JAM_ARG	81

#define		FUSE_DO			80
#define		FUSE_ARG		81

#define		ADD_STMT_DO		82
#define		ADD_STMT_ARG	83

#define		UNSWITCH_DO		84

/* 40, and 41, 56 are reserved */

extern int DEFAULT_DISTR_HEIGHT;

#endif



