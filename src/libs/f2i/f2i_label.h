/* $Id: f2i_label.h,v 1.3 1999/03/31 21:56:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdio.h>
#include <libs/support/memMgmt/mem.h>

#define LABEL_TABLE_IS_FULL       -1
#define LABEL_DEFAULT_SIZE	2047

#define LABEL_NEW 	aiNextLabel++

extern int label_table_dimension;

struct label_element   		/*  element of the label table  */
  {
    int	 Fortran;		/* source language label	*/
    int	 iloc;			/* target language label	*/
    int  assigned;		/* !0 => used in assign stmt	*/
    int  defined;		/* !0 => has been defined	*/
    AST_INDEX stmt_node;	/* used to track FORMAT stmts	*/
  };

extern struct label_element *label_table;  /*  name table  */

