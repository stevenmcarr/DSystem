/* $Id: ColumnEditor.C,v 1.2 1997/03/11 14:32:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/ColumnEditor.C					*/
/*									*/
/*	ColumnEditor -- Abstract class for column-oriented editors	*/
/*	Last edited: October 13, 1993 at 12:34 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/ColumnEditor.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* ColumnEditor object */

typedef struct ColumnEditor_Repr_struct
  {
    /* status */
      int			sortColumn;

  } ColumnEditor_Repr;


#define R(ob)		(ob->ColumnEditor_repr)


#define INHERITED	LineEditor






/*************************/
/*  Forward declarations */
/*************************/




/* none */






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void ColumnEditor::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(LineEditor);
}




void ColumnEditor::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(ColumnEditor)




ColumnEditor::ColumnEditor(Context context,
                           DB_FP * session_fd)
            : LineEditor(context, session_fd)
{
  /* allocate instance's private data */
    this->ColumnEditor_repr = (ColumnEditor_Repr *) get_mem(sizeof(ColumnEditor_Repr),
                                                            "ColumnEditor instance");
  /* initialize status */
    R(this)->sortColumn = UNUSED;
}




ColumnEditor::~ColumnEditor()
{
  free_mem((void*) this->ColumnEditor_repr);
}






/************/
/* Contents */
/************/




void ColumnEditor::SetSortColumn(int colNum)
{
  R(this)->sortColumn = colNum;
}




void ColumnEditor::GetSortColumn(int &colNum)
{
  colNum = R(this)->sortColumn;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */

