/* $Id: msg2.ansi.c,v 1.29 1997/03/11 14:28:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    msg2.c       Generates actual send/recv messages based on 
                 analysis performed in msg.c & elsewhere.

    Author : Seema Hiranandani 

*/

#include <libs/fortD/codeGen/private_dc.h>
#include <math.h>

typedef struct        /* structure used to summarize RSDs */
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
  int *bsize;                       /* pointer to size of buffer    */
} Mesg_data;

/* #include <libs/fortD/performance/staticInfo/MesgStaticSDDF.h>
 * Cannot include above file because the declaration of Mesg_data from
 * above is copied there.  Therefore, copy these extern decls. here:
 */
/*------------------------- extern definitions ------------------------*/

EXTERN(void,
       GetMessageSendParams,(Dist_Globals *dh,
			    Rsd_set* rset,	  /* RSD(s) sent in msg */
			    Mesg_data *mesgData,  /* summary of data sent */
			    Boolean buffered,	  /* whether buffered in pgm */
			    AST_INDEX sendStmt)); /* the actual send stmt */
EXTERN(void,
       GetMessageRecvParams,(Dist_Globals *dh,
			    Rsd_set* rset,	  /* RSD(s) sent in msg */
			    Mesg_data *mesgData,  /* summary of data sent */
			    Boolean buffered,	  /* whether buffered in pgm */
			    AST_INDEX recvStmt));  /* the actual recv stmt */

EXTERN(AST_INDEX, dc_send_guard,(Dist_Globals *dh, SNODE *sp, int *offsetlist));
EXTERN(AST_INDEX, dc_recv_guard,(Dist_Globals *dh, SNODE *sp, int *offsetlist));
EXTERN(AST_INDEX, pt_gen_comment,(char *str));
EXTERN(AST_INDEX, dc_get_sgroup_if,(Dist_Globals *dh, AST_INDEX loop));
EXTERN(char, *dc_get_mem,(Dist_Globals *dh, int size));

/*------------------------- global definitions ------------------------*/

int *compute_send_data(Dist_Globals *dh, Rsd_set *rset);
void compute_recv_data(Dist_Globals *dh, Rsd_set *rset, Iter_type msg_type, 
                       AST_INDEX loop);
AST_INDEX broadcast_scalar_send(Dist_Globals *dh, AST_INDEX *aux);
AST_INDEX broadcast_scalar_recv(Dist_Globals *dh, AST_INDEX *aux);
int *dc_alloc_msgbuf(Dist_Globals *dh, enum FORM form, Boolean zero);
void ast_to_str(AST_INDEX node, char *str);

/*------------------------- local definitions ------------------------*/

STATIC(void, make_send_proc,(Dist_Globals *dh, SNODE *sp, Mesg_data *m));   
STATIC(void, make_recv_proc,(Dist_Globals *dh, SNODE *sp, Mesg_data *m));   
STATIC(AST_INDEX, make_buffer_data,(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m));
STATIC(AST_INDEX, make_unbuffer_data,(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m));
STATIC(AST_INDEX, make_array_dims,(Dist_Globals *dh, SNODE *sp));
STATIC(AST_INDEX, make_array_buffer,(Dist_Globals *dh, SNODE *sp, Mesg_data *m));
STATIC(AST_INDEX, make_array_unbuffer,(Dist_Globals *dh, SNODE *sp, Mesg_data *m));
STATIC(char, *make_send_str,(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m));
STATIC(char, *make_recv_str,(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m));
STATIC(void, calc_msize,(Dist_Globals *dh, Mesg_data *m, Rsd_set *rset));    
STATIC(Boolean, is_contiguous_mesg,(Mesg_data *m));
STATIC(AST_INDEX, make_buffer_contiguous,(Dist_Globals *dh, Rsd_set *rset, 
                                          Mesg_data *m));
STATIC(AST_INDEX, make_unbuffer_contiguous,(Dist_Globals *dh, Rsd_set *rsert, 
                                            Mesg_data *m));
STATIC(AST_INDEX, make_buffer_assign,(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m));
STATIC(AST_INDEX, make_unbuffer_assign,(Dist_Globals *dh, Rsd_set *rset, 
                                        Mesg_data *m));
STATIC(AST_INDEX, make_csend_array_pos,(Dist_Globals *dh, SNODE *sp, Mesg_data *m,
                                        int offset));
STATIC(AST_INDEX, make_crecv_array_pos,(Dist_Globals *dh, SNODE *sp, Mesg_data *m,
                                        int offset));

STATIC(AST_INDEX, make_crecv,(Dist_Globals *dh, Comm_type ctype));
STATIC(AST_INDEX, make_csend,(Dist_Globals *dh, Comm_type ctype));
STATIC(AST_INDEX, make_csend_buffer,(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m));
STATIC(AST_INDEX, make_crecv_unbuffer,(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m));
STATIC(AST_INDEX, make_multiple,(AST_INDEX expr, int i));
STATIC(char, *make_buffer_name,(Dist_Globals *dh, Rsd_set *rset, Boolean pack));
STATIC(char, *make_buffer_contig_name,(Dist_Globals *dh, Rsd_set *rset));
STATIC(int, dc_max_procs,(Dist_Globals *dh));  
STATIC(AST_INDEX, alloc_buf,(Buf_info *buf, int **bufsize_ptr, Boolean zero));


/********************/
/* Global Functions */
/********************/

/*---------------------------------------------------------------------

  compute_send_data()    Make AST for sending message

*/

