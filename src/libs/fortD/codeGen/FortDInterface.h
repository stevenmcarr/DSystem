/* $Id: FortDInterface.h,v 1.10 1997/06/24 17:38:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*--------------------------------------------------------------------------

  FortDInterface.h      Interface to Fortran D data abstractions 


*/
#ifndef _FortDInterface_
#define _FortDInterface_

#include <libs/frontEnd/ast/ast.h>
#include <libs/fortD/misc/FortDEnum.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/fortD/misc/fd_string.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/support/database/context.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/codeGen/FortDInterface.i>


const int initSize = 8;


//-------------------------------------------------------------
// class for set of AST_INDEX  
//-------------------------------------------------------------
class FortDAstSet
{
  struct FortDAstSet_S* repr;

 public:
  FortDAstSet(int sz = initSize);     // constructor 
  ~FortDAstSet();                     // destructor
  int Size();                         // number of ASTs in set
  void Add(AST_INDEX ast);            // add AST to set
  AST_INDEX & operator [] (int i);    // access ASTs in set
  AST_INDEX operator ()();            // iterate over ASTs in set
};


//-------------------------------------------------------------
// simple description of the size & extent of an array section  
//-------------------------------------------------------------
class FortDRsd 
{
  struct FortDRsd_S* repr;
  StringBuffer* SendStr(void*);
  StringBuffer* RecvStr(void*);

 public:
  FortDRsd(void* dh, void* rsd);  // constructor
  ~FortDRsd();                    // destructor
  char* Name();                   // name of variable for RSD
  int Dims();                     // number of dimensions
  int Size();                     // total size of section
  int Lower(int dim);             // lower bound in dim
  int Upper(int dim);             // upper bound in dim
  FortDAstSet* Refs();            // ASTs of ref(s) composing RSD

  StringBuffer* SendStr(void* rs, void* info_str);
  StringBuffer* RecvStr(void* rs, void* info_str);

  Boolean SimpleShiftComm();
  void* GetRSet();
};


//-------------------------------------------------------------
// class for set of pointers to FortDRsd   
//-------------------------------------------------------------
class FortDRsdSet
{
  struct FortDRsdSet_S* repr;
  OrderedSetOfStrings* SendStr(void*);
  OrderedSetOfStrings* RecvStr(void*);

 public:
  FortDRsdSet(int sz = initSize);  // constructor
  ~FortDRsdSet();                  // destructor 
  int Size();                      // number of RSD ptrs in set
  void Add(FortDRsd* rsd);         // add RSD ptr to set
  FortDRsd* & operator [] (int i); // access RSD ptrs in set
  FortDRsd* operator ()();         // iterate over RSD ptrs in set
};


//-------------------------------------------------------------
// message information 
//-------------------------------------------------------------
class FortDMesg
{
  struct FortDMesg_S* repr;
  FD_String* ReducTypeStr();         // reduction type

  FD_String* ReducVarStr();          // reduction variable

  FD_String*  SendSection();     // send section for shift & send recv
  FD_String*  RecvSection();     // recv section for shift & send recv
  FD_String*  SendProcRange();   // send proc range for shift & send recv
  FD_String*  RecvProcRange();   // recv proc range for shift & send recv
  
  FD_String* BroadcastSendSectionStr(); // send section for broadcast
  FD_String* BroadcastRecvSectionStr(); // recv section for broadcast
  FD_String* BroadcastSendProcRange(int);  // send proc range for broadcast
  FD_String* BroadcastRecvProcRange();  // recv proc range for broadcast

  FD_String* RecvSProcRange();  // recv proc for single send/recv communication
  FD_String* SendRProcRange();  // send proc for single sen/recv communication

  FD_String* ReducSendSectionStr();   // reduction send section string
  FD_String* ReducRecvSectionStr();   // reduction recv section string
  FD_String* ReducSendProcRangeStr(); // reduction send proc range string
  FD_String* ReducRecvProcRangeStr(); // reduction recv proc range string
  void ReducSectionStr();       // reduction section string
  void ReducProcRangeStr();     // reduction proc range string


 public:
  FD_String* ReducMsgSizeStr();      // reduction mesg size
  FortDMesg(void* dh, void* rset, Boolean reduc = false);   // constructor
  ~FortDMesg();                    // destructor
  Mesg_type MesgType();            // returns message type 
  Comm_type CommType();            // returns communication type 
  Reduc_type ReducType();          // returns reduction type 
  FortDRsdSet* NonLocalRsds();     // set of nonlocal rsds for message
  AST_INDEX MesgLocation();        // returns location of message
  int MesgLevel();                 // returns loop level of message
  AST_INDEX ReducLhs();            // returns lhs for reduction
  Boolean Equal(void* rset, Boolean reduc = false);   // compare two messages

  FD_String* MesgTypeStr();              // mesg type string
  FD_String* CommTypeStr();              // communication type string
  FD_String* AggrMesgStr();              // if message aggregation, return str
  void MesgInfoStr(); // test function
// string functions that return array section and processor range information

