/* $Id: MesgStaticSDDF.C,v 1.2 2001/09/14 18:31:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: MesgStaticSDDF.C,v 1.2 2001/09/14 18:31:57 carr Exp $ -*-c++-*-
//**************************************************************************
// Definitions of member functions and C wrappers for class MesgStaticSDDF
//**************************************************************************

#include <stdio.h>
#include <string.h>
					// From rest of D compiler world:
#ifndef groups_h
#include <libs/frontEnd/ast/groups.h>
#endif
#ifndef AstIterators_h
#include <libs/frontEnd/ast/AstIterators.h>
#endif
#ifndef FD_CODEGEN
#include <libs/fortD/codeGen/private_dc.h>
#endif
#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif
					// From instrumentation world
#ifndef instrument_spmd_h
#include <libs/fortD/performance/instr/InstrumentSPMD.h>
#endif
#ifndef _SDDF_General_h
#include <libs/fortD/performance/staticInfo/SDDF_General.h>
#endif
#ifndef _SD_Base_h
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#endif
#ifndef _SD_Map_h
#include <libs/fortD/performance/staticInfo/SD_Map.h>
#endif
#ifndef _SD_DataInfo_h
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#endif
#ifndef _SD_SrcInfo_h
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>
#endif
#ifndef ArrayNameInfo_h
#include <libs/fortD/performance/staticInfo/ArrayNameInfo.h>
#endif
#include <libs/fortD/performance/staticInfo/MesgStaticSDDF.h>

static MessageInfo messageInfo;		// Later put this in thePabloGlobalInfo


//**************************************************************************
// C wrappers
//**************************************************************************

void GetMessageParams(Dist_Globals *dh,
		      AST_INDEX loop,
		      Rsd_set *rset)
{
    messageInfo.GetMessageParams(dh, loop, rset);
}

void GetMessageSendParams(Dist_Globals *dh,
			  Rsd_set* rset,
			  Mesg_data *mesgData,
			  Boolean buffered,
			  AST_INDEX sendStmt)
{
    messageInfo.GetMessageSendParams(dh, rset, mesgData, buffered, sendStmt);
}

void  GetMessageRecvParams(Dist_Globals *dh,
			   Rsd_set* rset,
			   Mesg_data *mesgData,
			   Boolean buffered,
			   AST_INDEX recvStmt)
{
    messageInfo.GetMessageRecvParams(dh, rset, mesgData, buffered, recvStmt);
}

void GetMessageSendParamsForReduc(Dist_Globals* dh,
				  Reduc_set* reducSet,
				  AST_INDEX reducStmt)
{
    messageInfo.GetMessageSendParamsForReduc(dh, reducSet, reducStmt);
}

void SDDF_StoreDepEdgeForRef(AST_INDEX rsideName,
			     DG_Edge* edge)
{
    messageInfo.StoreDepEdgeForRef(rsideName, edge);
}


//**************************************************************************
// Private inline functions and forward declarations.
//**************************************************************************

inline int gen_CONSTANT_get_value(AST_INDEX constAst)
{
    assert(is_constant(constAst));
    return atoi(gen_get_text(constAst));
}

// Get size of machine data type in bytes
// WARNING: Embedded machine info. Get this from the right place later.
//          But same constants also embedded in make_reduc_call().
    
inline int GetSizeofType(int elemType)
{
    assert(elemType == INTTYPE || elemType == REAL || elemType == DOUBLE_P);
    return (elemType == INTTYPE || elemType == REAL)? 4 : 8;
}


EXTERN(int, dc_array_size,	(SNODE *sp)); // In coll_msgs.ansi.c

STATIC(void, MakeDependRecords,	(SDDF_MessageSend* mesgSend,
				 Rsd_set* rset));
STATIC(SDDF_DependInfo*, MakeOneDependRecord, (AST_INDEX srcRef,
					       AST_INDEX sinkRef));
STATIC(void, PutRSDInfo,	(Dist_Globals* dh,
				 AST_INDEX sendStmt,
				 SDDF_MessageSend* mesgSend,
				 Rsd_set* rset, Mesg_data *mesgData));
STATIC(void, putMesgTypeAndPid,	(Dist_Globals* dh,
				 AST_INDEX sendStmt,
				 SDDF_Symbolic_Value* mesgType,
				 SDDF_Symbolic_Value* mesgPid));
STATIC(void, InstrumentMesgCall,(Dist_Globals* dh,
				 AST_INDEX stmt,
				 SPMDInstrumentation* instr,
				 Static_Id staticId));
STATIC(void, InstrumentSymbolic,(Dist_Globals* dh,
				 AST_INDEX intExpr,
				 AST_INDEX stmt,
				 SPMDInstrumentation* instr,
				 Static_Id staticId,
				 Boolean   addGuard,
				 int	   initialValue));


//**************************************************************************
// Member functions for class MessageInfo 
//**************************************************************************
 
MessageInfo::MessageInfo() {}
MessageInfo::~MessageInfo() {}
    
//--------------------------------------------------------------------------
// void MessageInfo::GetMessageParams(...)
//
// Call this for each communication inserted, after calling
// GetMessageSendParams, GetMessageRecvParams, etc.

void MessageInfo::GetMessageParams(Dist_Globals* /*dh*/,
				   AST_INDEX loop,
				   Rsd_set *rset)
{
    if (! thePabloGlobalInfo.wantPabloInfo) return;
  
    assert(nextMesgSend != (SDDF_MessageSend*) NULL); // Verify created in
						      // GetMessageSendParams
    //-----------------------------------------------------------------
    // Store send and recv msg line numbers in side-array
    
    // AST_INDEX stmt;
    // AstIterator tree_walk(dh->send_stmts, PreOrder, AST_ITER_STMTS_ONLY);
    // for ( ; stmt = tree_walk.Current(); tree_walk++) {
	// thePabloGlobalInfo.currentLocals->sideArray->SD_MapStmt(stmt);
    // }
    // tree_walk.ReConstruct(dh->recv_stmts, PreOrder, AST_ITER_STMTS_ONLY);
    // for ( ; stmt = tree_walk.Current(); tree_walk++) {
	// thePabloGlobalInfo.currentLocals->sideArray->SD_MapStmt(stmt);
    // }
    
    // int lineNum = (int)(ft_GetFromSideArray(thePabloGlobalInfo.
    //			currentLocals->sideArray, stmt, SD_START_LINE_INDEX));
    int lineNum = -1;
    SDDF_ProcInfo* procInfo = thePabloGlobalInfo.currentLocals->procInfo;
    nextMesgSend->SetPosition(lineNum, procInfo);

    // rset will be NULL for a reduction. Other than above, reduction is
    // handled entirely by GetMessageSendParamsForReduc. So return.
    // (When the MesgOperType value from class SDDF_MessageSend is made
    //  readable, test that it is a reduction type.)
    if (rset == (Rsd_set *) NULL) {
	assert(nextMesgRecv == (SDDF_MessageRecv*) NULL); // No recv on reduc.
	nextMesgSend = (SDDF_MessageSend*) NULL; // Clear pointer for next use
	return;
    }

    MakeDependRecords(nextMesgSend, rset);
    
    //-----------------------------------------------------------------
    // Depending on message type (logic from dc_rset_msgs()):
    //	-- Get recv records if needed
    //	-- Set mesg operation type.

    switch (rset->ctype) {
      case FD_COMM_SHIFT:
	assert(nextMesgRecv);	// Created in GetMessageRecvParams()
	nextMesgSend->SetMesgOperType(MESSAGESEND_SYNC_SEND_SYNC_RECV);
	break;

      case FD_COMM_BCAST:
	nextMesgSend->SetMesgOperType(MESSAGESEND_GLOBAL_BCAST);
	break;
      
      case FD_COMM_TRANSPOSE:	// NOTE: reductions are detected by
      case FD_COMM_REDUCE:		//       loop_level(loop) == 1 above.
	if (loop_level(loop) != 1)	// Therefore avoid that case here.
	    die_with_message("F77D: Transpose, gather not implemented\n");
	break;			// Error msg is from coll_msgs.ansi.c
	    
      case FD_COMM_SEND_RECV:
	assert(nextMesgRecv);
	nextMesgSend->SetMesgOperType(MESSAGESEND_SYNC_SEND_SYNC_RECV);
	break;
	    
      default: break;		// irreg
    }
    
    // Get associated loop for this communication.  If this comm. is for
    // a stmt group outside all loops, "loop" will not point to a loop
    // (instead, it will point to the first stmt in the group).
    
    if (loop != AST_NIL && is_loop(loop)) {
	// Replace until Mark removes assertion testing sideArrayInfo  != NULL
	//nextMesgSend->SetAssocLoop( (SDDF_SrcLoopInfo*)
	// thePabloGlobalInfo.currentLocals->
	// 				sideArray->getSDDFDescriptor(loop));
	
	PabloSideArrayInfo* sideArrayInfo = 
	    thePabloGlobalInfo.currentLocals->sideArray->getInfo(loop);
	if (sideArrayInfo != (PabloSideArrayInfo*) NULL)
	    if (sideArrayInfo->Size() > 0)
		nextMesgSend->SetAssocLoop((SDDF_SrcLoopInfo*)
					   sideArrayInfo->Get());
    }
    
    if (nextMesgRecv != (SDDF_MessageRecv*) NULL) {
	nextMesgSend->SetRecv(nextMesgRecv);
	nextMesgRecv->SetMessageSend(nextMesgSend);
	nextMesgRecv->SetPosition(lineNum, procInfo); // Could be moved down
    }						      // where nextMesgRecv
						      // is allocated.
    // Finally, clear the pointers holding this set of message records
    nextMesgSend = (SDDF_MessageSend*) NULL;
    nextMesgRecv = (SDDF_MessageRecv*) NULL;
    nextSendWait = (SDDF_MessageSendWait*) NULL;
    nextRecvWait = (SDDF_MessageRecvWait*) NULL;
}

