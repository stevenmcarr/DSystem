/* $Id: InterProc.C,v 1.4 1997/03/11 14:30:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/Interproc.C                                    */
/*                                                                      */
/*      Interproc -- interprocedural info annotation source             */
/*      Last edited: August 25, 1993 at 3:53 pm				*/
/*                                                                      */
/************************************************************************/

/*
 * Note to the reader: names of variables in this module can be very
 * confusing due to the fact that 'Annotation' means two different things
 * depending on whether you are referring to the Fortran Annotation
 * Browsing mechanism in Ned, or the Annotations which appear as data
 * associated with nodes and edges in the Call Graph. You'll have to try
 * to figure out which is which based on the context. 
 */




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotSrc.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnot.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/InterProc.h>

#include <libs/fileAttrMgmt/composition/Composition.h>





/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/*********************************/
/*  InterProcSrc Representation  */
/*********************************/




typedef struct InterProcSrc_Repr_struct
  {
    Context             module;

  } InterProcSrc_Repr;


#define RS(ob)          (ob->InterProcSrc_repr)


#define ISINHERITED     FortAnnotSrc






/***********************************/
/*  InterProcAnnot Representation  */
/***********************************/




/*
 * The data structure which is built by the IP browser is basically a
 * tree. The structure of the tree depends on whether the user is looking
 * at the "General Info" item or the "Selected Info" item. "General Info"
 * builds a tree that looks like
 * 
 *                  root
 *                 / | \ 
 *                /  |  \
 *               /   |   \
 *              /    |    \
 *            sub1  sub2  ...
 *           / | \
 *          /  |  \
 *         /   |   \
 *        /    |    \
 *   annot1  annot2   ....
 * 
 *
 * where "sub1"/"sun2" correspond to subroutines in the module, and
 * "annot1"/"annot2", ... correspond to the names of call graph node
 * annotations.
 *
 *
 * For the "Selected Info" case, the tree built looks like
 *
 *                  root
 *                 / | \ 
 *                /  |  \
 *               /   |   \
 *              /    |    \
 *            cs1   cs2  ...
 *           / | \
 *          /  |  \
 *         /   |   \
 *        /    |    \
 *   annot1  annot2   ....
 *
 * where "cs1"/"cs2" correspond to call sites, and "annot1"/"annot2"
 * correspond to the names of call graph edged annotations.
 * 
 * The tree is implemented using FortAnnotation objects -- interior
 * nodes are compound objects, and leaf nodes are simple objects. 
 * The InterprocAnnotation data structure below is the data stored
 * at each node in the tree. InterprocAnnotation is a variant
 * record, with different data depending on where the node is 
 * in the tree. 
 */




/* Data stored by all IPI annotations */

typedef struct InterProcAnnot_Repr_struct
  {
    InterProcSrc *      isrc;       /* InterprocSource for this tree */
    CallGraph *         cg;         /* call graph */

  } InterProcAnnot_Repr;




/*********************************/
/*  IPIRootAnnot Representation  */
/*********************************/




/* Data stored at the root node */

typedef struct IPIRootAnnot_Repr_struct
  {
    InterProcAnnot_Repr_struct  InterProcAnnot_repr;

    ipi_RootType        rtype;      /* which type of tree */
    FortTreeNode        node;       /* selected range, if Selected Info tree */

  } IPIRootAnnot_Repr;


#define RR(ob)          (ob->IPIRootAnnot_repr)
#define RRa(ob)         (ob->IPIRootAnnot_repr->InterProcAnnot_repr)


#define IAINHERITED     CompoundFortAnnot




/************************************/
/*  IPISubprogAnnot Representation  */
/************************************/




/* Data stored at a subprogram node */

typedef struct IPISubprogAnnot_Repr_struct
  {
    InterProcAnnot_Repr_struct  InterProcAnnot_repr;

    char *              name;       /* name of this function/subroutine */
    FortTreeNode        node;       /* AST node of function/subroutine */

  } IPISubprogAnnot_Repr;


#define RP(ob)          (ob->IPISubprogAnnot_repr)
#define RPa(ob)         (ob->IPISubprogAnnot_repr->InterProcAnnot_repr)




/*************************************/
/*  IPICallsiteAnnot Representation  */
/*************************************/




/* Data stored at a call site node */

typedef struct IPICallsiteAnnot_Repr_struct
  {
    InterProcAnnot_Repr_struct  InterProcAnnot_repr;

    char *              callername; /* name of enclosing function */
    FortTreeNode        node;       /* AST node of call site */
    int                 cls_id;     /* id for c.s. from ft_NodeToNumber */

  } IPICallsiteAnnot_Repr;