  FD_String* SendSectionString();
  FD_String* RecvSectionString();
  FD_String* SendProcessorRangeString();
  FD_String* RecvProcessorRangeString();
};


//-------------------------------------------------------------
// class for set of pointers to FortDMesg  
//-------------------------------------------------------------
class FortDMesgSet
{
  struct FortDMesgSet_S* repr;

 public:
  FortDMesgSet(int sz = initSize);  // constructor
  ~FortDMesgSet();                  // destructor
  int Size();                       // number of messages ptrs in set
  void Add(FortDMesg* mesg);        // add message ptr to set
  Boolean InSet(void* rset);        // whether Rsd_set already in set
  FortDMesg* & operator [] (int i); // access messages ptrs in set   
  FortDMesg* operator ()();         // iterate over messages ptrs in set   

};


//-------------------------------------------------------------
// base class for variable reference information 
//-------------------------------------------------------------
class FortDRef
{
  struct  FortDRef_S* repr;
  StringBuffer* GetUpper(int dim); // upper overlap for a dimension
  StringBuffer* GetLower(int dim); // lower overlap for a dimension
  StringBuffer* AlignSt();
  StringBuffer* DistribSt();
  StringBuffer* DecompSt();

 public:
  FortDRef(void* dh, AST_INDEX ref);  // constructor
  ~FortDRef();                               // destructor
  
  void* GetSymEntry();
  //--------------- information for entire variable-----------------//
  Boolean IsDistributed();     // is variable distributed across procs?
  int Numdim();                // return the number of dimensions
  OrderedSetOfStrings* AlignStr();    // alignment information as a string
  OrderedSetOfStrings* DistribStr();  // distribute information as a string 
  OrderedSetOfStrings* DecompStr();   // decomposition as a string
  OrderedSetOfStrings* OverlapStr();  // overlap information as a string
  OrderedSetOfStrings* DecompositionInfo(); // complete decomposition including
                                            // alignment and distribution
  //----------- returns the id that may be used to obtain--------------//
  // AST_INDEX of the decomposition directive using ft_NodeToNumber ---//
  int DecompId();               
  int AlignId();
  int DistribId();

#if 0
   //--- returns the context of the module containing the decomposition ----//
   //   align distribute directive                     
  Context Contxt();             
#endif

  //--------- information for each dimension of variable------------//
  Dist_type Distrib(int dim);   // distribution type of dimension
  int BkSize(int dim);          // block size for block_cyclic
  int Extent(int dim);          // extent of array in dimension
  int LowerBound(int dim);      // lower boundary of variable in dim
  int UpperBound(int dim);      // upper boundary of variable in dim
  int LowerOverlap(int dim);    // size of overlap in lower boundary of dim
  int UpperOverlap(int dim);    // size of overlap in upper boundary of dim

  //------- information for specific reference to variable---------//
  Boolean NonLocalAcc();        // does ref cause nonlocal accesses?
  FortDMesg* MesgInfo();        // returns info on message generated
  Boolean CrossProcDep(void* dg_edge);  // cross-processor dependence?
  Color_type CrossProcDepColor(void* dg_edge);  // cross-processor dep color
  Color_type GetColor();        // returns a color depending on how bad
                                // a reference may be
  FD_String *GetColorStr();     // returns the color in a string
  void Test();                  // test routine
};


//-----------------------------------------------------------------
// base class for statement information
//-----------------------------------------------------------------
class FortDStmt
{
  struct FortDStmt_S* repr;

 public:

  FortDStmt(void* dh, AST_INDEX stmt); // constructor
  ~FortDStmt();                     // destructor
  Stmt_type StmtType();             // returns type of statement 
  Boolean IsPrivate();              // is stmt assignment to private var?
  FortDAstSet* PrivateUse();        // if private, set of uses of var
  Boolean NonLocalAcc();            // does stmt cause nonlocal accesses?
  FortDAstSet* NonLocalRefs();      // set of nonlocal refs
  FortDAstSet* ArrayRefs();         // set of array refs
  FortDMesgSet* Mesgs();            // all msgs caused by stmt
  Boolean CrossProcDep(void* dg_edge);  // cross-processor dependence?
  Color_type CrossProcDepColor(void* dg_edge);  // cross-processor dep color
  Color_type GetColor();
  FD_String* GetColorStr();
  void Test();
};


//-----------------------------------------------------------------
// base class for loop information
//-----------------------------------------------------------------
class FortDLoop
{
  struct FortDLoop_S* repr;
  void* GetDistGlobals();           // returns Dist_Globals 

 public:

  FortDLoop(void* dh, AST_INDEX loop);  // constructor
  ~FortDLoop();                        // destructor
  Loop_type LoopType();                // returns loop type 
  Boolean IsUniform();                 // are all stmts in loop uniform?
  Boolean ItersPartitioned();          // whether iterations are partitioned
  int ItersExecuted();                 // # iterations executed by processor
  FortDAstSet* NonLocalRefs();         // set of nonlocal refs
  FortDAstSet* ArrayRefs();            // set of array refs
  FortDAstSet* NonLocalStmts();        // set of nonlocal statements

  FortDMesgSet* Mesgs();               // all msgs caused by loop
  FortDMesgSet* IndependentMesgs();    // loop-independent msgs
  FortDMesgSet* CarriedAllMesgs();     // carried-all msgs
  FortDMesgSet* CarriedPartMesgs();    // carried-part msgs 
  FortDMesgSet* ReductionMesgs();      // reduction msgs for loop

  Boolean CrossProcDep(void* dg_edge); // cross-processor dependence?
  Color_type CrossProcDepColor(void* dg_edge); // cross-processor dep color
  void AllMesgsStr();

};


//-----------------------------------------------------------------
// base class for procedure information
//-----------------------------------------------------------------
class FortDProc
{
  struct FortDProc_S* repr;
  char* BuildProcName(AST_INDEX node);
  void* GetReachAnnot();
  void* GetOverlapAnnot();
  char* GetName();
  void* GetCG();

 public:

  FortDProc(void* dh, AST_INDEX proc, void* cg);  // constructor
  ~FortDProc();                        // destructor
  OrderedSetOfStrings* ReachingDecompsList(); // reaching decompositions 

  FortDMesgSet* Mesgs();               // all msgs caused by procedure
  FortDAstSet* NonLocalRefs();         // set of nonlocal refs
  FortDAstSet* ArrayRefs();            // set of array refs
  FortDAstSet* NonLocalStmts();        // set of nonlocal statements
  FortDAstSet* AllLoops();             // all loops in procedure
  FortDAstSet* ReplicatedLoops();      // loops executed by all procs
  FortDAstSet* ParallelLoops();        // parallel loops
  FortDAstSet* PipelinedLoops();       // pipelined loops
  FortDAstSet* SequentialLoops();      // pipelined loops executed sequentially
  FortDAstSet* OneProcLoops();         // loops executed by one processor
  FortDAstSet* CommLoops();            // loops causing comm
  FortDAstSet* BroadcastLoops();       // loops causing broadcast comm
  FortDAstSet* ReductionLoops();       // loops causing nonlocal reductions
  FortDAstSet* GatherLoops();          // loops causing gather comm
  FortDAstSet* IrregLoops();           // loops causing irreg comm
  FortDAstSet* RuntimeLoops();         // loops causing runtime resolution
  Boolean CrossProcDep(void* dg_edge); // cross-processor dependence?
  Color_type CrossProcDepColor(void* dg_edge); // cross-processor dep color
};


//-----------------------------------------------------------------
// base class for program information
//-----------------------------------------------------------------
class FortDProg
{
  struct FortDProg_S* repr;

 public:

  FortDProg(void* dh, AST_INDEX prog);  // constructor
  ~FortDProg();                                //  destructor

  FortDMesgSet* Mesgs();               // all msgs caused by program
  FortDAstSet* NonLocalRefs();         // set of nonlocal refs
  FortDAstSet* NonLocalStmts();        // set of nonlocal statements
  FortDAstSet* AllLoops();             // all loops in program
  Boolean CrossProcDep(void* dg_edge); // cross-processor dependence?
  Color_type CrossProcDepColor(void* dg_edge); // cross-processor dep color

  void CommCost() {};                 // communication as a percentage of
                                      // total computation
  void CommInfo() {};                 // return all the communication that
                                      // occurs in the program
  void ComputeIntensiveProcs() {};    // return compute intensive procedures
  void ComputeIntensiveLoops() {};    // return compute intensive loops
  void CommIntensiveProcs() {};       // return communication intensive
                                      //   procedures
  void CommIntensiveLoops() {};       // return communication intensive
                                      //   loops
  void PartitionImpact() {};          // display effect of partition on
                                      //   parallelism and communication

};


//-----------------------------------------------------------------
// class for information gathered through Fortran D interface
//-----------------------------------------------------------------

class FortDInterface      // one member per Fortran D program
{
  struct FortDInterface_S* repr;
  void*  BuildCG(Context pgm_context); 

 public               :                 // constructor
  FortDInterface(void* pedinfo, AST_INDEX astroot, Context, void* cg, FortTree ft);

  ~FortDInterface();                    // destructor
  void Analyze();                       // analyze program
  void Transform();                     // apply optimizing tranformations
  FortDRef* GetRef(AST_INDEX ref);      // get info for references
  FortDStmt* GetStmt(AST_INDEX stmt);   // get info for statements
  FortDLoop* GetLoop(AST_INDEX loop);   // get info for loops
  FortDProc* GetProc(AST_INDEX proc);   // get info for procedures
  FortDProg* GetProg(AST_INDEX prog);   // get info for program
};


#endif 