//--------------------------------------------------------------------------
// Get the size, tag, pid, buffered params and array bounds
// Also get msgwait() for send if it exists
// To be called from compute_send_data()

void MessageInfo::GetMessageSendParams(Dist_Globals* dh,
				       Rsd_set* rset,
				       Mesg_data *mesgData,
				       Boolean buffered,
				       AST_INDEX sendStmt)
{
    if (! thePabloGlobalInfo.wantPabloInfo) return;
  
    assert (nextMesgSend == (SDDF_MessageSend*) NULL);
    nextMesgSend = new SDDF_MessageSend;
    nextMesgSend->SetBufferedFlag(buffered);
    
    //----- If asynchronous send, get sendWait record and put in pointers ----
    
    // AST_INDEX msgwaitForSend = AST_NIL;
    // SyncOrAsyncType msgSendType = SYNC_SEND;
    // if (dh->msg_id != AST_NIL) {	// Then it is an async send
	//Not supported yet.
	//msgwaitForSend = dh->send_sync;
	//nextMesgSend->SetSendWait(MakeSendWait(msgwaitForSend,nextMesgSend));
	//msgSendType = ASYNC_SEND;
    // }
    
    //----- Get symbolics for mesg size, type and pid -----
    
    // Use dh->bufsize, not dh->msgsize to get the message size.
    // dh->bufsize gives the AST of actual message, including all RSDs merged
    // in rset.  dh->msgsize only gives the size of the current RSD in rset.
    // dh->bufsize seems to be set even when buffering is not being used. (See
    // compute_send_data() in file msg2.ansi.c.)
    
    SDDF_Symbolic_Value* mesgSize = new SDDF_Symbolic_Value;
    AST_INDEX msgSizeAst = pt_simplify_expr(dh->bufsize);
    if (is_constant(msgSizeAst)) {
	int msgSize = atoi(gen_get_text(msgSizeAst));
	msgSize *= GetSizeofType(rset->sp->fform);
	mesgSize->SetSymbolicValue(msgSize);
    }
    else {
	msgSizeAst = gen_BINARY_TIMES(msgSizeAst,
				 pt_gen_int(GetSizeofType(rset->sp->fform)));
	InstrumentSymbolic(dh, msgSizeAst, sendStmt,
			   thePabloGlobalInfo.instr, mesgSize->GetId(),
			   /*addGuard*/ true, /*initialValue*/ 0);
    }
    
    // Get mesgType and mesgPid by parsing the send stmt. wotta kludge!
    SDDF_Symbolic_Value* mesgType = new SDDF_Symbolic_Value;
    SDDF_Symbolic_Value* mesgPid  = new SDDF_Symbolic_Value;
    putMesgTypeAndPid(dh, sendStmt, mesgType, mesgPid);
    
    nextMesgSend->SetMesgSize(mesgSize);
    nextMesgSend->SetMesgType(mesgType);
    nextMesgSend->SetMesgPid(mesgPid);
    
    //----- Get array name and lb,ub,step values from mesgData and rset -----
    PutRSDInfo(dh, sendStmt, nextMesgSend, rset, mesgData);
    
    //----- Enclosing Loop[] not yet implemented.
    
    //---- Finally, insert instrumentation for the send statement----
    InstrumentMesgCall(dh, sendStmt, thePabloGlobalInfo.instr,
		       nextMesgSend->GetId());
}
    

