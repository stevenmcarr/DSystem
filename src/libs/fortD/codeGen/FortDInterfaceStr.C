/* $Id: FortDInterfaceStr.C,v 1.9 2001/09/14 18:22:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>

#include <libs/fortD/codeGen/private_dc.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/fd_string.h>
#include <libs/fortD/codeGen/FortDInterface.h>
#include <libs/fortD/codeGen/FortDInterface.i>
#include <libs/support/strings/StringBuffer.h>
#undef is_open

#ifdef LINUX
#include <stream.h>
#else
#include <sys/stream.h>
#endif

#include <libs/frontEnd/ast/ast_include_all.h>
#include <libs/support/database/context.h>

EXTERN(void, init_msg_info, (Dist_Globals*));

EXTERN(void, compute_proc_offsets, (Dist_Globals*, 
                                    Rsd_set*, Rsd_section*));

EXTERN(void, compute_procs, (Dist_Globals*, int numdim));

EXTERN(void,dc_compute_proc_range,
                            (PedInfo, int*, int*, Rsd_set*, Iter_type));

EXTERN(void, ast_to_str,   (AST_INDEX, char*));

EXTERN(int, dc_array_size, (SNODE*));

//  used for computing the processor performing a broadcast 
EXTERN(AST_INDEX, make_sendproc, (Dist_Globals*, Rsd_set*));

struct Mesg_data        /* structure used to summarize RSDs */
{
  int numdim;
  int msize;                /* if msize = 0, symbolic message size */
  AST_INDEX size;           /* AST for message size if symbolic    */
  int lower[DC_MAXDIM];
  int upper[DC_MAXDIM];
  int offset[DC_MAXDIM];
  AST_INDEX sym[DC_MAXDIM];      /* if dimension is symbolic        */
  AST_INDEX extent[DC_MAXDIM];   /* if symbolic dimension has range */
  AST_INDEX sym_lower[DC_MAXDIM];   /* if symbolic lower bound      */
  AST_INDEX sym_upper[DC_MAXDIM];   /* if symbolic upper bound      */
};

//------------------------------------------------------
//------------ base class  ----------
//------------------------------------------------------
class CommInfo
{
 public:
 virtual StringBuffer* CommInfoStr(Rsd_set* r, Mesg_data* m)
 {  return 0; };
 virtual void PreProcess(void* rs_ps) { };
};

//------------------------------------------------------
//------------derived classes ----------
//------------------------------------------------------

//------------------------------------------------------
//----- message size information--------
//------------------------------------------------------
class CommMesgSize: public CommInfo
{ 
 public:
 virtual StringBuffer* CommInfoStr(Rsd_set*, Mesg_data*);

};

//------------------------------------------------------
//---------send section information------
//------------------------------------------------------
class CommSendSection: public CommInfo
{
 Comm_type c;
 public:
 CommSendSection(Comm_type cc) { c = cc; }
 
 virtual StringBuffer* CommInfoStr(Rsd_set*, Mesg_data*);
 virtual void PreProcess(void* rs_ps);
};

//------------------------------------------------------
//--------send processor range information----
//------------------------------------------------------
class CommSendProcRange: public CommInfo
{
 int *pmin, *pmax;
 Comm_type c;
 public:

 virtual StringBuffer* CommInfoStr(Rsd_set*, Mesg_data*);
 CommSendProcRange(int* min, int *max,  Comm_type cc) { 
 pmin = new int [10];
 pmax = new int [10];
 c = cc;
 
 for(int i = 0; i< 10; ++i)
 { 
 pmin[i] = min[i];
 pmax[i] = max[i];
 }
}

virtual ~CommSendProcRange()
 {
  delete pmin;
  delete pmax;
 };
};

//------------------------------------------------------
//-------recv section information----------------------
//------------------------------------------------------
class CommRecvSection: public CommInfo
{
 Comm_type c;
 public:
 CommRecvSection(Comm_type cc) { c = cc; }
 virtual void PreProcess(void* rs_ps);
 virtual StringBuffer* CommInfoStr(Rsd_set*, Mesg_data*);
};

//------------------------------------------------------
//-------recv processor range information---------------
//------------------------------------------------------
class CommRecvProcRange: public CommInfo
{
 public:
 virtual StringBuffer* CommInfoStr(Rsd_set*, Mesg_data*);
 int *pmin, *pmax;
 Comm_type c;
 public:

 CommRecvProcRange(int* min, int *max, Comm_type cc) { 
 pmin = new int [10];
 pmax = new int [10];
 c = cc;

 for(int i = 0; i< 10; ++i)
 { 
 pmin[i] = min[i];
 pmax[i] = max[i];
 }
}

virtual ~CommRecvProcRange()
 {
  delete pmin;
  delete pmax;
 };
};


struct Rsd_Info
{
 struct RSD_PARTS rs_parts[MAX_PROC][DC_MAXDIM];
};


/*  This is in stdlib.h
static int abs(int x)
{
 if (x < 0)
 return -x;

return x;
}
*/

//---------------------------------------------------------------------
//   return the number of processors involved in a message
//---------------------------------------------------------------------

static int 
dc_max_procs(struct RSD_PARTS rs_parts[MAX_PROC][DC_MAXDIM])
{
  int  maxproc, i;

  maxproc = rs_parts[0][0].numprocs;

  for (i = 1; i < DC_MAXDIM; ++i)
  {
    if (maxproc < rs_parts[0][i].numprocs)
      maxproc = rs_parts[0][i].numprocs;
  }
  return (maxproc + 1);
}



//------------------------------------------------------------
// Broadcast requires that total procs be set to 1 before
// computing the send section
//------------------------------------------------------------
void CommSendSection::PreProcess(void* rs_ps)
{
 if (c == FD_COMM_BCAST)
  {
   Rsd_Info *r = (Rsd_Info*) rs_ps;
   r->rs_parts[0][0].total_procs = 1;
  }
}

//------------------------------------------------------------
// Broadcast requires that total procs be set to 1 before
// computing the recv section
//------------------------------------------------------------
void CommRecvSection::PreProcess(void* rs_ps)
{
 if (c == FD_COMM_BCAST)
  {
   Rsd_Info *r = (Rsd_Info*) rs_ps;
   r->rs_parts[0][0].total_procs = 1;
  }
}


//-----------------------------------------------------------------------
// return the section of data involved in a send
//-----------------------------------------------------------------------
StringBuffer*
CommSendSection::CommInfoStr(Rsd_set *rset, struct Mesg_data *m)
{
  int i, blocksize, start, end, len;
  char section[MAXCOMMENT], buf[MAXCOMMENT], ivar[MAXCOMMENT];
  StringBuffer *s;
  s  = new StringBuffer(60);

  /*--------------------------------*/
  /* build 1 array section w/o name */

  /* go through each dimension of array and compute range */
  strcpy(section, "(");
  for (i = 0; i < rset->sp->numdim; i++)
  {
    if (i)
    strcat(section, ", ");

    if (m->sym[i])    /* if symbolic */
    {
      ast_to_str(m->sym[i],section);
      if (m->extent[i])
      {
        strcat(section, ":");
        ast_to_str(m->extent[i],section);
      }
    }
    else  /* not completely symbolic, calculate range */
    {
      switch (sp_is_part(rset->sp, i))
      {
        case FD_DIST_BLOCK:
          blocksize = sp_block_size1(rset->sp, i);
          start = abs(abs(m->lower[i]) - (abs(m->offset[i]) - 1) * blocksize);
          end = abs(abs(m->upper[i]) - (abs(m->offset[i]) - 1) * blocksize);
          break;

        default:
          start = m->lower[i];
          end = m->upper[i];
          break;
      }

      if (start == end)
      {
        sprintf(buf, "%d", start);
      }
      else
      {
        buf[0] = '\0';
        if (start == MININT)
          ast_to_str(m->sym_lower[i], buf);
        else
          sprintf(buf, "%d", start);
        strcat(buf,":");
        if (end == MAXINT)
          ast_to_str(m->sym_upper[i], buf);
        else
          sprintf(buf + strlen(buf), "%d", end);
      }

      strcat(section, buf);
    }
  }
 strcat(section, ")");

  /*------------------------------------*/
  /* build list of named array sections */

    sprintf(buf, "--<< Send ");

  for (i = rset->num_merge; i >= 0; i--, rset = rset->rsd_merge)
  {
    strcat(buf, rset->sp->id);
    strcat(buf, section);
    if (i)
      strcat(buf, ", ");
  }
  strcat(buf, " >>--");
  s->Append("%s", buf);
 return s;
}

//-----------------------------------------------------------------------
// return the section of data involved in a receive
//-----------------------------------------------------------------------

StringBuffer* 
CommRecvSection::CommInfoStr(Rsd_set *rset, struct Mesg_data *m)
{
  StringBuffer *s;  
  int i, len;
  char section[MAXCOMMENT], buf[MAXCOMMENT], ivar[MAXCOMMENT];

  s = new StringBuffer(60);

  /*--------------------------------*/
  /* build 1 array section w/o name */

  /* go through each dimension of array and compute range */
  strcpy(section, "(");
  for (i = 0; i < rset->sp->numdim; i++)
  {
    if (i)
      strcat(section, ", ");

    if (m->sym[i])    /* if symbolic */
    {
      ast_to_str(m->sym[i],section);
      if (m->extent[i])
      {
        strcat(section, ":");
        ast_to_str(m->extent[i],section);
      }
    }

    else    /* not completely symbolic, calculate range */
    {
      if (m->lower[i] == m->upper[i])
      {
        sprintf(buf, "%d", m->lower[i]);
      }
      else
      {
        buf[0] = '\0';
        if (m->lower[i] == MININT)
          ast_to_str(m->sym_lower[i], buf);
        else
          sprintf(buf, "%d", m->lower[i]);
        strcat(buf,":");
        if (m->upper[i] == MAXINT)
          ast_to_str(m->sym_upper[i], buf);
        else
          sprintf(buf + strlen(buf), "%d", m->upper[i]);
      }
      strcat(section, buf);
    }

  }
  strcat(section, ")");

  /*------------------------------------*/
  /* build list of named array sections */

  sprintf(buf, "--<< Recv ");

  for (i = rset->num_merge; i >= 0; i--, rset = rset->rsd_merge)
  {
    strcat(buf, rset->sp->id);
    strcat(buf, section);
    if (i)
      strcat(buf, ", ");
  }

  strcat(buf, " >>--");
  
  s->Append("%s", buf);
  return s;
}




//-----------------------------------------------------------------------
// Calculate message size for one RSD
//  Based on values in m->upper & m->lower
//  Assumption - message exists with at least size 1
//-----------------------------------------------------------------------

StringBuffer*
CommMesgSize::CommInfoStr(Rsd_set *rset, struct Mesg_data *m)
{
  int i, msize, idx;
  char* mult[DC_MAXDIM];
  char* buf[10];
  char  size_str[200];

  StringBuffer *s;
  s  = new StringBuffer(60);

  for(i=0;i<10;++i)
  buf[i] = new char [100];

  idx = 0;
  m->msize = 1;

  for (i = 0; i < m->numdim; ++i)
  {
    /* if NOT symbolic AND involved in message */

    if (!m->sym[i] && (m->upper[i] || m->lower[i]))
    {
      if (m->upper[i] == MAXINT)
      {
        strcat(buf[i], "(");
        ast_to_str(m->sym_upper[i], buf[i]);
        strcat(buf[i], "(");

        if (m->lower[i] == MININT)
        {
         strcat(buf[i], " - (");
         ast_to_str(m->sym_lower[i], buf[i]);
         strcat(buf[i], ")");
         strcat(buf[i], "+ 1");
        }
        else if (m->lower[i] != 1)
        {
         sprintf(buf[i], "%d", m->lower[i]-1);
        }
        mult[idx++] = buf[i];
      }
      else if (m->lower[i] == MININT)
      {
        strcat(buf[i], "("); 
        ast_to_str(m->sym_lower[i], buf[i]);
        strcat(buf[i], "(");
        sprintf(buf[i], "%d", m->upper[i]+1);
        mult[idx++] = buf[i];
      }
      else
      {
        m->msize *= m->upper[i] - m->lower[i] + 1;
      }
    }
	}

    if (!idx)  /* constant size message */
      sprintf(size_str, "%d", m->msize);

    else      /* variable size message */
    {
      strcat(size_str, mult[0]);
      for (i = 1; i < idx; i++)
			{
       strcat(size_str,"*");
       strcat(size_str, mult[i]);
  		}

      if (m->msize != 1)
       {
        strcat(size_str, " * ");
        sprintf(size_str, "%d", m->size);
      }
  }
 s->Append("%s", size_str);
 return s;
}

//------------------------------------------------------------
// return the sending processor range for a single rsd
//------------------------------------------------------------
StringBuffer* 
CommRecvProcRange::CommInfoStr(Rsd_set *rset, struct Mesg_data *m)
{
 SNODE *sp;
 int i, min, max;
 StringBuffer* s;
 
 s = new StringBuffer(100);
 sp = rset->sp;


  if(sp->perfect_align)
   sp = sp->decomp;
  
 for(i = 0; i < sp->numdim; ++i)
  {
   if (m->offset[i])
    {
    if(m->offset[i] < 0)
     {
     min = pmin[i] - 1 - m->offset[i];
     max = pmax[i] - 1 ;
     }
    else
     {
      min = pmin[i] - 1 ;
      max = pmax[i] - 1 - m->offset[i];
     }
     s->Append("%s %c %d %s %d %c %s ", "--<< Recv Processor Range: ",'(', min, ":", max, ')' ," >>--");
    }
 }
 return s;
}

//------------------------------------------------------------
// return receiving processor range for a single rsd 
//------------------------------------------------------------
StringBuffer* 
CommSendProcRange::CommInfoStr(Rsd_set *rset, struct Mesg_data *m)
{
 SNODE *sp;
 int i, min, max;
 StringBuffer *s;

 s = new StringBuffer(100);


 sp = rset->sp;
 
  if (sp->perfect_align)
   sp = sp->decomp;

  for (i = 0; i < sp->numdim; i++)
  {
   if ((sp_is_part(sp, i) != FD_DIST_LOCAL) && m->offset[i])
   {
    if(m->offset[i] < 0)
     {
     min = pmin[i] - 1 - m->offset[i];
     max = pmax[i] - 1 ;
     }
    else
     {
      min = pmin[i] - 1 ;
      max = pmax[i] - 1 - m->offset[i];
     }
    s->Append("%s %c %d %s %d %c %s ", "--<< Send Processor Range: ", '(', min, ":", max, ')' , " >>--"); 
   }
 }
  return s;
}

//---------------------------------------------------------------------
// makes the send string
//---------------------------------------------------------------------
StringBuffer* FortDRsd::SendStr(void* rs_info, void* info_string)
{
  int i, j, k,total_procs;
  int maxproc = 0;
  int proc_num= 0;
  struct Mesg_data mdata, *m;
  Rsd_set *rset = (Rsd_set*)GetRSet();
  Rsd_Info *r = (Rsd_Info*) rs_info;
  StringBuffer *s;
  CommInfo *info_s;
  
  info_s = (CommInfo*) info_string;

  info_s->PreProcess(rs_info);

  s = new StringBuffer(100);
  m = &mdata;
  m->numdim = rset->sp->numdim;
  maxproc = dc_max_procs(r->rs_parts);
  total_procs = r->rs_parts[0][0].total_procs;

  for (i = 0; i < maxproc; ++i)
  {
    m->offset[0] =  r->rs_parts[0][0].inv_proc_offset[i];
    m->upper[0] =  r->rs_parts[0][0].inv_upper[i];
    m->lower[0] =  r->rs_parts[0][0].inv_lower[i];
    m->sym[0] =  r->rs_parts[0][0].sym[0];
    m->extent[0] =  r->rs_parts[0][0].extent[0];
    m->sym_upper[0] =  r->rs_parts[0][0].sym_upper[0];
    m->sym_lower[0] =  r->rs_parts[0][0].sym_lower[0];

    for (j = 0; j < maxproc; ++j)
    {
      m->offset[1] =  r->rs_parts[0][1].inv_proc_offset[j];
      m->upper[1] =  r->rs_parts[0][1].inv_upper[j];
      m->lower[1] =  r->rs_parts[0][1].inv_lower[j];
      m->sym[1] =  r->rs_parts[0][1].sym[0];
      m->extent[1] =  r->rs_parts[0][1].extent[0];
      m->sym_upper[1] =  r->rs_parts[0][1].sym_upper[0];
      m->sym_lower[1] =  r->rs_parts[0][1].sym_lower[0];

      for (k = 0; k < maxproc; ++k)
      {
        m->offset[2] =  r->rs_parts[0][2].inv_proc_offset[k]; 
        m->upper[2] =  r->rs_parts[0][2].inv_upper[k]; 
        m->lower[2] =  r->rs_parts[0][2].inv_lower[k];
        m->sym[2] =  r->rs_parts[0][2].sym[0];
        m->extent[2] =  r->rs_parts[0][2].extent[0];
        m->sym_upper[2] =  r->rs_parts[0][2].sym_upper[0];
        m->sym_lower[2] =  r->rs_parts[0][2].sym_lower[0];

        /* insert more loops here to handle additional array dimensions */

        /*---        
        for(l=0;l< maxproc;++l){ m.offset[3] =
         r->rs_parts[0][3].inv_proc_offset[l]; m.upper[3] =
         r->rs_parts[0][3].inv_upper[l]; m.lower[3] =
         r->rs_parts[0][3].inv_lower[l];
        ---*/
        
        if (proc_num < total_procs)
        {
          ++proc_num;
           s->Append(info_s->CommInfoStr(rset, m));

        }
			}
		}
   }
 return s;
}


//---------------------------------------------------------------------
//   make the receive string
//---------------------------------------------------------------------

StringBuffer*
FortDRsd::RecvStr(void* rs_ps, void* info_string)
{
  int i, j, k, proc, proc_num, total_procs, maxproc;
  struct Mesg_data mdata, *m;
  StringBuffer *s;
  Rsd_set *rset = (Rsd_set*)GetRSet();
  Rsd_Info *r = (Rsd_Info*) rs_ps;
  CommInfo *info_s;

  
  s = new StringBuffer(100);
  m = &mdata;
  m->numdim = rset->sp->numdim;
  maxproc = dc_max_procs(r->rs_parts);
  proc_num = 0;

  info_s = (CommInfo*) info_string;

  info_s->PreProcess(rs_ps);
  total_procs = r->rs_parts[0][0].total_procs;


  for (i = 0; i < maxproc; ++i)
  {
    m->offset[0] =  r->rs_parts[0][0].proc_offset[i];
    m->upper[0] =  r->rs_parts[0][0].upper[i];
    m->lower[0] =  r->rs_parts[0][0].lower[i];
    m->sym[0] =  r->rs_parts[0][0].sym[0];
    m->extent[0] =  r->rs_parts[0][0].extent[0];
    m->sym_upper[0] =  r->rs_parts[0][0].sym_upper[0];
    m->sym_lower[0] =  r->rs_parts[0][0].sym_lower[0];

    for (j = 0; j < maxproc; ++j)
    {
      m->offset[1] =  r->rs_parts[0][1].proc_offset[j];
      m->upper[1] =  r->rs_parts[0][1].upper[j];
      m->lower[1] =  r->rs_parts[0][1].lower[j];
      m->sym[1] =  r->rs_parts[0][1].sym[0];
      m->extent[1] =  r->rs_parts[0][1].extent[0];
      m->sym_upper[1] =  r->rs_parts[0][1].sym_upper[0];
      m->sym_lower[1] =  r->rs_parts[0][1].sym_lower[0];


      for(k = 0; k < maxproc; ++k)
      {
        m->offset[2]=  r->rs_parts[0][2].proc_offset[k];
        m->upper[2] =  r->rs_parts[0][2].upper[k];
        m->lower[2] =  r->rs_parts[0][2].lower[k];
        m->sym[2] =  r->rs_parts[0][2].sym[0];
        m->extent[2] =  r->rs_parts[0][2].extent[0];
        m->sym_upper[2] =  r->rs_parts[0][2].sym_upper[0];
        m->sym_lower[2] =  r->rs_parts[0][2].sym_lower[0];

        /* insert more loops here to handle additional array dimensions */

        /*---        
        for(l=0;l< maxproc;++l){ m.offset[3] =
         r->rs_parts[proc][3].inv_proc_offset[l]; m->upper[3] =
         r->rs_parts[proc][3].inv_upper[l]; m->lower[3] =
         r->rs_parts[proc][3].inv_lower[l];
        ---*/
        
        if (proc_num < total_procs)
        {
          ++proc_num;
          s->Append(info_s->CommInfoStr(rset, m));
         }
        }
      }
    }
 return s;
}



//------------------------------------------------------------
// return sending processors in the form of a range for each
// message generated by the rsd
//------------------------------------------------------------
FD_String* FortDMesg::SendProcRange()
{
 StringBuffer *send_str, *recv_str;
 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoRecv;
 int *min, *max;
 min = new int [10];
 max = new int [10];

 CommSendProcRange *send_obj;
 CommRecvProcRange *recv_obj;
 

 f_rsd_set = NonLocalRsds();
  
 if (repr->RecvProcRangeStr == 0)
  DoRecv = true;
 else
  DoRecv = false;

 //----- if proc range information not compiled -----//
 if (repr->SendProcRangeStr == 0)  
  {
  repr->SendProcRangeStr = new FD_String();
 
  if(DoRecv)
    repr->RecvProcRangeStr = new FD_String();
  f_rsd = (*f_rsd_set)();
  //---------- compute the range for each rsd -------------//
  init_msg_info(repr->dh);
  compute_proc_offsets(repr->dh,(Rsd_set*)f_rsd->GetRSet(),
                                ((Rsd_set*)f_rsd->GetRSet())->rs);
  compute_procs(repr->dh, ((Rsd_set*)f_rsd->GetRSet())->sp->numdim);
  dc_compute_proc_range(repr->dh->ped, min, max,
                        (Rsd_set*)(f_rsd->GetRSet()), Iter_simple);

  send_obj = new CommSendProcRange(min, max, CommType());
  send_str = f_rsd->SendStr((void*)&(repr->dh->rs_parts), send_obj);
  repr->SendProcRangeStr->Append(send_str->Finalize());     
  delete send_obj;
   
   if(DoRecv)
   {
   recv_obj = new CommRecvProcRange(min, max, CommType());
   recv_str = f_rsd->RecvStr((void*)&(repr->dh->rs_parts), recv_obj);

   repr->RecvProcRangeStr->Append(recv_str->Finalize());
   delete recv_obj;
   }
  }

 while (f_rsd = (*f_rsd_set)()) {};
// reset the count. I don't traverse all
// the rsds since they have been taken
// into account due to message aggregation
 return repr->SendProcRangeStr;
}

//----------------------------------------------------------------------
// return the range of processors involved in the receive
//----------------------------------------------------------------------
FD_String* FortDMesg::RecvProcRange()
{
 StringBuffer *send_str, *recv_str;
 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoSend;
 int *min, *max;
 CommSendProcRange *send_obj;
 CommRecvProcRange *recv_obj;

 
 f_rsd_set = NonLocalRsds();
  
 if (repr->SendProcRangeStr == 0)
  DoSend = true;
 else
  DoSend = false;

 //----- if proc range information not compiled -----//
 if (repr->RecvProcRangeStr == 0)  
  {
  min = new int [10];
  max = new int [10];

  repr->RecvProcRangeStr = new FD_String();

  if(DoSend)
   repr->SendProcRangeStr = new FD_String();

   f_rsd = (*f_rsd_set)();
  //---------- compute the range for each rsd -------------//
  init_msg_info(repr->dh);
  compute_proc_offsets(repr->dh,(Rsd_set*)f_rsd->GetRSet(),
                                ((Rsd_set*)f_rsd->GetRSet())->rs);
  dc_compute_proc_range(repr->dh->ped, min, max,
                        (Rsd_set*)(f_rsd->GetRSet()), Iter_simple);
  compute_procs(repr->dh, ((Rsd_set*)f_rsd->GetRSet())->sp->numdim);
  recv_obj = new CommRecvProcRange(min, max, CommType());
  recv_str = f_rsd->RecvStr((void*)&(repr->dh->rs_parts), recv_obj);
  repr->RecvProcRangeStr->Append(recv_str->Finalize());     
  delete recv_obj;

   if(DoSend)
   {
   send_obj = new CommSendProcRange(min, max, CommType());
   send_str = f_rsd->SendStr((void*)&(repr->dh->rs_parts), send_obj);

   repr->SendProcRangeStr->Append(send_str->Finalize());
   delete send_obj;
   }
  }
 while (f_rsd = (*f_rsd_set)()) {};

 return repr->RecvProcRangeStr;
}


//--------------------------------------------------------------------
// returns a string  that contains the mesg type
//--------------------------------------------------------------------
FD_String* FortDMesg::MesgTypeStr()
{
 StringBuffer *s;
 FD_String *oss;

 oss = new FD_String();
 s = new StringBuffer(100);
 
 switch(MesgType())
 {
  case FD_MESG_INDEP:       /* loop-independent message                 */
  s->Append("%s", " LOOP INDEPENDENT DEPENDENCE ");
  break;

  case FD_MESG_CARRIED_ALL:  /* loop-carried message on non-partitioned loop */
  s->Append("%s", " LOOP CARRIED DEPENDENCE ON NON PARTITIONED LOOP ");
  break;
  
  case FD_MESG_CARRIED_PART: /* loop-carried message on partitioned loop  */
  s->Append("%s", " LOOP CARRIED DEPENDENCE ON PARTITIONED LOOP ");
  break;
 
  case FD_MESG_ALL:
  s->Append("%s", " ALL TYPES OF DEPENDENCES ");
  break;
 }
  oss->Append(s->Finalize());
  return oss;
}

//--------------------------------------------------------------------
// Returns a string that contains the Communication Type
//--------------------------------------------------------------------
FD_String* FortDMesg::CommTypeStr()
{
 StringBuffer *s;
 FD_String *oss;

 oss = new FD_String();
 s = new StringBuffer(60);

 switch(CommType())
 {
  case  FD_COMM_UNKNOWN :      /* unknown comm type */
  s->Append("%s", " UNKNOWN COMMUNICATION TYPE ");
  break;

  case FD_COMM_NONE  :         /* no communication */
  s->Append("%s"," NO COMMUNICATION ");
  break;

  case FD_COMM_SEND_RECV  :   /* single send/receive */
  s->Append("%s"," SINGLE SEND/RECEIVE COMMUNICATION ");
  break;

  case FD_COMM_SHIFT  :       /* shift by multiple procs */
  s->Append("%s"," SHIFT COMMUNICATION ");
  break;

  case  FD_COMM_BCAST :        /* broadcast/spread by proc */
  s->Append("%s"," BROADCAST COMMUNICATION ");
  break;

  case FD_COMM_REDUCE  :      /* reduce by procs */
  s->Append("%s"," GLOBAL ACCUMULATION "); 
  break;
 
  case FD_COMM_GATHER  :      /* gather by procs */
  s->Append("%s"," GATHER COMMUNICATION ");
  break;

  case FD_COMM_TRANSPOSE  :   /* transpose between procs */
  s->Append("%s"," TRANSPOSE COMMUNICATION ");
  break;
 
  case FD_COMM_INSPECT  :     /* communication taken care of by inspector */
  s->Append("%s"," INSPECTOR GENERATED COMMUNICATION ");
  break;

  case FD_COMM_RUNTIME  :     /* communication determined only at runtime */
  s->Append("%s"," RUNTIME COMMUNICATION ");
  break;
 
  default:
//  cout << "Unknown Communication type" << endl;
  break;
  }
 oss->Append(s->Finalize());
 return oss;
}

//--------------------------------------------------------------------
// Returns a string that contains the Reference color
//--------------------------------------------------------------------
FD_String* FortDRef::GetColorStr()
{
 StringBuffer *s;
 FD_String *oss;

 oss = new FD_String();
 s = new StringBuffer(60);

 switch(GetColor())
 {
  case  FD_RED :      
  s->Append("%s", " RED ");
  break;

  case FD_YELLOW  :   
  s->Append("%s"," YELLOW ");
  break;

  case FD_GREEN  :   
  s->Append("%s"," GREEN ");
  break;

  case FD_BLACK  : 
  s->Append("%s"," BLACK ");
  break;
  }
 oss->Append(s->Finalize());
 return oss;
}


//--------------------------------------------------------------------
// Returns a string that contains the Reference color
//--------------------------------------------------------------------
FD_String* FortDStmt::GetColorStr()
{
 StringBuffer *s;
 FD_String *oss;

 oss = new FD_String();
 s = new StringBuffer(60);

 switch(GetColor())
 {
  case  FD_RED :      
  s->Append("%s", " RED ");
  break;

  case FD_YELLOW  :   
  s->Append("%s"," YELLOW ");
  break;

  case FD_GREEN  :   
  s->Append("%s"," GREEN ");
  break;

  case FD_BLACK  : 
  s->Append("%s"," BLACK ");
  break;
  }
 oss->Append(s->Finalize());
 return oss;
}

//--------------------------------------------------------------------
// Returns a string that contains the Communication Type
//--------------------------------------------------------------------
FD_String* FortDMesg::AggrMesgStr()
{
 StringBuffer *s;
 FD_String *oss;

 oss = new FD_String();
 s = new StringBuffer(40);
 s->Append("%s", " OPTIMIZATION: MESSAGE AGGREGATION ");
 oss->Append(s->Finalize());
 return oss;
}

//--------------------------------------------------------------------
// Returns true if shift communication without boundary elements
//--------------------------------------------------------------------
Boolean FortDRsd::SimpleShiftComm()
{
 int k;
 for (k = 0; k < MAXLOOP; k++)  /* check boundary conditions */
    {
      if ((((Rsd_set*)GetRSet())->iterset->type[k] != Iter_simple) &&
          (((Rsd_set*)GetRSet())->iterset->type[k] != Iter_pre_only) &&
          (((Rsd_set*)GetRSet())->iterset->type[k] != Iter_post_only))
        return false;
    }
  return true;
}

//-----------------------------------------------------------------------
// demand driven send section string creation. Create the string if
// it has not been created. Also create the recv string since all the
// information has been computed
//----------------------------------------------------------------------
FD_String* FortDMesg::SendSection()
{
 StringBuffer *send_str, *recv_str;
 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoRecv;
 CommSendSection send_obj(CommType());
 CommRecvSection recv_obj(CommType());

 Rsd_section *rsd_s;

 f_rsd_set = NonLocalRsds();

//--------- check to see if there is a recv string --------//
 if (repr->RecvSectionStr)
 DoRecv = false;
 else
 DoRecv = true;

 //----- if send string information not compiled -----//
 if (repr->SendSectionStr == 0)  
  {
  repr->SendSectionStr = new FD_String();
  if (DoRecv)
  repr->RecvSectionStr = new FD_String();

  f_rsd = (*f_rsd_set)(); 
  
  init_msg_info(repr->dh);
  compute_proc_offsets(repr->dh,(Rsd_set*)f_rsd->GetRSet(),
                                ((Rsd_set*)f_rsd->GetRSet())->rs);
  compute_procs(repr->dh, ((Rsd_set*)f_rsd->GetRSet())->sp->numdim);
  send_str = f_rsd->SendStr((void*)&(repr->dh->rs_parts), &send_obj);
  repr->SendSectionStr->Append(send_str->Finalize());
 
  if(DoRecv)
   {
   recv_str = f_rsd->RecvStr((void*)&(repr->dh->rs_parts), &recv_obj);
   repr->RecvSectionStr->Append(recv_str->Finalize());
   }
  }
 while (f_rsd = (*f_rsd_set)()) {};
 return repr->SendSectionStr;

}


//-----------------------------------------------------------------------
// demand driven recv section string creation. Create the string if
// it has not been created. Also create the send string since all the
// information has been computed
//----------------------------------------------------------------------
FD_String* FortDMesg::RecvSection()
{
 StringBuffer *recv_str, *send_str;
 CommSendSection send_obj(CommType());
 CommRecvSection recv_obj(CommType());

 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoSend;

 Rsd_section *rsd_s;

 f_rsd_set = NonLocalRsds();
 
 //--------- check to see if there is a recv string --------//
 if (repr->SendSectionStr)
  DoSend = false;
 else
  DoSend = true;

 //----- if recv string information not compiled -----//
 if (repr->RecvSectionStr == 0)
 {
  repr->RecvSectionStr = new FD_String();
  if (DoSend)
   repr->SendSectionStr = new FD_String();
   f_rsd = (*f_rsd_set)(); 

   init_msg_info(repr->dh);
   compute_proc_offsets(repr->dh, (Rsd_set*)f_rsd->GetRSet(), 
                       ((Rsd_set*)f_rsd->GetRSet())->rs);
   compute_procs(repr->dh, ((Rsd_set*)f_rsd->GetRSet())->sp->numdim);
   recv_str = f_rsd->RecvStr((void*)&(repr->dh->rs_parts), &recv_obj);
   repr->RecvSectionStr->Append(recv_str->Finalize());
  
  if (DoSend)  
   {
   send_str = f_rsd->SendStr((void*)&(repr->dh->rs_parts), &send_obj);
   repr->SendSectionStr->Append(send_str->Finalize());
   }
  }
 while(f_rsd = (*f_rsd_set)()) {};
 return repr->RecvSectionStr;
}



//--------------------------------------------------------------------
// return type of reduction
//--------------------------------------------------------------------
FD_String* FortDMesg::ReducTypeStr()
{
  StringBuffer *s;
  FD_String* oss;
  s = new StringBuffer(100);
  oss = new FD_String();
  switch(ReducType())
  {
  case FD_REDUC_NONE:
  s->Append("%s ", "UNKNOWN REDUCTION");
  break;
 
  case FD_REDUC_PLUS: 
  s->Append("%s ", "SUM REDUCTION");
  break; 

  case FD_REDUC_MINUS:
  s->Append("%s", "MINUS REDUCTION");
  break;

  case FD_REDUC_TIMES:
  s->Append("%s", "MULTIPLY REDUCTION");
  break;

  case FD_REDUC_DIV: 
  s->Append("%s", "DIVIDE REDUCTION");
  break;

  case FD_REDUC_MIN: 
  s->Append("%s", "MIN REDUCTION");
  break;

  case FD_REDUC_MAX: 
  s->Append("%s", "MAX_REDUCTION");
  break;

  case FD_REDUC_OR:  
  s->Append("%s", "OR REDUCTION");
  break;

  case FD_REDUC_AND: 
  s->Append("%s", "AND REDUCTION");
  break;

  case FD_REDUC_XOR: 
  s->Append("%s", "EXCLUSIVE OR REDUCTION");
  break;

  case FD_REDUC_MIN_LOC:
  s->Append("%s", "MIN LOCATION REDUCTION");
  break;

  case FD_REDUC_MAX_LOC:
  s->Append("%s", "MAX LOCATION REDUCTION");
  break;
  
  default:
  cout << "Unknown Reduction" << endl;
  }
  oss->Append(s->Finalize());
 return oss;
}

//--------------------------------------------------------------------
// return  size of message for a reduction
//--------------------------------------------------------------------
FD_String* FortDMesg::ReducMsgSizeStr()
{
  SNODE* sp;
  StringBuffer *s;
  FD_String* oss;
  s = new StringBuffer(20);
  oss = new FD_String();

  if (is_subscript(ReducLhs()))
  {
    sp = (SNODE*)get_info(repr->dh->ped, 
                 gen_SUBSCRIPT_get_name(ReducLhs()), type_fd);
    if (sp == (SNODE*) NO_FD_INFO)
     {
     cout << "FortDMesg::ReducMsgSizeStr: Unknown size" << endl;
     return 0;
     }
    else
    {
    s->Append("%d", dc_array_size(sp));
    oss->Append(s->Finalize());
    return oss; 
    }
  }
  s->Append("%d", 1);
  oss->Append(s->Finalize());
  return oss;
 }


//--------------------------------------------------------------------
// return the array or scalar involved in the communication for a 
// reduction
//--------------------------------------------------------------------
FD_String* FortDMesg::ReducVarStr()
{
 FD_String *oss;
 StringBuffer *s;
 oss = new FD_String();
 s  = new StringBuffer(20);

 if (is_subscript(ReducLhs())) 
  s->Append("%s", gen_get_text(gen_SUBSCRIPT_get_name(ReducLhs())));
 else
  s->Append("%s", gen_get_text(ReducLhs()));
  
 oss->Append(s->Finalize());
 return oss;
}

//--------------------------------------------------------------------
// return the broadcast send string
//--------------------------------------------------------------------
FD_String* FortDMesg::BroadcastSendSectionStr()
{
 StringBuffer *send_str, *recv_str;
 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoRecv;
 CommSendSection send_obj(CommType());
 CommRecvSection recv_obj(CommType());

 Rsd_section *rsd_s;

 f_rsd_set = NonLocalRsds();

//--------- check to see if there is a recv string --------//
 if (repr->RecvSectionStr)
 DoRecv = false;
 else
 DoRecv = true;
//----- if send string information not compiled -----//
 if (repr->SendSectionStr == 0)  
  {
  repr->SendSectionStr = new FD_String();
  if (DoRecv)
  repr->RecvSectionStr = new FD_String();

  f_rsd = (*f_rsd_set)(); 
  
  init_msg_info(repr->dh);
  send_obj.PreProcess((void*)&(repr->dh->rs_parts));
  compute_proc_offsets(repr->dh,(Rsd_set*)f_rsd->GetRSet(),
                               ((Rsd_set*)f_rsd->GetRSet())->rs_carried ? 
 ((Rsd_set*)f_rsd->GetRSet())->rs_carried : ((Rsd_set*)f_rsd->GetRSet())->rs);

  send_str = f_rsd->SendStr((void*)&(repr->dh->rs_parts), &send_obj);
  repr->SendSectionStr->Append(send_str->Finalize());
 if(DoRecv)
   {
   recv_str = f_rsd->RecvStr((void*)&(repr->dh->rs_parts), &recv_obj);
   repr->RecvSectionStr->Append(recv_str->Finalize());
   }
  }
 while (f_rsd = (*f_rsd_set)()) {};
 return repr->SendSectionStr;
}

//---------------------------------------------------------------------
// Returns the string that contains the processors 
//---------------------------------------------------------------------
FD_String* FortDMesg::BroadcastRecvSectionStr()
{
 char *sstr;
 StringBuffer *recv_str, *send_str;
 CommSendSection send_obj(CommType());
 CommRecvSection recv_obj(CommType());

 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoSend;

 sstr = 0;
 sstr =  (char*)malloc(40);
 memset(sstr, 0, 40*sizeof(char));
 
 Rsd_section *rsd_s;

 f_rsd_set = NonLocalRsds();
 
 //--------- check to see if there is a recv string --------//
 if (repr->SendSectionStr)
  DoSend = false;
 else
  DoSend = true;

 //----- if recv string information not compiled -----//
 if (repr->RecvSectionStr == 0)
 {
  recv_str = new StringBuffer(40);
  repr->RecvSectionStr = new FD_String();
  if (DoSend)
   repr->SendSectionStr = new FD_String();
   f_rsd = (*f_rsd_set)(); 
   init_msg_info(repr->dh);
   recv_obj.PreProcess((void*)&(repr->dh->rs_parts));
   compute_proc_offsets(repr->dh,(Rsd_set*)f_rsd->GetRSet(),
                               ((Rsd_set*)f_rsd->GetRSet())->rs_carried ? 
  ((Rsd_set*)f_rsd->GetRSet())->rs_carried : ((Rsd_set*)f_rsd->GetRSet())->rs);
   recv_str = f_rsd->RecvStr((void*)&(repr->dh->rs_parts), &recv_obj);
   repr->RecvSectionStr->Append(recv_str->Finalize());
  
  if (DoSend)  
   {
   send_str = new StringBuffer(40);
   send_str = f_rsd->SendStr((void*)&(repr->dh->rs_parts), &send_obj);
   repr->SendSectionStr->Append(send_str->Finalize());
   }
  }
 while(f_rsd = (*f_rsd_set)()) {};
 return repr->RecvSectionStr;
}

//--------------------------------------------------------------------------
// return the processor performing the broadcast
//--------------------------------------------------------------------------

FD_String* FortDMesg::BroadcastSendProcRange(int type)
{ 
 char *sstr, *send_s;
 StringBuffer *send_str, *recv_str;
 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoRecv, DoSend = true;

 f_rsd_set = NonLocalRsds();
 sstr = send_s = 0;  
 sstr =    (char*)malloc(40);
 send_s =  (char*)malloc(40);

 memset(send_s, 0,40*sizeof(char));
 memset(sstr, 0, 40*sizeof(char));

 if (repr->RecvProcRangeStr == 0)
  DoRecv = true;
 else
  DoRecv = false;

 //----- if proc range information not compiled -----//
   if (repr->SendProcRangeStr) DoSend = false;

 if (DoSend)  
  {
  send_str = new StringBuffer(40);
  repr->SendProcRangeStr = new FD_String();

  if(DoRecv)
   repr->RecvProcRangeStr = new FD_String();

   f_rsd = (*f_rsd_set)();

  //---------- compute the send range for an rsd -------------//
  init_msg_info(repr->dh);
  compute_proc_offsets(repr->dh,(Rsd_set*)f_rsd->GetRSet(),
                                ((Rsd_set*)f_rsd->GetRSet())->rs);
  ast_to_str(make_sendproc(repr->dh, (Rsd_set*)f_rsd->GetRSet()), send_s);
   send_str->Append("%s%s%s", "--<< Send Processor: ", send_s, " >>--");
   repr->SendProcRangeStr->Append(send_str->Finalize());     

 //------------ compute the recv range for an rsd --------------//
   if(DoRecv)
   {
   recv_str = new StringBuffer(40);
   recv_str->Append("--<< Recv Processors:  All Processors except ");
   recv_str->Append(send_s);
   recv_str->Append("  -->>");
   repr->RecvProcRangeStr->Append(recv_str->Finalize());
   }
  }
 while (f_rsd = (*f_rsd_set)()) {};
 return repr->SendProcRangeStr;
}

//-------------------------------------------------------------------------
// Return a string that contains the processor involved in the receive part
// of a broadcast
//-------------------------------------------------------------------------
FD_String* FortDMesg::BroadcastRecvProcRange()
{
 char *r_str, *s_str, *send_st;
 StringBuffer *recv_str, *send_str;
 CommSendSection send_obj(CommType());
 CommRecvSection recv_obj(CommType());

 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Boolean DoSend;
 
 r_str = s_str = send_st = 0;
 r_str =   (char*)malloc(40);
 s_str =   (char*)malloc(40);
 send_st = (char*)malloc(40);

 memset(send_st, 0, 40*sizeof(char));
 memset(r_str, 0, 40*sizeof(char));
 memset(s_str, 0, 40*sizeof(char));

 Rsd_section *rsd_s;

 f_rsd_set = NonLocalRsds();
 
 //--------- check to see if there is a recv string --------//
 if (repr->SendProcRangeStr)
  DoSend = false;
 else
  DoSend = true;

  //----- if recv string information not compiled -----//
 if (repr->RecvProcRangeStr == 0)
 {
  repr->RecvProcRangeStr = new FD_String();
  if (DoSend)
   repr->SendProcRangeStr = new FD_String();
   f_rsd = (*f_rsd_set)(); 
   init_msg_info(repr->dh);
   recv_obj.PreProcess((void*)&(repr->dh->rs_parts));
   compute_proc_offsets(repr->dh,(Rsd_set*)f_rsd->GetRSet(),
                               ((Rsd_set*)f_rsd->GetRSet())->rs_carried ? 
  ((Rsd_set*)f_rsd->GetRSet())->rs_carried : ((Rsd_set*)f_rsd->GetRSet())->rs);
   sprintf(r_str, "%s", "Recv Processors: All Processors except ");
   ast_to_str(make_sendproc(repr->dh, (Rsd_set*)f_rsd->GetRSet()), send_st);
   strcat(r_str, send_st);
   recv_str->Append("%s", r_str);
   repr->RecvProcRangeStr->Append(recv_str->Finalize());
  
  if (DoSend)  
   {
   sprintf(s_str, "%s" , send_st);
   send_str->Append("%s", s_str);
   repr->SendProcRangeStr->Append(send_str->Finalize());
   }
  }
 while(f_rsd = (*f_rsd_set)()) {};
 return repr->RecvProcRangeStr;

}

//------------------------------------------------------------
//  helper function for the SendRProcRange; returns the string
//  containing the sender 
//------------------------------------------------------------
static FD_String* SendRProcRangeSt(Iter_set *iset, int depth)
{
  FD_String *st;
  int pre, post, MaxProc;
  char buf[100];

  pre = iset->pre_idle[depth];
  post = iset->post_idle[depth];
  MaxProc = sp_num_blocks(iset->lhs_sp,iset->dim_num[depth]) - 1;

  st = new FD_String();

  if ((pre + post) == (MaxProc))
  {
  sprintf(buf, "%s%d%s", "--<< Recv Processor: ", pre, " >>--");
  st->Append(buf);
  return st;
  }
  else
  {
   cout << "SendRProcRangeSt not valid \n";  
   return 0;
  }
}

//-------------------------------------------------------------------------
// Return a string that contains the processor involved in the receive part
// of a single send receive communication
//-------------------------------------------------------------------------
FD_String* FortDMesg::RecvSProcRange()
{
 return(BroadcastSendProcRange(1));
}

//-------------------------------------------------------------------------
// Return a string that contains the processor involved in the send part
// of a single send receive communication
//-------------------------------------------------------------------------
FD_String* FortDMesg::SendRProcRange()
{
 FortDRsdSet* f_rsd_set;
 FortDRsd *f_rsd;
 Rsd_section *rsd_s;
 Iter_set *iset;
 SNODE *sp;
 int i, dist_dim;

 f_rsd_set = NonLocalRsds();
 
  f_rsd = (*f_rsd_set)();
  iset = ((Rsd_set*)f_rsd->GetRSet())->iterset;
  sp = ((Rsd_set*)f_rsd->GetRSet())->sp;

    for(i=0; i< sp->numdim; ++i)
    {
     switch(sp_is_part(sp, i))
     {
      case FD_DIST_LOCAL:
      break;
 
      default:
      dist_dim = i;
     }
    }
  
  for(i=0;i<10;++i)
   {
   if (iset->dim_num[i] == dist_dim)
    {
     repr->RecvProcRangeStr = SendRProcRangeSt(iset, i);
    }
   }
  while(f_rsd = (*f_rsd_set)()) {};
  return repr->RecvProcRangeStr;
}


//--------------------------------------------------------------------
// the id returned may be used to obtain the AST_INDEX of the 
// decomposition directive using the function ft_NumberToNode
//--------------------------------------------------------------------
int FortDRef::DecompId()
{
 return(sp_get_decomp_id(repr->symEntry));
}

//--------------------------------------------------------------------
// the id returned may be used to obtain the AST_INDEX of the 
// distribute directive using the function ft_NumberToNode
//--------------------------------------------------------------------
int FortDRef::DistribId()
{
 return(sp_get_distrib_id(repr->symEntry));
}

//--------------------------------------------------------------------
// the id returned may be used to obtain the AST_INDEX of the 
// align directive using the function ft_NumberToNode
//--------------------------------------------------------------------
int FortDRef::AlignId()
{
 return(sp_get_align_id(repr->symEntry, 0));
}

