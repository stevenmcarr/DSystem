/* $Id: FortDInterface.C,v 1.9 1997/03/11 14:28:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*--------------------------------------------------------------------------

  FortDInterface.C   Implementation of Fortran D data abstractions

*/

#include <assert.h>
#include <iostream.h>

#include <libs/support/database/context.h>

#include <libs/fortD/codeGen/FortDInterface.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/driver/driver.h>

#include <libs/fortD/codeGen/FortDInterface.i>
#include <libs/ipAnalysis/problems/fortD/FortDNumProcsAnnot.h>



//*********************************************************************
// External function declarations
//*********************************************************************

EXTERN(void, dc_init, (Dist_Globals* dh, Fd_opts* fd_opts));
EXTERN(void, dc_analyze, (Dist_Globals* dh));
EXTERN(void, dc_find_loops, (Dist_Globals* dh));
EXTERN(void, dc_partition, (Dist_Globals* dh));
EXTERN(void, dc_loop_info, (Dist_Globals* dh));
EXTERN(void, ped_rsd_vector_init, (PedInfo ped, AST_INDEX root));
EXTERN(void, dc_comm, (Dist_Globals* dh));
EXTERN(void, dc_pipe_loops, (Dist_Globals* dh));
EXTERN(void, dc_finalize, (Dist_Globals* dh));
EXTERN(Boolean, is_loop, (AST_INDEX node));
EXTERN(int, pt_perf_nest, (AST_INDEX node));
EXTERN(AST_INDEX, dt_ast_sub, (AST_INDEX node));
EXTERN(void, dt_update_loopref, (DT_info* dt, SideInfo* infoPtr, 
                                 AST_INDEX root));

EXTERN(void, dc_compute_local_decomp, (AST_INDEX, FD_Composition_HT *fortd_ht));

extern int procs_and_functions (AST_INDEX stmt, int level, Generic ht);
 
extern char* FORTD_REACH_ANNOT;
extern char* FORTD_COMMON_BLOCK_ANNOT;
extern char* FORTD_OVERLAP_ANNOT;

//*********************************************************************
// local function declarations
//*********************************************************************

STATIC(FortDAstSet*, getNonlocal,(PedInfo ped, AST_INDEX node, Ast_level ref));

STATIC(int, findNonlocalRefs,(AST_INDEX node, Generic refParams));
STATIC(int, findNonlocalStmts,(AST_INDEX node, int level, Generic refParams));
STATIC(int, findLoops,(AST_INDEX node, int level, Generic Loops));

STATIC(void, getFortDMesgSet,(Dist_Globals* dh, FortDMesgSet* mset, 
        AST_INDEX loop, Mesg_type type, Rsd_set_info* rinfo, FortD_LI* fli));

STATIC(void, getAllMesgs,(Dist_Globals* dh, FortDAstSet* loopSet, 
           FortDMesgSet* mset));

STATIC(void, classify_loops,(Dist_Globals* dh));

STATIC(Boolean, classify_CrossProcDep,(Dist_Globals* dh, DG_Edge* edge));

STATIC(Color_type, classify_CrossProcDepColor,(Dist_Globals* dh, DG_Edge* edge));
STATIC(Color_type, GenerateStmtColor,(Dist_Globals *dh, AST_INDEX stmt));
STATIC(int, findArrayRefs,(AST_INDEX node, int level, Generic refParams));
STATIC(int, findArrRefs,(AST_INDEX node, Generic refParams));
STATIC(FortDAstSet*, getArrayRefs,(PedInfo ped, AST_INDEX node, Ast_level astLevel));

STATIC(void, ModuleSetUp,(AST_INDEX root,PedInfo ped, FortTree ft, Context c, 
                          CallGraph* cg));


struct proc_list {
  AST_INDEX node[100];
  int count;
};

//*********************************************************************
// Member functions for class FortDInterface
//*********************************************************************

//--------------------------------------------------------------------------
// initializes interface given Ped, AST of procedure and the program context

FortDInterface::FortDInterface(void* init_ped, AST_INDEX init_root, 
                               Context pgm_context, void* cg, FortTree ft)
{
  repr = new struct FortDInterface_S;
  repr->ped = (PedInfo)init_ped;
  repr->root = init_root;
  repr->context = pgm_context;
  repr->dh = (Dist_Globals*) get_mem(sizeof(Dist_Globals), "FdInterface");
  repr->cg = cg;
  ModuleSetUp(init_root, repr->ped, ft, pgm_context, (CallGraph*)cg);
  Analyze();
}

//--------------------------------------------------------------------------
// cleans up FortDInterface

FortDInterface::~FortDInterface()
{
  dc_finalize(repr->dh);
  free_mem((int*) repr->dh);
  delete repr;
}

//--------------------------------------------------------------------------
// Perform optimizing transformations on program

void FortDInterface::Transform()
{
  dc_pipe_loops(repr->dh); 
  dt_update_loopref(PED_DT_INFO(repr->ped), PED_INFO(repr->ped), repr->root); 
  Analyze();
}

//--------------------------------------------------------------------------
// Analyze Fortran D information for current Program

void FortDInterface::Analyze()
{
  Fd_opts fd_opts;

  memset(&fd_opts, 0, sizeof(Fd_opts));   // no special options

  dc_init(repr->dh, &fd_opts);  // initialize dc data structures 
  repr->dh->ped = repr->ped;
  repr->dh->root = repr->root;
  repr->dh->in_ped = false;
  repr->dh->in_ded = true;

  dc_analyze(repr->dh);         // analyze data decomposition     
  dc_find_loops(repr->dh);      // find & record all loops 
  dc_partition(repr->dh);       // partition computation analysis    
  dc_loop_info(repr->dh);       // update information on loops & indices 
                                // build Rsd_vector structs 
  ped_rsd_vector_init((PedInfo) repr->ped, repr->root); 

  dc_comm(repr->dh);            // determine needed data movement   

  classify_loops(repr->dh);  // class loops as PAR, PIPE, SEQ
}

//-----------------------------------------------------
// initializes Fortran D information for reference

FortDRef* FortDInterface::GetRef(AST_INDEX node)
{
  FortDRef* info = new FortDRef((void*) repr->dh, node);
  return info;
}

//-----------------------------------------------------
// initializes Fortran D information for statement

FortDStmt* FortDInterface::GetStmt(AST_INDEX node)
{
  FortDStmt* info = new FortDStmt((void*) repr->dh, node);
  return info;
}

//-----------------------------------------------------
// initializes Fortran D information for loop

FortDLoop* FortDInterface::GetLoop(AST_INDEX node)
{
  FortDLoop* info = new FortDLoop((void*) repr->dh, node);
  return info;
}

//-----------------------------------------------------
// initializes Fortran D information for procedure

FortDProc* FortDInterface::GetProc(AST_INDEX node)
{
  FortDProc* info = new FortDProc((void*) repr->dh, node, repr->cg);
  return info;
}

//-----------------------------------------------------
// initializes Fortran D information for program

FortDProg* FortDInterface::GetProg(AST_INDEX node)
{
  FortDProg* info = new FortDProg((void*) repr->dh, node);
  return info;
}


//*********************************************************************
// Member functions for class FortDRef
//*********************************************************************

//-----------------------------------------------------
// initializes interface given dh & AST of procedure

