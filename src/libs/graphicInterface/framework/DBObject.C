/* $Id: DBObject.C,v 1.7 1997/03/11 14:32:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/DBObject.C						*/
/*									*/
/*	DBObject -- Abstract class for all persistent objects		*/
/*	Last edited: October 13, 1993 at 5:40 pm      			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DBObject object */

typedef struct DBObject_Repr_struct
  {
    /* creation parameters */
      /* ... */

  } DBObject_Repr;


#define R(ob)		(ob->DBObject_repr)

#define INHERITED	Object






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




void DBObject::InitClass(void)
{
  /* ... */
}




void DBObject::FiniClass(void)
{
  /* ... */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DBObject)




DBObject::DBObject(Context context, File * session_fp)
{
  /* allocate instance's private data */
    this->DBObject_repr = (DBObject_Repr *) get_mem(sizeof(DBObject_Repr),
                                                    "DBObject instance");

  /* save creation parameters */
    /* ... */
}




DBObject::~DBObject(void)
{
  free_mem((void*) this->DBObject_repr);
}






/*************/
/*  Database */
/*************/




void DBObject::Open(Context context,
		    Context mod_in_pgm_context,
		    Context pgm_context,
		    File * session_fp)
{
  Boolean existing;
  File *fp;
  char * attr;

  /* this test ought to be whether 'context' specifies a module-version */
    existing = BOOL(session_fp != DB_NULLFP);

  if( ! existing )
    this->isnew(context);
  else
    { this->GetAttribute(attr);
      if( strlen(attr) > 0 )
        fp = context->GetExternalAttributeFile(attr, 1);
      else
        fp = 0;

      this->read(fp, session_fp);

      if ( fp != 0 )
	context->CloseExternalAttributeFile(fp);
    }
}




void DBObject::Close(void)
{
  /* close object */
    this->INHERITED::Destroy();
}




void DBObject::Save(Context context, File * session_fp)
{
  File *fp;
  char * attr;

  this->GetAttribute(attr);
  if( strlen(attr) > 0 )
    fp =  context->CreateExternalAttributeFile(attr);
  else
    fp = 0;

  this->write(fp, session_fp);

  if( fp != 0 )
    context->CloseExternalAttributeFile(fp);
}




void DBObject::GetAttribute(char * &attr)
{
  /* to make other no-op methods work smoothly */

  attr = "";
}






/***********************/
/*  Protected methods  */
/***********************/




void DBObject::isnew(Context context)
{
  /* nothing */
}




void DBObject::read(File* fp, File * session_fp)
{
  /* nothing */
}




void DBObject::write(File * fp, File * session_fp)
{
  /* nothing */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
