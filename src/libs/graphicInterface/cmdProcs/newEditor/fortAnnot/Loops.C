/* $Id: Loops.C,v 1.3 1997/03/11 14:30:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/Loops.C                                        */
/*                                                                      */
/*      Loops -- loop-nesting Fortran annotation and source             */
/*      Last edited: June 18, 1993 at 4:00 pm                           */
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotSrc.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnot.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/Loops.h>

#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/************************************/
/*  LoopsSource Representation      */
/************************************/




/* LoopsSource objects */

typedef struct LoopsSrc_Repr_struct
  {
    Context             module;

  } LoopsSrc_Repr;


#define RS(ob)          (ob->LoopsSrc_repr)


#define LSINHERITED     FortAnnotSrc






/************************************/
/*  LoopsAnnotation Representation  */
/************************************/




/* LoopsAnnotation objects */

typedef struct LoopsAnnot_Repr_struct
  {
    LoopsSrc *          loo;
    FortTreeNode        node;

  } LoopsAnnot_Repr;


#define RA(ob)          (ob->LoopsAnnot_repr)


#define LAINHERITED     SimpleFortAnnot






/************************/
/*  Miscellaneous       */
/************************/


static int LoopsSrc_initCount = 0;

static int LoopsAnnot_initCount = 0;






/************************/
/* Forward declarations */
/************************/


static FortAnnotSrc * makeSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                              Context module);
static LoopsAnnot *   makeLA(LoopsSrc * loo, FortTreeNode node);






/************************************************************************/
/*      Interface Operations                                            */
/************************************************************************/




/*****************************/
/*  LoopsSrc Initialization  */
/*****************************/




void LoopsSrc_Init(void)
{
  if( LoopsSrc_initCount++ == 0 )
    { /* register with the manager of annotation sources */
        FortAnnotMgr::RegisterSrc(makeSrc);
    }
}




void LoopsSrc_Fini(void)
{
  if( --LoopsSrc_initCount == 0 )
    { /* nothing */
    }
}






/*******************************/
/*  LoopsAnnot Initialization  */
/*******************************/




void LoopsAnnot_Init(void)
{
  if( LoopsAnnot_initCount++ == 0 )
    { /* nothing */
    }
}




void LoopsAnnot_Fini(void)
{
  if( --LoopsAnnot_initCount == 0 )
    { /* nothing */
    }
}






/*************************************/
/* LoopsSrc Instance Initialization  */
/*************************************/




META_IMP(LoopsSrc)




LoopsSrc::LoopsSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                   Context module)
         : FortAnnotSrc (context, fp, fam, module)
{
  /* allocate a new instance */
    this->LoopsSrc_repr = (LoopsSrc_Repr *) get_mem(sizeof(LoopsSrc_Repr),
                                                    "Loops:LoopsSource");
}




LoopsSrc::~LoopsSrc(void)
{
  /* free annotation source */
    free_mem((void*) this->LoopsSrc_repr);
}






/**************/
/*  Database  */
/**************/




void LoopsSrc::Open(Context context, Context mod_in_pgm_context,
                    Context pgm_context, DB_FP * session_fp)
{
  /* initialize the parts */
    RS(this)->module = this->getModule();
}




void LoopsSrc::Close(void)
{
  /* close annotation source */
    this->LSINHERITED::Close();
}




void LoopsSrc::Save(Context context, DB_FP * fp)
{
  /* nothing */
}






/*************************/
/*  Change notification  */
/*************************/




void LoopsSrc::NoteChange(Object * ob, int kind, void * change)
{
  /* ... */
}






/***************************/
/*  Access to Annotations  */
/***************************/




void LoopsSrc::GetGlobal(CompoundFortAnnot * fan)
{
  /* nothing */
}




