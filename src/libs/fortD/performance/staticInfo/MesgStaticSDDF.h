/* $Id: MesgStaticSDDF.h,v 1.1 1997/03/11 14:28:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: MesgStaticSDDF.h,v 1.1 1997/03/11 14:28:55 carr Exp $ -*-c++-*-
 ***************************************************************************
 * Declarations and C wrappers for class MesgStaticSDDF
 **************************************************************************/

#ifndef FD_CODEGEN
#include <libs/fortD/codeGen/private_dc.h>
#endif
#include <libs/fortD/performance/staticInfo/SDDF_Instrumentation.h>
#include <libs/fortD/performance/staticInfo/SDDF_General.h>
#include <libs/fortD/performance/staticInfo/SD_MsgInfo.h>

/*
   For i = 1..numdim:
     dimension i is of type range if lb[i] != ub[i]
     sym[i]		is the symbolic lower bound in dim i if not type RANGE
     sym_lower[i]	is the symbolic lower bound in dim i if type RANGE
     extent[i]		is the symbolic upper bound in dim i
     lower[i]		is the numeric lower bound in dim i if !sym[i]
     			  and array is not  FD_DIST_BLOCK
     lower[i]+(offset[i] * blocksize) is the numeric lower bound in dim i
     				      if !sym[i] and the array is FD_DIST_BLOCK
 */
typedef struct        /* structure used to summarize RSDs */
{		      /* Coped here from msg2.ansi.c */
  int numdim;
  int msize;                /* if msize == 0, symbolic message size */
  AST_INDEX size;           /* AST for message size if symbolic    */
  int lower[DC_MAXDIM];
  int upper[DC_MAXDIM];
  int offset[DC_MAXDIM];
  AST_INDEX sym[DC_MAXDIM];      /* if dimension is symbolic        */
  AST_INDEX extent[DC_MAXDIM];   /* if symbolic dimension has range */
  AST_INDEX sym_lower[DC_MAXDIM];   /* if symb. lower bound && dim has range */
  AST_INDEX sym_upper[DC_MAXDIM];   /* if symb. upper bound && dim has range */
  int *bsize;                       /* pointer to size of buffer    */
} Mesg_data;


/*--------------------------------------------------------------------------*
 * C wrappers.
 */

/* Call this routine for each communication inserted, after send and receive
 * stmts and corresponding msgwaits have all been inserted
 */
EXTERN(void,
       GetMessageParams,(Dist_Globals *dh,
			 AST_INDEX loop,	/* associated loop */
			 Rsd_set *rset));	/* RSD(s) sent in msg */

/* Call this for each communication inserted (except reductions),
 * after send data is computed
 */
EXTERN(void,
       GetMessageSendParams,(Dist_Globals *dh,
			     Rsd_set* rset,	  /* RSD(s) sent in msg */
			     Mesg_data *mesgData, /* summary of data sent */
			     Boolean buffered,	  /* whether buffered in pgm */
			     AST_INDEX sendStmt));/* the actual send stmt */
			     
/* Call this for each communication that requires a receive operation,
 * after recv data is computed. E.g, a reduction doesn't require a receive.
 */
EXTERN(void,
       GetMessageRecvParams,(Dist_Globals *dh,
			     Rsd_set* rset,	  /* RSD(s) sent in msg */
			     Mesg_data *mesgData, /* summary of data sent */
			     Boolean buffered,	  /* whether buffered in pgm */
			     AST_INDEX recvStmt));/* the actual recv stmt */

/* Call this for each reduction communication, after send data is computed.
 */
EXTERN(void,
   GetMessageSendParamsForReduc,(Dist_Globals* dh,
				 Reduc_set* reducSet,  /* info re data sent */
				 AST_INDEX reducStmt));/* reduction call */

/* Call this for each RSH reference that will generate communication.
 * rsideName is the name of the array referenced.  edge is the "last" true
 * dep edge, i.e., the message would be inserted after the source of this edge.
 */
EXTERN(void,
       SDDF_StoreDepEdgeForRef,	(AST_INDEX rsideName,/* nameof subscript ref*/
				 DG_Edge* edge));    /* dep-edge causing comm*/


/*--------------------------------------------------------------------------*
 * Rest is the declarations of the classes. actually used only internally,
 * invoked via the C wrappers above. See above for explanation of arguments.
 */
 
#ifdef __cplusplus

class MessageInfo {				// A permanent data structure
  public:
    MessageInfo();
    ~MessageInfo();
    
    void GetMessageParams(Dist_Globals *dh,
			  AST_INDEX loop,
			  Rsd_set *rset);
    
    void GetMessageSendParams(Dist_Globals *dh,
			      Rsd_set* rset,
			      Mesg_data *mesgData,
			      Boolean buffered,
			      AST_INDEX sendStmt);
    
    void GetMessageRecvParams(Dist_Globals *dh,
			      Rsd_set* rset,
			      Mesg_data *mesgData,
			      Boolean buffered,
			      AST_INDEX recvStmt);

    void GetMessageSendParamsForReduc(Dist_Globals* dh,
				      Reduc_set* reducSet,
				      AST_INDEX reducStmt);
    
    void StoreDepEdgeForRef(AST_INDEX rsideName,
			    DG_Edge* edge);
  
  private:
    SDDF_MessageSend*	  nextMesgSend;
    SDDF_MessageRecv*	  nextMesgRecv;
    SDDF_MessageSendWait* nextSendWait;
    SDDF_MessageRecvWait* nextRecvWait;
};

#endif 	/* __cplusplus */

/*--------------------------------------------------------------------------*/