//--------------------------------------------------------------------------
void MessageInfo::GetMessageRecvParams(Dist_Globals* 	dh,
				       Rsd_set*     	/* rset */,
				       Mesg_data* 	/* mesgData */,
				       Boolean 	 	/* buffered */,
				       AST_INDEX	recvStmt)
{
    if (! thePabloGlobalInfo.wantPabloInfo) return;
  
    assert (nextMesgRecv == (SDDF_MessageRecv*) NULL);
    nextMesgRecv = new SDDF_MessageRecv;
    
    //---- Insert instrumentation for the recv statement----
    InstrumentMesgCall(dh, recvStmt, thePabloGlobalInfo.instr,
		       nextMesgRecv->GetId());
    return;
}

//--------------------------------------------------------------------------
// Get the message size and array bounds for a reduction
// To be called from check_reduction()

void MessageInfo::GetMessageSendParamsForReduc(Dist_Globals* dh,
					       Reduc_set* reducSet,
					       AST_INDEX reducStmt)
{
    if (! thePabloGlobalInfo.wantPabloInfo) return;
  
    //---- Set message oper type to reduction type and clear buffered flag ----
    assert (nextMesgSend == (SDDF_MessageSend*) NULL);
    nextMesgSend = new SDDF_MessageSend;

    if (reducSet->local) {
	// reduction is done on a single processor (relaxed owner computes)
	// broadcast send/recv is used to distribute answer to all processors
	// Have to generate the appropriate send and receive info accurately.
	return;
    }
    //else
    // reduction call reducStmt is directly inserted after reducSet->loop
    
    Message_Send_Oper_Type reducType;
    switch(reducSet->rtype) {
      case FD_REDUC_PLUS  : reducType = MESSAGESEND_GLOBAL_SUM ; break;
      case FD_REDUC_TIMES : reducType = MESSAGESEND_GLOBAL_PROD; break;
      case FD_REDUC_MIN   : reducType = MESSAGESEND_GLOBAL_MIN ; break;
      case FD_REDUC_MAX   : reducType = MESSAGESEND_GLOBAL_MAX ; break;
      case FD_REDUC_MINUS : 
      case FD_REDUC_DIV   : 
      case FD_REDUC_OR    : 
      case FD_REDUC_AND   : 
      case FD_REDUC_XOR   : 
      case FD_REDUC_MIN_LOC:
      case FD_REDUC_MAX_LOC:
      default              : reducType = MESSAGESEND_INVALID; break;
    }
    nextMesgSend->SetMesgOperType(reducType);
    nextMesgSend->SetBufferedFlag(false);
    
    //---- Set assoc loop info -----
    PabloSideArrayInfo* sideArrayInfo = 
	thePabloGlobalInfo.currentLocals->sideArray->getInfo(reducSet->loop);
    if (sideArrayInfo != (PabloSideArrayInfo*) NULL)
	if (sideArrayInfo->Size() > 0)
	  nextMesgSend->SetAssocLoop((SDDF_SrcLoopInfo*)sideArrayInfo->Get());
    
    //----- Set symbolic mesg size, variable name, and lb, ub, step (if any)
    // Much of this code is copied from make_reduc_call()
    
    int numElems;
    char* varName[MAX_NAME];
    SNODE* lhs_sp = findadd2(reducSet->lhs, 0, 0, dh);
    if (is_subscript(reducSet->lhs)) {
	// Retrieve ptr to SDDF_ArrayInfo for the lhs array
	assert(strlen(reducSet->lhs_name) > 0);	// Verify name has been stored
	SDDF_ArrayInfo* arrayDesc =
	  thePabloGlobalInfo.arrayInfo->GetArrayInfo(reducSet->lhs_name);
	
	// Add array descriptor pointer to mesgSend record and vice versa
	assert(arrayDesc != (SDDF_ArrayInfo*) NULL);
	nextMesgSend->AddArray(arrayDesc);
	arrayDesc->AddMessage(nextMesgSend);
	
	// Then find the number of data elements sent
	numElems = dc_array_size(lhs_sp);
    }
    else {				// Otherwise scalar value is sent
	numElems = 1;
    }
    
    // Store the mesg size in the appropriate symbolic record and set pointers
    // NOTE: make_reduc_call assumes a constant message size.
    SDDF_Symbolic_Value* mesgSize = new SDDF_Symbolic_Value;
    mesgSize->SetSymbolicValue(numElems * GetSizeofType(lhs_sp->fform));
    nextMesgSend->SetMesgSize(mesgSize);

    // Get lb, ub symbolics from SNODE* sp directly.
    SDDF_Symbolic_Value* symbBound;
    for (int dim = 0; dim < lhs_sp->numdim; dim++) {	// each array dimension
	symbBound = new SDDF_Symbolic_Value;
	symbBound->SetSymbolicValue(sp_get_lower(lhs_sp, dim));
	nextMesgSend->AddArrayLB(symbBound);
    }
    for (int dim = 0; dim < lhs_sp->numdim; dim++) {	// each array dimension
	symbBound = new SDDF_Symbolic_Value;
	symbBound->SetSymbolicValue(sp_get_upper(lhs_sp, dim));
	nextMesgSend->AddArrayUB(symbBound);
    }   
    for (int dim = 0; dim < lhs_sp->numdim; dim++) {	// each array dimension
	symbBound = new SDDF_Symbolic_Value;
	symbBound->SetSymbolicValue(1);		// step is always 1 it seems
	nextMesgSend->AddArrayStep(symbBound);
    }   
    
    //----- Enclosing Loop[] not yet implemented.

    //---- Get dependence information here instead of in GetMessageParams()---
    AST_INDEX srcRef, sinkRef;
    assert(is_identifier(reducSet->ref) || is_subscript(reducSet->ref));
    assert(is_identifier(reducSet->lhs) || is_subscript(reducSet->lhs));
    srcRef = (is_identifier(reducSet->ref))?
	reducSet->ref : gen_SUBSCRIPT_get_name(reducSet->ref);
    sinkRef = (is_identifier(reducSet->lhs))?
	reducSet->lhs : gen_SUBSCRIPT_get_name(reducSet->lhs);

    SDDF_DependInfo* dependInfo = MakeOneDependRecord(srcRef, sinkRef);
    nextMesgSend->AddDepend(dependInfo);

    //---- Finally, insert instrumentation for the reduction call ----
    InstrumentMesgCall(dh, reducStmt, thePabloGlobalInfo.instr,
		       nextMesgSend->GetId());
}

