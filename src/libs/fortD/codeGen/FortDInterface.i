/* $Id: FortDInterface.i,v 1.5 1997/03/27 20:32:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*--------------------------------------------------------------------------

  FortDInterface.i   Private data structures for Fortran D data abstractions

*/
#ifndef FortDInterface_i
#define FortDInterface_i


#include <libs/fortD/codeGen/private_dc.h>

class FortDAstSet;
class FortDMesgSet;
class FortDRsdSet;
class FortDMesg;
class FortDRsd;

typedef enum
{
  FD_REF,                         // reference level
  FD_STMT                         // statement level
} Ast_level;


//*********************************************************************
// Private structure definitions
//*********************************************************************

struct FortDAstSet_S {
  int s;                          // total size of set
  int last;                       // last element in use
  int current;                    // current element for iterator
  AST_INDEX* buf;                 // pointer to members of set
};

struct FortDRsd_S {
  Dist_Globals* dh;               // handle to Dist_Globals
  Rsd_set* rset;                  // handle to message descriptor
  FortDAstSet* refs;              // set of refs contributing to RSD
};

struct FortDRsdSet_S {
  int s;                          // total size of set
  int last;                       // last element in use
  int current;                    // current element for iterator
  FortDRsd** buf;                 // pointer to members of set
};

struct FortDMesg_S {
  Dist_Globals* dh;                    // handle to Dist_Globals
  Rsd_set* rset;                       // handle to message descriptor
  Reduc_set* reduc_set;                // handle to reduction set descriptor
  FortDRsdSet* rsdSet;                 // set of Rsds for message
  FD_String *RecvSectionStr;   // string containing recv information
  FD_String *SendSectionStr;   // string containing send information
  FD_String *RecvProcRangeStr; // string containing recv processors
  FD_String *SendProcRangeStr; // string containing send processors
};

struct FortDMesgSet_S {
  int s;                          // total size of set
  int last;                       // last element in use
  int current;                    // current element for iterator
  FortDMesg** buf;                // pointer to members of set
};

struct FortDRef_S {
  Dist_Globals* dh;               // handle to results from the compiler
  AST_INDEX myRef;                // AST_INDEX of reference
  SNODE* symEntry;                // symbol table entry
  Rsd_set* rset;                  // RSD for nonlocal access, if any
  Boolean distributed;            // whether array is distributed
};

struct FortDStmt_S {
  Dist_Globals* dh;               // handle to Dist_Globals
  AST_INDEX myStmt;               // AST node examined
  Iter_set* iset;                 // handle to Iter_set
  FortDAstSet* nonLocalRefs;      // set of nonlocal refs in stmt
  FortDAstSet* arrayRefs;         // set of array refs
  Color_type stmtColor;           // statement color
};

struct FortDLoop_S {
  Dist_Globals* dh;               // handle to Dist_Globals
  AST_INDEX myLoop;               // AST node examined
  FortD_LI* loopInfo;             // handle to FortD_LI
  Rsd_set_info* rsetInfo;         // handle to Rsd_set_info
  FortDAstSet* nonLocalRefs;      // set of nonlocal refs in loop
  FortDAstSet* arrayRefs;         // set of array refs
  FortDAstSet* nonLocalStmts;     // set of nonlocal stmts in loop

  FortDMesgSet* mesgs;            // all messages in loop
  FortDMesgSet* mesgs_indep;      // indep messages at loop
  FortDMesgSet* mesgs_c_all;      // carried-all messages  at loop
  FortDMesgSet* mesgs_c_part;     // carried-part messages at loop
  FortDMesgSet* mesgs_reduc;      // reduction messages at loop
};

struct FortDProc_S {
  Dist_Globals* dh;               // handle to Dist_Globals
  void* cg;                       // call graph
  AST_INDEX myProc;               // AST node examined
  char* name;                     // name of procedure
  void* reach_annot;              // annotations for interprocedural info
  void* overlap_annot;            // annotations for overlaps

  FortDAstSet* nonLocalRefs;      // set of nonlocal refs in procedure
  FortDAstSet* nonLocalStmts;     // set of nonlocal stmts in procedure
  FortDAstSet* arrayRefs;         // set of array refs
  FortDAstSet* loops;             // all loops in procedure
  FortDAstSet* loops_parallel;    // loops executed in parallel
  FortDAstSet* loops_pipelined;   // loops executed in pipeline
  FortDAstSet* loops_sequential;  // pipelined loops executed sequentially
  FortDAstSet* loops_replicated;  // loops executed by all procs
  FortDAstSet* loops_oneproc;     // loops executed by one proc

  FortDAstSet* loops_comm;        // loops containing in parallel
  FortDAstSet* loops_reduc;       // loops containing reduction comm
  FortDAstSet* loops_bcast;       // loops containing broadcast
  FortDAstSet* loops_gather;      // loops containing gather
  FortDAstSet* loops_irreg;       // loops containing irreg comm
  FortDAstSet* loops_runtime;     // loops containing run-time resolution

  FortDMesgSet* mesgs;            // all messages in procedure
};

struct FortDProg_S {
  Dist_Globals* dh;               // handle to Dist_Globals
  AST_INDEX myProg;               // AST node examined
  FortDAstSet* loops;             // all loops in program
  FortDAstSet* nonLocalRefs;      // set of nonlocal refs in program
  FortDAstSet* nonLocalStmts;     // set of nonlocal stmts in program
  FortDMesgSet* mesgs;            // all messages in program
}; 

struct FortDInterface_S {
  Dist_Globals* dh;               // handle to Dist_Globals
  PedInfo ped;                    // handle to Ped data
  AST_INDEX root;                 // handle to root of program/subroutine
  Context context;                // handle to program context
  void* cg;                       // handle to the call graph
};

struct findNonlocalParams {
  PedInfo ped;
  FortDAstSet* astSet;            // set of matching nonlocal AST found
  Ast_level astLevel;             // select refs or stmts
};


#endif