#define RC(ob)          (ob->IPICallsiteAnnot_repr)
#define RCa(ob)         (ob->IPICallsiteAnnot_repr->InterProcAnnot_repr)




/****************************************/
/*  IPISubprogLeafAnnot Representation  */
/****************************************/




/* Data stored at a subprogram/annotation leaf node */

typedef struct IPISubprogLeafAnnot_Repr_struct
  {
    InterProcAnnot_Repr_struct  InterProcAnnot_repr;

    FortTreeNode        fnode;      /* AST node for subprogram */
    char *              fname;      /* name of function */
    char *              aname;      /* name of annotation */

  } IPISubprogLeafAnnot_Repr;


#define RPL(ob)         (ob->IPISubprogLeafAnnot_repr)
#define RPLa(ob)        (ob->IPISubprogLeafAnnot_repr->InterProcAnnot_repr)


#define ILINHERITED     SimpleFortAnnot




/*****************************************/
/*  IPICallsiteLeafAnnot Representation  */
/*****************************************/




/* Data stored at callsite/annotation leaf node */

typedef struct IPICallsiteLeafAnnot_Repr_struct
  {
    InterProcAnnot_Repr_struct  InterProcAnnot_repr;

    FortTreeNode        csnode;     /* AST node for call site */
    int                 cls_id;     /* id for c.s. from ft_NodeToNumber */
    char *              aname;      /* annotation name */
    char *              callername; /* name of enclosing function */

  } IPICallsiteLeafAnnot_Repr;


#define RCL(ob)         (ob->IPICallsiteLeafAnnot_repr)
#define RCLa(ob)        (ob->IPICallsiteLeafAnnot_repr->InterProcAnnot_repr)






/************************/
/*  Miscellaneous       */
/************************/


static int InterProcSrc_initCount = 0;

static int InterProcAnnot_initCount = 0;


/* Name of ip info category as it appears in general/selected dialog */

#define IPINFONAME_SELECTED "IP info"
#define IPINFONAME_GLOBAL   "IP info"

#define GETMEMSTR           "FortAnnot:Interproc"


Composition *checked_program_context = 0;






/**************************/
/*  Forward declarations  */
/**************************/




static FortAnnotSrc *         makeSrc(Context context, DB_FP * fp,
                                      FortAnnotMgr * fam, Context module);
static IPIRootAnnot *         makeIPIRoot(char * name, InterProcSrc * isrc,
                                          CallGraph * cg, FortTreeNode node,
                                          ipi_RootType rtype);
static IPISubprogAnnot *      makeIPISubprog(InterProcSrc * isrc,
                                             CallGraph * cg, FortTreeNode node,
                                             char * name);
static IPICallsiteAnnot *     makeIPICallsite(char * name, InterProcSrc * isrc,
                                              CallGraph * cg, FortTreeNode node,
                                              int cls_id, char * callername);
static IPISubprogLeafAnnot *  makeIPISubprogLeaf(InterProcSrc * isrc,
                                                 CallGraph * cg,
                                                 FortTreeNode fnode,
                                                 char * funcname, char * aname);
static IPICallsiteLeafAnnot * makeIPICallsiteLeaf(InterProcSrc * isrc,
                                                  CallGraph * cg,
                                                  FortTreeNode csnode,
                                                  int cls_id, char * aname,
                                                  char * callername);

static char *   get_name_of_enclosing_function(AST_INDEX node);
static void     delete_stringlist(char * *stringlist);
static void     searchForCallSites(IPIRootAnnot * ipian, FortTreeNode node,
                                   CallGraph *cg);
static void     processCallSite(IPIRootAnnot * ipian, FortTreeNode callnode,
                                CallGraph *cg);
static char **  getSubprogIPData(CallGraph * cg, char * annotname,
                                 char * funcname);
static char **  getCallSiteIPData(CallGraph * cg, char * callername, int cs_id,
                                  char * annotname);
static char **
  convert_OrderedSetOfStrings_to_list_of_strings(OrderedSetOfStrings * oss);






/************************************************************************/
/*      Interface Operations                                            */
/************************************************************************/




/*********************************/
/*  InterProcSrc Initialization  */
/*********************************/




void InterProcSrc_Init(void)
{
  if( InterProcSrc_initCount++ == 0 )
    { /* register with the manager of annotation sources */
        FortAnnotMgr::RegisterSrc(makeSrc);
    }
}