//--------------------------------------------------------------------------
// Store the "deepest" true dependence that defines the value read by
// this RHS reference.

void MessageInfo::StoreDepEdgeForRef(AST_INDEX rsideName,
				     DG_Edge* edge)
{
    if (! thePabloGlobalInfo.wantPabloInfo) return;
  
    // Check that rsideName is the name of a subscripted reference:
    AST_INDEX rhsRef = tree_out(rsideName);
    assert(is_subscript(rhsRef) && rsideName== gen_SUBSCRIPT_get_name(rhsRef));

    PabloSideArrayInfo* SAInfo =
	thePabloGlobalInfo.currentLocals->sideArray->getInfo(rsideName);
    
    if (SAInfo) {
	// Check: only a single comm-causing dep-edge will be stored there
	assert(SAInfo->getDGEdge() == (DG_Edge*) NULL);
	SAInfo->putDGEdge(edge);
    }
}


//**************************************************************************
// Helper functions for class MessageInfo
//**************************************************************************

// Create the depend record(s) for this message and insert the pointers
// from the Mesg Send record to these depend record(s).

static void MakeDependRecords(SDDF_MessageSend* mesgSend, Rsd_set* rset)
{
    assert(mesgSend != (SDDF_MessageSend*) NULL);
    assert(rset != (Rsd_set*) NULL);
    
    SDDF_DependInfo* dependInfo;
    DG_Edge* DGedge;
    AST_INDEX srcRef, sinkRef;
    Rsd_set* nextRset;
    PabloSideArrayInfo* SAInfo;
    SDDF_SideArray *SA = thePabloGlobalInfo.currentLocals->sideArray;

    // For all the RSD sections merged in this RSD (No case with > 1 found yet)
    for (nextRset = rset; nextRset != NULL; nextRset = nextRset->rsd_merge) {
	
	// For all the non-local RHS refs included in this RSD section:
	for (int i = nextRset->num_subs - 1; i >= 0; i--) {
	    
	    // Get side-array info for RHS ref. that causes this RSD member
	    assert(is_subscript(nextRset->subs[i]));
	    sinkRef = gen_SUBSCRIPT_get_name(nextRset->subs[i]);
	    SAInfo = SA->getInfo(sinkRef);
	    assert(SAInfo);

	    // Get the dependence edge causing this message.
	    // This was saved in the side-array info struct for the sink ref
	    // (see rsd_get_level() in rsd_sec.ansi.c). However, for some RSD
	    // sections (e.g., on a broadcast), the dep. edge does not seem to
	    // be used.  In such cases, the source reference cannot be found
	    // so those fields in the Depend record will remain undefined.
	
	    DGedge = SAInfo->getDGEdge();	 // Check that the DG edge:
	    if (DGedge != (DG_Edge*) NULL) {	 // (1) has been saved
		assert(DGedge->sink == sinkRef); // (2) sinks at this reference
		assert(DGedge->type == dg_true); // (3) is a true dep.
		srcRef = DGedge->src;
	    }
	    else 
		srcRef = AST_NIL;
	    dependInfo = MakeOneDependRecord(srcRef, sinkRef);
	    mesgSend->AddDepend(dependInfo);
	}
    }
}

