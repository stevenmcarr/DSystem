/* $Id: Contents.C,v 1.4 1997/03/11 14:30:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/Contents.C                                     */
/*                                                                      */
/*      Contents -- table-of-contents annotation and source             */
/*      Last edited: August 25, 1993 at 3:53 pm				*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotSrc.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnot.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/Contents.h>






/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/********************************/
/*  ContentsSrc Representation  */
/********************************/




typedef struct ContentsSrc_Repr_struct
  {
    Context             module;

  } ContentsSrc_Repr;


#define RS(ob)          (ob->ContentsSrc_repr)


#define CSINHERITED     FortAnnotSrc






/**********************************/
/*  ContentsAnnot Representation  */
/**********************************/




typedef struct ContentsAnnot_Repr_struct
  {
    ContentsSrc *       toc;

  } ContentsAnnot_Repr;






/************************************/
/*  TOCSubprogAnnot Representation  */
/************************************/




typedef struct TOCSubprogAnnot_Repr_struct
  {
    ContentsAnnot_Repr_struct ContentsAnnot_repr;

    int                 dummy;

  } TOCSubprogAnnot_Repr;


#define RP(ob)          (ob->TOCSubprogAnnot_repr)
#define RPa(ob)         (ob->TOCSubprogAnnot_repr->ContentsAnnot_repr)


#define CAINHERITED     CompoundFortAnnot






/***********************************/
/*  TOCCommonAnnot Representation  */
/***********************************/




typedef struct TOCCommonAnnot_Repr_struct
  {
    ContentsAnnot_Repr_struct ContentsAnnot_repr;

    int                 dummy;

  } TOCCommonAnnot_Repr;


#define RC(ob)          (ob->TOCCommonAnnot_repr)
#define RCa(ob)         (ob->TOCCommonAnnot_repr->ContentsAnnot_repr)






/**********************************/
/*  TOCErrorAnnot Representation  */
/**********************************/




typedef struct TOCErrorAnnot_Repr_struct
  {
    ContentsAnnot_Repr_struct ContentsAnnot_repr;

    int                 first;
    int                 last;

  } TOCErrorAnnot_Repr;


#define RE(ob)          (ob->TOCErrorAnnot_repr)
#define REa(ob)         (ob->TOCErrorAnnot_repr->ContentsAnnot_repr)






/************************/
/*  Miscellaneous       */
/************************/


static int ContentsSrc_initCount = 0;

static int ContentsAnnot_initCount = 0;






/************************/
/* Forward declarations */
/************************/


static FortAnnotSrc *    makeSrc(Context context, DB_FP * fp,
                                 FortAnnotMgr * fam, Context module);
static TOCSubprogAnnot * makeTOCSubprog(ContentsSrc * toc, Boolean sorted);
static TOCCommonAnnot *  makeTOCCommon(ContentsSrc * toc, Boolean sorted);
static TOCErrorAnnot *   makeTOCError(ContentsSrc * toc, Boolean sorted, 
                                      int first, int last);
static FortAnnot *       makeTOCLeaf(char * name, FortAnnotSrc * toc,
                                     Boolean sorted, void * data);
static char *            getIdentifierName(FortTreeNode node);






/************************************************************************/
/*      Interface Operations                                            */
/************************************************************************/




/********************************/
/*  ContentsSrc Initialization  */
/********************************/




void ContentsSrc_Init(void)
{
  if( ContentsSrc_initCount++ == 0 )
    { /* register with the manager of annotation sources */
         FortAnnotMgr::RegisterSrc(makeSrc);
    }
}




void ContentsSrc_Fini(void)
{
  if( --ContentsSrc_initCount == 0 )
    { /* nothing */
    }
}






/**********************************/
/*  ContentsAnnot Initialization  */
/**********************************/




void ContentsAnnot_Init(void)
{
  if( ContentsAnnot_initCount++ == 0 )
    { /* nothing */
    }
}




void ContentsAnnot_Fini(void)
{
  if( --ContentsAnnot_initCount == 0 )
    { /* nothing */
    }
}






/*****************************************/
/*  ContentsSrc Instance Initialization  */
/*****************************************/




META_IMP(ContentsSrc)




ContentsSrc::ContentsSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                         Context module)
            : FortAnnotSrc (context, fp, fam, module)
{
  /* allocate a new instance */
    this->ContentsSrc_repr = (ContentsSrc_Repr *)
                             get_mem(sizeof(ContentsSrc_Repr),
                                     "Contents:ContentsSource");
}




ContentsSrc::~ContentsSrc(void)
{
  /* free annotation source */
    free_mem((void*) this->ContentsSrc_repr);
}






/**************/
/*  Database  */
/**************/




void ContentsSrc::Open(Context context, Context mod_in_pgm_context,
                       Context pgm_context, DB_FP * session_fp)
{
  /* initialize the parts */
    RS(this)->module = this->getModule();
}




void ContentsSrc::Close(void)
{
  /* close annotation source */
    this->CSINHERITED::Close();
}




