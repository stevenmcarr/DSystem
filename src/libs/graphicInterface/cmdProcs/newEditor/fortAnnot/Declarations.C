/* $Id: Declarations.C,v 1.4 1997/03/11 14:30:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/Declarations.c                                 */
/*                                                                      */
/*      Declarations -- defining-occurrence annotation and source       */
/*      Last edited: August 25, 1993 at 3:53 pm				*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotSrc.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnot.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/Declarations.h>






/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/*******************************/
/*  DeclSource Representation  */
/*******************************/




/* DeclSource objects */

typedef struct DeclSrc_Repr_struct
  {
    Context             module;

  } DeclSrc_Repr;


#define RS(ob)          (ob->DeclSrc_repr)


#define DSINHERITED     FortAnnotSrc






/***********************************/
/*  DeclAnnotation Representation  */
/***********************************/




/* DeclAnnotation objects */

typedef struct DeclAnnot_Repr_struct
  {
    DeclSrc *           dcl;
    FortTreeNode        node;

  } DeclAnnot_Repr;


#define RA(ob)          (ob->DeclAnnot_repr)


#define DAINHERITED     CompoundFortAnnot
#define DLINHERITED     SimpleFortAnnot






/************************/
/*  Miscellaneous       */
/************************/


static int DeclSrc_initCount = 0;

static int DeclAnnot_initCount = 0;






/************************/
/* Forward declarations */
/************************/


static FortAnnotSrc *  makeSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                               Context module);
static DeclAnnot *     makeDA(DeclSrc * dcl, FortTreeNode node);
static FortAnnot *     makeDALeaf(char * name, FortAnnotSrc * dcl,
                                  Boolean sorted, void * data);

static void            addTextLine(DeclLeafAnnot * annot, Context module,
                                   FortTreeNode lineNode,
                                   FortTreeNode linkNode);

static char *          typeToString(SymDescriptor d, fst_index_t index);
static char *          boundsToString(SymDescriptor d, fst_index_t index);
static char *          usageToString(SymDescriptor d, fst_index_t index,
                                     FortTreeNode scope, char * name);

static Boolean         searchArrayDeclLenList(FortTreeNode adlList, char * name,
                                              FortTreeNode * id2);
static Boolean         searchCommonEltList(SymDescriptor d,
                                           FortTreeNode ceList,
                                           fst_index_t index,
                                           fst_index_t leader,
                                           FortTreeNode * id);
static Boolean         searchEquivEltList(SymDescriptor d, FortTreeNode eeList,
                                          fst_index_t index, fst_index_t leader,
                                          FortTreeNode * id2);






/************************************************************************/
/*      Interface Operations                                            */
/************************************************************************/




/****************************/
/*  DeclSrc Initialization  */
/****************************/




void DeclSrc_Init(void)
{
  if( DeclSrc_initCount++ == 0 )
    { /* register with the manager of annotation sources */
        FortAnnotMgr::RegisterSrc(makeSrc);
  }
}
 



void DeclSrc_Fini(void)
{
  if( --DeclSrc_initCount == 0 )
    { /* nothing */
    }
}






/******************************/
/*  DeclAnnot Initialization  */
/******************************/




void DeclAnnot_Init(void)
{
  if( DeclAnnot_initCount++ == 0 )
    { /* nothing */
  }
}
 



void DeclAnnot_Fini(void)
{
  if( --DeclAnnot_initCount == 0 )
    { /* nothing */
    }
}






/************************************/
/* DeclSrc Instance Initialization  */
/************************************/




META_IMP(DeclSrc)




DeclSrc::DeclSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                 Context module)
        : FortAnnotSrc (context, fp, fam, module)
{
  /* allocate a new instance */
    this->DeclSrc_repr = (DeclSrc_Repr *) get_mem(sizeof(DeclSrc_Repr),
                                                  "Declarations:DeclSource");
}