FortDRef::FortDRef(void* dh_handle, AST_INDEX node)
{
  AST_INDEX node2;

  repr = new struct FortDRef_S;
  repr->dh = (Dist_Globals*) dh_handle;
  repr->myRef = node;
  repr->distributed = false;
 
  if(is_subscript(node))
    node2 = gen_SUBSCRIPT_get_name(node);
  else
    node2 = node;

    repr->symEntry = (SNODE*) get_info(repr->dh->ped, node2, type_fd);


  if (repr->symEntry == (SNODE*) NO_FD_INFO)
  {
    repr->symEntry = NULL;
  }
  else
  {
    int i;
    for (i = sp_numdim(repr->symEntry) - 1; i >= 0; i--)
    {
      if (sp_is_part(repr->symEntry, i) != FD_DIST_LOCAL)
        repr->distributed =  true;
    }
  }

  repr->rset = (Rsd_set*) get_info(repr->dh->ped, node2, type_dc);
  if (repr->rset == (Rsd_set*) NO_DC_INFO)
    repr->rset = NULL;
}

//-----------------------------------------------------
// destructor cleans up

FortDRef::~FortDRef()
{
  delete repr;
}

//---------------------------------------------------------------------
// return internal Rsd_set

void* FortDRsd::GetRSet()
{
  return (void*) repr->rset;
}

//-----------------------------------------------------
// find # of dimensions of array

int FortDRef::Numdim()
{
  return (repr->symEntry) ? sp_numdim(repr->symEntry) : 0;
}

//-----------------------------------------------------
// query whether variable is distributed across processors

Boolean FortDRef::IsDistributed()
{
  return repr->distributed;
}

//-----------------------------------------------------
// type of distribution of array in given dimension

Dist_type FortDRef::Distrib(int dim)
{
  return IsDistributed() ? sp_is_part(repr->symEntry, dim-1) : 
                           FD_DIST_LOCAL;
}

//-----------------------------------------------------
// block size of dimension distributed FD_BLOCK_CYCLIC

int FortDRef::BkSize(int dim)
{
  return (Distrib(dim) == FD_DIST_BLOCK_CYCLIC) ? 
         sp_bksize(repr->symEntry, dim-1) : 0;
}

//-----------------------------------------------------
// size of distributed dimension

int FortDRef::Extent(int dim)
{
  if (Distrib(dim) == FD_DIST_LOCAL) 
    return  sp_get_upper(repr->symEntry, dim-1) - 
            sp_get_lower(repr->symEntry, dim-1) + 1;

  return sp_block_size1(repr->symEntry, dim-1);
}

//-----------------------------------------------------
// lower bound of array in dimension (not including overlap)

int FortDRef::LowerBound(int dim)
{
  return (repr->symEntry) ? sp_get_lower(repr->symEntry, dim-1) : 0;
}

//-----------------------------------------------------
// upper bound of array in dimension (not including overlap)

int FortDRef::UpperBound(int dim)
{
  return (Distrib(dim) == FD_DIST_LOCAL) ? 
         sp_get_upper(repr->symEntry, dim-1) : 
         LowerBound(dim) + Extent(dim) - 1;
}

//-----------------------------------------------------
// overlap offset of lower bound of array in dimension

int FortDRef::LowerOverlap(int dim)
{
  return (Distrib(dim) == FD_DIST_LOCAL) ? 0 :
         LowerBound(dim) - sp_min_access(repr->symEntry, dim-1);
}

//-----------------------------------------------------
// overlap offset of upper bound of array in dimension

int FortDRef::UpperOverlap(int dim)
{
  return (Distrib(dim) == FD_DIST_LOCAL) ? 0 :
           sp_max_access(repr->symEntry, dim-1) - 
           LowerBound(dim) - Extent(dim) + 1;
}

//-----------------------------------------------------
// whether reference causes nonlocal accesses

Boolean FortDRef::NonLocalAcc()
{
  return (repr->rset == NULL) ? false : true;
}

//----------------------------------------------------------------------
// reference coloring based on a coarse grain approximation of how bad
// the communication will be
//----------------------------------------------------------------------
Color_type FortDRef::GetColor()
{
  if (repr->rset) 
  {
    switch(repr->rset->mtype)
    {
      case FD_MESG_CARRIED_PART:
        switch(GenerateStmtColor(repr->dh, repr->myRef))
        {
          case FD_RED:
            return (FD_RED);
          break;
       
          case FD_YELLOW:
            return (FD_YELLOW);
          break;

          default:
            cerr << "FortDRef::GetColor: illegal color"
                 << endl;
            return (FD_GREEN);
          break;
        }
      break;
    
      case FD_MESG_CARRIED_ALL:
        return (FD_GREEN);
      break;

      case FD_MESG_INDEP:
        return (FD_GREEN);
      break;

      default:
        cerr << "FortDRef::GetColor: illegal message type"
             << endl;
      break;
    }
  }

  return (FD_BLACK);
}

//-----------------------------------------------------
// message generated by reference, if any
//-----------------------------------------------------

FortDMesg* FortDRef::MesgInfo()
{
  return NonLocalAcc() ? new FortDMesg(repr->dh, repr->rset) : NULL;
}

//-----------------------------------------------------
// return the symbol table entry for the reference
//-----------------------------------------------------

void* FortDRef::GetSymEntry()
{
  return (void*) repr->symEntry;
}

//-----------------------------------------------------
// return whether dependence edge is cross-processor

Boolean FortDRef::CrossProcDep(void* edge)
{
  return classify_CrossProcDep(repr->dh, (DG_Edge*) edge);
}

Color_type FortDRef::CrossProcDepColor(void* edge)
{
  return classify_CrossProcDepColor(repr->dh, (DG_Edge*) edge);
}



//*********************************************************************
// Member functions for class FortDStmt
//*********************************************************************

//-----------------------------------------------------
// initializes interface given dh & AST of procedure

FortDStmt::FortDStmt(void* dh_handle, AST_INDEX node)
{
  repr = new struct FortDStmt_S;
  memset(repr, 0, sizeof(struct FortDStmt_S));

  repr->dh = (Dist_Globals*) dh_handle;
  repr->myStmt = node;
  repr->nonLocalRefs = NULL;  

  repr->arrayRefs = NULL;  

  repr->stmtColor = GenerateStmtColor(repr->dh, repr->myStmt);
  repr->iset = (Iter_set*) get_info(repr->dh->ped, node, type_dc);
  if (repr->iset == (Iter_set*) NO_DC_INFO)
    repr->iset = NULL;
}

//-------------------------------------------------------------------
// generates the color for a statement based on the type of the inner
// most loop surrounding the statement
//-------------------------------------------------------------------
static Color_type 
GenerateStmtColor(Dist_Globals *dh, AST_INDEX stmt)
{
  FortD_LI *loopInfo;
  AST_INDEX do_node = stmt;

  while (!(is_do(do_node)) && (do_node != AST_NIL))
  {
    do_node =  tree_out(do_node);
  }

  if (do_node == AST_NIL) 
  {
    return FD_BLACK;
  }
  else
  {
    loopInfo = (FortD_LI*) get_info(dh->ped, do_node, type_fd);

    if (loopInfo == (FortD_LI*) NO_FD_INFO) return FD_BLACK;

    switch(loopInfo->ltype)
    {
      case FD_LOOP_PARALLEL:
        return FD_GREEN;
      break;

      case FD_LOOP_PIPELINED:
        return FD_YELLOW;
      break;
 
      case FD_LOOP_SEQUENTIAL:
        return FD_RED;
      break;

      default:
        return FD_BLACK;
      break;
    }
  }
}

//----------------------------------------------------
// return the stmt color

Color_type FortDStmt::GetColor()
{
 return (repr->stmtColor);
}
//-----------------------------------------------------
// destructor for FortDStmt