// Make one depend record with given src, sink references
// NOTE: dependence type is always "true" and is not printed.
//       We should replace this field in the SDDF file.

static SDDF_DependInfo*
MakeOneDependRecord(AST_INDEX srcRef, AST_INDEX sinkRef) 
{
    assert(sinkRef != AST_NIL);		// Sink ref always available, not src
    
    SDDF_DependInfo* dependInfo;
    SDDF_SideArray *SA = thePabloGlobalInfo.currentLocals->sideArray;
    
    // The info for each depend record.
    SDDF_ProcInfo* procInfo = thePabloGlobalInfo.currentLocals->procInfo;
    int srcLineNum=-1, sinkLineNum=-1;
    int srcCharPos=-1, sinkCharPos=-1;
    
    if (srcRef != AST_NIL) {
	// Get the position information for source ref of the DG edge
	srcLineNum = SA->getInfo(srcRef)->GetStartLine();
	srcCharPos = SA->getInfo(srcRef)->GetStartChar();
    }
    
    // Get the position information for sink ref
    sinkLineNum = SA->getInfo(sinkRef)->GetStartLine();
    sinkCharPos = SA->getInfo(sinkRef)->GetStartChar();
	
    // Allocate a depend info record and store the fields.
    // Do it all in one place here, even though src fields may be undef.
    dependInfo = new SDDF_DependInfo;
    dependInfo->SetPosition(sinkLineNum, procInfo);
    dependInfo->SetSinkPos(sinkLineNum, sinkCharPos);
    dependInfo->SetSrcPos(srcLineNum, srcCharPos);
    
    return dependInfo;
}

//--------------------------------------------------------------------------
// Get array name and lb,ub,step values from mesgData and rset
// Logic is basically copied from make_send_str

static void PutRSDInfo(Dist_Globals* dh,
		       AST_INDEX sendStmt,
		       SDDF_MessageSend* mesgSend,
		       Rsd_set* rset,
		       Mesg_data *mesgData)
{
    assert(rset->sp->fform1 == ARRAYTYPE);
    SDDF_ArrayInfo* arrayDesc;
    SDDF_Symbolic_Value *lb, *ub, *step;
    int blocksize, start, end;
    Rsd_set* nextRset = rset;
    AST_INDEX lbExpr, ubExpr;
    
    for (int i= nextRset->num_merge; i>=0; i--, nextRset= nextRset->rsd_merge)
    {
      // get array sddf descriptor by looking up array name for each RSD
      arrayDesc= thePabloGlobalInfo.arrayInfo->GetArrayInfo(nextRset->sp->id);
      assert(arrayDesc != (SDDF_ArrayInfo*) NULL);
    
      // Add array descriptor pointer to mesgSend record and vice versa
      mesgSend->AddArray(arrayDesc);
      arrayDesc->AddMessage(mesgSend);
    }

    // NOTE: make_send_str() assumes that lb,ub,step is the same for every
    //       array combined in this message, i.e., every rset in Rsd_set* rset.
    
    // Get lb, ub symbolics from mesgData directly.
    // Logic copied from make_send_str()
    
    for (int i = 0; i < mesgData->numdim; i++) {
	lb = new SDDF_Symbolic_Value;
	ub = new SDDF_Symbolic_Value;
	step = new SDDF_Symbolic_Value;
	step->SetSymbolicValue(1);	// step always seems to be 1

	lbExpr = ubExpr = AST_NIL;

	if (mesgData->sym[i]) {    // if symbolic, print values at runtime
	    lbExpr = mesgData->sym[i];
	    ubExpr = (mesgData->extent[i] != AST_NIL)? mesgData->extent[i]
						     : mesgData->sym[i];
	}
	else {	/* not completely symbolic, calculate range */
	    switch (sp_is_part(rset->sp, i)) {
	      case FD_DIST_BLOCK:
		blocksize = sp_block_size1(rset->sp, i);
		start = abs(abs(mesgData->lower[i]) -
			    (abs(mesgData->offset[i]) - 1) * blocksize);
		end   = abs(abs(mesgData->upper[i]) -
			    (abs(mesgData->offset[i]) - 1) * blocksize);
		break;
	      default:
		start = mesgData->lower[i];
		end = mesgData->upper[i];
		break;
	    }
	    
	    if (start != MININT) lb->SetSymbolicValue(start);
	    else lbExpr = mesgData->sym_lower[i];
		
	    if (end  != MAXINT) ub->SetSymbolicValue(end);
	    else ubExpr = mesgData->sym_upper[i];
	}
	
	SPMDInstrumentation* instr = thePabloGlobalInfo.instr;
	if (instr->GetInstrOpts()->InstrFullSymbolics()) {
	    // Print lb, ub at runtime if not const.
	    // Add guard to test if value has changed since last print.
	    // (Actually, should not add guard to innermost loop index, but
	    // thats a refinement we can defer.  Printing all these runtime
	    // values is far too much overhead anyway.)
	    if (lbExpr != AST_NIL)
		InstrumentSymbolic(dh, lbExpr, sendStmt, instr,
				   lb->GetId(), true, /*initialValue*/ MAXINT);
	    if (ubExpr != AST_NIL)
		InstrumentSymbolic(dh, ubExpr, sendStmt, instr,
				   ub->GetId(), true,
				   /*initialValue*/ -(MAXINT-1)); //MININT is
	}    						//exported as -1
	
	mesgSend->AddArrayLB(lb);
	mesgSend->AddArrayUB(ub);
	mesgSend->AddArrayStep(step);
    }
}

