/* $Id: UserFilter.C,v 1.6 1997/03/11 14:32:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/UserFilter.C						*/
/*									*/
/*	UserFilter -- ViewFilter programmed in user notation		*/
/*	Last edited: October 13, 1993 at 11:00 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/UserFilter.h>

#include <libs/graphicInterface/framework/UserFilterDef.h>
#include <libs/support/lists/list.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* UserFilter object */

typedef struct UserFilter_Repr_struct
  {
    /* creation parameters */
      UserFilterDef *	def;
      void *		environment;

    /* entry in def's list */
      UtilNode *	node;

    /* status */
      Boolean		doConcealed;
      Boolean		doErrors;
      int		elision;

  } UserFilter_Repr;


#define R(ob)		(ob->UserFilter_repr)

#define INHERITED	CViewFilter






/*************************/
/*  Miscellaneous	 */
/*************************/




/* answer from 'UserFilter::GetName' */

static char UserFilter_name[200];







/*************************/
/*  Forward declarations */
/*************************/




/* none */






/************************************************************************/
/*	Interface Operations						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void UserFilter::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CViewFilter);
    REQUIRE_INIT(UserFilterDef);
}




void UserFilter::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(UserFilter)




UserFilter::UserFilter(Context context, DB_FP * session_fp,
                       UserFilterDef * def, void * environment)
   : CViewFilter (context, session_fp)
{
  char * dummy;
  Boolean concealed, errors;

  /* allocate instance's private data */
    this->UserFilter_repr = (UserFilter_Repr *) get_mem(sizeof(UserFilter_Repr),
                                                        "UserFilter instance");

  /* save creation arguments */
      R(this)->def = def;
      R(this)->def->addFilter(this);

      R(this)->environment = environment;

    /* initialize view modifiers */
      R(this)->def->GetDefinition(dummy, concealed, errors);
      R(this)->doConcealed = concealed;
      R(this)->doErrors    = errors;
      R(this)->elision     = ViewFilter_ELISION_ELLIPSIS;
}




UserFilter::~UserFilter()
{
  R(this)->def->removeFilter(this);
  free_mem((void*) this->UserFilter_repr);
}






/*****************************/
/* Access to filter settings */
/*****************************/




char * UserFilter::GetName(Boolean withError)
{
  UserFilterDef * def = R(this)->def;
  char * defName;
  char * dummy;
  Boolean concealed, errors;

  def->GetName(defName);
  (void) strcpy(UserFilter_name, defName);

  if( withError )
    { def->GetDefinition(dummy, concealed, errors);

      if( R(this)->doErrors  &&  ! errors )
        (void) strcat(UserFilter_name, " + errors");

      else if( ! R(this)->doErrors  &&  errors )
        (void) strcat(UserFilter_name, " - errors");
    }

  return UserFilter_name;
}




void UserFilter::SetShowErrors(Boolean show)
{
  R(this)->doErrors = show;
  this->NoteChange(nil, 0, nil);	/* ??? */
}




void UserFilter::GetShowErrors(Boolean &show)
{
  show = R(this)->doErrors;
}






/*************/
/* Filtering */
/*************/




Boolean UserFilter::filterLine(Boolean countOnly,
                               int line,
                               int &subline,
                               TextString &text,
                               TextData &data)
{
  return R(this)->def->filterLine(this, countOnly, line, subline, text, data,
                                  R(this)->environment);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */

