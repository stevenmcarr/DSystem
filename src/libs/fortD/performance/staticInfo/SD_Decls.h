/* $Id: SD_Decls.h,v 1.1 1997/03/11 14:29:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* Dummy Declarations of classes to minimize include volumeu */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_Decls.h,v 1.1 1997/03/11 14:29:01 carr Exp $
//
*/
// The tag should exactly match the filename
#ifndef _SD_Decls_h
#define _SD_Decls_h

// From file SD_Base.h
class StaticDescriptorBase;
class SDDF_Symbolic_Value;
// From file SD_DataInfo.h
class SDDF_DecompInfo;
class SDDF_AlignInfo;
class SDDF_DistInfo;
class SDDF_ArrayInfo;
// From file SD_SrcInfo.h
class SDDF_ProcInfo;
class SDDF_SrcLoopInfo;
class SDDF_DependInfo;
class SDDF_SPMD_LoopInfo;
// From file SD_MsgInfo.h
class SDDF_MessageSend;
class SDDF_MessageRecv;
class SDDF_MessageSendWait;
class SDDF_MessageRecvWait;


enum SDDF_RECORD_TYPE { SDDF_UNUSED, SDDF_DECOMPINFO,  SDDF_ALIGNINFO,  SDDF_DISTINFO,
 SDDF_ARRAYINFO,  SDDF_PROCINFO, SDDF_SRCLOOPINFO, SDDF_DEPENDINFO,
 SDDF_SPMD_LOOPINFO, SDDF_MESSAGESEND, SDDF_MESSAGERECV, SDDF_MESSAGESENDWAIT,
 SDDF_MESSAGERECVWAIT, SDDF_SYMBOLIC_VALUE};

// Don't forget terminal semicolon on classes!
#endif