FortDStmt::~FortDStmt()
{
  if (repr->nonLocalRefs) delete repr->nonLocalRefs;
  delete repr;
}

//-----------------------------------------------------
// return execution type of statement

Stmt_type FortDStmt::StmtType()
{
  if (repr->iset->allproc)
    return FD_STMT_REPLICATED;

  if (repr->iset->oneproc)
    return FD_STMT_ONE_PROC;

  if (repr->iset->reduc_set)
  {
    switch (repr->iset->reduc_set->rtype)
    {
      case FD_REDUC_PLUS:
        return FD_STMT_SUM_REDUCT;
      break;

      case FD_REDUC_TIMES:
        return FD_STMT_PROD_REDUCT;
      break;

      case FD_REDUC_MIN:
        return FD_STMT_MIN_REDUCT;
      break;

      case FD_REDUC_MAX:
        return FD_STMT_MAX_REDUCT;
      break;

      case FD_REDUC_MIN_LOC:
        return FD_STMT_MINLOC_REDUCT;
      break;

      case FD_REDUC_MAX_LOC:
        return FD_STMT_MAXLOC_REDUCT;
      break;

      default:
        cerr << "FortDStmt::StmtType(): Unrecognized reduction"
             << endl;
      break;
    }
  }

  return FD_STMT_PARALLEL;
}

//-----------------------------------------------------
// if statement assigns a value to a private variable

Boolean FortDStmt::IsPrivate()
{
  return repr->iset->lhs_private;
}

//-----------------------------------------------------
// if private var, pointer to where var is used in loop

FortDAstSet* FortDStmt::PrivateUse()
{
  FortDAstSet* astSet = new FortDAstSet(1);
  astSet->Add(repr->iset->private_use);
  return astSet;
}

//-----------------------------------------------------
// set of array references

FortDAstSet* FortDStmt::ArrayRefs()
{
 if(!repr->arrayRefs)
  repr->arrayRefs = getArrayRefs(repr->dh->ped, repr->myStmt, FD_REF);
 return repr->arrayRefs;
}


//-----------------------------------------------------
// set of nonlocal references in statement

FortDAstSet* FortDStmt::NonLocalRefs()
{
  if (!repr->nonLocalRefs)
    repr->nonLocalRefs = getNonlocal(repr->dh->ped, repr->myStmt, FD_REF);

  return repr->nonLocalRefs;
}

//-----------------------------------------------------
// whether statement causes any accesses

Boolean FortDStmt::NonLocalAcc()
{
  return (NonLocalRefs()->Size() == 0) ? false : true;
}

//-----------------------------------------------------
// get all messages caused by stmt

FortDMesgSet* FortDStmt::Mesgs()
{
  FortDMesgSet* mset = new FortDMesgSet();
  AST_INDEX node;

  if (!repr->iset)
    return mset;

  Reduc_set* reduc_s = repr->iset->reduc_set;
  if (reduc_s && !reduc_s->local)
    mset->Add(new FortDMesg(repr->dh, reduc_s, true));

  // add messages for all nonlocal refs, watch out for duplicates
  while ((node = (*(NonLocalRefs()))()) != AST_NIL)
  {
    Rsd_set* rset = (Rsd_set*) get_info(repr->dh->ped, node, type_dc);
    if ((rset != (Rsd_set*) NO_DC_INFO) && !mset->InSet((void*) rset))
      mset->Add(new FortDMesg(repr->dh, rset));
  }

  return mset;
}


//-----------------------------------------------------
// return whether dependence edge is cross-processor

Boolean FortDStmt::CrossProcDep(void* edge)
{
  return classify_CrossProcDep(repr->dh, (DG_Edge*) edge);
}

Color_type FortDStmt::CrossProcDepColor(void* edge)
{
  return classify_CrossProcDepColor(repr->dh, (DG_Edge*) edge);
}


//*********************************************************************
// Member functions for class FortDLoop
//*********************************************************************

//-----------------------------------------------------
// initializes interface given dh & AST of loop

FortDLoop::FortDLoop(void* dh_handle, AST_INDEX node)
{
  repr = new struct FortDLoop_S;
  memset(repr, 0, sizeof(struct FortDLoop_S));

  repr->dh = (Dist_Globals*) dh_handle;
  repr->myLoop = node;

  repr->loopInfo = (FortD_LI*) get_info(repr->dh->ped, node, type_fd);
  if (repr->loopInfo == (FortD_LI*) NO_FD_INFO)
    repr->loopInfo = NULL;

  repr->rsetInfo = (Rsd_set_info*) get_info(repr->dh->ped, node, type_dc);
  if (repr->rsetInfo == (Rsd_set_info*) NO_DC_INFO)
    repr->rsetInfo = NULL;

  repr->nonLocalRefs = NULL;
  repr->nonLocalStmts = NULL;
  
  repr->arrayRefs = NULL;

  repr->mesgs = NULL;
  repr->mesgs_indep = NULL;
  repr->mesgs_c_all = NULL;
  repr->mesgs_c_part = NULL;
}

//-----------------------------------------------------
// destructor for FortDLoop

FortDLoop::~FortDLoop()
{
  if (repr->nonLocalRefs) delete repr->nonLocalRefs;
  if (repr->nonLocalStmts) delete repr->nonLocalStmts;
  if (repr->mesgs) delete repr->mesgs;
  if (repr->mesgs_indep) delete repr->mesgs_indep;
  if (repr->mesgs_c_all) delete repr->mesgs_c_all;
  if (repr->mesgs_c_part) delete repr->mesgs_c_part;
  if (repr->mesgs_reduc) delete repr->mesgs_reduc;
  delete repr->arrayRefs;
  delete repr;
}

//---------------------------------------------------------------------
// return internal Dist_globals

void* FortDLoop::GetDistGlobals()
{
  return (void*) repr->dh;
}

//-----------------------------------------------------
// execution type of loop

Loop_type FortDLoop::LoopType()
{
  return repr->loopInfo->ltype;
}

//-----------------------------------------------------
// whether all statements in loop are executed on 
// the same iterations by the local processor

Boolean FortDLoop::IsUniform()
{
  return repr->loopInfo->uniform;
}

//-----------------------------------------------------
// whether iterations of loop are partitioned

Boolean FortDLoop::ItersPartitioned()
{
  return (repr->loopInfo->dist_type == FD_DIST_LOCAL) ? false : true;
}

//-----------------------------------------------------
// number of iterations executed 
// Note: only applies if type = LOOP_PARALLEL

int FortDLoop::ItersExecuted()
{
  return repr->loopInfo->bksize;
}

//-----------------------------------------------------
// set of array  references in loop

FortDAstSet* FortDLoop::ArrayRefs()
{
  if (!repr->arrayRefs)
    repr->arrayRefs = getArrayRefs(repr->dh->ped, repr->myLoop, FD_REF);

  return repr->arrayRefs;
}

//-----------------------------------------------------
// set of nonlocal references in loop

FortDAstSet* FortDLoop::NonLocalRefs()
{
  if (!repr->nonLocalRefs)
    repr->nonLocalRefs = getNonlocal(repr->dh->ped, repr->myLoop, FD_REF);

  return repr->nonLocalRefs;
}

//-----------------------------------------------------
// set of nonlocal statements in loop

FortDAstSet* FortDLoop::NonLocalStmts()
{
  if (!repr->nonLocalStmts)
    repr->nonLocalStmts = getNonlocal(repr->dh->ped, repr->myLoop, FD_STMT);

  return repr->nonLocalStmts;
}

