/* $Id: FortAnnotMgr.C,v 1.4 1997/03/11 14:30:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/FortAnnotMgr.C                                 */
/*                                                                      */
/*      FortAnnotMgr  -- annotation manager mechanism for Fortran       */
/*      Last edited: August 25, 1993 at 4:02 pm				*/
/*                                                                      */
/************************************************************************/


#include <assert.h>


#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotSrc.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnot.h>






/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/*************************************/
/*  FortAnnotManager Representation  */
/*************************************/




/* FortAnnotManager objects */

typedef struct FortAnnotMgr_Repr_struct
  {
    Context             module;

  /* sub annotation sources */
    FortAnnotSrc * *    sources;

  } FortAnnotMgr_Repr;


#define R(ob)           (ob->FortAnnotMgr_repr)


#define FAMINHERITED    DBObject




/* registered annotation sources */

#define MAX_SOURCES    10

static int fam_numSources = 0;

static fam_MakeSrcFunc fam_makeSrc[1 + MAX_SOURCES];






/************************/
/*  Miscellaneous       */
/************************/


static int FortAnnotMgr_initCount = 0;




/* record of known modules */

#define MAX_MODULES 99

typedef struct
  {
    Context        module;
    FortEditorCP   edcp;
    fam_GotoFunc   gotoFunc;
    FortEditor     ed;

  } fam_ModuleInfo;

static fam_ModuleInfo fam_modules[MAX_MODULES];

static int fam_numModules = 0;




/************************/
/* Forward declarations */
/************************/


static int   findModule(Context module);






/************************************************************************/
/*        Interface Operations                                          */
/************************************************************************/




/************************/
/*  Initialization      */
/************************/




void FortAnnotMgr_Init(void)
{
  if( FortAnnotMgr_initCount++ == 0 )
    { /* ... */
    }
}




void FortAnnotMgr_Fini(void)
{
  if( --FortAnnotMgr_initCount == 0 )
    { /* ... */
    }
}






/******************************/
/*  Ned procedural interface  */
/******************************/




Generic fam_Open(Context context, DB_FP * fp, FortEditorCP edcp,
                 fam_GotoFunc gotoFunc, FortEditor ed)
{
  FortAnnotMgr * fam;

  fam = new FortAnnotMgr(context, fp, context);
  fam->Open(context, fp, context, edcp, gotoFunc, ed);

  return (Generic) fam;
}




void fam_Close(Generic fam)
{
  FortAnnotMgr * fam_ptr;

  fam_ptr = (FortAnnotMgr *) fam;
  fam_ptr->Close();
}




void fam_Save(Generic fam, Context context, DB_FP * fp)
{
  FortAnnotMgr * fam_ptr;

  fam_ptr = (FortAnnotMgr *) fam;
  fam_ptr->Save(context, fp);
}




Generic fam_GetGlobal(Generic fam)
{
  FortAnnotMgr * fam_ptr;

  fam_ptr = (FortAnnotMgr *) fam;
  return (Generic) fam_ptr->GetGlobal();
}




Generic fam_GetSelection(Generic fam, int l1, int c1, int l2, int c2)
{
  FortAnnotMgr * fam_ptr;

  fam_ptr = (FortAnnotMgr *) fam;
  return (Generic) fam_ptr->GetSelection(l1, c1, l2, c2);
}






/***********************/
/*  Access to modules  */
/***********************/




void accessModule(Context module, FortEditor * ed, FortTree * ft,
                  FortTreeNode * root)
{
  int m = findModule(module);

  *ed = fam_modules[m].ed;
  ed_GetTree(*ed, ft);
  ed_GetRoot(*ed, root);
}




void gotoLink(Context module, FortTreeNode node, goto_Link kind)
{
  int m = findModule(module);
  FortEditor ed = fam_modules[m].ed;
  FortEditor edcp = fam_modules[m].edcp;
  fam_GotoFunc gotoFunc = fam_modules[m].gotoFunc;
  int line1, line2, dummy;

  switch( kind )
    {
      case LINK_TO_NODE:
        ed_SetSelectedNode(ed, node);
        break;

      case LINK_TO_FIRST:
        ed_NodeToText(ed, node, &line1, &dummy, &dummy, &dummy);
        ed_SetSelection(ed, UNUSED, line1, line1);
        break;

      case LINK_TO_LAST:
        ed_NodeToText(ed, node, &dummy, &dummy, &line2, &dummy);
        ed_SetSelection(ed, UNUSED, line2, line2);
        break;

      default:
        break;
    }

  gotoFunc(edcp);
}