int * 
compute_send_data(Dist_Globals *dh, Rsd_set *rset)
{
  int i, j, k, num, proc, proc_num, total_procs, tempt, maxproc, *bufsize_p;
  int dist_dim;
  AST_INDEX bufstmts, csend, send_stmts, rhs2, comment, node, guard;
  char buf[MAXCOMMENT], *send_str;
  Mesg_data mdata, *m;

  m = &mdata;

  for (i=0; i< rset->sp->numdim; ++i)
	{
		if ((sp_is_part(rset->sp,i) != FD_DIST_LOCAL) ||
        (sp_is_part(rset->sp,i) != FD_DIST_DC_NONE))
     dist_dim = i;
  }

  m->numdim = rset->sp->numdim;
  proc = dh->proc;
  send_stmts = dh->send_stmts;
  if (send_stmts == AST_NIL)
    send_stmts = list_create(AST_NIL);
  proc_num = 0;
  tempt = 0;
  maxproc = dc_max_procs(dh);
  total_procs = dh->rs_parts[proc][0].total_procs;

  for (i = 0; i < maxproc; ++i)
  {
   if (dist_dim == 0)
    j = i;
   else
    j = 0;
    m->offset[0] = dh->rs_parts[proc][0].inv_proc_offset[j];
    m->upper[0] = dh->rs_parts[proc][0].inv_upper[j];
    m->lower[0] = dh->rs_parts[proc][0].inv_lower[j];
    m->sym[0] = dh->rs_parts[proc][0].sym[0];
    m->extent[0] = dh->rs_parts[proc][0].extent[0];
    m->sym_upper[0] = dh->rs_parts[proc][0].sym_upper[0];
    m->sym_lower[0] = dh->rs_parts[proc][0].sym_lower[0];
 
    if (dist_dim == 1)
     j = i;
    else 
     j = 0;
      m->offset[1] = dh->rs_parts[proc][1].inv_proc_offset[j];
      m->upper[1] = dh->rs_parts[proc][1].inv_upper[j];
      m->lower[1] = dh->rs_parts[proc][1].inv_lower[j];
      m->sym[1] = dh->rs_parts[proc][1].sym[0];
      m->extent[1] = dh->rs_parts[proc][1].extent[0];
      m->sym_upper[1] = dh->rs_parts[proc][1].sym_upper[0];
      m->sym_lower[1] = dh->rs_parts[proc][1].sym_lower[0];

     if (dist_dim == 2)
      j = i;
     else
      j = 0;
        m->offset[2] = dh->rs_parts[proc][2].inv_proc_offset[j]; 
        m->upper[2] = dh->rs_parts[proc][2].inv_upper[j]; 
        m->lower[2] = dh->rs_parts[proc][2].inv_lower[j];
        m->sym[2] = dh->rs_parts[proc][2].sym[0];
        m->extent[2] = dh->rs_parts[proc][2].extent[0];
        m->sym_upper[2] = dh->rs_parts[proc][2].sym_upper[0];
        m->sym_lower[2] = dh->rs_parts[proc][2].sym_lower[0];

        /* insert more loops here to handle additional array dimensions */

        /*---        
        for(l=0;l< maxproc;++l){ m->offset[3] =
        dh->rs_parts[proc][3].inv_proc_offset[l]; m->upper[3] =
        dh->rs_parts[proc][3].inv_upper[l]; m->lower[3] =
        dh->rs_parts[proc][3].inv_lower[l];
        ---*/
       if(dist_dim > 2)
      die_with_message("Fortran D  prototype does not support arrays with more than 3 dimensions \n");
        
        if (proc_num < total_procs)
        {
         
          calc_msize(dh, m, rset);
          ++proc_num;
          ++dh->mtype;
          dh->msg_id = AST_NIL;

          if (rset->ctype == FD_COMM_BCAST)
          {
            dh->srproc = pt_gen_int(-1);
          }
          else if (rset->ctype == FD_COMM_SEND_RECV)
          {
            dh->srproc = ast_map_logical_to_physical(dh,
				     pt_gen_int(rset->iterset->proc[0]));
  
          }
          else
          {
            make_send_proc(dh, rset->sp, m);
          }

          /* check whether to use unbuffered send (isend on Intel) */

          if (dh->opts.unbuffered_send && (dh->arch == FD_NX) &&
              (m->msize >= dh->opts.min_unbuffered_send) &&
              (rset->mtype != FD_MESG_CARRIED_PART))
          {
            sprintf(buf, "id$%d", dh->msg_id_num++);
            dh->msg_id = pt_gen_ident(buf);
            dc_new_var(dh, tree_copy(dh->msg_id), INTTYPE);
          }

          bufsize_p = dc_alloc_msgbuf(dh, rset->sp->fform, 
            (rset->ctype == FD_COMM_SHIFT) && (dh->msg_id == AST_NIL));

          send_str = make_send_str(dh, rset, m);
          sprintf(buf, "--<< %s %s >>--", 
            (rset->ctype == FD_COMM_BCAST) ? "Broadcast" : "Send", send_str);
          comment = pt_gen_comment(buf);

          bufstmts = make_csend_buffer(dh, rset, m);
          csend = make_csend(dh, rset->ctype);
          ++tempt;

          if ((rset->ctype == FD_COMM_BCAST) || 
              (rset->ctype == FD_COMM_SEND_RECV))
          {
            guard = dh->bcast;
            if (bufstmts)
            {
              if ((rset->ctype == FD_COMM_BCAST) && !dh->send_userbuf)
                rhs2 = list_insert_first(bufstmts, csend);
              else
                rhs2 = list_insert_last(bufstmts, csend);
            }
            else
            {
              rhs2 = list_create(csend);
            }
            rhs2 = gen_GUARD(AST_NIL, guard, rhs2);
            rhs2 = gen_IF(AST_NIL, AST_NIL, list_create(rhs2));
          }
          else
          {
            guard = dc_send_guard(dh, rset->sp, m->offset);
            rhs2 = list_append(bufstmts, list_create(csend));
            rhs2 = gen_GUARD(AST_NIL, guard, rhs2);
            rhs2 = gen_IF(AST_NIL, AST_NIL, list_create(rhs2));
          }

          /* insert all the function calls etc into a single list */

          (void) list_insert_last(send_stmts, comment);
          (void) list_insert_last(send_stmts, rhs2);

          /* build call to synch routine for unbuffered send */

          if (dh->msg_id != AST_NIL)
          {
            if (!is_if(rhs2))
              printf("compute_send_data(): if stmt not found \n");

            node = list_create(tree_copy(dh->msg_id));
            node = list_create(pt_gen_call(dh->syscalls.msgwait, node));
            node = gen_GUARD(AST_NIL, tree_copy(guard), node);
            node = gen_IF(AST_NIL, AST_NIL, list_create(node));
            node = list_create(node);

            sprintf(buf, "--<< Wait %s %s >>--", 
              (rset->ctype == FD_COMM_BCAST) ? "Broadcast" : "Send", send_str);
            comment = list_create(pt_gen_comment(buf));

            dh->send_sync = list_insert_last(comment, node);
          }

           /* update size of buffer if needed */

          if (dh->send_userbuf || (rset->ctype != FD_COMM_SHIFT))
          {
            num = dh->const_msize ? m->msize * (rset->num_merge + 1) : 0;
            if (num > *bufsize_p)
              *bufsize_p = num;
          }

	  /* 6/17/94, VSA: Added to generate SDDF record */
	  GetMessageSendParams(dh,rset, &mdata,BOOL(bufstmts!=AST_NIL), csend);
  
          sfree(send_str);
        }
      }

  dh->mtype = dh->mtype - tempt;
  dh->send_stmts = send_stmts;

  return bufsize_p;
}


/*---------------------------------------------------------------------

  compute_recv_data()    Make AST for receiving message

*/