//--------------------------------------------------------------------------
// Parse send stmt to get mesg type and mesg pid. Ugh!

static void putMesgTypeAndPid(Dist_Globals* dh,
			      AST_INDEX sendStmt,
			      SDDF_Symbolic_Value* mesgType,
			      SDDF_Symbolic_Value* mesgPid)
{
    AST_INDEX invocation;
    if (is_call(sendStmt))
	invocation = gen_CALL_get_invocation(sendStmt);
    else if (is_assignment(sendStmt)) {
	invocation = gen_ASSIGNMENT_get_rvalue(sendStmt);
	if (! is_invocation(invocation))
	    die_with_message("1. Expected this to be a send invocation!");
    }
    else
	die_with_message("2. Expected this to be a send invocation!");

    // Now, <invocation> contains the AST_INDEX of the function call
    // ASSUME: send stmt call arguments are: send(tag, ... , pid)
    AST_INDEX argList = gen_INVOCATION_get_actual_arg_LIST(invocation);

    //-------------------------------------------------------------
    // WARNING: This 2 statements ASSUME this send call is for the
    //		(iPSC) NX message passing library.
    //-------------------------------------------------------------
    AST_INDEX typeExpr = list_first(argList);
    AST_INDEX pidExpr  = list_last(argList);
    
    // If type is a constant print its symbolic record, else print at runtime
    if (is_constant(typeExpr))
	mesgType->SetSymbolicValue(gen_CONSTANT_get_value(typeExpr));
    else
	InstrumentSymbolic(dh, typeExpr, sendStmt, thePabloGlobalInfo.instr,
			   mesgType->GetId(), /*addGuard*/ true, 0);

    // If pid is a constant print its symbolic record, else print at runtime
    if (is_constant(pidExpr))
	mesgPid->SetSymbolicValue(gen_CONSTANT_get_value(pidExpr));
    else
	InstrumentSymbolic(dh, pidExpr, sendStmt, thePabloGlobalInfo.instr,
			   mesgPid->GetId(), /*addGuard*/ true, -1);
}