/****************************/
/* Instance Initialization  */
/****************************/




META_IMP(FortAnnotMgr)




FortAnnotMgr::FortAnnotMgr(Context context, DB_FP * fp, Context module)
             : DBObject (context, fp)
{
  /* allocate a new instance */
    this->FortAnnotMgr_repr = (FortAnnotMgr_Repr *) 
                              get_mem(sizeof(FortAnnotMgr_Repr),
                                      "FortAnnot:FortAnnotManager");

  /* initialize the parts */
    /* set creation parameters */
      R(this)->module = module;

  /* make subparts */
    R(this)->sources = (FortAnnotSrc * *)
                       get_mem((1+fam_numSources) * sizeof(FortAnnotSrc *),
                               "FortAnnot:sources");
}




FortAnnotMgr::~FortAnnotMgr(void)
{
  /* free annotation manager */
    free_mem((void*) R(this)->sources);
    free_mem((void*) this->FortAnnotMgr_repr);
}






/*************************/
/*  Source registration  */
/*************************/




void FortAnnotMgr::RegisterSrc(fam_MakeSrcFunc makeSrcFunc)
{
  if( fam_numSources < MAX_SOURCES )
    { fam_numSources += 1;
      fam_makeSrc[fam_numSources] = makeSrcFunc;
    }
  else
      die_with_message("FortAnnot: too many annotation sources.");
}






/**************/
/*  Database  */
/**************/




void FortAnnotMgr::Open(Context context, DB_FP * fp, Context module,
                        FortEditorCP edcp, fam_GotoFunc gotoFunc, FortEditor ed)
{
  int k, m;
  fam_MakeSrcFunc makeSrcFunc;

  /* allocate and open sources */
    for( k = 1;  k <= fam_numSources;  k++ )
      {
        makeSrcFunc = fam_makeSrc[k];
        R(this)->sources[k] = makeSrcFunc(context, fp, this, module);
      }

  /* record this as a known module */
    m = findModule(CONTEXT_NULL);
    if( m == UNUSED )
      { m = (fam_numModules += 1);
        if( fam_numModules >= MAX_MODULES )
          die_with_message("FortAnnot: too many modules.");
      }

    fam_modules[m].module    = module;
    fam_modules[m].edcp      = edcp;
    fam_modules[m].gotoFunc  = gotoFunc;
    fam_modules[m].ed        = ed;
}




void FortAnnotMgr::Close(void)
{
  int k;

  /* Destroy annotation sources */
    for( k = 1;  k <= fam_numSources;  k++ )
      {
        R(this)->sources[k]->Close();
      }

  k = findModule(R(this)->module);

  assert(k >= 0); // 12 Sept 1994 -- JMC 

  fam_modules[k].module = CONTEXT_NULL;
  fam_numModules--;

  /* close annotation manager */
    this->FAMINHERITED::Close();
}




void FortAnnotMgr::Save(Context context, DB_FP * fp)
{
  int k;

  for( k = 1;  k <= fam_numSources;  k++ )
    R(this)->sources[k]->Save(context, fp);
}






/*************************/
/*  Change notification  */
/*************************/




void FortAnnotMgr::NoteChange(Object * ob, int kind, void * change)
{
  /* ... */
}






/***************/
/*  Selection  */
/***************/




FortAnnot * FortAnnotMgr::GetGlobal(void)
{
  CompoundFortAnnot * fan;
  int k;

  fan = new CompoundFortAnnot("<<top level>>", (FortAnnotSrc*)nil, false);
  fan->Init();

  for( k = 1;  k <= fam_numSources;  k++ )
    R(this)->sources[k]->GetGlobal(fan);

  return fan;
}




FortAnnot * FortAnnotMgr::GetSelection(int l1, int c1, int l2, int c2)
{
  CompoundFortAnnot * fan;
  int k;

  fan = new CompoundFortAnnot("<<top level>>", (FortAnnotSrc*)nil, false);
  fan->Init();

  for( k = 1;  k <= fam_numSources;  k++ )
    R(this)->sources[k]->GetSelection(fan, l1, c1, l2, c2);

  return fan;
}






/************************************************************************/
/*          Private Operations                                          */
/************************************************************************/




static
int findModule(Context module)
{
  int k;

  for( k = 1;  k <= fam_numModules;  k++ )
    if( fam_modules[k].module == module )
      return k;

  return UNUSED;
}