void
compute_recv_data(Dist_Globals *dh, Rsd_set *rset, Iter_type msg_type, AST_INDEX loop)
  /*AST_INDEX loop: needed to get guard for FD_COMM_SEND_RECV */
{
  int i, j, k, num, proc, proc_num, total_procs, maxproc, *bufsize_p, dist_dim;
  AST_INDEX crecv, recv_stmts, node, bufstmts, comment, guard;
  char buf[MAXCOMMENT], *recv_str;
  Mesg_data mdata, *m;
  
  AST_INDEX crecv_stmt;			/* VSA: For passing to SDDF code */

  m = &mdata;
  m->numdim = rset->sp->numdim;
  recv_stmts = dh->recv_stmts;
  if (recv_stmts == AST_NIL)
    recv_stmts = list_create(AST_NIL);

  for (i=0; i< rset->sp->numdim; ++i)
	{
		if ((sp_is_part(rset->sp,i) != FD_DIST_LOCAL) ||
        (sp_is_part(rset->sp,i) != FD_DIST_DC_NONE))
     dist_dim = i;
  }
  proc = dh->proc;
  maxproc = dc_max_procs(dh);
  proc_num = 0;
  total_procs = dh->rs_parts[proc][0].total_procs;

  for (i = 0; i < maxproc; ++i)
  {
   if (dist_dim == 0)
    j = i;
   else
    j = 0;
    m->offset[0] = dh->rs_parts[proc][0].proc_offset[j];
    m->upper[0] = dh->rs_parts[proc][0].upper[j];
    m->lower[0] = dh->rs_parts[proc][0].lower[j];
    m->sym[0] = dh->rs_parts[proc][0].sym[0];
    m->extent[0] = dh->rs_parts[proc][0].extent[0];
    m->sym_upper[0] = dh->rs_parts[proc][0].sym_upper[0];
    m->sym_lower[0] = dh->rs_parts[proc][0].sym_lower[0];
     if (dist_dim == 1)
       j = i;
     else
       j = 0;
      m->offset[1] = dh->rs_parts[proc][1].proc_offset[j];
      m->upper[1] = dh->rs_parts[proc][1].upper[j];
      m->lower[1] = dh->rs_parts[proc][1].lower[j];
      m->sym[1] = dh->rs_parts[proc][1].sym[0];
      m->extent[1] = dh->rs_parts[proc][1].extent[0];
      m->sym_upper[1] = dh->rs_parts[proc][1].sym_upper[0];
      m->sym_lower[1] = dh->rs_parts[proc][1].sym_lower[0];
     if (dist_dim == 2)
       j = i;
     else
       j = 0;
        m->offset[2]= dh->rs_parts[proc][2].proc_offset[j];
        m->upper[2] = dh->rs_parts[proc][2].upper[j];
        m->lower[2] = dh->rs_parts[proc][2].lower[j];
        m->sym[2] = dh->rs_parts[proc][2].sym[0];
        m->extent[2] = dh->rs_parts[proc][2].extent[0];
        m->sym_upper[2] = dh->rs_parts[proc][2].sym_upper[0];
        m->sym_lower[2] = dh->rs_parts[proc][2].sym_lower[0];

       if(dist_dim > 2)
      die_with_message("Fortran D  prototype does not support arrays with more than 3 dimensions \n");
      
   /* insert more loops here to handle additional array dimensions */

        /*---        
        for(l=0;l< maxproc;++l){ m->offset[3] =
        dh->rs_parts[proc][3].inv_proc_offset[l]; m->upper[3] =
        dh->rs_parts[proc][3].inv_upper[l]; m->lower[3] =
        dh->rs_parts[proc][3].inv_lower[l];
        ---*/
        
        if (proc_num < total_procs)
        {
          calc_msize(dh, m, rset);
          ++proc_num;
          ++dh->mtype;
          bufsize_p = NULL;

          /* check whether to use unbuffered receive (irecv on Intel) */

          if (dh->opts.unbuffered_recv && (dh->arch == FD_NX) &&
              (m->msize >= dh->opts.min_unbuffered_recv) &&
              (rset->mtype != FD_MESG_CARRIED_PART))
          {
            sprintf(buf, "id$%d", dh->msg_id_num++);
            dh->msg_id = pt_gen_ident(buf);
            dc_new_var(dh, tree_copy(dh->msg_id), INTTYPE);
          }
          else if (dh->msg_id != AST_NIL)  /* used isend */
          {
            printf("compute_recv_data(): using isend & crecv\n");
          }

          if (rset->ctype == FD_COMM_BCAST)
          {
            dh->startpoint = dh->bufpoint;  /* always receive in send buffer */
            bufstmts = list_create(AST_NIL);
            dh->recv_userbuf = false;  
          }
          else 
          {
            /* not needed for Intel NX routines */
            /* make_recv_proc(dh, rset->sp, m); */

            /* alloc new buffer if unbuffered receive for SHIFT */
            /* unbuffered BCAST & SEND_RECV can use same buffer */

            if ((rset->ctype == FD_COMM_SHIFT) && (dh->msg_id != AST_NIL))
              bufsize_p = dc_alloc_msgbuf(dh, rset->sp->fform, false);

            bufstmts = make_crecv_unbuffer(dh, rset, m);
          }

          crecv_stmt =
	  crecv = make_crecv(dh, rset->ctype);

          /* build call to synch routine for unbuffered recv */

          if (dh->msg_id != AST_NIL)
          {
            dh->recv_sync = crecv;
            crecv = list_create(tree_copy(dh->msg_id));
            crecv = pt_gen_call(dh->syscalls.msgwait, crecv);
          }

          recv_str = make_recv_str(dh, rset, m);
          sprintf(buf, "--<< %s %s >>--", 
            (dh->msg_id == AST_NIL) ? "Recv" : "Wait Recv", recv_str);
          comment = pt_gen_comment(buf);

          switch (rset->ctype)
          {
            case FD_COMM_SHIFT: 
              guard = dc_recv_guard(dh, rset->sp, m->offset);
              break;

            case FD_COMM_SEND_RECV: 
              guard = dc_get_sgroup_if(dh, loop);
              guard = list_first(gen_IF_get_guard_LIST(guard)); 
              guard = tree_copy(gen_GUARD_get_rvalue(guard));
              break;

            case FD_COMM_BCAST: 
              node = tree_copy(gen_BINARY_EQ_get_rvalue1(dh->bcast));
              guard = tree_copy(gen_BINARY_EQ_get_rvalue2(dh->bcast));
              guard = gen_BINARY_NE(node,guard);
              break;
          }

          crecv = list_create(crecv);
          node = list_append(crecv, bufstmts);

          if ((msg_type != Iter_simple) || 
              (rset->mtype == FD_MESG_CARRIED_PART)) 
          {
            node = gen_GUARD(AST_NIL, guard, node);
            node = gen_IF(AST_NIL, AST_NIL, list_create(node));
          }

          /* insert all the function calls etc into a single list */

          (void) list_insert_last(recv_stmts, comment);
          (void) list_insert_last(recv_stmts, node);

          if (dh->msg_id != AST_NIL)
          {
            sprintf(buf, "--<< %s %s >>--", "Post Recv", recv_str);
            comment = list_create(pt_gen_comment(buf));

	    node = (is_list(dh->recv_sync))?
	      gen_GUARD(AST_NIL, tree_copy(guard), dh->recv_sync) :
	      gen_GUARD(AST_NIL, tree_copy(guard), list_create(dh->recv_sync));
            node = gen_IF(AST_NIL, AST_NIL, list_create(node));
            dh->recv_sync = list_insert_last(comment, node);
          }

          sfree(recv_str);

           /* update size of buffer if new buffer allocated for irecv */

          if (bufsize_p && dh->recv_userbuf)
          {
            num = dh->const_msize ? m->msize * (rset->num_merge + 1) : 0;
            if (num > *bufsize_p)
              *bufsize_p = num;
          }
  /* insert all the function calls etc into a single list */
      }
    }

  dh->recv_stmts = recv_stmts;
  /*dh->mtype = dh->mtype - tempt; */
  
  /* 6/17/94, VSA: Added to generate SDDF record */
  GetMessageRecvParams(dh, rset, &mdata, BOOL(bufstmts != AST_NIL),crecv_stmt);
}


/*---------------------------------------------------------------------

  broadcast_scalar_send()    Make AST for receiving scalars

*/

AST_INDEX
broadcast_scalar_send(Dist_Globals *dh, AST_INDEX *aux)
{
  AST_INDEX args, slist, node, rhs;
  int i, num, size, *bsize;
  SNODE *sp; 
  enum FORM type[32], all_type;
  char buf[MAXCOMMENT];

  /* count up arguments & determine their types */
  for (num = 0; aux[num] != AST_NIL; num++)
  {
    if (sp = findadd2(aux[num], 0, 0, dh))
    {
      type[num] = sp->fform;

      if (!num || (type[num] == DOUBLE_P) ||
          ((type[num] == REAL) && (all_type == INTTYPE)))
        all_type = type[num];
    }
    else
    {
      type[num] = all_type = DOUBLE_P;
    }
  }

  size = (all_type == DOUBLE_P ) ? 8*num : 4*num;
  ++dh->mtype;

  bsize = dc_alloc_msgbuf(dh, all_type, true);   /* alloc buffer & size */
  if (size > *bsize)
    *bsize = size;

  /* generate comment */
  strcpy(buf, "--<< Broadcast ");
  for (i = 0; i < num; i++)
  {
    strcat(buf, gen_get_text(aux[i]));
    if (i < num-1)
      strcat(buf, ", ");
  }
  strcat(buf, " >>--");

  slist = pt_gen_comment(buf);
  slist = list_create(slist);

  if (num == 1)
  {
    dh->bufpoint = tree_copy(aux[0]);
  }
  else
  {
    switch (all_type)
    {
    case INTTYPE:
      dh->bufpoint = pt_gen_ident(IBUF_NAME);
      break;
    case REAL:
      dh->bufpoint = pt_gen_ident(RBUF_NAME);
      break;
    case DOUBLE_P:
      dh->bufpoint = pt_gen_ident(DPBUF_NAME);
      break;
    }

    for (i = 0; i < num; i++)
    {
      node = list_create(pt_gen_int(i+1));
      node = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);
      rhs = tree_copy(aux[i]);
      node = gen_ASSIGNMENT(AST_NIL, node, rhs);
      slist = list_insert_last(slist, node);
    }
  }

  switch (dh->arch)
  {
    case FD_NX:
      args = list_create(pt_gen_int(dh->mtype));
      args = list_insert_last(args, dh->bufpoint);
      args = list_insert_last(args, pt_gen_int(size));
      args = list_insert_last(args, pt_gen_int(-1));
      args = list_insert_last(args, pt_gen_ident(dh->mypid));
      list_insert_last(slist, pt_gen_call(dh->syscalls.bcast, args));
      break;

    case FD_CMMD:
      args = list_create(dh->bufpoint);
      args = list_insert_last(args, pt_gen_int(size));
      args = pt_gen_invoke(dh->syscalls.bcast, args);
      args = gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->ret), args);
      list_insert_last(slist, args);
      break;
  }

  return slist;
}

