/* $Id: FortScrap.C,v 1.1 1997/06/25 13:43:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/FortScrap.c					*/
/*									*/
/*	FortScrap -- fragments of Fortran for the public scrap		*/
/*	Last edited: October 22, 1987 at 8:52 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/NedScrap.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditor.i>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortScrap.h>

#include <libs/support/arrays/FlexibleArray.h>







/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    FortTree	ft;
    Boolean	small;
    TextString	text;
    Flex *	flex;

    int		refcount;

  } fsc_Repr;

#define	R(ob)		((fsc_Repr *) ob)




typedef struct
  {
    TextString  text;
    int         conceal;
  }  ScrapElement;






/************************/
/*  Scrap storage	*/
/************************/




static char *  fsc_typeName = "Fortran source";

STATIC(void, destroyScrap,(SC_Type type, Generic owner, Generic scrap));

static SC_Methods fsc_methods =
  {
    destroyScrap
  };



static FortTree fsc_scrapTree;






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void fsc_Init()
{
  fsc_Type = sc_Register(fsc_typeName,&fsc_methods);
  /*** fsc_scrapTree = ft_Open(tempContext("FortranScrap"),UNUSED); ***/
}




void fsc_Fini()
{
  /*** ft_Close(fsc_scrapTree); ***/
  sc_Unregister(fsc_Type);
}




SC_Type fsc_Type;




FortScrap fsc_CreateSmall(FortTree ft, TextString text, int start, int count)
{
  FortScrap fsc;

  fsc = (FortScrap) get_mem(sizeof(fsc_Repr),"FortScrap");
  R(fsc)->ft = ft;
  R(fsc)->small = true;
  R(fsc)->text = subTextString(text,start,count);
  R(fsc)->refcount = 1;

  return fsc;
}




FortScrap fsc_CreateLarge(FortTree ft)
{
  FortScrap fsc;

  fsc = (FortScrap) get_mem(sizeof(fsc_Repr),"FortScrap");
  R(fsc)->ft = ft;
  R(fsc)->small = false;
  R(fsc)->flex = flex_create(sizeof(ScrapElement));
  R(fsc)->refcount = 1;

  return fsc;
}




void fsc_AddLarge(FortScrap fsc, TextString text, int conceal)
{
  Flex * flex = R(fsc)->flex;
  ScrapElement elem;

  elem.text = copyTextString(text);
  elem.conceal = conceal;
  flex_insert_one(flex, flex_length(flex), (char *) &elem);
}




void fsc_Destroy(FortScrap fsc)
{
  Flex * flex;
  int k;
  ScrapElement elem;

  if( R(fsc)->refcount > 1 )
    R(fsc)->refcount -= 1;
  else
    { if( R(fsc)->small )
        destroyTextString(R(fsc)->text);
      else
        { flex = R(fsc)->flex;
          for( k = 0;  k < flex_length(flex);  k++ )
            { (void) flex_get_buffer(flex,k,1,(char*) &elem);
              destroyTextString(elem.text);
            }
          flex_destroy(R(fsc)->flex);
        }

      free_mem((void*) fsc);
    }
}





/************************/
/*  Scrap access	*/
/************************/



Boolean fsc_IsEmpty(FortScrap fsc)
{
  if( R(fsc)->small )
    return BOOL( R(fsc)->text.num_tc == 0 );
  else
    return BOOL( flex_length(R(fsc)->flex) == 0 );
}




Boolean fsc_IsSmall(FortScrap fsc)
{
  return R(fsc)->small;
}




TextString fsc_GetSmall(FortScrap fsc)
{
  if( R(fsc)->small )
    return R(fsc)->text;
  else
   { message("fsc_GetSmall: not small");
     return emptyTextString;
   }
}




int fsc_GetLargeLength(FortScrap fsc)
{
  if( ! R(fsc)->small )
    return flex_length(R(fsc)->flex);
  else
    { message("fsc_GetLargeLength: not large");
      return UNUSED;
    }
}




void fsc_GetLarge(FortScrap fsc, int k, TextString *text, int *conceal)
{
  ScrapElement elem;

  if( ! R(fsc)->small )
    { (void) flex_get_buffer(R(fsc)->flex,k,1,(char *) &elem);
      *text = elem.text;
      *conceal = elem.conceal;
    }
  else
    die_with_message("fsc_GetLarge: not large");
}




FortScrap fsc_Copy(FortScrap fsc)
{
  R(fsc)->refcount += 1;
  return fsc;
}





/************************/
/*  Scrap protocol	*/
/************************/




/* ... */






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*ARGSUSED*/
static
void destroyScrap(SC_Type type, Generic owner, Generic scrap)
{
  fsc_Destroy((FortScrap) scrap);
}