void ContentsSrc::Save(Context context, DB_FP * fp)
{
  /* nothing */
}






/*************************/
/*  Change notification  */
/*************************/




void ContentsSrc::NoteChange(Object * ob, int kind, void * change)
{
  /* ... */
}






/***************************/
/*  Access to Annotations  */
/***************************/




void ContentsSrc::GetGlobal(CompoundFortAnnot * fan)
{
  CompoundFortAnnot * ob;

  ob = makeTOCSubprog(this, true);
  fan->AddElement(ob);

  ob = makeTOCCommon(this, true);
  fan->AddElement(ob);

  ob = makeTOCError(this, false, 0, 99999);
  fan->AddElement(ob);
}




void ContentsSrc::GetSelection(CompoundFortAnnot * fan, int l1, int c1,
                               int l2, int c2)
{
  FortEditor ed;
  FortTextTree ftt;
  FortTree ft;
  FortTreeNode root, node;
  Boolean errors;
  int k;
  CompoundFortAnnot * ob;

  /* determine the selected node etc */
    accessModule(RS(this)->module, &ed, &ft, &root);
    ed_GetTextTree(ed, &ftt);

    ed_TextToNode(ed, l1, c1, l2, c2, &node);
    ed_NodeToText(ed, node, &l1, &c1, &l2, &c2);

  /* see if there are errors in the selected range */
    errors = false;
    k = l1;
    while( k <= l2  &&  ! errors )
      if( ftt_IsErroneous(ftt,k) )
        errors = true;
      else
        k += 1;

  /* add a top-level annotation if so */
    if( errors )
      { ob = makeTOCError(this, false, 11, 12);
        fan->AddElement(ob);
      }
}






/********************************************/
/* TOCSubprogAnnot Instance Initialization  */
/********************************************/




META_IMP(TOCSubprogAnnot)