/*---------------------------------------------------------------------

  broadcast_scalar_recv()    Make AST for receiving scalars

*/

AST_INDEX
broadcast_scalar_recv(Dist_Globals *dh, AST_INDEX *aux)
{
  AST_INDEX args, slist, node, rhs;
  int i, num, size;
  SNODE *sp; 
  enum FORM type[32], all_type;
  char buf[MAXCOMMENT];

  /* count up arguments & determine their types */
  for (num = 0; aux[num] != AST_NIL; num++)
  {
    if (sp = findadd2(aux[num], 0, 0, dh))
    {
      type[num] = sp->fform;

      if (!num || (type[num] == DOUBLE_P) ||
          ((type[num] == REAL) && (all_type == INTTYPE)))
        all_type = type[num];
    }
    else
    {
      type[num] = all_type = DOUBLE_P;
    }
  }

  size = (all_type == DOUBLE_P ) ? 8*num : 4*num;

  /* generate comment */
  strcpy(buf, "--<< Recv ");
  for (i = 0; i < num; i++)
  {
    strcat(buf, gen_get_text(aux[i]));
    if (i < num-1)
      strcat(buf, ", ");
  }
  strcat(buf, " >>--");

  slist = pt_gen_comment(buf);
  slist = list_create(slist);

  if (num == 1)
  {
    dh->bufpoint = tree_copy(aux[0]);
  }
  else
  {
    switch (all_type)
    {
    case INTTYPE:
      dh->bufpoint = pt_gen_ident(IBUF_NAME);
      break;
    case REAL:
      dh->bufpoint = pt_gen_ident(RBUF_NAME);
      break;
    case DOUBLE_P:
      dh->bufpoint = pt_gen_ident(DPBUF_NAME);
      break;
    }
  }

  switch (dh->arch)
  {
    case FD_NX:
      args = list_create(pt_gen_int(dh->mtype));
      args = list_insert_last(args, dh->bufpoint);
      args = list_insert_last(args, pt_gen_int(size));
      list_insert_last(slist, pt_gen_call(dh->syscalls.brecv, args));
      break;

    case FD_CMMD:
      args = list_create(dh->bufpoint);
      args = list_insert_last(args, pt_gen_int(size));
      args = pt_gen_invoke(dh->syscalls.brecv, args);
      args = gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->ret), args);
      list_insert_last(slist, args);
      break;
  }


  if (num > 1)
  {
    for (i = 0; i < num; i++)
    {
      node = tree_copy(aux[i]);
      rhs = list_create(pt_gen_int(i+1));
      rhs = gen_SUBSCRIPT(tree_copy(dh->bufpoint), rhs);
      node = gen_ASSIGNMENT(AST_NIL, node, rhs);
      slist = list_insert_last(slist, node);
    }
  }

  return slist;
}


/*--------------------------------------------------------------------

  dc_alloc_msgbuf()    Set up AST for buffer & bufsize

  Returns:  Pointer to buffer size (so can update later if necessary)

*/

int *
dc_alloc_msgbuf(Dist_Globals *dh, enum FORM form, Boolean zero)
 /* Boolean zero: whether to use default temp buffer */
{
  int *bufsize;

  switch (form)
  {
    case INTTYPE:
      dh->buftype = pt_gen_int(IBUF_ELEM_SIZE);  /* i$size */
      dh->bufpoint = alloc_buf(&dh->bufs.i, &bufsize, zero);
      break;

    case DOUBLE_P:
      dh->buftype = pt_gen_int(DPBUF_ELEM_SIZE); /* dp$size */
      dh->bufpoint = alloc_buf(&dh->bufs.dp, &bufsize, zero);
      break;

    case REAL:
    default:
      dh->buftype = pt_gen_int(RBUF_ELEM_SIZE);  /* r$size */
      dh->bufpoint = alloc_buf(&dh->bufs.r, &bufsize, zero);
      break;
  }

  return bufsize;
}


/*-----------------------------------------------------------------------

  ast_to_str()      Convert simple AST expr to a string

*/

void
ast_to_str(AST_INDEX node, char *str)
{
  if (is_identifier(node))
    strcat(str, gen_get_text(node));
  else if (is_constant(node))
    strcat(str, gen_get_text(node));
  else if (is_binary_plus(node))
  {
    if (gen_get_parens(node))
      strcat(str,"(");
    ast_to_str(gen_BINARY_PLUS_get_rvalue1(node), str);
    strcat(str, "+");
    ast_to_str(gen_BINARY_PLUS_get_rvalue2(node), str);
    if (gen_get_parens(node))
      strcat(str,")");
  }
  else if (is_binary_minus(node))
  {
    if (gen_get_parens(node))
      strcat(str,"(");
    ast_to_str(gen_BINARY_MINUS_get_rvalue1(node), str);
    strcat(str, "-");
    ast_to_str(gen_BINARY_MINUS_get_rvalue2(node), str);
    if (gen_get_parens(node))
      strcat(str,")");
  }
  else if (is_binary_times(node))
  {
    if (gen_get_parens(node))
      strcat(str,"(");
    ast_to_str(gen_BINARY_TIMES_get_rvalue1(node), str);
    strcat(str, "*");
    ast_to_str(gen_BINARY_TIMES_get_rvalue2(node), str);
    if (gen_get_parens(node))
      strcat(str,")");
  }
  else if (is_binary_divide(node))
  {
    if (gen_get_parens(node))
      strcat(str,"(");
    ast_to_str(gen_BINARY_DIVIDE_get_rvalue1(node), str);
    strcat(str, "/");
    ast_to_str(gen_BINARY_DIVIDE_get_rvalue2(node), str);
    if (gen_get_parens(node))
      strcat(str,")");
  }
  else
    strcat(str, "?");
}


/*******************/
/* Local Functions */
/*******************/

/*---------------------------------------------------------------------

  make_csend_buffer()   Make AST to copy data into of csend() buffer

*/

static AST_INDEX 
make_csend_buffer(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX bufstmt;

  bufstmt = AST_NIL;
  dh->send_userbuf = true;  
  dh->startpoint = dh->bufpoint;

  if (is_contiguous_mesg(m))
  {
    if (!rset->num_merge)
    {
      dh->send_userbuf = false;    /* no buffer, send data in place */
      dh->startpoint = make_csend_array_pos(dh, rset->sp, m, 0);

      /* if broadcast, will need buffer anyway */
      if (rset->ctype != FD_COMM_BCAST)
        return AST_NIL;
    }

    if ((m->msize < dh->opts.min_buf_size) && dh->const_msize)
      bufstmt = make_buffer_assign(dh, rset, m);
    else
      bufstmt = make_buffer_contiguous(dh, rset, m);
  }
  else
  {
    if ((m->msize < dh->opts.min_buf_size) && dh->const_msize)
      bufstmt = make_buffer_assign(dh, rset, m);
    else
      bufstmt = make_buffer_data(dh, rset, m);
  }

  return bufstmt;
}


/*---------------------------------------------------------------------

  make_crecv_unbuffer()   Make AST to copy data out of crecv() buffer

  Don't unbuffer if COMM_SEND_RECV, since not using overlap

*/