DeclSrc::~DeclSrc(void)
{
  /* free annotation source */
    free_mem((void*) this->DeclSrc_repr);
}






/**************/
/*  Database  */
/**************/




void DeclSrc::Open(Context context, Context mod_in_pgm_context,
                   Context pgm_context, DB_FP * session_fp)
{
  /* initialize the parts */
    RS(this)->module = this->getModule();
}




void DeclSrc::Close(void)
{
  /* close annotation source */
    this->DSINHERITED::Close();
}




void DeclSrc::Save(Context context, DB_FP * fp)
{
  /* nothing */
}






/*************************/
/*  Change notification  */
/*************************/




void DeclSrc::NoteChange(Object * ob, int kind, void * change)
{
  /* ... */
}






/***************************/
/*  Access to annotations  */
/***************************/




void DeclSrc::GetGlobal(CompoundFortAnnot * fan)
{
  /* nothing */
}




void DeclSrc::GetSelection(CompoundFortAnnot * fan, int l1, int c1,
                           int l2, int c2)
{
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, node;
  DeclAnnot * ob;

  /* determine what node is selected */
    accessModule(RS(this)->module, &ed, &ft, &root);
    ft_AstSelect(ft);
    ed_TextToNode(ed, l1, c1, l2, c2, &node);

  /* add a top-level annotation */
    ob = makeDA(this, node);
    fan->AddElement(ob);
}






/**************************************/
/* DeclAnnot Instance Initialization  */
/**************************************/




META_IMP(DeclAnnot)