#if 0
//----------------------------------------------------------------------
// returns the context of the module containing the decomposition, align
// and distribute directive for the reference.
// Current restriction is that they are all
// in the same module
//--------------------------------------------------------------------
Context FortDRef::Contxt()
{
 return (sp_get_context(repr->symEntry));
}
#endif

//---------------------------------------------------------------------
// returns the reduc section strings for a reduction 
//---------------------------------------------------------------------
void FortDMesg::ReducSectionStr()
{
  StringBuffer *s;
  SNODE *sp = (SNODE*) NO_FD_INFO;
  int i;
  char *str, *strg;
  str = (char*)malloc(100);

  memset(str, 0, 100*sizeof(char));

  s = new StringBuffer(100);

if (repr->SendSectionStr == 0)
 {
  repr->SendSectionStr = new FD_String();
  repr->RecvSectionStr = new FD_String();

  if (is_subscript(ReducLhs()))
   {
    sp = (SNODE*)get_info(repr->dh->ped, 
                 gen_SUBSCRIPT_get_name(ReducLhs()), type_fd);
   }
  if (sp == (SNODE*) NO_FD_INFO)
   {
     s->Append("%s", "--<< Global Accumulation of ");
     if(is_subscript(ReducLhs()))
      {
       s->Append(gen_get_text(gen_SUBSCRIPT_get_name(ReducLhs())));
      }
     else
      {
       s->Append(gen_get_text(ReducLhs()));
      }

      s->Append("%s",  " >>--\0");
      strg = s->Finalize();
      repr->SendSectionStr->Append(strg);
      repr->RecvSectionStr->Append(strg);
      return;
    }   
//---------- check to see if there is a send string ---------
//---------- create the string -----------
     s->Append("%s", "--<< Global Accumulation of ");
     s->Append("%s", sp->id);
     if(sp->numdim != 0)
      s->Append("%c", '(');

     for (i = 0; i < sp->numdim; i++)
     {
      sprintf(str, "%d", sp_get_lower(sp, i));
      s->Append("%s:", str);
      sprintf(str, "%d", sp_get_upper(sp, i));
      s->Append("%s", str);
      if (i == sp->numdim-1)
        s->Append("%c", ')');
      else
       s->Append("%c", ',');
   }
  s->Append("%s",  " >>--");
  }
 strg = s->Finalize();
 repr->SendSectionStr->Append(strg);

//---------- compute the recv section of the string -------//
  repr->RecvSectionStr->Append(strg);
}


//---------------------------------------------------------------------
// returns the send section string for a reduction
//---------------------------------------------------------------------
FD_String* FortDMesg::ReducSendSectionStr()
{
 ReducSectionStr();
 return (repr->SendSectionStr);
}

//---------------------------------------------------------------------
// returns the recv section string for a reduction 
//---------------------------------------------------------------------
FD_String* FortDMesg::ReducRecvSectionStr()
{
  ReducSectionStr();
  return (repr->RecvSectionStr);
}

//---------------------------------------------------------------------
// returns the processor range string for a reduction 
//---------------------------------------------------------------------
void FortDMesg::ReducProcRangeStr()
{
 StringBuffer *s, *r;
 char stemp[10], *strg;
 s = new StringBuffer(100);
 r = new StringBuffer(100);

 if (repr->SendProcRangeStr == 0)
 { 

  s->Append("%s ", "--<< Send Processors: All Processors  >>--");
  r->Append("%s ", "--<< Recv Processors: All Processors  >>--");
  repr->SendProcRangeStr = new FD_String();
  repr->SendProcRangeStr->Append(s->Finalize());
  if (repr->RecvProcRangeStr == 0)
   {
    repr->RecvProcRangeStr = new FD_String();
    repr->RecvProcRangeStr->Append(r->Finalize());
   }
 }
}

//---------------------------------------------------------------------
// returns the send processor range string for a reduction 
//---------------------------------------------------------------------
FD_String* FortDMesg::ReducSendProcRangeStr()
{
 ReducProcRangeStr();
 return (repr->SendProcRangeStr);
}
//---------------------------------------------------------------------
// returns the recv processor range string for a reduction 
//---------------------------------------------------------------------
FD_String* FortDMesg::ReducRecvProcRangeStr()
{
 ReducProcRangeStr();
 return (repr->RecvProcRangeStr);
}

//---------------------------------------------------------------------
// returns the send section string
//---------------------------------------------------------------------
FD_String* FortDMesg::SendSectionString()
{
 switch(CommType())
 {
  case FD_COMM_SHIFT:
  case FD_COMM_SEND_RECV:
  return(SendSection());
  break;

  case FD_COMM_BCAST:
  return(BroadcastSendSectionStr());
  break;

  case FD_COMM_REDUCE:
  return(ReducSendSectionStr());
  break;
 
  default:
   cout << "communication type not implemented \n";
   exit(0);
   return (FD_String*)NULL;
  break;
 }
}

//---------------------------------------------------------------------
// returns the recv section string
//---------------------------------------------------------------------
FD_String* FortDMesg::RecvSectionString()
{
 switch(CommType())
 {
  case FD_COMM_SHIFT:
  case FD_COMM_SEND_RECV:
  return(RecvSection());
  break;

  case FD_COMM_BCAST:
  return(BroadcastRecvSectionStr());
  break;

  case FD_COMM_REDUCE:
  return(ReducRecvSectionStr());
  break;
 
  default:
   cout << "communication type not implemented \n";
   exit(0);
   return (FD_String*)NULL;
  break;
 }
}

//---------------------------------------------------------------------
// returns the send processor range  string
//---------------------------------------------------------------------
FD_String* FortDMesg::SendProcessorRangeString()
{
 switch(CommType())
 {
  case FD_COMM_SHIFT:
  return(SendProcRange());
  break; 

  case FD_COMM_BCAST:
  return(BroadcastSendProcRange(0));

  case FD_COMM_SEND_RECV: 
  return(BroadcastSendProcRange(1));
  break;

  case FD_COMM_REDUCE:
  return(ReducSendProcRangeStr());
  break;


  default:
   cout << "communication type not implemented \n";
   exit(0);
   return (FD_String*)NULL;
  break;
 }
}

//---------------------------------------------------------------------
// returns the recv processor range string
//---------------------------------------------------------------------
FD_String* FortDMesg::RecvProcessorRangeString()
{
 switch(CommType())
 {
  case FD_COMM_SHIFT:
  return(RecvProcRange());
  break;

  case FD_COMM_BCAST:
  return(BroadcastRecvProcRange());
  break;

  case FD_COMM_REDUCE:
  return(ReducRecvProcRangeStr());
  break;

  case FD_COMM_SEND_RECV: 
  return(SendRProcRange());
  break;

  default:
   cout << "communication type not implemented \n";
   exit(0);
   return (FD_String*)NULL;
  break;
 }
}