static AST_INDEX 
make_crecv_unbuffer(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX bufstmt;

  bufstmt = AST_NIL;
  dh->recv_userbuf = true;  
  dh->startpoint = dh->bufpoint;  

  if ((rset->ctype == FD_COMM_SHIFT) && is_contiguous_mesg(m))
  {
    if (!rset->num_merge)
    {
      dh->recv_userbuf = false;  /* no buffer, recv data in place */
      dh->startpoint = make_crecv_array_pos(dh, rset->sp, m, 0);
    }
    else if (rset->ctype != FD_COMM_SEND_RECV)
    {
      if ((m->msize < dh->opts.min_buf_size) && dh->const_msize)
        bufstmt = make_unbuffer_assign(dh, rset, m);
      else
        bufstmt = make_unbuffer_contiguous(dh, rset, m);
    }
  }
  else if (rset->ctype != FD_COMM_SEND_RECV)
  {
    if ((m->msize < dh->opts.min_buf_size) && dh->const_msize)
      bufstmt = make_unbuffer_assign(dh, rset, m);
    else
      bufstmt = make_unbuffer_data(dh, rset, m);
  }

  return bufstmt;
}


/*---------------------------------------------------------------------

  make_send_proc()   Make the AST for proc num of csend() target

*/

static void 
make_send_proc(Dist_Globals *dh, SNODE *sp, Mesg_data *m)
{
  AST_INDEX myproc, myproc1, aoffset, proc, numblocks, proc1;
  int i;
  Boolean was_local = true;

  dh->srproc = AST_NIL;
  aoffset = AST_NIL;
  /* sp1 = sp->decomp; */

  for (i = 0; i < sp->numdim; ++i)
  {
    myproc = ast_get_logical_myproc(dh);

    /* switch (sp_is_part(sp1, i)) */
    switch (sp_is_part(sp, i))
    {
      case FD_DIST_BLOCK:
        if (was_local || !i)
        {
          if (m->offset[i] < 0)
          {
            aoffset = pt_gen_int(-m->offset[i]);
            proc = gen_BINARY_MINUS(myproc, aoffset);
            gen_put_parens(proc, 1);
          }
          else if (m->offset[i] > 0)
          {
            aoffset = pt_gen_int(m->offset[i]);
            proc = gen_BINARY_PLUS(myproc, aoffset);
            gen_put_parens(proc, 1);
          }
          else /* m->offset[i] == 0 */
          {
            proc = myproc;
          }
        }
        else
        {
          if (m->offset[i])
          {
            aoffset = pt_gen_int(abs(m->offset[i]));

            if (m->offset[i] < 0)
              myproc1 = gen_BINARY_MINUS(myproc1, aoffset);
            else                /* m->offset[i] > 0 */
              myproc1 = gen_BINARY_PLUS(myproc1, aoffset);

            gen_put_parens(myproc1, 1);
          }
          else
            myproc1 = myproc;

          numblocks = pt_gen_int(sp_num_blocks(sp, i));
          proc1 = gen_BINARY_TIMES(myproc1, numblocks);
          gen_put_parens(proc1, 1);

          proc = gen_BINARY_PLUS(tree_copy(proc), proc1);
          gen_put_parens(proc, 1);
        }
        was_local = false;
        break;

      case FD_DIST_LOCAL:
        was_local = true;
        break;

      case FD_DIST_CYCLIC:
        was_local = false;
        break;
    }
  }

  /* dh->multdim_srproc[dh->n_srproc] = proc; */

  gen_put_parens(proc, 0);  /* get rid of outermost pair of parens */
  dh->srproc = ast_map_logical_to_physical(dh, proc);
}


/*---------------------------------------------------------------------

  make_recv_proc()   Make the AST for proc num of crecv() target

*/

static void 
make_recv_proc(Dist_Globals *dh, SNODE *sp, Mesg_data *m)
{
  AST_INDEX myproc, myproc1, aoffset, proc, numblocks, proc1;
  int was_local, i;

  was_local = 0;
  dh->srproc = AST_NIL;

  myproc = pt_gen_ident(dh->myproc);

  for (i = 0; i < sp->numdim; ++i)
  {
    switch (sp_is_part(sp, i))
    {

      case FD_DIST_BLOCK:
        if (!i || (dh->srproc == AST_NIL))
        {
          myproc1 = tree_copy(myproc);
          aoffset = pt_gen_int(abs(m->offset[i]));

          if (m->offset[i] < 0)
            proc = gen_BINARY_MINUS(myproc1, aoffset);

          else if (m->offset[i] > 0)
            proc = gen_BINARY_PLUS(myproc1, aoffset);

          gen_put_parens(proc, 1);

        }

        if (was_local == 1)
        {

          myproc1 = tree_copy(myproc);
          aoffset = pt_gen_int(abs(m->offset[i]));

          if (m->offset[i] < 0)
            proc = gen_BINARY_MINUS(myproc1, aoffset);

          else if (m->offset[i] > 0)
            proc = gen_BINARY_PLUS(myproc1, aoffset);

          gen_put_parens(proc, 1);

        }

        else
        {
          numblocks = pt_gen_int(sp_num_blocks(sp, i));
          myproc1 = tree_copy(myproc);

          aoffset = pt_gen_int(abs(m->offset[i]));

          proc1 = gen_BINARY_TIMES(myproc1, numblocks);
          gen_put_parens(proc1, 1);

          proc = gen_BINARY_PLUS(tree_copy(proc), proc1);
          gen_put_parens(proc, 1);
        }

        was_local = 0;
        break;

      case FD_DIST_LOCAL:
        was_local = 1;
        break;

      case FD_DIST_CYCLIC:
        was_local = 0;
        break;

    }
  }

/* dh->multdim_srproc[dh->n_srproc] = proc; */
  dh->srproc = ast_map_logical_to_physical(dh, proc);
}


/*-----------------------------------------------------------------------

  make_buffer_data()   Makes up AST for buffer_data()

  Buffers array section into contiguous 1D buffer for SEND message.

  Returns:  call buffer_data^N(A, dl1, du1, ... , dl^N, du^N,
                                   l1, u1, s1, ... , l^N, u^N, s^N, buf)

  Where    N       = dimension of array
           A       = name of array
           dl,du   = lower & upper bounds of dimension 1 ... N
           l,u,s   = lower bound, upper bound, step of data unbuffered
           buf     = name of buffer for message

*/

static AST_INDEX 
make_buffer_data(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX args, node, result;
  char buf[MAXCOMMENT];
  int i, num;

  /* store size of buffer now, or catch size later after var2buf () */
  if ((rset->ctype != FD_COMM_BCAST) && (rset->ctype != FD_COMM_SEND_RECV))
  {
    num = dh->const_msize ? m->msize * (rset->num_merge + 1) : 0;
  }

  result = list_create(AST_NIL);
  num = rset->num_merge;

  for (i = 0; i <= num; i++, rset = rset->rsd_merge)
  {
    /* make array dim args */
    args = make_array_dims(dh, rset->sp);

    /* make array rsd args */ 
    args = list_append(args, make_array_buffer(dh, rset->sp, m));   

    /* make buffer arg */
    if (dh->const_msize)
      node = list_create(pt_gen_int((m->msize * i) + 1));
    else
      node = list_create(make_multiple(dh->msgsize, i));

    node = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);
    args = list_insert_last(args, node);

    /* make call to buffer_data */
    node = pt_gen_call(make_buffer_name(dh, rset, true), args);
    result = list_insert_last(result, node);
  }

  return result;
}


/*-----------------------------------------------------------------------

  make_unbuffer_data()   Makes up AST for unbuffer_data()

  Copies data from 1D buffer back to array section for RECV message.

  Returns:  call unbuffer_data^N(A, dl1, du1, ... , dl^N, du^N,
                                   l1, u1, s1, ... , l^N, u^N, s^N, buf)

  Where    N       = dimension of array
           A       = name of array
           dl,du   = lower & upper bounds of dimension 1 ... N
           l,u,s   = lower bound, upper bound, step of data unbuffered
           buf     = name of buffer for message

*/

static AST_INDEX 
make_unbuffer_data(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX args, node, result;
  char buf[MAXCOMMENT];
  int i, num;

  result = list_create(AST_NIL);
  num = rset->num_merge;

  for (i = 0; i <= num; i++, rset = rset->rsd_merge)
  {
    /* make array dim args */
    args = make_array_dims(dh, rset->sp);

    /* make array rsd args */ 
    args = list_append(args, make_array_unbuffer(dh, rset->sp, m));   

    /* make buffer arg */
    if (dh->const_msize)
      node = list_create(pt_gen_int((m->msize * i) + 1));
    else
      node = list_create(make_multiple(dh->msgsize, i));

    node = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);
    args = list_insert_last(args, node);

    /* make call to unbuffer_data */
    node = pt_gen_call(make_buffer_name(dh, rset, false), args);
    result = list_insert_last(result, node);
  }

  return result;
}