void InterProcSrc_Fini(void)
{
  if( --InterProcSrc_initCount == 0 )
    { /* nothing */
    }
}




/***********************************/
/*  InterProcAnnot Initialization  */
/***********************************/




void InterProcAnnot_Init(void)
{
  if( InterProcAnnot_initCount++ == 0 )
    { /* nothing */
    }
}




void InterProcAnnot_Fini(void)
{
  if( --InterProcAnnot_initCount == 0 )
    { /* nothing */
    }
}






/******************************************/
/*  InterProcSrc Instance Initialization  */
/******************************************/




META_IMP(InterProcSrc)




InterProcSrc::InterProcSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                           Context module)
            : FortAnnotSrc (context, fp, fam, module)
{
  /* allocate a new instance */
    this->InterProcSrc_repr = (InterProcSrc_Repr *)
                               get_mem(sizeof(InterProcSrc_Repr),
                                       "InterProc:InterProcSource");
}




InterProcSrc::~InterProcSrc(void)
{
  /* free annotation source */
    free_mem((void*) this->InterProcSrc_repr);
}






/**************/
/*  Database  */
/**************/




void InterProcSrc::Open(Context context, Context mod_in_pgm_context,
                        Context pgm_context, DB_FP * session_fp)
{
  /* initialize the parts */
    RS(this)->module = this->getModule();
}




void InterProcSrc::Close(void)
{
  /* close annotation source */
    this->ISINHERITED::Close();
}




void InterProcSrc::Save(Context context, DB_FP * fp)
{
  /* nothing */
}






/*************************/
/*  Change notification  */
/*************************/




void InterProcSrc::NoteChange(Object * ob, int kind, void * change)
{
  /* ... */
}






/***************************/
/*  Access to annotations  */
/***************************/




/* Called when the user selects the "Global Info" item in the FortAnnot
   browser. At this point we make the root node, but not anything
   else, since this is a demand driven system.  */

void InterProcSrc::GetGlobal(CompoundFortAnnot * fan)
{
  IPIRootAnnot * ipian;
  
  ipian = makeIPIRoot(IPINFONAME_GLOBAL, this, 0, AST_NIL, ipi_Root_Global);

  fan->AddElement(ipian);
}




/* Called when the user selects the "Selected Info" item in the FortAnnot
   browser. */

void InterProcSrc::GetSelection(CompoundFortAnnot * fan, int l1, int c1,
                                int l2, int c2)
{
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, node;
  IPIRootAnnot * ipian; 
  
  /* determine what node is selected */
    accessModule(RS(this)->module, &ed, &ft, &root);
    ft_AstSelect(ft);
    ed_TextToNode(ed, l1, c1, l2, c2, &node);

    ipian = makeIPIRoot(IPINFONAME_SELECTED, this, 0, node, ipi_Root_Selected);

    fan->AddElement(ipian);
}






/******************/
/*  IPIRootAnnot  */
/******************/




META_IMP(IPIRootAnnot)