//-----------------------------------------------------
// get all messages caused by loop

FortDMesgSet* FortDLoop::Mesgs()
{
  if (!repr->mesgs)
  {
    repr->mesgs = new FortDMesgSet();
    getFortDMesgSet(repr->dh, repr->mesgs, repr->myLoop, 
                        FD_MESG_ALL, repr->rsetInfo, repr->loopInfo);
  }
  return repr->mesgs;
}

//-----------------------------------------------------
// get all loop-independent messages caused by loop

FortDMesgSet* FortDLoop::IndependentMesgs()
{
  if (!repr->mesgs_indep)
  {
    repr->mesgs_indep = new FortDMesgSet();
    getFortDMesgSet(repr->dh, repr->mesgs_indep, repr->myLoop, 
                        FD_MESG_INDEP, repr->rsetInfo, NULL);
  }
  return repr->mesgs_indep;
}

//-----------------------------------------------------
// get all carried-all messages caused by loop

FortDMesgSet* FortDLoop::CarriedAllMesgs()
{
  if (!repr->mesgs_c_all)
  {
    repr->mesgs_c_all = new FortDMesgSet();
    getFortDMesgSet(repr->dh, repr->mesgs_c_all, repr->myLoop, 
                        FD_MESG_CARRIED_ALL, repr->rsetInfo, NULL);
  }
  return repr->mesgs_c_all;
}

//-----------------------------------------------------
// get all carried-part messages caused by loop

FortDMesgSet* FortDLoop::CarriedPartMesgs()
{
  if (!repr->mesgs_c_part)
  {
    repr->mesgs_c_part = new FortDMesgSet();
    getFortDMesgSet(repr->dh, repr->mesgs_c_part, repr->myLoop, 
                        FD_MESG_CARRIED_PART, repr->rsetInfo, NULL);
  }
  return repr->mesgs_c_part;
}

//-----------------------------------------------------
// get all reduction messages caused by loop

FortDMesgSet* FortDLoop::ReductionMesgs()
{
  if (!repr->mesgs_reduc)
  {
    repr->mesgs_reduc = new FortDMesgSet();
    getFortDMesgSet(repr->dh, repr->mesgs_reduc, repr->myLoop, 
                        FD_MESG_REDUC, NULL, repr->loopInfo);
  }
  return repr->mesgs_reduc;
}

//-----------------------------------------------------
// return whether dependence edge is cross-processor

Boolean FortDLoop::CrossProcDep(void* edge)
{
    // cross-processor dependences are generally true dependences
    // whose sink references cause nonlocal accesses

  return classify_CrossProcDep(repr->dh, (DG_Edge*) edge);
}

Color_type FortDLoop::CrossProcDepColor(void* edge)
{
  return classify_CrossProcDepColor(repr->dh, (DG_Edge*) edge);
}


//*********************************************************************
// Member functions for class FortDProc
//*********************************************************************

//---------------------------------------------------------------------
// initializes interface given dh, AST of procedure, and name of procedure

FortDProc::FortDProc(void* dh_handle, AST_INDEX node, void* cg)
{
  repr = new struct FortDProc_S;
  memset(repr, 0, sizeof(struct FortDProc_S));

  repr->dh = (Dist_Globals*) dh_handle;
  repr->myProc = node;
  repr->cg = cg;
  repr->name = ssave(BuildProcName(node));
  repr->reach_annot = GetReachAnnot();
  repr->overlap_annot = GetOverlapAnnot();
  repr->loops = new FortDAstSet;
  walk_statements(node, LEVEL1, findLoops, NULL, (Generic) repr->loops);

  repr->nonLocalRefs = NULL;
  repr->nonLocalStmts = NULL;

  repr->arrayRefs = NULL;

  repr->loops_parallel = NULL;
  repr->loops_pipelined = NULL;
  repr->loops_replicated = NULL;
  repr->loops_oneproc = NULL;

  repr->loops_comm = NULL;
  repr->loops_reduc = NULL;
  repr->loops_bcast = NULL;
  repr->loops_gather = NULL;
  repr->loops_irreg = NULL;
  repr->loops_runtime = NULL;
}

//---------------------------------------------------------------------
// cleans up Procedure interface

FortDProc::~FortDProc()
{
  sfree(repr->name);

  if (repr->nonLocalRefs) delete repr->nonLocalRefs;
  if (repr->nonLocalStmts) delete repr->nonLocalStmts;

  if (repr->loops_parallel) delete repr->loops_parallel;
  if (repr->loops_pipelined) delete repr->loops_pipelined;
  if (repr->loops_replicated) delete repr->loops_replicated;
  if (repr->loops_oneproc) delete repr->loops_oneproc;

  if (repr->loops_comm) delete repr->loops_comm;
  if (repr->loops_reduc) delete repr->loops_reduc;
  if (repr->loops_bcast) delete repr->loops_bcast;
  if (repr->loops_gather) delete repr->loops_gather;
  if (repr->loops_irreg) delete repr->loops_irreg;
  if (repr->loops_runtime) delete repr->loops_runtime;

  delete repr;
}

//-----------------------------------------------------
// set of array references in loop

FortDAstSet* FortDProc::ArrayRefs()
{
  if (!repr->arrayRefs)
    repr->arrayRefs = getArrayRefs(repr->dh->ped, repr->myProc, FD_REF);

  return repr->arrayRefs;
}

//-----------------------------------------------------
// set of nonlocal references in procedure

FortDAstSet* FortDProc::NonLocalRefs()
{
  if (!repr->nonLocalRefs)
    repr->nonLocalRefs = getNonlocal(repr->dh->ped, repr->myProc, FD_REF);

  return repr->nonLocalRefs;
}

//-----------------------------------------------------
// set of nonlocal statements in procedure

FortDAstSet* FortDProc::NonLocalStmts()
{
  if (!repr->nonLocalStmts)
    repr->nonLocalStmts = getNonlocal(repr->dh->ped, repr->myProc, FD_STMT);

  return repr->nonLocalStmts;
}

//-----------------------------------------------------
// get all messages caused by procedure

FortDMesgSet* FortDProc::Mesgs()
{
  if (!repr->mesgs)
  {
    repr->mesgs = new FortDMesgSet;
    getAllMesgs(repr->dh, AllLoops(), repr->mesgs);
  }

  return repr->mesgs;
}

//-----------------------------------------------------
// get AST_INDEX of all loops in procedure

FortDAstSet* FortDProc::AllLoops()
{
  return repr->loops;
}

//-----------------------------------------------------
// get all parallel loops in procedure (partitioned)

FortDAstSet* FortDProc::ParallelLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_parallel)
  {
    repr->loops_parallel = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) &&
          (fli->ltype == FD_LOOP_PARALLEL))
        repr->loops_parallel->Add(loop);
    }
  }
  return repr->loops_parallel;
}

//-----------------------------------------------------
// get all pipelined loops in procedure (requiring synch)

FortDAstSet* FortDProc::PipelinedLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_pipelined)
  {
    repr->loops_pipelined = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) &&
          (fli->ltype == FD_LOOP_PIPELINED))
        repr->loops_pipelined->Add(loop);
    }
  }
  return repr->loops_pipelined;
}

//-----------------------------------------------------
// get all sequential loops in procedure

FortDAstSet* FortDProc::SequentialLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_sequential)
  {
    repr->loops_sequential = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) &&
          (fli->ltype == FD_LOOP_SEQUENTIAL))
        repr->loops_sequential->Add(loop);
    }
  }
  return repr->loops_sequential;
}