/*------------------------------------------------------------------------

  make_array_dims()   Make up argument list for passing array param

  Returns:  AST_INDEX of list of arguments representing array
            Used as head of argument list to copy_data() & buffer_data()

*/

static AST_INDEX
make_array_dims(Dist_Globals *dh, SNODE *sp)
{
  int i;
  AST_INDEX args;

  /* first put in name of the array */

  args = list_create(pt_gen_ident(sp->id));

  /* then find and put in the bounds of each dimension */

  for (i = 0; i < sp->numdim; i++)
  {
    /* put in lower bound */
    args = list_insert_last(args,
             pt_gen_int(sp_min_access(sp, i)));

    /* put in upper bound */
    args = list_insert_last(args, 
             pt_gen_int(sp_max_access(sp, i)));
  }

  return args;
}


/*------------------------------------------------------------------------

  make_array_buffer()   Make up argument list for array region to be sent

  Returns:  AST_INDEX of list of arguments representing array region
            Used in argument list to buffer_data()

  Note that step is always 1 for now

*/

static AST_INDEX
make_array_buffer(Dist_Globals *dh, SNODE *sp, Mesg_data *m)
{
  AST_INDEX args, node;
  int i, blocksize, start, end;

  args = list_create(AST_NIL);

  /* go through each dimension of array and compute range */
  for (i = 0; i < sp->numdim; i++)
  {
    /* if symbolic, then only one element in this dim */
    if (m->sym[i])
    {
      /* make start/end AST of symbolic and save in arg list */
      args = list_insert_last(args, tree_copy(m->sym[i]));

      if (m->extent[i])
        args = list_insert_last(args, tree_copy(m->extent[i]));
      else
        args = list_insert_last(args, tree_copy(m->sym[i]));
    }

    /* else not symbolic, calculate range */
    else
    {
      switch (sp_is_part(sp, i))
      {
        case FD_DIST_BLOCK:
          blocksize = sp_block_size1(sp, i);
          start = abs(abs(m->lower[i]) - (abs(m->offset[i]) - 1) * blocksize);
          end = abs(abs(m->upper[i]) - (abs(m->offset[i]) - 1) * blocksize);
          break;

        default:
          start = m->lower[i];
          end = m->upper[i];
      }

      /* make start position into AST and save in arg list */
      node = (start == MININT) ? 
             tree_copy(m->sym_lower[i]) : pt_gen_int(start);
      args = list_insert_last(args, node);

      /* make end position into AST and save in arg list */
      node = (end == MAXINT) ?  
             tree_copy(m->sym_upper[i]) : pt_gen_int(end);
      args = list_insert_last(args, node);
    }

    /* make step into AST and save in arg list */
    args = list_insert_last(args, pt_gen_int(1));
  }

  return args;
}

/*------------------------------------------------------------------------

  make_array_unbuffer()   Make up argument list for array region to be recv'd

  Returns:  AST_INDEX of list of arguments representing array region
            Used in argument list to unbuffer_data()

  Note step is always 1 for now

*/

static AST_INDEX
make_array_unbuffer(Dist_Globals *dh, SNODE *sp, Mesg_data *m)
{
  AST_INDEX args, node;
  int i;

  args = list_create(AST_NIL);

  /* go through each dimension of array and compute range */
  for (i = 0; i < sp->numdim; i++)
  {
    /* if symbolic, then only one element in this dim */
    if (m->sym[i])
    {
      /* make start/end AST of symbolic and save in arg list */
      args = list_insert_last(args, tree_copy(m->sym[i]));
      if (m->extent[i])
        args = list_insert_last(args, tree_copy(m->extent[i]));
      else
        args = list_insert_last(args, tree_copy(m->sym[i]));
    }

    /* else not symbolic, calculate range */
    else
    {
      /* make start position into AST and save in arg list */
      node = (m->lower[i] == MININT) ? 
             tree_copy(m->sym_lower[i]) : pt_gen_int(m->lower[i]);
      args = list_insert_last(args, node);

      /* make end position into AST and save in arg list */
      node = (m->upper[i] == MAXINT) ?  
             tree_copy(m->sym_upper[i]) : pt_gen_int(m->upper[i]);
      args = list_insert_last(args, node);
    }

    /* make step into AST and save in arg list */
    args = list_insert_last(args, pt_gen_int(1));
  }

  return args;
}


/*-----------------------------------------------------------------------

  make_send_str()   Makes up string for array sections sent

*/

static char* 
make_send_str(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX node;
  int i, blocksize, start, end, len;
  char section[MAXCOMMENT], buf[MAXCOMMENT];

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

  buf[0] = '\0';
  for (i = rset->num_merge; i >= 0; i--, rset = rset->rsd_merge)
  {
    strcat(buf, rset->sp->id);
    strcat(buf, section);
    if (i)
      strcat(buf, ", ");
  }

  return ssave(buf);
}


/*-----------------------------------------------------------------------

  make_recv_str()   Makes up string for array sections received

*/

static char* 
make_recv_str(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  int i, len;
  char section[MAXCOMMENT], buf[MAXCOMMENT];

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

  buf[0] = '\0';
  for (i = rset->num_merge; i >= 0; i--, rset = rset->rsd_merge)
  {
    strcat(buf, rset->sp->id);
    strcat(buf, section);
    if (i)
      strcat(buf, ", ");
  }

  return ssave(buf);
}


/*---------------------------------------------------------------------

  make_buffer_contiguous()   Make AST for buffering contiguous data

*/

static AST_INDEX
make_buffer_contiguous(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX args, node, result;
  int i, num;

  /* store size of buffer now, or catch size later after var2buf () */
  if ((rset->ctype != FD_COMM_BCAST) && (rset->ctype != FD_COMM_SEND_RECV))
  {
    num = dh->const_msize ? m->msize * (rset->num_merge + 1) : 0;
  }

  result = list_create(AST_NIL);
  num = rset->num_merge;

  for (i = 0; i <= num; i++, rset = rset->rsd_merge)
  {
    /* make array source */
    args = list_create(make_csend_array_pos(dh, rset->sp, m, 0));

    /* make buffer arg */
    if (dh->const_msize)
      node = list_create(pt_gen_int((m->msize * i) + 1));
    else
      node = list_create(make_multiple(dh->msgsize, i));

    node = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);
    args = list_insert_last(args, node);

    /* make size */
    args = list_insert_last(args, tree_copy(dh->msgsize));

    /* make call to buffer_data */
    node = pt_gen_call(make_buffer_contig_name(dh, rset), args);
    result = list_insert_last(result, node);
  }

  return result;
}

/*---------------------------------------------------------------------

  make_unbuffer_contiguous()   Make AST for unbuffering contiguous data

*/

static AST_INDEX
make_unbuffer_contiguous(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX args, node, result;
  int i, num;

  result = list_create(AST_NIL);
  num = rset->num_merge;

  for (i = 0; i <= num; i++, rset = rset->rsd_merge)
  {
    /* make buffer arg */
    if (dh->const_msize)
      node = list_create(pt_gen_int((m->msize * i) + 1));
    else
      node = list_create(make_multiple(dh->msgsize, i));

    node = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);
    args = list_create(node);

    /* make array source */
    args = list_insert_last(args, make_crecv_array_pos(dh, rset->sp, m, 0));

    /* make size */
    args = list_insert_last(args, tree_copy(dh->msgsize));

    /* make call to buffer_data */
    node = pt_gen_call(make_buffer_contig_name(dh, rset), args);
    result = list_insert_last(result, node);
  }

  return result;
}


/*---------------------------------------------------------------------

  make_buffer_assign()   Make AST for assigning array element
                         directly to the buffer element

*/