//--------------------------------------------------------------------------

void InstrumentMesgCall(Dist_Globals* dh,
			AST_INDEX stmt,
			SPMDInstrumentation* instr,
			Static_Id staticId)
{
    assert(in_list(stmt));
    if (instr->GetInstrOpts()->InstrMessages()) {
	AST_INDEX invocation = AST_NIL;
	if (is_call(stmt))
	    invocation = gen_CALL_get_invocation(stmt);
	else if (is_assignment(stmt))
	    invocation = gen_ASSIGNMENT_get_rvalue(stmt);
	assert(is_invocation(invocation)); // Must be an assignment or call
	(void) instr->instrumentMesgCall(dh, invocation, stmt, staticId);
    }
}

//--------------------------------------------------------------------------

static void InstrumentSymbolic(Dist_Globals* dh,
			       AST_INDEX intExpr,
			       AST_INDEX stmt,
			       SPMDInstrumentation* instr,
			       Static_Id staticId,
			       Boolean	 addGuard,
			       int initialValue)
{
    assert(in_list(stmt));
    if (instr->GetInstrOpts()->InstrMessages())
	(void) instr->PrintSymbolic(dh, intExpr, stmt,
				    (StaticInfo) staticId,
				    addGuard, initialValue);
}
//--------------------------------------------------------------------------