//-----------------------------------------------------
// get all replicated loops in procedure (executed by all procs)

FortDAstSet* FortDProc::ReplicatedLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_replicated)
  {
    repr->loops_replicated = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) &&
          (fli->ltype == FD_LOOP_REPLICATED))
        repr->loops_replicated->Add(loop);
    }
  }
  return repr->loops_replicated;
}

//-----------------------------------------------------
// get all replicated loops in procedure (executed by all procs)

FortDAstSet* FortDProc::OneProcLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_oneproc)
  {
    repr->loops_oneproc = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) &&
          (fli->ltype == FD_LOOP_ONEPROC))
        repr->loops_oneproc->Add(loop);
    }
  }
  return repr->loops_oneproc;
}

//-----------------------------------------------------
// get all loops in procedure causing any communication

FortDAstSet* FortDProc::CommLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_comm)
  {
    repr->loops_comm = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) &&
          (fli->num_reduc || fli->num_c_part_send ||
           fli->num_c_all_send || fli->num_indep_send || 
           fli->num_c_all_bcast || fli->num_indep_bcast ||
           fli->num_c_all_gather || fli->num_indep_gather)) 
      {
          repr->loops_comm->Add(loop);
      }
    }
  }
  return repr->loops_comm;
}

//-----------------------------------------------------
// get all loops in procedure causing nonlocal reductions

FortDAstSet* FortDProc::ReductionLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_reduc)
  {
    repr->loops_reduc = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) && fli->num_reduc)
        repr->loops_reduc->Add(loop);
    }
  }
  return repr->loops_reduc;
}

//-----------------------------------------------------
// get all loops in procedure causing collective communication

FortDAstSet* FortDProc::BroadcastLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_bcast)
  {
    repr->loops_bcast = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) && 
          (fli->num_c_all_bcast || fli->num_indep_bcast))
        repr->loops_bcast->Add(loop);
    }
  }
  return repr->loops_bcast;
}

//-----------------------------------------------------
// get all loops in procedure causing reductions

FortDAstSet* FortDProc::GatherLoops()
{
  AST_INDEX loop;
  FortD_LI* fli;

  if (!repr->loops_gather)
  {
    repr->loops_gather = new FortDAstSet;

    while ((loop = (*(repr->loops))()) != AST_NIL)
    {
      fli = (FortD_LI*) get_info(repr->dh->ped, loop, type_fd);

      if ((fli != (FortD_LI*) NO_FD_INFO) && 
          (fli->num_c_all_gather || fli->num_indep_gather))
        repr->loops_gather->Add(loop);
    }
  }
  return repr->loops_gather;
}

//-----------------------------------------------------
// get all loops in procedure causing irreg communication

FortDAstSet* FortDProc::IrregLoops()
{
  return NULL;
}

//-----------------------------------------------------
// get all loops in procedure causing run-time resolution

FortDAstSet* FortDProc::RuntimeLoops()
{
  return NULL;
}

//------------------------------------------------------
//  return the call graph

void* FortDProc::GetCG()
{
  return repr->cg;
}

//------------------------------------------------------
//  return the procedure name

char* FortDProc::GetName()
{
  return repr->name;
}


//-----------------------------------------------------
// return whether dependence edge is cross-processor

Boolean FortDProc::CrossProcDep(void* edge)
{
  return classify_CrossProcDep(repr->dh, (DG_Edge*) edge);
}

Color_type FortDProc::CrossProcDepColor(void* edge)
{
  return classify_CrossProcDepColor(repr->dh, (DG_Edge*) edge);
}


//*********************************************************************
// Member functions for class FortDProg
//*********************************************************************

//-----------------------------------------------------
// initializes interface given dh & AST of procedure

FortDProg::FortDProg(void* dh_handle, AST_INDEX node)
{
  repr = new struct FortDProg_S;
  memset(repr, 0, sizeof(struct FortDProg_S));

  repr->dh = (Dist_Globals*) dh_handle;
  repr->myProg = node;
  repr->loops = new FortDAstSet;
  walk_statements(node, LEVEL1, findLoops, NULL, (Generic) repr->loops);
}

//-----------------------------------------------------
// initializes interface given dh & AST of procedure

FortDProg::~FortDProg()
{
  if (repr->nonLocalRefs) delete repr->nonLocalRefs;
  if (repr->nonLocalStmts) delete repr->nonLocalStmts;
  delete repr->loops;
  delete repr;
}

//-----------------------------------------------------
// set of nonlocal references in program

FortDAstSet* FortDProg::NonLocalRefs()
{
  if (!repr->nonLocalRefs)
    repr->nonLocalRefs = getNonlocal(repr->dh->ped, repr->myProg, FD_REF);

  return repr->nonLocalRefs;
}

//-----------------------------------------------------
// set of nonlocal statements in program

FortDAstSet* FortDProg::NonLocalStmts()
{
  if (!repr->nonLocalStmts)
    repr->nonLocalStmts = getNonlocal(repr->dh->ped, repr->myProg, FD_STMT);

  return repr->nonLocalStmts;
}


//-----------------------------------------------------
// get all messages caused by program

FortDMesgSet* FortDProg::Mesgs()
{
  if (!repr->mesgs)
  {
    repr->mesgs = new FortDMesgSet;
    getAllMesgs(repr->dh, AllLoops(), repr->mesgs);
  }

  return repr->mesgs;
}

//-----------------------------------------------------
// get AST_INDEX of all loops in program

FortDAstSet* FortDProg::AllLoops()
{
  return repr->loops;
}

//-----------------------------------------------------
// return whether dependence edge is cross-processor

Boolean FortDProg::CrossProcDep(void* edge)
{
  return classify_CrossProcDep(repr->dh, (DG_Edge*) edge);
}

Color_type FortDProg::CrossProcDepColor(void* edge)
{
  return classify_CrossProcDepColor(repr->dh, (DG_Edge*) edge);
}


//*********************************************************************
// Member functions for class FortDAstSet
//*********************************************************************

//-----------------------------------------------------
// constructor

FortDAstSet::FortDAstSet(int sz)
{
  repr = new struct FortDAstSet_S;
  repr->s = sz;
  repr->last = 0;
  repr->current = 0;
  repr->buf = new AST_INDEX[sz]; 
}

//-----------------------------------------------------
// destructor 

FortDAstSet::~FortDAstSet()
{
  delete repr->buf;
  delete repr;
}

//-----------------------------------------------------
// returns size of set

FortDAstSet::Size()
{
  return repr->last;
}

//-----------------------------------------------------
// add AST to list

void FortDAstSet::Add(AST_INDEX ast)
{
  if (repr->last == repr->s)
  {
    AST_INDEX* buf = new AST_INDEX[2*repr->s];
    memcpy(buf, repr->buf, sizeof(AST_INDEX)*repr->s);
    delete repr->buf;
    repr->buf = buf;
    repr->s *= 2;
  }

  repr->buf[repr->last++] = ast;
}

//-----------------------------------------------------
// returns element

AST_INDEX & FortDAstSet::operator[](int i)
{
  assert(i < repr->last);
  return repr->buf[i];
}

//-----------------------------------------------------
// iterates over elements in list

AST_INDEX FortDAstSet::operator () ()
{
  if (repr->current < repr->last)
    return repr->buf[repr->current++];

  repr->current = 0;
  return AST_NIL;
}

//*********************************************************************
// Member functions for class FortDRsdSet
//*********************************************************************

//-----------------------------------------------------
// constructor

