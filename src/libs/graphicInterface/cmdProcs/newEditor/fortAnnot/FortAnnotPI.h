/* $Id: FortAnnotPI.h,v 1.2 1997/03/11 14:30:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/FortAnnotPI.h                                  */
/*                                                                      */
/*      FortAnnot - Procedural interfaces for Fortran Annotation.       */
/*      Last edited: June 18, 1993 at 4:00 pm                           */
/*                                                                      */
/************************************************************************/




#ifndef FortAnnotPI_h
#define FortAnnotPI_h


#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortVFilter.h>
#include <libs/graphicInterface/cmdProcs/newEditor/FortEditor.h>
#include <libs/graphicInterface/include/FortEditorCP.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




/*******************/
/*  Miscellaneous  */
/*******************/


/* Goto function function pointer typedef */
  typedef FUNCTION_POINTER(void, fam_GotoFunc, (Generic edcp));


/* goto link enumerated values */
  typedef enum
    { LINK_TO_NODE,
      LINK_TO_FIRST,
      LINK_TO_LAST,
      LINK_UNUSED
    } goto_Link;






/******************************************/
/*  FortAnnotMgr Initialization Routines  */
/******************************************/


EXTERN (void, FortAnnotMgr_Init,   (void));

EXTERN (void, FortAnnotMgr_Fini,   (void));






/************************************/
/*  Source Initialization Routines  */
/************************************/


EXTERN (void, InterProcSrc_Init,   (void));

EXTERN (void, InterProcSrc_Fini,   (void));

EXTERN (void, ContentsSrc_Init,    (void));

EXTERN (void, ContentsSrc_Fini,    (void));

EXTERN (void, DeclSrc_Init,        (void));

EXTERN (void, DeclSrc_Fini,        (void));

EXTERN (void, LoopsSrc_Init,       (void));

EXTERN (void, LoopsSrc_Fini,       (void));






/**************************************/
/*  Fort Annotation Manager Routines  */
/**************************************/


EXTERN (Generic,   fam_Open,            (Context context, DB_FP * fp,
                                         FortEditorCP edcp,
                                         fam_GotoFunc gotoFunc,
                                         FortEditor ed));

EXTERN (void,      fam_Close,           (Generic fam));

EXTERN (void,      fam_Save,            (Generic fam, Context context,
                                         DB_FP * fp));

EXTERN (Generic,   fam_GetGlobal,       (Generic fam));

EXTERN (Generic,   fam_GetSelection,    (Generic fam, int l1, int c1,
                                         int l2, int c2));






/******************************/
/*  Fort Annotation Routines  */
/******************************/


EXTERN (Generic,   fan_CreateSimple,    (char * name, Boolean sorted,
                                         int source, Generic ob));

EXTERN (Generic,   fan_CreateCompound,  (char * name, Boolean sorted,
                                         int source, Generic ob));

EXTERN (void,      fan_Destroy,         (Generic fan));

EXTERN (void,      fan_GetName,         (Generic fan, char * *name));

EXTERN (Boolean,   fan_IsCompound,      (Generic fan));

EXTERN (void,      fan_AddTextLine,     (Generic fan, char * text,
                                         Context module, FortTreeNode node,
                                         goto_Link kind));

EXTERN (int,       fan_NumTextLines,    (Generic fan));

EXTERN (void,      fan_GetTextLine,     (Generic fan, int k, char * *text));

EXTERN (Boolean,   fan_HasLink,         (Generic fan, int k));

EXTERN (void,      fan_GotoLink,        (Generic fan, int k));

EXTERN (void,      fan_AddElement,      (Generic fan, Generic elem));

EXTERN (int,       fan_NumElements,     (Generic fan));

EXTERN (Generic,   fan_GetElement,      (Generic fan, int k));






#endif /* not FortAnnotPI_h */