IPIRootAnnot::IPIRootAnnot(char * name, InterProcSrc * isrc, CallGraph * cg,
                           ipi_RootType rtype, FortTreeNode node)
             : CompoundFortAnnot (name, isrc, true)
{
  /* allocate a new instance */
    this->IPIRootAnnot_repr = (IPIRootAnnot_Repr *)
                              get_mem(sizeof(IPIRootAnnot_Repr),
                                      "IPIRoot:InterProcAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RRa(this).isrc  = isrc;
      RRa(this).cg    = cg;

      RR(this)->rtype = rtype;
      RR(this)->node  = node;
}




IPIRootAnnot::~IPIRootAnnot(void)
{
  if (RRa(this).cg != 0)
    checked_program_context->DetachAttribute(RRa(this).cg);

  /* free representation */
    free_mem((void*) this->IPIRootAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void IPIRootAnnot::Init(void)
{
  /* nothing */
}




void IPIRootAnnot::Destroy(void)
{
  /* destroy annotation */
    this->IAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




void IPIRootAnnot::realize(void)
{
  if( RR(this)->rtype == ipi_Root_Global )
    realizeGlobal();
  else 
    realizeSelected();
}




/* This routine is called when the user clicks on the "IP info" root node
   in the "Global Info" browser. At this point we want to build up a list
   of subprograms as the next level in the tree.  */
    
void IPIRootAnnot::realizeGlobal(void)
{
  InterProcSrc * isrc = RRa(this).isrc;
  Context module = RS(isrc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, id, subprog, subprogList;
  IPISubprogAnnot * spannot;
  char * name;
  
  /* Check to see if NED was started in the context of a program. If not,
  then we are not going to be able to display any interprocedural
    information.  */
  
  if( checked_program_context == 0 )
    return;
  
  /* Build/read the call graph. Store a pointer to it in the 
    root annotation, so we can free it. */
  
  if( ! RRa(this).cg )
    { 
      RRa(this).cg = (CallGraph *)  
	checked_program_context->AttachAttribute(CLASS_NAME(CallGraph));
      if( !RRa(this).cg )
        return;
    }
  
  /* access the selected subtree */
  accessModule(module, &ed, &ft, &root);
  ft_AstSelect(ft);
  
  /* At this point, walk through the top level list of nodes for the AST
    looking for entry points. For each entry point, add a new compound
    annotation to the list.  */
  
  subprogList = gen_GLOBAL_get_subprogram_scope_LIST(root);
  subprog = list_first(subprogList);
  while( subprog != AST_NIL )
    { if( gen_get_node_type(subprog) == GEN_SUBROUTINE ||
          gen_get_node_type(subprog) == GEN_PROGRAM ||
          gen_get_node_type(subprog) == GEN_FUNCTION )
        { id = get_name_in_entry(subprog);
          name = gen_get_text(id);
          if( name == 0 )
            { fprintf(stderr,
                      "FortAnnot/Interproc: Unnamed function/subroutine.\n");
              continue;
            }
          spannot = makeIPISubprog(isrc, RRa(this).cg, subprog, name);
          this->AddElement(spannot);
        }
      subprog = list_next(subprog);
    }
}




/* This routine is called when the user clicks on the "IP info" item in
   the top level list of stuff for the "Selected Info" browser. Read in
   the call graph, and build a list of call sites as the next level in the
   tree.  */

void IPIRootAnnot::realizeSelected(void)
{
  InterProcSrc * isrc = RRa(this).isrc;
  Context module = RS(isrc)->module;
  FortTreeNode selection_node = RR(this)->node;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root;

  /* Check to see if NED was started in the context of a program. If not,
     then we are not going to be able to display any interprocedural
     information.  */

  if( checked_program_context == 0 )
    return;

  /* Build/read the call graph */
    if( !RRa(this).cg )
      {
        RRa(this).cg = (CallGraph *)
	  checked_program_context->AttachAttribute(CLASS_NAME(CallGraph));
        if( !RRa(this).cg )
          return;
      }
  
  /* access the selected subtree */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);

  /* Scan through the module looking for call sites */
    searchForCallSites(this, selection_node, RRa(this).cg);
}






/*********************/
/*  IPISubprogAnnot  */
/*********************/




META_IMP(IPISubprogAnnot)




IPISubprogAnnot::IPISubprogAnnot(InterProcSrc * isrc, CallGraph * cg,
                                 FortTreeNode node, char * name)
                : CompoundFortAnnot (name, isrc, true)
{
  /* allocate a new instance */
    this->IPISubprogAnnot_repr = (IPISubprogAnnot_Repr *)
                                 get_mem(sizeof(IPISubprogAnnot_Repr),
                                         "IPISubprog:InterProcAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RPa(this).isrc = isrc;
      RPa(this).cg   = cg;

      RP(this)->node = node;
      RP(this)->name = ssave(name);
}




IPISubprogAnnot::~IPISubprogAnnot(void)
{
  /* free representation */
    free_mem((void*) this->IPISubprogAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void IPISubprogAnnot::Init(void)
{
  /* nothing */
}




void IPISubprogAnnot::Destroy(void)
{
  /* free name */
    sfree(RP(this)->name);

  /* destroy annotation */
    this->IAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




/* This routine is called when the user clicks on the name of a subprogram
   as part of the "General Info" browser. At this point we want to build
   up a list of call graph node annotations for that subprogram.  */

void IPISubprogAnnot::realize(void)
{
  InterProcSrc * isrc = RPa(this).isrc;
  Context module = RS(isrc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root;
  IPISubprogLeafAnnot * spleaf;
  char * name, * *anames;
  int i;
  
  /* Check to see if NED was started in the context of a program. If not,
     then we're not going to be able to display any interprocedural
     information.  */

  if( checked_program_context == 0 )
    return;

  /* access the selected subtree */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);

  /* Create a list of the IP call graph node annotations to display for
     this node */
    anames = ipi_GetCGnodeAnnotList();
    for( i = 0;  anames[i];  i++ )
      { spleaf = makeIPISubprogLeaf(isrc, RPa(this).cg, RP(this)->node,
                                    RP(this)->name, anames[i]);
        this->AddElement(spleaf); 
      }
    delete_stringlist(anames);
}






/**********************/
/*  IPICallsiteAnnot  */
/**********************/




META_IMP(IPICallsiteAnnot)




IPICallsiteAnnot::IPICallsiteAnnot(char * name, InterProcSrc * isrc,
                                   CallGraph * cg, FortTreeNode node,
                                   int cls_id, char * callername)
                 : CompoundFortAnnot (name, isrc, true)
{
  /* allocate a new instance */
    this->IPICallsiteAnnot_repr = (IPICallsiteAnnot_Repr *)
                                  get_mem(sizeof(IPICallsiteAnnot_Repr),
                                          "IPICallsite:InterProcAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RCa(this).isrc = isrc;
      RCa(this).cg   = cg;

      RC(this)->node       = node;
      RC(this)->cls_id     = cls_id;
      RC(this)->callername = ssave(callername);
}




IPICallsiteAnnot::~IPICallsiteAnnot(void)
{
  /* free representation */
    free_mem((void*) this->IPICallsiteAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void IPICallsiteAnnot::Init(void)
{
  /* nothing */
}




void IPICallsiteAnnot::Destroy(void)
{
  /* free callername */
    sfree(RC(this)->callername);

  /* destroy annotation */
    this->IAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




/* Realize an interior (callsite) node in the "Selected Info" tree. Given
   a call site, display all of the annotations which are available for it,
   and add them to the compound object for the call site.  */

void IPICallsiteAnnot::realize(void)
{
  InterProcSrc * isrc = RCa(this).isrc;
  Context module = RS(isrc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root;
  IPICallsiteLeafAnnot * csleaf;
  char * *anames;
  int i;
  
  /* Check to see if NED was started in the context of a program. If not,
     then we're not going to be able to display any interprocedural
     information.  */

  if( checked_program_context == 0 )
    return;

  /* access the selected subtree */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);

  /* At this point, create a list of the IP call graph edge annotations to
     display for this node */
    anames = ipi_GetCGedgeAnnotList();
    for( i = 0;  anames[i];  i++ )
      { csleaf = makeIPICallsiteLeaf(isrc, RCa(this).cg, RC(this)->node,
                                     RC(this)->cls_id, anames[i],
                                     RC(this)->callername);
        this->AddElement(csleaf); 
      }
    delete_stringlist(anames);
}






/*************************/
/*  IPISubprogLeafAnnot  */
/*************************/




META_IMP(IPISubprogLeafAnnot)




IPISubprogLeafAnnot::IPISubprogLeafAnnot(InterProcSrc * isrc, CallGraph * cg,
                                         FortTreeNode fnode, char * funcname,
                                         char * aname)
                    : SimpleFortAnnot (aname, isrc, false)
{
  /* allocate a new instance */
    this->IPISubprogLeafAnnot_repr = (IPISubprogLeafAnnot_Repr *)
                                     get_mem(sizeof(IPISubprogLeafAnnot_Repr),
                                          "IPISubprogLeaf:InterProcAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RPLa(this).isrc  = isrc;
      RPLa(this).cg    = cg;

      RPL(this)->fnode = fnode;
      RPL(this)->fname = ssave(funcname);
      RPL(this)->aname = ssave(aname);
}




IPISubprogLeafAnnot::~IPISubprogLeafAnnot(void)
{
  /* free representation */
    free_mem((void*) this->IPISubprogLeafAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void IPISubprogLeafAnnot::Init(void)
{
  /* nothing */
}




void IPISubprogLeafAnnot::Destroy(void)
{
  /* free names */
    sfree(RPL(this)->fname);
    sfree(RPL(this)->aname);

  /* destroy annotation */
    this->ILINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




/* Realize a leaf node in the "Global Info" tree. At this point we know
   the function name, the FortTreeNode for the function, and the name of
   the CallGraphNode annotation that we want to look at. We just need to
   get the actual data now.  */

void IPISubprogLeafAnnot::realize(void)
{
  InterProcSrc * isrc = RPLa(this).isrc;
  Context module = RS(isrc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root;
  char **stringlist, **listptr;
  
  /* Check to see if NED was started in the context of a program. If not,
     then we're not going to be able to display any interprocedural
     information.  */

  if( checked_program_context == 0 )
    return;

  /* access the selected subtree */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);

  /* Pass the buck to a C++ routine which will interpret the call graph
     and pass back a list of strings */
    stringlist = getSubprogIPData(RPLa(this).cg, RPL(this)->aname,
                                  RPL(this)->fname);
    if( !stringlist )
      { fprintf(stderr, "FortAnnot:Interproc: cannot get CG annotation\n");
        return;
      }
    for( listptr = stringlist;  *listptr;  listptr++ )
      this->AddTextLine(*listptr, module, RPL(this)->fnode, LINK_TO_FIRST);
    delete_stringlist(stringlist);
}






/**************************/
/*  IPICallsiteLeafAnnot  */
/**************************/




META_IMP(IPICallsiteLeafAnnot)




IPICallsiteLeafAnnot::IPICallsiteLeafAnnot(InterProcSrc * isrc, CallGraph * cg,
                                           FortTreeNode csnode, int cls_id, 
                                           char * aname, char * callername)
                     : SimpleFortAnnot (aname, isrc, false)
{
  /* allocate a new instance */
    this->IPICallsiteLeafAnnot_repr = 
      (IPICallsiteLeafAnnot_Repr *) get_mem(sizeof(IPICallsiteLeafAnnot_Repr),
                                         "IPICallsiteLeaf:InterProcAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RCLa(this).isrc       = isrc;
      RCLa(this).cg         = cg;

      RCL(this)->csnode     = csnode;
      RCL(this)->cls_id     = cls_id;
      RCL(this)->aname      = ssave(aname);
      RCL(this)->callername = ssave(callername);
}




IPICallsiteLeafAnnot::~IPICallsiteLeafAnnot(void)
{
  /* free representation */
    free_mem((void*) this->IPICallsiteLeafAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void IPICallsiteLeafAnnot::Init(void)
{
  /* nothing */
}




void IPICallsiteLeafAnnot::Destroy(void)
{
  /* free names */
    sfree(RCL(this)->callername);
    sfree(RCL(this)->aname);

  /* destroy annotation */
    this->ILINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




/* Realize a leaf node in the "Selected Info" tree. At this point, we know
   the CallGraph annotation name, the call site id, and the name of the
   calling function, so we can translate this into the actual call graph
   edge.  */

void IPICallsiteLeafAnnot::realize(void)
{
  InterProcSrc * isrc = RCLa(this).isrc;
  Context module = RS(isrc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root;
  char **stringlist, **listptr;
  
  /* Check to see if NED was started in the context of a program. If not,
     then we're not going to be able to display any interprocedural
     information.  */

  if( checked_program_context == 0 )
    return;

  /* access the selected subtree */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);

  /* Pass the buck to a C++ routine which will interpret the call graph
     and pass back a list of strings */
    stringlist = getCallSiteIPData(RCLa(this).cg, RCL(this)->callername,
                                   RCL(this)->cls_id, RCL(this)->aname);
    if( !stringlist )
      { fprintf(stderr, "FortAnnot:Interproc: cannot get CG annotation\n");
        return;
      }
    for( listptr = stringlist;  *listptr;  listptr++ )
      this->AddTextLine(*listptr, module, RCL(this)->csnode, LINK_TO_FIRST);
    delete_stringlist(stringlist);
}






/************************************************************************/
/*      Private Operations                                              */
/************************************************************************/




static
FortAnnotSrc * makeSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                       Context module)
{
  InterProcSrc * isrc;

  isrc = new InterProcSrc(context, fp, fam, module);
  isrc->Open(context, 0, 0, fp);

  return isrc;
}




static
IPIRootAnnot * makeIPIRoot(char * name, InterProcSrc * isrc, CallGraph * cg, 
                           FortTreeNode node, ipi_RootType rtype)
{
  IPIRootAnnot * ipi;

  ipi = new IPIRootAnnot(name, isrc, cg, rtype, node);
  ipi->Init();

  return ipi;
}




static
IPISubprogAnnot * makeIPISubprog(InterProcSrc * isrc, CallGraph * cg,
                                 FortTreeNode node, char * name)
{
  IPISubprogAnnot * ipi;

  ipi = new IPISubprogAnnot(isrc, cg, node, name);
  ipi->Init();

  return ipi;
}




static
IPICallsiteAnnot * makeIPICallsite(char * name, InterProcSrc * isrc,
                                   CallGraph * cg,  FortTreeNode node,
                                   int cls_id, char * callername)
{
  IPICallsiteAnnot * ipi;

  ipi = new IPICallsiteAnnot(name, isrc, cg, node, cls_id, callername);
  ipi->Init();

  return ipi;
}




static
IPISubprogLeafAnnot * makeIPISubprogLeaf(InterProcSrc * isrc, CallGraph * cg,
                                         FortTreeNode fnode, char * funcname,
                                         char * aname)
{
  IPISubprogLeafAnnot * ipi;

  ipi = new IPISubprogLeafAnnot(isrc, cg, fnode, funcname, aname);
  ipi->Init();

  return ipi;
}




static
IPICallsiteLeafAnnot * makeIPICallsiteLeaf(InterProcSrc * isrc, CallGraph * cg,
                                           FortTreeNode csnode, int cls_id,
                                           char * aname, char * callername)
{
  IPICallsiteLeafAnnot * ipi;

  ipi = new IPICallsiteLeafAnnot(isrc, cg, csnode, cls_id, aname, callername);
  ipi->Init();

  return ipi;
}




/* Find the name of the function which encloses a node (in our case we use
   it for call sites) */

static
char * get_name_of_enclosing_function(AST_INDEX node)
{
  AST_INDEX node_out;

  do
    { node_out = tree_out(node);
      if( node_out == AST_NIL || node_out == node )
        return 0;
      if( gen_get_node_type(node_out) == GEN_SUBROUTINE ||
          gen_get_node_type(node_out) == GEN_PROGRAM ||
          gen_get_node_type(node_out) == GEN_FUNCTION )
        { AST_INDEX id = get_name_in_entry(node_out);
          return gen_get_text(id);
        }
      else node = node_out;
    } while( 1 );
}




static
void delete_stringlist(char * *stringlist)
{
  char * *listptr;

  if( !stringlist )
    return;
  for( listptr = stringlist;  *listptr;  listptr++ )
    free_mem((void*) *listptr);
  free_mem((void*) stringlist);
}




/* Recursive routine to search for call sites in an AST subtree. */

static
void searchForCallSites(IPIRootAnnot * ipian, FortTreeNode node, CallGraph *cg)
{
  FortTreeNode elem;
  int numSons, k;
  
  switch( NT(node) )
    {
      case GEN_INVOCATION:
        /* case GEN_CALL: */
          processCallSite(ipian, node, cg);
          break;
    
      case GEN_LIST_OF_NODES:
        elem = list_first(node);
        while( elem != AST_NIL )
          { searchForCallSites(ipian, elem, cg);
            elem = list_next(elem);
          }
        break;
    
      default:
        numSons = gen_how_many_sons(gen_get_node_type(node));
        for( k = 1;  k <= numSons;  k++ )
          searchForCallSites(ipian, gen_get_son_n(node, k), cg);
        break;
    }
}




/* We have found a call site. Is it a call to an intrinsic or generic? If
   so, then we basically ignore it, on the assumption that there won't be
   any interesting interprocedural information for it.
   
   Otherwise, process it as follows. Find the enclosing function for this
   call site. Get its name. Then get the ID of the call site. Get the name
   of the function/subroutine which is called at this call site. Finally,
   create an InterprocAnnotation object, stash it inside a FortAnnotion
   object, and add the FortAnnotation to the root which we are dealing
   with.  */

static
void processCallSite(IPIRootAnnot * ipian, FortTreeNode callnode, CallGraph *cg)
{
  InterProcSrc * isrc = RRa(ipian).isrc;
  Context module = RS(isrc)->module;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root;
  IPICallsiteAnnot * csannot;
  FortTreeNode namenode;
  char * callername, * calledname;
  char callsitestring[100];
  char lnum[40];
  char cnum[40];
  char apos[40];
  int l1 = UNUSED, l2 = UNUSED, c1 = UNUSED, c2 = UNUSED;
  int cls_id;
  SymDescriptor d;
  fst_index_t index;
  int objclass, storclass;

  /* Figure out where the callsite is in the source */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);
    ed_NodeToText(ed, callnode, &l1, &c1, &l2, &c2);
  
  /* Figure out what the names of the caller/called functions are, etc.  */
    namenode = gen_INVOCATION_get_name(callnode);
    calledname = gen_get_text(namenode);
    callername = get_name_of_enclosing_function(callnode);
    cls_id = ft_NodeToNumber(ft, callnode);
    d = ft_SymGetTable(ft, callername);
 
  /* Is the invocation an invocation of an intrinsic or generic? If so
     then bail out immediately */
    index = fst_QueryIndex(d, calledname);
    objclass = fst_GetFieldByIndex(d, index, SYMTAB_OBJECT_CLASS);
    storclass = fst_GetFieldByIndex(d, index, SYMTAB_STORAGE_CLASS);
    if( (objclass & OC_IS_EXECUTABLE) && 
        (storclass & (SC_INTRINSIC | SC_GENERIC)) )
      return;

  /* Build a string which describes the call site in terms of the called
     function/subroutine and the line number where the call takes place */
    if( l1 >= 1 ) 
      sprintf(lnum, "%d", l1);
    else
      strcpy(lnum, "???");

    if( NT(callnode) == GEN_CALL ) 
      sprintf(apos, "(line %s)", lnum);
    else 
      { if( c1 >= 1 ) 
          sprintf(cnum, "%d", c1);
        else
          strcpy(cnum, "???");
  
        sprintf(apos, "(line %s, char %s)\n", lnum, cnum);
      }
    sprintf(callsitestring, "%s %s", calledname, apos);

  /* Now create a compound annotation for this call site. The children of
     this compound annotation will be the names of the IP call graph edge
     annotations which are available for it */
    csannot = makeIPICallsite(callsitestring, isrc, cg, callnode, cls_id,
                              callername);
    ipian->AddElement(csannot);
}




/* Given the name of a function and the name of a call graph node
   annotation, get the annotation, request that it produce an
   OrderedSetOfStrings to display itself, convert the ordered set of
   strings to a list of strings, and return it.  */

static
char **getSubprogIPData(CallGraph * cg, char * annotname, char * funcname)
{
  CallGraphNode * cgn;
  CallGraphAnnot * cga;
  OrderedSetOfStrings * oss;
  char **arr;
  
  /* Don't do anything if we don't already have a call graph */
    if( !cg )
      return 0;

  /* Start by looking up the name of the node in the callgraph. Then get
     the annotation in question */
    if( (cgn = cg->LookupNode(funcname)) == NULL )
      return 0;
    else if( (cga = cgn->GetAnnotation(annotname, true)) == NULL )
      return 0;

  /* Get the ordered set of strings which contains a displayable
     description of the annotation. Then convert the ordered set of
     strings into a list of strings, so that C code can digest it */
    if( (oss = cga->CreateOrderedSetOfStrings()) == NULL )
      return 0;
    arr = convert_OrderedSetOfStrings_to_list_of_strings(oss);
    delete oss;

  /* Return the result*/
    return arr;
}




/* Given a call site, the name of the function which encloses it, and the
   name of a call graph edge annotation, get the annotation, request that
   it produce an OrderedSetOfStrings to display itself, convert the
   ordered set of strings to a list of strings, and return it. */

static
char **getCallSiteIPData(CallGraph * cg, char * callername, int cs_id,
                         char * annotname)
{
  CallGraphEdge * cge;
  CallGraphAnnot * cga;
  OrderedSetOfStrings * oss;
  char **arr;

  /* Don't do anything if we don't already have a call graph */
    if( !cg )
      return 0;

  /* Start by looking up the edge in the callgraph. Then get
     the annotation in question */
    if( (cge = cg->LookupEdge(callername, cs_id)) == NULL )
      return 0;
    else if( (cga = cge->GetAnnotation(annotname, true)) == NULL )
      return 0;

  /* Get the ordered set of strings which contains a displayable
     description of the annotation. Then convert the ordered set of
     strings into a list of strings, so that C code can digest it */
    if( (oss = cga->CreateOrderedSetOfStrings()) == NULL )
      return 0;
    arr = convert_OrderedSetOfStrings_to_list_of_strings(oss);
    delete oss;

  /* Return the result */
    return arr;
}




/* Converts an OrderedSetOfStrings object into a simple array of strings
   which can be digested by C code. The array is of pointers to strings,
   with a NULL pointer indicating the end. Both the array and the strings
   themselves are allocated using "get_mem".  */

static
char **convert_OrderedSetOfStrings_to_list_of_strings(OrderedSetOfStrings * oss)
{
  int numstrings = oss->Size();
  char **arr, *t;
  int i;

  arr = (char **) get_mem(sizeof(char *) * (numstrings + 1), GETMEMSTR);
  for( i = 0;  i < numstrings;  i++ )
    { t = (*oss)[i];
      arr[i] = (char *) get_mem(strlen(t)+1, GETMEMSTR);
      strcpy(arr[i], t);
    }
  arr[numstrings] = 0;
  return arr;
}