FortDRsdSet::FortDRsdSet(int sz) 
{
  repr = new struct FortDRsdSet_S;
  repr->s = (sz > 0) ? sz : 1; 
  repr->last = 0;
  repr->current = 0;
  repr->buf = (FortDRsd**) new char[sizeof(FortDRsd*)*sz];
}

//-----------------------------------------------------
// destructor

FortDRsdSet::~FortDRsdSet()
{
  for (int i = 0; i < repr->last; i++)
    delete repr->buf[i];
  delete repr->buf;
  delete repr;
}

//-----------------------------------------------------
// returns size of set

int FortDRsdSet::Size()
{
  return repr->last;
}

//-----------------------------------------------------
// add element to FortDRsdSet

void FortDRsdSet::Add(FortDRsd* rsd)
{
  if (repr->last == repr->s)
  {
    FortDRsd** buf = (FortDRsd**) new char[2*sizeof(FortDRsd*)*repr->s];
    memcpy(buf, repr->buf, sizeof(FortDRsd*)*repr->s);
    delete repr->buf;
    repr->buf = buf;
    repr->s *= 2;
  }

  repr->buf[repr->last++] = rsd; 
}

//-----------------------------------------------------
// returns element of FortDRsdSet

FortDRsd* & FortDRsdSet::operator [](int i)
{
  assert(i < repr->last);
  return repr->buf[i]; 
}

//-----------------------------------------------------
// iterate over elements in set

FortDRsd* FortDRsdSet::operator () ()
{
  if (repr->current < repr->last)
    return repr->buf[repr->current++]; 

  repr->current = 0;
  return NULL;
}


//*********************************************************************
// Member functions for class FortDRsd
//*********************************************************************

//-----------------------------------------------------
// constructor

FortDRsd::FortDRsd(void* dh, void* rsdset) 
{
  repr = new struct FortDRsd_S;
  repr->rset = (Rsd_set*) rsdset;
  repr->refs = new FortDAstSet(repr->rset->num_subs);
  repr->dh = (Dist_Globals*) dh;

  for (int i = 0; i < repr->rset->num_subs; i++)
    repr->refs->Add(repr->rset->subs[i]);
}

//-----------------------------------------------------
// destructor

FortDRsd::~FortDRsd()
{
  delete repr->refs;
  delete repr;
}

//-----------------------------------------------------
// returns name of variable for RSD

char* FortDRsd::Name()
{
  return repr->rset->sp->id;
}

//-----------------------------------------------------
// returns number of dimensions in RSD

int FortDRsd::Dims()
{
  return repr->rset->rs->dims;
}

//-----------------------------------------------------
// returns size of RSD

int FortDRsd::Size()
{
  int i, size;

  size = 1;
  for (i = 0; i < Dims(); i++)
    size *= Upper(i) - Lower(i) + 1;

  return size;
}

//-----------------------------------------------------
// returns Lower bound in dimension

int FortDRsd::Lower(int i)
{
  return repr->rset->rs->subs[i].lo_b;
}

//-----------------------------------------------------
// returns Upper bound in dimension

int FortDRsd::Upper(int i)
{
  return repr->rset->rs->subs[i].up_b;
}

//-----------------------------------------------------
// returns AST of variable reference(s) used to build RSD

FortDAstSet* FortDRsd::Refs()
{
  return repr->refs;
}


//*********************************************************************
// Member functions for class FortDMesgSet
//*********************************************************************

//-----------------------------------------------------
// constructor for FortDMesgSet

FortDMesgSet::FortDMesgSet(int sz)
{
  repr = new struct FortDMesgSet_S;
  repr->s = (sz > 0) ? sz : 1; 
  repr->last = 0;
  repr->current = 0;
  repr->buf = (FortDMesg**) new char[sizeof(FortDMesg*)*sz];
}

//-----------------------------------------------------
// destructor for FortDMesgSet

FortDMesgSet::~FortDMesgSet()
{
  for (int i = 0; i < repr->last; i++)
    delete repr->buf[i];
  delete repr->buf;
  delete repr;
}

//-----------------------------------------------------
// return size of FortDMesgSet

FortDMesgSet::Size()
{
  return repr->last;
}

//-----------------------------------------------------
// add member to FortDMesgSet

void FortDMesgSet::Add(FortDMesg* mesg)
{
  if (repr->last == repr->s)
  {
    FortDMesg** buf = (FortDMesg**) new char[2*sizeof(FortDMesg*)*repr->s];
    memcpy(buf, repr->buf, sizeof(FortDMesg*)*repr->s);
    delete repr->buf;
    repr->buf = buf;
    repr->s  *= 2;
  }

  repr->buf[repr->last++] = mesg;
}

//-----------------------------------------------------
// whether message is already in FortDMesgSet

Boolean FortDMesgSet::InSet(void* rset)
{
  int i;

  for (i = 0; i < repr->last; i++)
  {
    if (repr->buf[i]->Equal(rset))
      return true;
  }
  return false;
}

//-----------------------------------------------------
// return member of FortDMesgSet

FortDMesg* & FortDMesgSet::operator[](int i)
{
  assert(i < repr->last);
  return repr->buf[i];
}

//-----------------------------------------------------
// iterates over elements in set

FortDMesg* FortDMesgSet::operator () ()
{
  if (repr->current < repr->last)
    return repr->buf[repr->current++];

  repr->current = 0;
  return NULL;
}


//*********************************************************************
// Member functions for class FortDMesg
//*********************************************************************

//-----------------------------------------------------
// initializes interface for a FortD message object given Rsd_set

FortDMesg::FortDMesg(void* dh, void* rset, Boolean reduc)
{
  repr = new struct FortDMesg_S;

  assert (rset);  // not null

  repr->dh = (Dist_Globals*) dh;
  repr->SendSectionStr = 0;
  repr->RecvSectionStr = 0;
  repr->SendProcRangeStr = 0;
  repr->RecvProcRangeStr = 0;

  if (reduc)
  {
    repr->reduc_set = (Reduc_set*) rset;
    repr->rset = NULL;
    repr->rsdSet = NULL;
  }
  else
  {
    repr->rset = (Rsd_set*) rset;
    repr->rsdSet = new FortDRsdSet(repr->rset->num_merge+1);
    repr->reduc_set = NULL;
  
    Rsd_set* rset = repr->rset;
    while (rset)
    {
      FortDRsd* rsd = new FortDRsd(repr->dh, rset);
      repr->rsdSet->Add(rsd);
      rset = rset->rsd_merge;
    }
  }
}

//-----------------------------------------------------
// destructor

FortDMesg::~FortDMesg()
{
  if (repr->rsdSet) delete repr->rsdSet;
  delete repr;
}

//-----------------------------------------------------
// type of message based on data dependences

Mesg_type FortDMesg::MesgType()
{
  return (repr->rset) ? repr->rset->mtype : FD_MESG_INDEP;
}

//-----------------------------------------------------
// how message is communicated

Comm_type FortDMesg::CommType()
{
  return (repr->rset) ? repr->rset->ctype : FD_COMM_REDUCE;
}

//-----------------------------------------------------
// how message is communicated

Reduc_type FortDMesg::ReducType()
{
  return (repr->rset) ? FD_REDUC_NONE : repr->reduc_set->rtype;
}

//-----------------------------------------------------

FortDRsdSet* FortDMesg::NonLocalRsds()
{
  return (repr->rset) ? repr->rsdSet : NULL;
}

//-----------------------------------------------------
// statement following (estimated) location of message