DeclAnnot::DeclAnnot(DeclSrc * dcl, FortTreeNode node)
          : CompoundFortAnnot ("Names", dcl, true)
{
  /* allocate a new instance */
    this->DeclAnnot_repr = (DeclAnnot_Repr *) get_mem(sizeof(DeclAnnot_Repr),
                                              "Declarations:DeclAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RA(this)->dcl  = dcl;
      RA(this)->node = node;
}




DeclAnnot::~DeclAnnot(void)
{
  /* free representation */
    free_mem((void*) this->DeclAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void DeclAnnot::Init(void)
{
  /* nothing */
}




void DeclAnnot::Destroy(void)
{
  /* destroy annotation */
    this->DAINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




void DeclAnnot::realize(void)
{
  FortTreeNode node = RA(this)->node;

  /* add an annotation for each identifier in the selected subtree */
    realizeIdentifier(node);
}




void DeclAnnot::realizeIdentifier(FortTreeNode node)
{
  DeclSrc * dcl = RA(this)->dcl;
  DeclLeafAnnot * dan;
  char * name;
  FortTreeNode elem;
  int numSons, k;

  switch( NT(node) )
    {
      case GEN_IDENTIFIER:
        /* make a unique annotation for this identifier */
          name = gen_get_text(node);
          dan = (DeclLeafAnnot *)
                this->findElement(name, true, true, false, makeDALeaf, dcl,
                                  (void *) &node);

        break;

      case GEN_LIST_OF_NODES:
        elem = list_first(node);
        while( elem != AST_NIL )
          { realizeIdentifier(elem);
            elem = list_next(elem);
          }
        break;

      default:
        numSons = gen_how_many_sons(gen_get_node_type(node));
        for( k = 1;  k <= numSons;  k++ )
          realizeIdentifier(gen_get_son_n(node,k));
        break;
    }
}






/******************************************/
/* DeclLeafAnnot Instance Initialization  */
/******************************************/




META_IMP(DeclLeafAnnot)




DeclLeafAnnot::DeclLeafAnnot(char * name, DeclSrc * dcl, Boolean sorted,
                             FortTreeNode node)
              : SimpleFortAnnot (name, dcl, sorted)
{
  /* allocate a new instance */
    this->DeclAnnot_repr = (DeclAnnot_Repr *) get_mem(sizeof(DeclAnnot_Repr),
                                              "Declarations:DeclAnnotation");

  /* initialize the parts */
    /* set creation parameters */
      RA(this)->dcl  = dcl;
      RA(this)->node = node;
}




DeclLeafAnnot::~DeclLeafAnnot(void)
{
  /* free representation */
    free_mem((void*) this->DeclAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void DeclLeafAnnot::Init(void)
{
  /* nothing */
}




void DeclLeafAnnot::Destroy(void)
{
  /* destroy annotation */
    this->DLINHERITED::Destroy();
}






/***********************/
/*  Protected methods  */
/***********************/




/************************/
/*  Realize Annotation  */
/************************/




void DeclLeafAnnot::realize(void)
{
  DeclSrc * dcl = RA(this)->dcl;
  Context module = RS(dcl)->module;
  FortTreeNode id = RA(this)->node;
  FortEditor ed;
  FortTree ft;
  FortTreeNode root, subprog, stmt, f, ceList, adlList, eeList, id2;
  SymDescriptor symdesc;
  fst_index_t sindex, leader;
  char * name;
  char * name2;
  char text[100];
  int offset;
  int size;
  Boolean leaderIsCommon;
  Boolean relevant;

  /* get the identifier's declaration facts */
    accessModule(module, &ed, &ft, &root);
    ft_AstSelect(ft);
    subprog = find_scope(id);
    symdesc = ft_SymGetTable(ft, gen_get_text(get_name_in_entry(subprog)));

  /* make a unique annotation for this identifier */
    name = gen_get_text(id);

    sindex = ((symdesc != 0) ? fst_QueryIndex(symdesc, name) : 
                               SYM_INVALID_INDEX);

  /* see whether declaration info exists in the tree */
    if( symdesc == 0 || !fst_index_is_valid(sindex) )
      { this->AddTextLine("No semantic information is available.",
                           CONTEXT_NULL, AST_NIL, LINK_UNUSED);
        this->AddTextLine("The module either needs checking or is erroneous.",
                           CONTEXT_NULL, AST_NIL, LINK_UNUSED);
        return;
      }


  /* add text lines for summary info */
    sprintf(text, "  NAME: %s", name);
    this->AddTextLine(text, CONTEXT_NULL, AST_NIL, LINK_UNUSED);

    sprintf(text, "  TYPE: %s", typeToString(symdesc, sindex));
    this->AddTextLine(text, CONTEXT_NULL, AST_NIL, LINK_UNUSED);

    sprintf(text, "  DIMS: %s", boundsToString(symdesc, sindex));
    this->AddTextLine(text, CONTEXT_NULL, AST_NIL, LINK_UNUSED);

    sprintf(text, "  KIND: %s", usageToString(symdesc, sindex, subprog, name));
    this->AddTextLine(text, CONTEXT_NULL, AST_NIL, LINK_UNUSED);

  /* add text lines for each relevant declaration */
  /* start with blank lines for nice layout */
    this->AddTextLine("", CONTEXT_NULL, AST_NIL, LINK_UNUSED);
    this->AddTextLine("", CONTEXT_NULL, AST_NIL, LINK_UNUSED);

  /* check all subprograms' names */
    subprog = list_first(gen_GLOBAL_get_subprogram_scope_LIST(root));
    while( subprog != AST_NIL )
      { if( NT(subprog) != GEN_COMMENT )
          { id2 = get_name_in_entry(subprog);
            name2 = gen_get_text(id2);
            if( strcmp(name, name2) == 0 )
              addTextLine(this, module, subprog, id2);
          }

        subprog = list_next(subprog);
      }

  /* examine this subprogram's header */

  /* check this subprogram's formal parameters */
    relevant = false;
    f = list_first(get_formals_in_entry(subprog));
    while( f != AST_NIL && ! relevant )
      { /* ASSERT: 'f' is either GEN_IDENTIFIER or GEN_STAR */
        name2 = gen_get_text(f);
        relevant = BOOL( strcmp(name, name2) == 0 );
        f = list_next(f);
      }

  /* add an annotation if the subprogram header is relevant */
    if( relevant )
      addTextLine(this, module, subprog, f);

  /* examine each statement in turn */
    subprog = find_scope(id);
    stmt = list_first(get_stmts_in_scope(subprog));

    fst_Symbol_To_EquivLeader_Offset_Size(symdesc, sindex, &leader, &offset, 
                                          (unsigned int*)&size);
    if( fst_index_is_valid(leader) && 
        (fst_GetFieldByIndex(symdesc, leader, SYMTAB_OBJECT_CLASS) 
        & OC_IS_COMMON_NAME) )
      { leaderIsCommon = true;
      }
      else leaderIsCommon = false;

    while( stmt != AST_NIL )
      { relevant = false;
        switch( NT(stmt) )
          {
            case GEN_TYPE_STATEMENT:
              adlList = gen_TYPE_STATEMENT_get_array_decl_len_LIST(stmt);
              relevant = searchArrayDeclLenList(adlList, name, &id2);
              break;

            case GEN_DIMENSION:
              adlList = gen_DIMENSION_get_array_decl_len_LIST(stmt);
              relevant = searchArrayDeclLenList(adlList, name, &id2);
              break;

            case GEN_COMMON:
              if( leaderIsCommon )
                { ceList = gen_COMMON_get_common_elt_LIST(stmt);
                  relevant = searchCommonEltList(symdesc, ceList, sindex, 
                                                 leader, &id2);
                }
              break;

            case GEN_EQUIVALENCE:
              eeList = gen_EQUIVALENCE_get_equiv_elt_LIST(stmt);
              relevant = searchEquivEltList(symdesc, eeList, sindex, leader, 
                                            &id2);
              break;

            default:
              /* not relevant */
                break;
          }

        /* add an annotation if the statement is relevant */
          if( relevant )
            addTextLine(this, module, stmt, id2);

          stmt = list_next(stmt);
      }
}






/************************************************************************/
/*      Private Operations                                              */
/************************************************************************/




static
FortAnnotSrc * makeSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                       Context module)
{
  DeclSrc * das;

  das = new DeclSrc(context, fp, fam, module);
  das->Open(context, CONTEXT_NULL, CONTEXT_NULL, fp);

  return das;
}




static
DeclAnnot * makeDA(DeclSrc * dcl, FortTreeNode node)
{
  DeclAnnot * dan;

  dan = new DeclAnnot(dcl, node);
  dan->Init();

  return dan;
}




static
FortAnnot * makeDALeaf(char * name, FortAnnotSrc * dcl, Boolean sorted,
                       void * data)
{
  DeclLeafAnnot * dan;
  
  dan = new DeclLeafAnnot(name, (DeclSrc *) dcl, sorted,
                          *( (FortTreeNode *) data ));
  dan->Init();

  return dan;
}




static
void addTextLine(DeclLeafAnnot * annot, Context module, FortTreeNode lineNode,
                 FortTreeNode linkNode)
{
  FortEditor ed;
  FortTree ft;
  FortTreeNode root;
  char * text;

  accessModule(module, &ed, &ft, &root);

  text = getLine(ed, lineNode, LINK_TO_FIRST);
  annot->AddTextLine(text, module, linkNode, LINK_TO_NODE);
  sfree(text);
}




static
char * typeToString(SymDescriptor d, fst_index_t index)
{
  int type = fst_GetFieldByIndex(d, index, SYMTAB_TYPE);
  int len;
  static char buffer[100];
  char tmp[32];
  char * s;

  /* ASSERT: caller will not retain the pointer returned here */

  switch( type )
    {
      case TYPE_INTEGER:          
        s = "integer";          
        break;

      case TYPE_REAL:             
        s = "real";             
        break;

      case TYPE_CHARACTER:        
        s = "character";        
        break;

      case TYPE_DOUBLE_PRECISION: 
        s = "double precision"; 
        break;

      case TYPE_COMPLEX:          
        s = "complex";          
        break;

      case TYPE_LOGICAL:          
        s = "logical";          
        break;

      case TYPE_EXACT:            
        s = "exact precision";  
        break;

      case TYPE_SEMAPHORE:        
        s = "semaphore";        
        break;

      case TYPE_EVENT:            
        s = "event";            
        break;

      case TYPE_BARRIER:          
        s = "barrier";          
        break;

      default:                    
        s = "";                 
        break;

    }

  strcpy(buffer, s);

  if( type == TYPE_CHARACTER )
    { len = fst_GetFieldByIndex(d, index, SYMTAB_CHAR_LENGTH);
      if( len != CHAR_LEN_STAR )
        { sprintf(tmp, " * %d", len);
          strcat(buffer, tmp);
        }
    }

  return buffer;
}




static
char * boundsToString(SymDescriptor d, fst_index_t index)
{
  int ndims;
  static char buffer[100];
  FortTreeNode boundsList, elem, lower, upper;
  int oc;

  /* ASSERT: caller will not retain the pointer returned here */

  ndims = fst_GetFieldByIndex(d, index, SYMTAB_NUM_DIMS);

  /* check for degenerate cases */
    if( ndims == 0 )  return "";

  oc = fst_GetFieldByIndex(d, index, SYMTAB_OBJECT_CLASS);
  if( !(oc & OC_IS_DATA) )  return "";

  boundsList = fst_GetFieldByIndex(d, index, SYMTAB_DIM_LIST);
  strcpy(buffer, "(");
  elem = list_first(boundsList);
  while( elem != AST_NIL )
    { 
      lower = gen_DIM_get_lower(elem);
      upper = gen_DIM_get_upper(elem);

      if( lower != AST_NIL )
        { 
          strcat(buffer, gen_get_text(lower));
          strcat(buffer, ":");
        }
      strcat(buffer, gen_get_text(upper));

      elem = list_next(elem);
      if( elem != AST_NIL )
        strcat(buffer, ", ");
    }
  strcat(buffer, ")");

  return buffer;
}




#define append_usage_string(s)                       \
        {                                            \
          if( !nullflag ) { strcat(buffer, " "); }   \
          nullflag = false;                          \
          strcat(buffer, s);                         \
        }




static
char * usageToString(SymDescriptor d, fst_index_t index, FortTreeNode scope,
                     char * name)
{
  int usage;
  int oc, sc;
  int offset;
  int size;
  char * scopename;
  char * cbName;
  char * val;
  static char buffer[100];
  Boolean nullflag = true;
  fst_index_t leader;

  buffer[0] = (char) 0;

  /* ASSERT: caller will not retain the pointer returned here */

  oc = fst_GetFieldByIndex(d, index, SYMTAB_OBJECT_CLASS);
  sc = fst_GetFieldByIndex(d, index, SYMTAB_STORAGE_CLASS);

  scopename = gen_get_text(get_name_in_entry(scope));

  if( sc & SC_EXTERNAL )
    { /* external subroutine or function */
      append_usage_string("external");
      if( !(sc & (SC_FUNCTION | SC_SUBROUTINE)) ) /* unknown */
        append_usage_string("subroutine or function");
    }
  if( sc & SC_PROGRAM )
    { /* program declaration */
      append_usage_string("main program");
    }
  if( sc & SC_BLOCK_DATA )
    { /* block data decl. */
      append_usage_string("block data");
    }
  if( sc & SC_INTRINSIC )
    { append_usage_string("intrinsic");
    }
  if( sc & SC_FUNCTION )
    { append_usage_string("function");
    }
  if( sc & SC_SUBROUTINE )
    { append_usage_string("subroutine");
    }
  if( sc & SC_ENTRY )
    { /* function or subroutine entry */
      append_usage_string("entry");
    }
  if( oc & OC_IS_COMMON_NAME )
    { /* common name decl. */
      append_usage_string("common block");
    }
  if( oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG) )
    { /* dummy argument */
      append_usage_string("formal of \"");
      strcat(buffer, scopename);
      strcat(buffer, "\"");
    }
  if( oc & OC_IS_DATA )
    { fst_Symbol_To_EquivLeader_Offset_Size(d, index, &leader, 
                                            &offset, (unsigned int*)&size);

      if( index != leader && 
          (fst_GetFieldByIndex(d, leader, SYMTAB_OBJECT_CLASS ) & 
           OC_IS_COMMON_NAME) )
        { append_usage_string("common variable in");
          append_usage_string( (char*) fst_GetFieldByIndex(d, leader,
                                                           SYMTAB_NAME) );
        }
      else if( sc & SC_CONSTANT )
        { /* constant variable */
          FortTreeNode expr = fst_GetFieldByIndex(d, index, SYMTAB_EXPR);
          append_usage_string("constant");
          val = gen_get_text(expr);
          if( strlen(val) > (size_t)0 )
            { append_usage_string(" = ");
              append_usage_string(val);
            }
        }
      else if( !(oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) )
        { append_usage_string("local variable of \"");
          strcat(buffer, scopename);
          strcat(buffer, "\"");
        }
    }
  if( sc & SC_STMT_FUNC )
    { append_usage_string("statement function");
    }
  if( sc & SC_STMT_LABEL )
    { /* a label */
      append_usage_string("statement label");
    }
  if( oc & OC_IS_DATA )
    { if( fst_GetFieldByIndex(d, index, SYMTAB_SAVE_STMT) != AST_NIL )
        append_usage_string("(save variable)");
    }

  return buffer;
}




static
Boolean searchArrayDeclLenList(FortTreeNode adlList, char * name,
                               FortTreeNode * id2)
{
  FortTreeNode adl;
  Boolean found;
  char * name2;

  adl = list_first(adlList);
  found = false;
  while( adl != AST_NIL && ! found )
    { *id2 = gen_ARRAY_DECL_LEN_get_name(adl);
      name2 = gen_get_text(*id2);
      found = BOOL( strcmp(name, name2) == 0 );
      adl = list_next(adl);
    }

  return found;
}




static
Boolean searchCommonEltList(SymDescriptor d, FortTreeNode ceList, 
                            fst_index_t index, fst_index_t leader,
                            FortTreeNode * id)
{
  FortTreeNode ce = list_first(ceList);
  while( ce != AST_NIL )
    { FortTreeNode name = gen_COMMON_ELT_get_name(ce);
      fst_index_t cbindex = fst_QueryIndex(d, gen_get_text(name));
      if( cbindex == leader )
        { *id = name;
          return true;
        }
      ce = list_next(ce);
    }

  return false;
}




static
Boolean searchEquivEltList(SymDescriptor d, FortTreeNode eeList,
                           fst_index_t index, fst_index_t leader,
                           FortTreeNode * id2)
{
  FortTreeNode ee, lv;
  fst_index_t nleader;
  int offset;
  int size;

  ee = list_first(eeList);
  while( ee != AST_NIL )
    { lv = list_first(gen_EQUIV_ELT_get_lvalue_LIST(ee));
      while( lv != AST_NIL )
        { fst_index_t eqindex, nleader;
          if( NT(lv) == GEN_SUBSCRIPT )
            *id2 = gen_SUBSCRIPT_get_name(lv);
          else
            /* NT(lv) == GEN_IDENTIFIER */
              *id2 = lv;

          eqindex = fst_QueryIndex(d, gen_get_text(*id2));
          if( fst_index_is_valid(eqindex) )
            { fst_Symbol_To_EquivLeader_Offset_Size(d, eqindex, &nleader,
                                                    &offset, (unsigned int*)&size);
              if( nleader == leader ) return true;
            }
          lv = list_next(lv);
        }
      ee = list_next(ee);
    } 

  return false;
}


