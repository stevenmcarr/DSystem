/* $Id: FortAnnotSrc.h,v 1.2 1997/03/11 14:30:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/FortAnnotSrc.h                                 */
/*                                                                      */
/*      FortAnnotSrc -- Abstract class for all Fortran Annotation       */
/*                      Sources                                         */
/*      Last edited: June 18, 1993 at 4:00 pm                           */
/*                                                                      */
/************************************************************************/




#ifndef FortAnnotSrc_h
#define FortAnnotSrc_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




EXTERN (void, FortAnnotSrc_Init, (void))


EXTERN (void, FortAnnotSrc_Fini, (void))






/************************************************************************/
/*      Object-oriented Interface                                       */
/************************************************************************/


#ifdef __cplusplus




/***********************************/
/* Fortran Annotation Source Class */
/***********************************/




class FortAnnotMgr;
class FortAnnot;
class CompoundFortAnnot;


struct FortAnnotSrc_Repr_struct;


class FortAnnotSrc: public DBObject
{
  public:

    FortAnnotSrc_Repr_struct * FortAnnotSrc_repr;


  public:

    /* initialization */

      META_DEF(FortAnnotSrc)

      FortAnnotSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                   Context module);

      virtual
      ~FortAnnotSrc(void);


    /* database */

      virtual
      void Open(Context context,
                Context mod_in_pgm_context,
                Context pgm_context,
                DB_FP * session_fp);

      virtual
      void Close(void);

      virtual
      void Save(Context context, DB_FP * fp);


    /* change notification */

      virtual
      void NoteChange(Object * ob, int kind, void * change);


    /* selection */

      virtual
      void GetGlobal(CompoundFortAnnot * fan);

      virtual
      void GetSelection(CompoundFortAnnot * fan, int l1, int c1,
                        int l2, int c2);


  protected:

    /* module access */

      Context getModule(void);
};






#endif /* __cplusplus */

#endif /* not FortAnnotSrc_h */