static AST_INDEX
make_buffer_assign(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX node, lhs, rhs, result;
  int i, j, num;

  result = list_create(AST_NIL);
  num = rset->num_merge;

  for (i = 0; i <= num; i++, rset = rset->rsd_merge)
  {
    for (j = 0; j < m->msize; j++)
    {
      /* make buffer arg */
      node = list_create(pt_gen_int((m->msize * i) + j + 1));
      lhs = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);

      /* make array source */
      rhs = make_csend_array_pos(dh, rset->sp, m, j);

      result = list_insert_last(result, gen_ASSIGNMENT(AST_NIL, lhs, rhs));
    }
  }

  return result;
}


/*---------------------------------------------------------------------

  make_unbuffer_assign()   Make AST for assigning buffer 
                           directly to the array element

*/

static AST_INDEX
make_unbuffer_assign(Dist_Globals *dh, Rsd_set *rset, Mesg_data *m)
{
  AST_INDEX node, lhs, rhs, result;
  int i, j, num;

  result = list_create(AST_NIL);
  num = rset->num_merge;

  for (i = 0; i <= num; i++, rset = rset->rsd_merge)
  {
    for (j = 0; j < m->msize; j++)
    {
      /* make array source */
      lhs = make_crecv_array_pos(dh, rset->sp, m, j);

      /* make buffer arg */
      node = list_create(pt_gen_int((m->msize * i) + j + 1));
      rhs = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);

      result = list_insert_last(result, gen_ASSIGNMENT(AST_NIL, lhs, rhs));
    }
  }

  return result;
}

/*-------------------------------------------------------------------------

  make_csend_array_pos()

  Generate starting array position for sending contiguous data.

  Returns:  AST representing starting position

*/

static AST_INDEX
make_csend_array_pos(Dist_Globals *dh, SNODE *sp, Mesg_data *m, int offset)
{
  AST_INDEX node, subs;
  int blocksize, i, idx;

  /* make AST for each subscript */

  subs = list_create(AST_NIL);
  for (i = 0; i < sp->numdim; i++)
  {
    if (m->sym[i])                   /* symbolic subscript */
      node = tree_copy(m->sym[i]);
    else if (m->sym_lower[i])        /* symbolic range */
      node = tree_copy(m->sym_lower[i]);
    else 
    {
      switch (sp_is_part(sp, i))
      {
        case FD_DIST_BLOCK:
          blocksize = sp_block_size1(sp, i);
          idx = abs(abs(m->lower[i]) - ((abs(m->offset[i]) - 1) * blocksize));
          break;

        default:
          idx = m->lower[i];
          break;
      }

      if (m->lower[i] != m->upper[i])    /* increment if dim is range */
        idx += offset;

      node = pt_gen_int(idx);
    }

    subs = list_insert_last(subs, node); /* add to list of subscripts */
  }

  /* make up subscripted reference */

  return gen_SUBSCRIPT(pt_gen_ident(sp->id), subs);
}


/*-------------------------------------------------------------------------

  make_crecv_array_pos()

  Generate starting array position for receiving contiguous data.

  Returns:  AST representing starting position

*/

static AST_INDEX
make_crecv_array_pos(Dist_Globals *dh, SNODE *sp, Mesg_data *m, int offset)
{
  AST_INDEX node, subs;
  int i;

  /* make AST for each subscript */

  subs = list_create(AST_NIL);
  for (i = 0; i < sp->numdim; i++)
  {
    if (m->sym[i])
      node = tree_copy(m->sym[i]);          /* symbolic subscript */

    else if (m->lower[i] == m->upper[i])    /* constant subscript */
      node = pt_gen_int(m->lower[i]);

    else  /* constant subscript in dimension w/offset */
    {
      node = pt_gen_int(m->lower[i] + offset);
    }

    subs = list_insert_last(subs, node);    /* add to list of subscripts */
  }

  /* make up subscripted reference */

  return gen_SUBSCRIPT(pt_gen_ident(sp->id), subs);
}


/*----------------------------------------------------------------------

  make_csend()     Make AST representing call to send routine

  For FD_NX    call csend(type, addr, size, proc, pid)
      FD_CMMD  ret$ = cmmd_send_noblock(proc, type, addr, size)
               ret$ = cmmd_bc_to_nodes(addr, size)
*/

static AST_INDEX 
make_csend(Dist_Globals *dh, Comm_type ctype)
{
  AST_INDEX node, bsize;

  /*-------------------------------*/
  /* create AST for buffer size    */

  if (is_constant(dh->bufsize) &&
      (atoi(gen_get_text(dh->bufsize)) == 1))
  {
    bsize = tree_copy(dh->buftype);    /* don't multiply by 1 */
  }
  else
  {
    bsize = tree_copy(dh->bufsize);
    bsize = pt_simplify_expr(bsize);
    bsize = gen_BINARY_TIMES(bsize, tree_copy(dh->buftype));
  }

  /*---------------------*/
  /* create AST for send */

  switch (dh->arch)
  {
    case FD_NX:
      node = list_create(pt_gen_int(dh->mtype));
      node = list_insert_last(node, tree_copy(dh->startpoint));
      node = list_insert_last(node, bsize);
      node = list_insert_last(node, tree_copy(dh->srproc));
      node = list_insert_last(node, pt_gen_ident(dh->mypid));

      if (dh->msg_id != AST_NIL)  /* use unbuffered message */
      {
        node = pt_gen_invoke(dh->syscalls.isend, node);
        node = gen_ASSIGNMENT(AST_NIL, tree_copy(dh->msg_id), node);
      }
      else
        node = pt_gen_call(dh->syscalls.csend, node);

      break;

    case FD_CMMD:
      if (ctype == FD_COMM_BCAST)
      {
        node = list_create(tree_copy(dh->startpoint));
        node = list_insert_last(node, bsize);
        node = pt_gen_invoke(dh->syscalls.bcast, node);
      }
      else
      {
        node = list_create(tree_copy(dh->srproc));
        node = list_insert_last(node, pt_gen_int(dh->mtype));
        node = list_insert_last(node, tree_copy(dh->startpoint));
        node = list_insert_last(node, bsize);
        node = pt_gen_invoke(dh->syscalls.csend, node);
      }
      node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->ret), node);
      break;
  }

  return node;
}


/*----------------------------------------------------------------------

  make_crecv()     Make AST representing call to crecv routine

  For FD_NX    call crecv(type, addr, size)
      FD_CMMD  ret$ = cmmd_recv(CMMD_ANY_NODE, type, addr, size)
               ret$ = cmmd_receive_bc_from_node(addr, size)

*/

static AST_INDEX 
make_crecv(Dist_Globals *dh, Comm_type ctype)
{
  AST_INDEX node, bsize;

  /*-------------------------------*/
  /* create AST for buffer size    */

  if (is_constant(dh->bufsize) &&
      (atoi(gen_get_text(dh->bufsize)) == 1))
  {
    bsize = tree_copy(dh->buftype);    /* don't multiply by 1 */
  }
  else
  {
    bsize = tree_copy(dh->bufsize);
    bsize = pt_simplify_expr(bsize);
    bsize = gen_BINARY_TIMES(bsize, tree_copy(dh->buftype));
  }

  /*---------------------*/
  /* create AST for recv */

  switch (dh->arch)
  {
    case FD_NX:
      node = list_create(pt_gen_int(dh->mtype));
      node = list_insert_last(node, tree_copy(dh->startpoint));
      node = list_insert_last(node, bsize);
      if (dh->msg_id != AST_NIL)  /* use unbuffered message */
      {
        node = pt_gen_invoke(dh->syscalls.irecv, node);
        node = gen_ASSIGNMENT(AST_NIL, tree_copy(dh->msg_id), node);
      }
      else
        node = pt_gen_call(dh->syscalls.crecv, node);
      break;

    case FD_CMMD:
      if (ctype == FD_COMM_BCAST)
      {
        node = list_create(tree_copy(dh->startpoint));
        node = list_insert_last(node, bsize);
        node = pt_gen_invoke(dh->syscalls.brecv, node);
      }
      else
      {
        node = list_create(pt_gen_ident("CMMD_ANY_NODE"));
        node = list_insert_last(node, pt_gen_int(dh->mtype));
        node = list_insert_last(node, tree_copy(dh->startpoint));
        node = list_insert_last(node, bsize);
        node = pt_gen_invoke(dh->syscalls.crecv, node);
      }
      node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->ret), node);
      break;
  }

  return node;
}