AST_INDEX FortDMesg::MesgLocation()
{
  if (repr->reduc_set)
    return repr->reduc_set->loop;

  if (repr->rset->mtype != FD_MESG_CARRIED_ALL)
    return repr->rset->location;

  return list_first(gen_DO_get_stmt_LIST(repr->rset->location));

}

//-----------------------------------------------------

int FortDMesg::MesgLevel()
{
  return (repr->rset) ? repr->rset->build_level : 
                        loop_level(repr->reduc_set->loop);
}

//-----------------------------------------------------

AST_INDEX FortDMesg::ReducLhs()
{
  return (repr->reduc_set) ? repr->reduc_set->lhs : AST_NIL;
}

//-----------------------------------------------------

Boolean FortDMesg::Equal(void* rset, Boolean reduc)
{
  return (Boolean)(reduc ? (rset == (void*)repr->reduc_set) :
                           (rset == (void*)repr->rset));
}


//*********************************************************************
// helper functions 
//*********************************************************************

static int 
findArrRefs(AST_INDEX node, Generic refParams)
{
  if (is_subscript(node))
  {
    struct findNonlocalParams* p = (findNonlocalParams*) refParams;
       p->astSet->Add(node);
  }
  return WALK_CONTINUE;
}

//-----------------------------------------------------
// helper function for walk_expression() in findNonlocalStmts()

static int 
findNonlocalRefs(AST_INDEX node, Generic refParams)
{
  if (is_subscript(node))
  {
    struct findNonlocalParams* p = (findNonlocalParams*) refParams;
    if (get_info(p->ped, gen_SUBSCRIPT_get_name(node), type_dc) != NO_DC_INFO)
      p->astSet->Add(node);
  }
  return WALK_CONTINUE;
}


//-----------------------------------------------------
// helper function for walk_statements() in getArrayRefs()

static int 
findArrayRefs(AST_INDEX node, int level, Generic refParams)
{
  if (is_assignment(node))
  {
         walk_expression(node, findArrRefs, NULL, refParams);
  }
  return WALK_CONTINUE;
}
//-----------------------------------------------------
// helper function for walk_statements() in getNonlocal()

static int 
findNonlocalStmts(AST_INDEX node, int level, Generic refParams)
{
  if (is_assignment(node))
  {
    struct findNonlocalParams* p = (findNonlocalParams*) refParams;

    Iter_set* iset = (Iter_set*) get_info(p->ped, node, type_dc);
    if ((iset != (Iter_set*) NO_DC_INFO) && iset->nonlocal_refs)
    {
      if (p->astLevel == FD_REF)
        walk_expression(node, findNonlocalRefs, NULL, refParams);
      else
        p->astSet->Add(node);
    }
  }
  return WALK_CONTINUE;
}

//-----------------------------------------------------
// find all array references

static FortDAstSet* 
getArrayRefs(PedInfo ped, AST_INDEX node, Ast_level astLevel)
{
  struct findNonlocalParams refParams;
  refParams.ped = ped;
  refParams.astSet = new FortDAstSet;
  refParams.astLevel = astLevel;  

  walk_statements(node, LEVEL1, findArrayRefs, NULL, 
                  (Generic) &refParams);

  return refParams.astSet;
}
//-----------------------------------------------------
// find statements causing nonlocal accesses

static FortDAstSet* 
getNonlocal(PedInfo ped, AST_INDEX node, Ast_level astLevel)
{
  struct findNonlocalParams refParams;
  refParams.ped = ped;
  refParams.astSet = new FortDAstSet;
  refParams.astLevel = astLevel;  

  walk_statements(node, LEVEL1, findNonlocalStmts, NULL, 
                  (Generic) &refParams);

  return refParams.astSet;
}

//-----------------------------------------------------
// find all loops in procedure
// helper function for walk_statements() in FortDProc constructor

static int 
findLoops(AST_INDEX node, int level, Generic Loops)
{
  if (is_loop(node))
    ((FortDAstSet*) Loops)->Add(node);

  return WALK_CONTINUE;
}

//-----------------------------------------------------
// helper function for generating set of specific message types

static void
getFortDMesgSet(Dist_Globals* dh, FortDMesgSet* mset, AST_INDEX loop, 
                Mesg_type type, Rsd_set_info* rinfo, FortD_LI* fli)
{
  int i;
  Rsd_set* rset;
  Reduc_set* reduc_s;

  // get messages for reductions
  if (fli && fli->num_reduc)
  {
    for (i = 0; i < fli->sgroup.group_num; i++)
    {
      reduc_s = fli->sgroup.groups[i]->iset->reduc_set;
      if (reduc_s && !reduc_s->local && (reduc_s->loop == loop))
        mset->Add(new FortDMesg(dh, reduc_s, true));
    }
  }

  if (!rinfo)
    return;

  // get messages caused by nonlocal accesses
  for (i = 0; i < rinfo->num_ref; i++)
  {
    rset = rinfo->rsd_s[i];
    while (rset)
    {
      if ((type == FD_MESG_ALL) ||
          ((type == FD_MESG_INDEP) && 
           (rset->mtype == FD_MESG_INDEP)) ||
          ((type ==  FD_MESG_CARRIED_ALL) && 
           (rset->mtype == FD_MESG_CARRIED_ALL)) ||
          ((type ==  FD_MESG_CARRIED_PART) && 
           (rset->mtype ==  FD_MESG_CARRIED_PART)))
      {
        mset->Add(new FortDMesg(dh, rset));
      }
      rset = rset->rsd_next;
    }
  }
}

//-----------------------------------------------------
// find all messages in AST

static void 
getAllMesgs(Dist_Globals* dh, FortDAstSet* loopSet, FortDMesgSet* mset)
{
  Rsd_set_info* rset;
  Rsd_set* rsd;
  FortD_LI* fli;
  AST_INDEX node;
  int i, j;

  // get communication in loops
  while ((node = (*loopSet)()) != AST_NIL)
  {
    fli = (FortD_LI*) get_info(dh->ped, node, type_fd);
    if (fli == (FortD_LI*) NO_FD_INFO)
      fli = NULL;

    rset = (Rsd_set_info*) get_info(dh->ped, node, type_dc);
    if (rset == (Rsd_set_info*) NO_DC_INFO)
      rset = NULL;

    getFortDMesgSet(dh, mset, node, FD_MESG_ALL, rset, fli);
  }

  // get communication out of loops (assumes 1 proc in module!)
  for (i = 0; i < dh->sgroup.group_num; i++)
  {
    for (j = 0; j < dh->sgroup.groups[i]->rset->num_ref; j++)
    {
      rsd = dh->sgroup.groups[i]->rset->rsd_s[j];
      while (rsd)
      {
        mset->Add((FortDMesg*)rsd);
        rsd = rsd->rsd_next;
      }
    }
  }
}


//-----------------------------------------------------
// classify loop types based on types of loops in each loop nest