TOCSubprogAnnot::TOCSubprogAnnot(char * name, ContentsSrc * toc, Boolean sorted)
                : CompoundFortAnnot (name, toc, sorted)
{
  /* allocate a new instance */
    this->TOCSubprogAnnot_repr = (TOCSubprogAnnot_Repr *)
                                 get_mem(sizeof(TOCSubprogAnnot_Repr),
                                         "TOCSubprog:ContentsAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RPa(this).toc  = toc;
}




TOCSubprogAnnot::~TOCSubprogAnnot(void)
{
  /* free representation */
    free_mem((void*) this->TOCSubprogAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void TOCSubprogAnnot::Init(void)
{
  /* nothing */
}




void TOCSubprogAnnot::Destroy(void)
{
  /* destroy annotation */
    this->CAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




void TOCSubprogAnnot::realize(void)
{
  ContentsSrc * toc = RPa(this).toc;
  Context module = RS(toc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, subprogList, subprog, id;
  SimpleFortAnnot * can;
  char * name;
  char * text;

  /* get the list of subprogram nodes */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);
    subprogList = gen_GLOBAL_get_subprogram_scope_LIST(root);

  /* add a TOC item for each (nontrivial) subprogram */
    subprog = list_first(subprogList);
    while( subprog != AST_NIL )
      { if( NT(subprog) != GEN_COMMENT )
          { id = get_name_in_entry(subprog);
            name = getIdentifierName(id);
            text = getLine(ed, id, LINK_TO_FIRST);
            can = (SimpleFortAnnot *) makeTOCLeaf(name, toc, false, nil);
            this->AddElement(can);
            can->AddTextLine(text, module, id, LINK_TO_FIRST);
            sfree(text);
          }

        subprog = list_next(subprog);
      }
}






/*******************************************/
/* TOCCommonAnnot Instance Initialization  */
/*******************************************/




META_IMP(TOCCommonAnnot)




TOCCommonAnnot::TOCCommonAnnot(char * name, ContentsSrc * toc, Boolean sorted)
               : CompoundFortAnnot (name, toc, sorted)
{
  /* allocate a new instance */
    this->TOCCommonAnnot_repr = (TOCCommonAnnot_Repr *)
                                get_mem(sizeof(TOCCommonAnnot_Repr),
                                        "TOCCommon:ContentsAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RCa(this).toc  = toc;
}




TOCCommonAnnot::~TOCCommonAnnot(void)
{
  /* free representation */
    free_mem((void*) this->TOCCommonAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void TOCCommonAnnot::Init(void)
{
  /* nothing */
}




void TOCCommonAnnot::Destroy(void)
{
  /* destroy annotation */
    this->CAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




void TOCCommonAnnot::realize(void)
{
  ContentsSrc * toc = RCa(this).toc;
  Context module = RS(toc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, subprogList, subprog, statList, stmt;
  FortTreeNode celemList, celem, id;
  SimpleFortAnnot * can;
  char * name;
  char * text;

  /* get the list of subprogram nodes */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);
    subprogList = gen_GLOBAL_get_subprogram_scope_LIST(root);

  /* examine each subprogram in turn */
    subprog = list_first(subprogList);
    while( subprog != AST_NIL )
      { /* get the list of statement nodes */
          statList = get_stmts_in_scope(subprog);

        /* examine each statement in turn */
          stmt = list_first(statList);
          while( stmt != AST_NIL )
            { if( NT(stmt) == GEN_COMMON )
                { /* get the list of "common elements" */
                    celemList = gen_COMMON_get_common_elt_LIST(stmt);

                  /* add a TOC item for each "common element" */
                    celem = list_first(celemList);
                    while( celem != AST_NIL )
                      { id = gen_COMMON_ELT_get_name(celem);
                        name = getIdentifierName(id);
                        text = getLine(ed, id, LINK_TO_FIRST);
                        can = (SimpleFortAnnot *)
                              this->findElement(name, true, false, false,
                                                makeTOCLeaf, toc, nil);
                        can->AddTextLine(text, module, id, LINK_TO_FIRST);
                        sfree(text);

                        celem = list_next(celem);
                      }
                }

              stmt = list_next(stmt);
            }

        subprog = list_next(subprog);
      }
}






/******************************************/
/* TOCErrorAnnot Instance Initialization  */
/******************************************/




META_IMP(TOCErrorAnnot)




TOCErrorAnnot::TOCErrorAnnot(char * name, ContentsSrc * toc, Boolean sorted,
                             int first, int last)
              : CompoundFortAnnot (name, toc, sorted)
{
  /* allocate a new instance */
    this->TOCErrorAnnot_repr = (TOCErrorAnnot_Repr *)
                               get_mem(sizeof(TOCErrorAnnot_Repr),
                                       "TOCError:ContentsAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      REa(this).toc  = toc;

      RE(this)->first = first;
      RE(this)->last  = last;
}




TOCErrorAnnot::~TOCErrorAnnot(void)
{
  /* free representation */
    free_mem((void*) this->TOCErrorAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void TOCErrorAnnot::Init(void)
{
  /* nothing */
}




void TOCErrorAnnot::Destroy(void)
{
  /* destroy annotation */
    this->CAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




void TOCErrorAnnot::realize(void)
{
  ContentsSrc * toc = REa(this).toc;
  Context module = RS(toc)->module;
  FortEditor ed;
  FortTextTree ftt;
  FortTree ft;
  int numLines, k, first, last, dummy;
  FortTreeNode root, node, subid;
  SimpleFortAnnot * can;
  char name[100];
  char * subname;
  char * text;

  /* get the number of lines in the module */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);
    ed_GetTextTree(ed, &ftt);
    numLines = ftt_NumLines(ftt);

  /* add a TOC item for each erroneous line in the remembered range */
    first = RE(this)->first;
    last  = min(RE(this)->last, numLines-1);

    for( k = first;  k <= last;  k++ )
      if( ftt_IsErroneous(ftt, k) )
        { ftt_GetLineInfo(ftt, k, &node, &dummy);
          subid = get_name_in_entry(find_scope(node));
          subname = getIdentifierName(subid);
          sprintf(name, "%s, line %d", subname, k);
          text = ed_GetTextLine(ed, k);
          can = (SimpleFortAnnot *) makeTOCLeaf(name, toc, false, nil);
          this->AddElement(can);
          can->AddTextLine(text, module, node, LINK_TO_FIRST);
          sfree(text);
        }
}






/************************************************************************/
/*      Private Operations                                              */
/************************************************************************/




static
FortAnnotSrc * makeSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                       Context module)
{
  ContentsSrc * cas;

  cas = new ContentsSrc(context, fp, fam, module);
  cas->Open(context, CONTEXT_NULL, CONTEXT_NULL, fp);

  return cas;
}




static
TOCSubprogAnnot * makeTOCSubprog(ContentsSrc * toc, Boolean sorted)
{
  TOCSubprogAnnot * can;
  
  can = new TOCSubprogAnnot("Subprograms", toc, sorted);
  can->Init();

  return can;
}




static
TOCCommonAnnot * makeTOCCommon(ContentsSrc * toc, Boolean sorted)
{
  TOCCommonAnnot * can;
  
  can = new TOCCommonAnnot("Commons", toc, sorted);
  can->Init();

  return can;
}




static
TOCErrorAnnot * makeTOCError(ContentsSrc * toc, Boolean sorted, int first, 
                             int last)
{
  TOCErrorAnnot * can;
  
  can = new TOCErrorAnnot("Errors", toc, sorted, first, last);
  can->Init();

  return can;
}




static
FortAnnot * makeTOCLeaf(char * name, FortAnnotSrc * toc, Boolean sorted,
                        void * data)
{
  SimpleFortAnnot * can;
  
  can = new SimpleFortAnnot(name, toc, sorted);
  can->Init();

  return can;
}




static
char * getIdentifierName(FortTreeNode node)
{
  if( NT(node) == GEN_PLACE_HOLDER )
    return "<name>";
  else
    return gen_get_text(node);
}