/*-----------------------------------------------------------------------

  calc_msize()    Calculate message size for one RSD

  Based on values in m->upper & m->lower

  Assumption - message exists with at least size 1

  Sets:   m->msize          size of message (integer)
          dh->msgsize       AST for individual message size
          dh->bufsize       AST for total buffer size
          dh->const_msize   whether msize is compile-time constant
*/

static void
calc_msize(Dist_Globals *dh, Mesg_data *m, Rsd_set *rset)
{
  int i, msize, idx;
  AST_INDEX node, mult[DC_MAXDIM];

  idx = 0;
  m->msize = 1;
  dh->const_msize = true;

  for (i = 0; i < m->numdim; ++i)
  {
    /* if NOT symbolic AND involved in message */

    if (!m->sym[i] && (m->upper[i] || m->lower[i]))
    {
      if (m->upper[i] == MAXINT)
      {
        node = tree_copy(m->sym_upper[i]);

        if (m->lower[i] == MININT)
        {
          node = pt_gen_sub(node,tree_copy(m->sym_lower[i]));
          gen_put_parens(node,1);
          node = pt_gen_add(node, pt_gen_int(1));
          gen_put_parens(node,1);
        }
        else if (m->lower[i] != 1)
        {
          node = pt_gen_sub(node,pt_gen_int(m->lower[i]-1));
          gen_put_parens(node,1);
        }
        mult[idx++] = node;
      }
      else if (m->lower[i] == MININT)
      {
        node = tree_copy(m->sym_lower[i]);
        node = pt_gen_sub(pt_gen_int(m->upper[i]+1), node);
        gen_put_parens(node,1);
        mult[idx++] = node;
      }
      else
      {
        m->msize *= m->upper[i] - m->lower[i] + 1;
      }
    }
  }

  if (!dh->cgp_grain)  /* coarse grain pipelining not performed */
  {
    if (!idx)  /* constant size message */
    {
      dh->msgsize = pt_gen_int(m->msize);
      dh->bufsize = pt_gen_int(m->msize * (rset->num_merge + 1));
    }
    else  /* variable size message */
    {
      dh->const_msize = false;
      node = mult[0];
      for (i = 1; i < idx; i++)
        node = pt_gen_mul(node, mult[i]);

      if (m->msize == 1)
      {
        dh->msgsize = tree_copy(node);

        if (rset->num_merge) 
          dh->bufsize = pt_gen_mul(pt_gen_int(rset->num_merge + 1), node);
        else 
          dh->bufsize = node;
      }
      else
      {
        dh->msgsize = pt_gen_mul(pt_gen_int(m->msize), node);
        node = pt_gen_int(rset->num_merge + 1);
        dh->bufsize = pt_gen_mul(tree_copy(dh->msgsize), node);
      }
    }
  }
  else if (dh->cgp_size == AST_NIL)  /* constant strip size */
  {
    m->msize *= dh->cgp_grain;
    dh->msgsize = pt_gen_int(m->msize);
    dh->bufsize = pt_gen_int(m->msize * (rset->num_merge + 1));
  }
  else  /* symbolic strip size */
  {
    dh->const_msize = false;
    dh->msgsize = pt_gen_mul(pt_gen_int(m->msize), tree_copy(dh->cgp_size));
    msize = m->msize * (rset->num_merge + 1);
    dh->bufsize = (msize == 1) ? tree_copy(dh->cgp_size) :
              pt_gen_mul(pt_gen_int(msize), tree_copy(dh->cgp_size));
  }
}


/*-----------------------------------------------------------------------

  is_contiguous_mesg()    Determines whether message is contiguous

  Returns:  true if contiguous, false otherwise

*/

static Boolean
is_contiguous_mesg(Mesg_data *m)
{
  int i;

  /* skip 1st dimension, since that's contiguous in Fortran */
  /* all other dimensions must be 1 wide only               */

  for (i = 1; i < m->numdim; i++)
  {
    if (m->sym[i] && m->extent[i])
      return false;
    if (m->sym_lower[i] || m->sym_upper[i])
      return false;
    if (m->upper[i] != m->lower[i])
      return false;
  }

  return true;
}


/*---------------------------------------------------------------------

  dc_max_procs()  

*/

static int 
dc_max_procs(Dist_Globals *dh)
{
  int  maxproc, i;

  maxproc = dh->rs_parts[0][0].numprocs;

  for (i = 1; i < DC_MAXDIM; ++i)
  {
    if (maxproc < dh->rs_parts[0][i].numprocs)
      maxproc = dh->rs_parts[0][i].numprocs;
  }
  return (maxproc + 1);
}


/*-----------------------------------------------------------------------

  make_multiple()      Returns AST for (expr*i) + 1

*/

static AST_INDEX
make_multiple(AST_INDEX expr, int i)
{
  AST_INDEX one;

  one = pt_gen_int(1);

  if (!i)
     return one;

  if (i == 1)
     return pt_gen_add(tree_copy(expr), one);

  return pt_gen_add(pt_gen_mul(tree_copy(expr),pt_gen_int(i)), one);
}


/*-----------------------------------------------------------------------

  make_buffer_name()      Choose name for buffer routine

*/

static char*
make_buffer_name(Dist_Globals *dh, Rsd_set *rset, Boolean pack)
 /* Boolean pack: true = buffer data, false = unbuffer data */
{
  char buf[MAXCOMMENT];
  char *name, *type;

  name = pack ? BUF_NAME : UNBUF_NAME;

  switch (rset->sp->fform)
  {
    case INTTYPE:
      type = "i";
      break;

    case DOUBLE_P:
      type = "dp";
      break;

    case REAL:
    default:
      type = "r";
      break;
  }

  sprintf(buf, "%s%dD$%s", name, rset->sp->numdim, type);

  name = dc_get_mem(dh, strlen(buf) + 1);
  strcpy(name, buf);
  return name;
}


/*-----------------------------------------------------------------------

  make_buffer_contig_name()      Choose name for buffer routine
                                 for contiguous data
*/

static char*
make_buffer_contig_name(Dist_Globals *dh, Rsd_set *rset)
{
  char buf[MAXCOMMENT];
  char *name, *type;

  switch (rset->sp->fform)
  {
    case INTTYPE:
      type = "i";
      break;

    case DOUBLE_P:
      type = "dp";
      break;

    case REAL:
    default:
      type = "r";
      break;
  }

  sprintf(buf, "%s$%s", BUF_NAME, type);

  name = dc_get_mem(dh, strlen(buf) + 1);
  strcpy(name, buf);
  return name;
}


/*--------------------------------------------------------------------

  alloc_buf()    Set up AST for next inactive (free) buffer

  Returns:  AST for buffer

*/

static AST_INDEX
alloc_buf(Buf_info *buf, int **bufsize_ptr, Boolean zero)
{
  char buffer[MAX_NAME];
  int i;

  if (zero)  /* use standard temp buffer for csend/crecv */
  {
    *bufsize_ptr = buf->size;
    return pt_gen_ident(buf->name);
  }

  for (i = 1; i < MAXFDBUF; i++)
  {
    if (!buf->size[i] || NOT(buf->active[i]))
      break;
  }
  if (i == MAXFDBUF)
    printf("alloc_buf(): Too many buffers\n");

  buf->active[i] = true;

  if (buf->num < i)      /* new buffer must be allocated */
    buf->num = i;

  *bufsize_ptr = buf->size+i;     /* pass back pointer to buf size */

  sprintf(buffer, "%s%d", buf->name, i);
  return pt_gen_ident(buffer);
}