static void
classify_loops(Dist_Globals* dh)
{
  int i, j, nest_level, pipe, par;
  FortD_LI *fli[MAXLOOP], *f;

  for (i = 0; i < dh->numdoloops; i++)
  {
    nest_level = pt_perf_nest(dh->doloops[i]);

    if (nest_level == 1)   // does not enclose other loops
    {
      f = (FortD_LI*) get_info(dh->ped, dh->doloops[i], type_fd);
      assert (f != (FortD_LI*) NO_FD_INFO);

      if ((f->ltype == FD_LOOP_REPLICATED) || 
          (f->ltype == FD_LOOP_ONEPROC))
        f->ltype = FD_LOOP_SEQUENTIAL;
    }
    else if (nest_level > 1)
    {
      par = -1;
      pipe = -1;

      for (j = 0; j < nest_level; j++)
      {
        fli[j] = (FortD_LI*) get_info(dh->ped, dh->doloops[i+j], type_fd);
        assert (fli[j] != (FortD_LI*) NO_FD_INFO);

        switch (fli[j]->ltype)
        {
          case FD_LOOP_PARALLEL:
            par = j;
            break;

          case FD_LOOP_PIPELINED:
            pipe = j;
            break;
        }
      }

      // classify loop nest based on loops contained

      if (pipe > -1)   // pipelined loop found 
      {
        // inner loops are sequentialized
        for (j = pipe+1; j < nest_level; j++)
          fli[j]->ltype = FD_LOOP_SEQUENTIAL;

        // outer parallel loops are pipelined
        for (j = pipe-1; j >= 0; j--)
          fli[j]->ltype = FD_LOOP_PIPELINED;

        // if outermost loop pipelined, make entire computation seq
        if (!pipe)
          fli[0]->ltype = FD_LOOP_SEQUENTIAL;

        // make all enclosing loops seq/pipelined
        if (fli[0]->depth > 1)
        {
          AST_INDEX node = dh->doloops[i];
          while (node != AST_NIL)
          {
            node = tree_out(node);
            if (is_loop(node))
            {
              f = (FortD_LI*) get_info(dh->ped, node, type_fd);
              assert (f != (FortD_LI*) NO_FD_INFO);

              if ((f->ltype == FD_LOOP_REPLICATED) || 
                  (f->ltype == FD_LOOP_ONEPROC))
              {
                f->ltype = pipe ? FD_LOOP_PIPELINED : FD_LOOP_SEQUENTIAL;
              }
            }
          }
        }
      }
      else if (par > -1)  // parallel loop found
      {
        // entire perfect loop nest is parallel 
        for (j = 0; j < nest_level; j++)
          fli[j]->ltype = FD_LOOP_PARALLEL;
      }

      i += nest_level-1;  // skip other loops in entire nest
    }
  }

  // any replicated loops left must be imperfectly nested.  make parallel

  for (i = 0; i < dh->numdoloops; i++)
  {
    f = (FortD_LI*) get_info(dh->ped, dh->doloops[i], type_fd);

    if (f->ltype == FD_LOOP_REPLICATED) 
    {
      // assert (!pt_perf_nest(dh->doloops[i]));
      f->ltype = FD_LOOP_PARALLEL;
    }
    else if (f->ltype == FD_LOOP_ONEPROC)
    {
      f->ltype = FD_LOOP_SEQUENTIAL;
    }

  }

}


//-----------------------------------------------------
// classify dependence as cross-processor

static Boolean
classify_CrossProcDep(Dist_Globals* dh, DG_Edge* edge)
{
  Rsd_set* rset = 0;

  if(edge->type == dg_control) return false;

  rset = (Rsd_set*)get_info(dh->ped, edge->sink, type_dc);

  if ((rset == (Rsd_set*)0) || (rset == (Rsd_set*)NO_DC_INFO)) 
  {
    return false;
  }
  else 
  {
     return true;
  }

//  else
//  {
//    if (dh->ped->selected_loop == rset->location)
//    {
//      return true;
//    }
//    else
//    {
//      return false;
//    }
//  }

}

//-----------------------------------------------
// classify dependence color 
static Color_type
classify_CrossProcDepColor(Dist_Globals* dh, DG_Edge* edge)
{
  Rsd_set *rset;
  AST_INDEX node;

  if(edge->type == dg_control) return (FD_BLACK);

  rset = (Rsd_set*)get_info(dh->ped, edge->sink, type_dc);
  if ((rset == 0) || (rset == (Rsd_set*)NO_DC_INFO))
  {
    return (FD_BLACK);
  }
  else 
  {
    switch(rset->mtype) 
    {
      case FD_MESG_INDEP:
        return (FD_GREEN);
      break;

      case FD_MESG_CARRIED_ALL:
        return (FD_GREEN);
      break;

        // At this point  there is a cross processor dependence on a partitioned
        // loop.  Determine if the loop is sequential or pipelined by obtaining
        // the statement color. 
      case FD_MESG_CARRIED_PART:
        node = edge->sink;
        if (dh->ped->selected_loop == rset->location)
        {
          switch (GenerateStmtColor(dh, node))
          {
            case FD_RED:
              return (FD_RED);
            break;

            case FD_YELLOW:    
              return (FD_YELLOW);
            break;

            default:
              cerr << "classify_CrossProcDepColor: illegal stmt color"
                   << endl;
              return (FD_BLACK);
            break;
          }
        }
        else
        {
          return (FD_BLACK);
        }
      break;

      default:
        return (FD_BLACK);
      break;
    }
  }
}

//------------------------------------------------------------------
// 1. traverse the AST, perform local reaching decomposition
// 2. walk the tree, storing sp pointers at each reference
//------------------------------------------------------------------
static void ModuleSetUp(AST_INDEX root,PedInfo ped, FortTree ft, Context c, CallGraph* cg)
{
  Generic dummy;
  CallGraphNode *n;
  struct proc_list p;

  FD_Composition_HT *fortd_ht = new FD_Composition_HT(cg);


 cNameValueTable analyses = NameValueTableAlloc
 (8, (NameCompareCallback)strcmp,(NameHashFunctCallback) hash_string);

//-----------------------------------------------------
// record the fortran d annotations in the hash table  
 NameValueTableAddPair(analyses, (Generic)FORTD_REACH_ANNOT, 0, &dummy);
 NameValueTableAddPair(analyses, (Generic)FORTD_COMMON_BLOCK_ANNOT, 0, &dummy);
 NameValueTableAddPair(analyses, (Generic)FORTD_OVERLAP_ANNOT, 0, &dummy);

//------------------------------
// initialize p structure
  p.count = 0;

//----------------------------------------------  
// at this point the call graph has been built

//---------------------------------------------------
// get the list of procedures in the the module

  walk_statements(root, LEVEL1, procs_and_functions, NULL, (Generic)&p);

//------------------------------------------------------------
// for each procedure get the reach and overlap annotation 

 fortd_ht->record(ft, 0, root, 0, ped);
   
 for( int i = 0; i < p.count; i++) {
   n = cg->LookupNode(gen_get_text(get_name_in_entry(p.node[i])));

   if (n == 0)
     cout << "Unable to find node "
          << gen_get_text((get_name_in_entry(p.node[i])))
          << " in call graph \n";

  FD_Reach_Annot* reach_annot = 
     (FD_Reach_Annot*)n->GetAnnotation(FORTD_REACH_ANNOT, true);
  reach_annot->MapReachNodeAnnot(n);
  
  FD_Overlap_Annot *overlap_annot = 
    (FD_Overlap_Annot*)n->GetAnnotation(FORTD_OVERLAP_ANNOT, true);

  fortd_ht->add(n->procName, p.node[i], gen_get_node_type(p.node[i]));
  
  FD_ProcEntry *entry = fortd_ht->GetEntry(n->procName);
  entry->PutReadAnnotation(reach_annot, overlap_annot);
  fortd_ht->put_proc(entry);

//---------------------------------------------------------
// store the ft and context in the Fortran D info structure

  fortd_ht->proc()->proc_annot->f->ContextFt(c, ft);

//----------------------------------------------- 
// perform local decomposition analysis
 
  dc_compute_local_decomp(entry->ast, fortd_ht);
  }

  NameValueTableFree(analyses);
}