void LoopsSrc::GetSelection(CompoundFortAnnot * fan, int l1, int c1,
                            int l2, int c2)
{
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, node, n;
  Boolean insideLoop;
  LoopsAnnot * ob;

  /* determine what node is selected */
    accessModule(RS(this)->module, &ed, &ft, &root);
    ft_AstSelect(ft);
    ed_TextToNode(ed, l1, c1, l2, c2, &node);

  /* add a top-level annotation if relevant */
    /* determine if 'node' is within any loop */
      insideLoop = false;
      n = tree_out(node);
      while( n != AST_NIL  &&  ! insideLoop )
        if( NT(n) == GEN_DO  ||  NT(n) == GEN_DO_ALL )
          insideLoop = true;
        else
          n = tree_out(n);
      
    if( insideLoop )
      { ob = makeLA(this, node);
        fan->AddElement(ob);
      }
}






/***************************************/
/* LoopsAnnot Instance Initialization  */
/***************************************/




META_IMP(LoopsAnnot)




LoopsAnnot::LoopsAnnot(LoopsSrc * loo, FortTreeNode node)
           : SimpleFortAnnot ("Loops", loo, false)
{
  /* allocate a new instance */
    this->LoopsAnnot_repr = (LoopsAnnot_Repr *) get_mem(sizeof(LoopsAnnot_Repr),
                                                       "Loops:LoopsAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RA(this)->loo  = loo;
      RA(this)->node = node;
}




LoopsAnnot::~LoopsAnnot(void)
{
  /* free representation */
    free_mem((void*) this->LoopsAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void LoopsAnnot::Create(void)
{
  /* nothing */
}




void LoopsAnnot::Destroy(void)
{
  /* destroy annotation */
    this->LAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




void LoopsAnnot::realize(void)
{
  LoopsSrc * loo = RA(this)->loo;
  FortTreeNode node = RA(this)->node;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, n, prev_n, label;
  Flex * loops;
  goto_Link kind;
  int numLoops, k, l1, c1, l2, c2;
  char * text;

  /* access the selected subtree */
    accessModule(RS(loo)->module, &ed, &ft, &root);
    ft_AstSelect(ft);

  /* find the nest of loop nodes above 'node' */
    loops = flex_create(sizeof(FortTreeNode));

    n = tree_out(node);
    while( n != AST_NIL )
      { if( NT(n) == GEN_DO  ||  NT(n) == GEN_DO_ALL )
          flex_insert_one(loops, 0, (char *) &n);
        n = tree_out(n);
      }

  /* add a text line for the "do" of each loop */
    numLoops = flex_length(loops);
    for( k = 0;  k < numLoops;  k++ )
      { flex_get_buffer(loops, k, 1, (char *) &n);
        text = getLine(ed, n, LINK_TO_FIRST);
        this->AddTextLine(text, RS(loo)->module, n, LINK_TO_FIRST);
        sfree(text);
      }

  /* add a text line for the "enddo" of each loop, in reverse  */
  /*    (this is complicated by loops which share a last line) */
    prev_n = nil;

    for( k = numLoops-1;  k >= 0;  k-- )
      { flex_get_buffer(loops, k, 1, (char *) &n);

        /* determine the closing statement */
          if( NT(n) == GEN_DO )
            label = gen_DO_get_lbl_ref(n);
          else
            label = gen_DO_ALL_get_lbl_ref(n);

          if( label == AST_NIL )
            kind = LINK_TO_LAST;
          else
            { ed_NodeToText(ed, n, &l1, &c1, &l2, &c2);
              ed_TextToNode(ed, l2, UNUSED, l2, UNUSED, &n);
              kind = LINK_TO_FIRST;
            }

        /* collapse sequences of the same closing statement into one */
          if( n != prev_n )
            { text = getLine(ed, n, kind);
              this->AddTextLine(text, RS(loo)->module, n, kind);
              sfree(text);

              prev_n = n;
            }
      }

  flex_destroy(loops);
}






/************************************************************************/
/*      Private Operations                                              */
/************************************************************************/




static
FortAnnotSrc * makeSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                       Context module)
{
  LoopsSrc * las;

  las = new LoopsSrc(context, fp, fam, module);
  las->Open(context, CONTEXT_NULL, CONTEXT_NULL, fp);

  return las;
}




static
LoopsAnnot * makeLA(LoopsSrc * loo, FortTreeNode node)
{
  LoopsAnnot * lan;

  lan = new LoopsAnnot(loo, node);
  lan->Create();

  return lan;
}




