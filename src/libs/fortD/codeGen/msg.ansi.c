/* $Id: msg.ansi.c,v 1.29 1997/03/11 14:28:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    msg.c        Contains analysis needed for generating messages  
                 for nonlocal RSDs after loops have been partitioned

    Author : Seema Hiranandani 

    Currently message are instantiated one at a time, each modifying
    the program AST.  This approach may cause messages to interfere
    with one another unnecessarily.  Even worse, messages are built
    piece-wise, with pieces stored in the global Dist_Globals struct.

    The next version of the Fortran D compiler needs to produce an
    abstract object that encapsulates the AST generated for each 
    message.  Only after all messages have been constructed should
    their ASTs be inserted in the actual program AST.

*/


#include <libs/fortD/codeGen/private_dc.h>
#include <math.h>

/* #include <libs/fortD/performance/staticInfo/MesgStaticSDDF.h>  For SDDF instr. */

/*------------------------- extern definitions ------------------------*/

EXTERN(void, GetMessageParams,(Dist_Globals *dh,
			       AST_INDEX loop,	/* associated loop */
			       Rsd_set *rset));	/* RSD(s) sent in msg */

EXTERN(void, dc_compute_proc_range,(PedInfo, int*, int*, Rsd_set*, Iter_type));
EXTERN(void, dc_boundary_msgs,(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset));
EXTERN(void, dc_collective_msgs,(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset));
EXTERN(void, dc_irreg_msgs,(Generic irr, AST_INDEX loop, int num_subs, AST_INDEX *subs));
EXTERN(void, dc_reduction_msgs,(Dist_Globals *dh, AST_INDEX loop, int level));
EXTERN(int, *compute_send_data,(Dist_Globals *dh, Rsd_set *rset));
EXTERN(void, compute_recv_data,(Dist_Globals *dh, Rsd_set *rset, Iter_type msg_type,
                                AST_INDEX loop));
EXTERN(void, dc_send_recv_msgs,(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset));
EXTERN(AST_INDEX, dc_vector_pipe_send,(Dist_Globals *dh, Rsd_set *rset, 
                                       AST_INDEX start));
EXTERN(AST_INDEX, dc_vector_pipe_recv,(Dist_Globals *dh, Rsd_set *rset, 
                                       AST_INDEX start));
EXTERN(AST_INDEX, dc_cgp_loop,(Dist_Globals *dh, AST_INDEX loop, int pipe_idx, 
                               Rsd_set **pipe));
EXTERN(void, dc_cgp_clear,(Dist_Globals *dh));
EXTERN(void, decomp_local_bounds,(SNODE *sp, int dim, int *dupper, int *dlower));
EXTERN(AST_INDEX, dc_ins_send_sync,(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start));

/*------------------------- global definitions ------------------------*/

void dc_messages(Dist_Globals *dh);
void print_rsds(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset, Iter_type msg_type);
void init_msg_info(Dist_Globals *dh);
void compute_procs(Dist_Globals *dh, int numdim);          
void compute_proc_offsets(Dist_Globals *dh, Rsd_set *rset, Rsd_section *rsd_sect);
void insert_indep(Dist_Globals *dj, AST_INDEX loop, Iter_type msg_type, Rsd_set *rset);
AST_INDEX ast_get_sub(AST_INDEX subs, int dim);
AST_INDEX dc_get_sgroup_if(Dist_Globals *dh, AST_INDEX loop);

/*------------------------- local definitions ------------------------*/

STATIC(void, dc_rset_msgs,(Dist_Globals *dh, AST_INDEX loop, 
                           Rsd_set_info *rsd_loop_list));
STATIC(void, finish_local_range,(Dist_Globals *dh, int _local_dim, int maxproc));

STATIC(void, compute_num_nodes,(Dist_Globals *dh, int num_dim));

STATIC(void, perform_shift_comm,(Dist_Globals *dh, AST_INDEX loop, 
                                 Rsd_section *rsd_sect, Rsd_set *rset, 
                                 Iter_type msg_type));
STATIC(void, do_positive_range,(Dist_Globals *dh, int i, int l2, int u2, int bk1,
                                SNODE *sp));
STATIC(void, do_negative_range,(Dist_Globals *dh, int i, int l2, int u2, int bk1));
STATIC(void, do_negative_proc_offsets,(Dist_Globals *dh, int i, int blocksize,
                                       int start_pos, int end_pos));
STATIC(void, do_positive_proc_offsets,(Dist_Globals *dh, int i, int blocksize,
                                       int start_pos, int end_pos, SNODE *sp));
STATIC(void, do_pos_recv_grd,(Dist_Globals *dh, AST_INDEX loop, Iter_type msg_type));

STATIC(void, do_inverse_negative,(Dist_Globals *dh, int i, int start_pos, int bk1,
                                  int end_pos));
STATIC(void, do_inverse_neg_offsets,(Dist_Globals *dh, int start_pos, int end_pos, 
                                     int i));
STATIC(void, do_inverse_positive,(Dist_Globals *dh, int i, int start_pos, int end_pos,
                                  int bk1));
STATIC(void, do_inverse_pos_offsets,(Dist_Globals *dh, int start_pos, int end_pos,
                                     int i));
STATIC(void, do_local_range,(Dist_Globals *dh, int i, int l2, int u2));

STATIC(void, insert_carried_all,(Dist_Globals *dh, AST_INDEX loop, Iter_type msg_type,
                                 Rsd_set *rset));
STATIC(void, insert_carried_part,(Dist_Globals *dh, AST_INDEX loop, Iter_type msg_type));
STATIC(AST_INDEX, get_sym_bound,(Rsd_set *rset, int dim, Boolean upper));

/*--------------------------------------------------------------------

  dc_message()   Main routine to generate explicit communications

  Use the regular section information in the side array to produce 
  the communications needed before loops. RSD information was 
  generated by dc_comm() in rsd_sec.c.

*/

void
dc_messages(Dist_Globals *dh)
{
  Rsd_set_info *rsd_loop_list;
  AST_INDEX loop, node;
  int i, j;
  FortD_LI *fli;

  /*----------------------------------------------*/
  /* iterate over all statement groups in program */

  for (i = dh->sgroup.group_num - 1; i >= 0; i--)
  {
    if (rsd_loop_list = dh->sgroup.groups[i]->rset)
    {

/***** hard to tell what's free, for now do not reuse buffers **

      for (j = 1; j < MAXFDBUF; j++)
      {
        dh->bufs.i.active[j] = false;
        dh->bufs.r.active[j] = false;
        dh->bufs.dp.active[j] = false;
      }
*****/

      dc_rset_msgs(dh, dh->sgroup.groups[i]->stmt[0], rsd_loop_list);
    }
  }

  /*-----------------------------------*/
  /* iterate over all loops in program */

  for (i = dh->numdoloops - 1; i >= 0; i--)
  {
    loop = dh->doloops[i];

/***** hard to tell what's free, for now do not reuse buffers **

    if (loop_level(loop) == 1)
    {
      for (j = 1; j < MAXFDBUF; j++)
      {
        dh->bufs.i.active[j] = false;
        dh->bufs.r.active[j] = false;
        dh->bufs.dp.active[j] = false;
      }
    }
*****/

    dh->head_send = AST_NIL;
    dh->head_recv = AST_NIL;

    fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
    dh->loop_first = (fli->init == AST_NIL) ?
      list_first(gen_DO_get_stmt_LIST(loop)) : list_next(fli->init);

    /* loop-carried messages at level of loop */

    rsd_loop_list = (Rsd_set_info *) get_info(dh->ped, loop, type_dc);
    if (rsd_loop_list != (Rsd_set_info *) NO_DC_INFO)
    {
      dc_rset_msgs(dh, loop, rsd_loop_list);
    }

    /* loop-independent messages at deepest level, in body of loop */
    for (j = 0; j < fli->sgroup.group_num; j++)
    {
      if (rsd_loop_list = fli->sgroup.groups[j]->rset)
      {
        if (!j)
          printf("dc_messages(): unexpected comm at 1st stmt group\n");

        /* location to insert communication */
        node = fli->sgroup.groups[j]->guard;
        if (node == AST_NIL)
          node = fli->sgroup.groups[j]->stmt[0];
        dc_rset_msgs(dh, node, rsd_loop_list);
      }
    }

    /* insert sends & irecvs at head of loop */

    node = gen_DO_get_stmt_LIST(loop);
    if (dh->head_send != AST_NIL)
      (void) list_insert_first(node, dh->head_send);
    if (dh->head_recv != AST_NIL)
      (void) list_insert_first(node, dh->head_recv);
  }
}


/*--------------------------------------------------------------------

  print_rsds()   Insert explicit messages in program for RSDs

  Prints out the regular sections data into the program and based 
  on that then calls the function to split up the rsds among the
  processors

  Printing the regular sections means producing the code for
  communication between processors.

  If MESG_INDEP            insert send/recv before loop 
     MESG_CARRIED_ALL      insert send/recv inside loop
     MESG_CARRIED_PART     insert recv before loop & send after loop 
                           (i.e., pipelined computation)

  Examples of Iter_simple code when guards are present:

     MESG_INDEP      MESG_CARRIED_ALL     MESG_CARRIED_PART

        send()           [no if]            if
        if               do                   recv()
           recv()          send()             do
           do              recv()               <body>
             <body>        <body>             enddo
           enddo         enddo              endif
        endif                               send()


  Parameters:

    dh - structure containing all compiler information   
     
    loop - AST index of loop for which communication is produced  
 
    rset - RSD set information about right-hand reference
           within assignment statements inside loop   
 
    msgtype -  checks for boundary conditions                      

*/

void 
print_rsds(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset, Iter_type msg_type)
{
  Rsd_section *rsd_sect; /* regular section info, part of side array info */

  /* Prepare the dh for the next communication. */

  init_msg_info(dh);
  dh->send_stmts = AST_NIL;
  dh->recv_stmts = AST_NIL;
  dh->send_sync = AST_NIL;
  dh->recv_sync = AST_NIL;
  dh->msg_id = AST_NIL;

  /* choose rsd structure depending on the boundary conditions */

  switch (msg_type)
  {
    case Iter_simple:
    case Iter_pre:
    case Iter_post:
    case Iter_mid_only:
      rsd_sect = rset->rs;
      break;

    case Iter_post_only:
      rsd_sect = rset->rs_post;
      break;

    case Iter_pre_only:
      rsd_sect = rset->rs_pre;
      break;

    default:
      printf("print_rsds(): message type not handled\n");
      return;
  }

  if (rset->ctype == FD_COMM_SHIFT)
  {
    perform_shift_comm(dh, loop, rsd_sect, rset, msg_type);

    /* Send & recv statements now in dh->send_stmts & dh->recv_stmts */
    /* determine where to insert them & add guards if needed         */

    if (dh->send_stmts != AST_NIL)
    {
      switch (rset->mtype)
      {
      case FD_MESG_INDEP:
        insert_indep(dh, loop, msg_type, rset);
        break;

      case FD_MESG_CARRIED_ALL:
        insert_carried_all(dh, loop, msg_type, rset);
        break;

      case FD_MESG_CARRIED_PART:
        insert_carried_part(dh, loop, msg_type);
        break;
      }

      GetMessageParams(dh, loop, rset); /* 6/17/94, VSA: Make static SDDF */

      dh->send_stmts = AST_NIL;
      dh->recv_stmts = AST_NIL;
      dh->send_sync = AST_NIL;
      dh->recv_sync = AST_NIL;
      dh->msg_id = AST_NIL;
    }
  }
  else
  {
     printf("print_rsds(): Comm type not yet supported\n");
  }

}

/*--------------------------------------------------------------------

  insert_indep()  Insert message for MESG_INDEP rsd

*/

void
insert_indep(Dist_Globals *dh, AST_INDEX loop, Iter_type msg_type, Rsd_set *rset)
{
    AST_INDEX ifstmt, guard, body, location, node;

    ifstmt = dc_get_sgroup_if(dh, loop);

    if (is_if(ifstmt))
    {
      if (dh->opts.vect_mesg_pipe)
        location = dc_vector_pipe_send(dh, rset, ifstmt);
      else 
        location = ifstmt;

      (void) list_insert_before(location, dh->send_stmts);

      guard = list_first(gen_IF_get_guard_LIST(ifstmt)); /* guard */

      if (msg_type == Iter_simple)
      {
        /* put recv stmts inside existing guard */

        body = gen_GUARD_get_stmt_LIST(guard);
        (void) list_insert_first(body, dh->recv_stmts);
        /* gen_GUARD_put_stmt_LIST(guard, body); */

      }
      else 
      {
        (void) list_insert_before(ifstmt, dh->recv_stmts);
      }
    }

    /* If a mask does not exist, then simply place the send  */
    /* communication out of the loop.                        */

    else
    {
      if (dh->opts.vect_mesg_pipe)
        location = dc_vector_pipe_send(dh, rset, loop);
      else 
        location = loop;

      (void) list_insert_before(location, dh->send_stmts);
      do_pos_recv_grd(dh, loop, msg_type);
    }

    /* place synchronization for unbuffered messages */

    location = is_if(ifstmt) ? ifstmt : list_prev(loop);

    if (dh->send_sync != AST_NIL)
    {
      node = dc_ins_send_sync(dh, rset, location);
      (void) list_insert_after(node, dh->send_sync);
    }

    if (dh->recv_sync != AST_NIL)
    {
      node = dc_ins_recv_sync(dh, rset, location);
      (void) list_insert_before(node, dh->recv_sync);
    }
}


/*--------------------------------------------------------------------

  dc_get_sgroup_if()  Get IF stmt for statement group, if any

*/

AST_INDEX 
dc_get_sgroup_if(Dist_Globals *dh, AST_INDEX loop)
{
    FortD_LI *fli;
    S_group *sgroup;
    AST_INDEX ifstmt;

    if (is_loop(loop))
    {
      fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
      ifstmt = (fli == (FortD_LI *) NO_FD_INFO) ? AST_NIL : fli->guard;
    }
    else
    {
      sgroup = (S_group *) get_info(dh->ped, loop, type_fd);
      ifstmt = (sgroup == (S_group *) NO_FD_INFO) ? AST_NIL : sgroup->guard;
    }

    return ifstmt;
}


/********************************************************************/
/* This function goes through the data structure that stores the    */
/* processor offsets and computes the number of processors          */
/* communicated with for each dimension of the global array         */
/********************************************************************/
void 
compute_procs(Dist_Globals *dh, int numdim)
{
  int i, j, proc, total_procs;

  total_procs = 1;

  for (proc = 0; proc < MAX_PROC; ++proc)
  {
    for (i = 0; i < numdim; i++)
    {
      for (j = 0; j < MAX_PROC; ++j)
      {
        if (dh->rs_parts[proc][i].proc_offset[j])
          ++dh->rs_parts[proc][i].numprocs;
      }
    }
  }


/* compute the total number of procs using the following formula */
/* fact(numprocs)/fact(1)*fact(numprocs-1)-1           */

  for (proc = 0; proc < MAX_PROC; ++proc)
  {
    for (i = 0; i < numdim; i++)
    {
      if (dh->rs_parts[proc][i].numprocs)
        total_procs = total_procs * (dh->rs_parts[proc][i].numprocs + 1);

/*      (fact(dh->rs_parts[proc][i].numprocs+1)/
      (numdim * (fact(dh->rs_parts[proc][i].numprocs+1) - numdim)))
          - 1;
*/

    }
    dh->rs_parts[proc][0].total_procs = total_procs - 1;
    total_procs = 1;
  }
}


/*-----------------------------------------------------------------------

  compute_proc_offsets()  

  Computes the processors that each processor needs to receive
  data from.  It's all stored as processor offsets in dh->rs_parts[][].
  Also computes the data that needs to be sent and received by the
  processors

  Symbolic dimensions are assumed not to cause communication,
  they can be used as is for message generation.  No offsets
  are needed, just the symbolic range in that dimension.

*/

void 
compute_proc_offsets(Dist_Globals *dh, Rsd_set *rset, Rsd_section *rsd_sect)
{
  int i, bk1, k, u2, l2, u22, l22, start_pos;
  Boolean local_dim[DC_MAXDIM];
  SNODE *sp;
  AST_INDEX sub;

  sp = rset->sp;

  for (i = 0; i < DC_MAXDIM; ++i)
    local_dim[i] = false;

  for (i = 0; i < sp->numdim; ++i)
  {
    /*-------------------------------------*/
    /* check whether dimension is symbolic */

    if (rset->rs_carried) 
    {
      switch (rset->rs_carried->subs[i].type)
      {
        case RSD_RANGE:
        case RSD_CONSTANT:
          break;         /* not symbolic dimension */

        default:
          printf("compute_proc_offsets(): Unknown RSD type\n");
          /* fall through, treat as EXPR */

        case RSD_EXPR:
          /* found symbolic expression, extract from original subscript */
          sub = ast_get_sub(rset->subs[0], i);

          /* if subscript matches that of coarse-grain pipelined loop */

          if (dh->cgp_grain && 
              pt_find_var_node(sub, gen_get_text(dh->cgp_ivar)))
          {
            sub = tree_copy(sub);
            if (ast_equiv(sub, dh->cgp_ivar))
              sub = tree_copy(dh->cgp_ivar2);
            else
              pt_var_replace(sub, gen_get_text(dh->cgp_ivar), dh->cgp_ivar2);

            dh->rs_parts[dh->proc][i].sym[0] = sub;   
            dh->rs_parts[dh->proc][i].extent[0] = tree_copy(dh->cgp_up);
          }
          else
          {
            dh->rs_parts[dh->proc][i].sym[0] = sub;             
            dh->rs_parts[dh->proc][i].extent[0] = AST_NIL;
          }
          continue;      /* go on to next dimension */
       }
    }

    /*----------------------------------------------------*/
    /* dimension is constant, calculate processor offsets */

    if ((u2 = dc_rsd_upper(rsd_sect, i)) == MAXINT)
      dh->rs_parts[dh->proc][i].sym_upper[0] = get_sym_bound(rset,i,true);

    if ((l2 = dc_rsd_lower(rsd_sect, i)) == MININT)
      dh->rs_parts[dh->proc][i].sym_lower[0] = get_sym_bound(rset,i,false);

    switch (sp_is_part(sp, i))
    {
      case FD_DIST_BLOCK:

        /* this is again a special case. */

        bk1 = sp_block_size1(sp, i);

        /* error checking mechanism */
        if (!bk1)
          die_with_message("compute_proc_offsets(): bk1 = 0");

        /*----------------------------------------------------*/
        /* if broadcast, convert to local indices  */

        if (rset->ctype == FD_COMM_BCAST)
        {
          l22 = ((l2-1)%bk1)+1;
          u22 = ((u2-1)%bk1)+1;

          for (k = 0; k <= dh->num_dim_nodes[i]; ++k)
          {
            dh->rs_parts[dh->proc][i].lower[k] = l2; 
            dh->rs_parts[dh->proc][i].upper[k] = u2;
            dh->rs_parts[dh->proc][i].inv_lower[k] = l22;
            dh->rs_parts[dh->proc][i].inv_upper[k] = u22;
            dh->rs_parts[dh->proc][i].inv_proc_offset[k] = 1;
          }
        }

        /*----------------------------------------------------*/
        /* if shift, compute nonlocal data needed */

        /* case 1 : lower and upper are negative        */
        /* case 2 : lower and upper are positive       */
        /* case 3 : lower is negative and upper is positive */

        else if ((rset->ctype == FD_COMM_SHIFT) || 
                 (rset->ctype == FD_COMM_SEND_RECV))
        {
          switch (dc_range_type(l2, u2))
          {
          case NEG_NEG:
            do_negative_range(dh, i, l2, u2, bk1);
            do_negative_proc_offsets(dh, i, bk1, 0, dh->num_dim_nodes[i]);

            do_inverse_negative(dh, i, 0, dh->num_dim_nodes[i], bk1);
            do_inverse_neg_offsets(dh, 0, dh->num_dim_nodes[i], i);
            break;

          case POS_POS:
            do_positive_range(dh, i, l2, u2, bk1, sp);
            do_inverse_positive(dh, i, 0, dh->num_dim_nodes[i], bk1);

            do_positive_proc_offsets(dh, i, bk1, 0, dh->num_dim_nodes[i], sp);
            do_inverse_pos_offsets(dh, 0, dh->num_dim_nodes[i], i);
            break;

          case NEG_POS:
            do_negative_range(dh, i, l2, 0, bk1);
            do_inverse_negative(dh, i, 0, dh->num_dim_nodes[i], bk1);

            do_negative_proc_offsets(dh, i, bk1, 0, dh->num_dim_nodes[i]);
            do_inverse_neg_offsets(dh, 0, dh->num_dim_nodes[i], i);

            start_pos = dh->num_dim_nodes[i];

            do_positive_range(dh, i, 1, u2, bk1, sp);
            do_inverse_positive(dh, i, start_pos, dh->num_dim_nodes[i], bk1);

            do_positive_proc_offsets(dh, i, bk1, start_pos,
                                           dh->num_dim_nodes[i], sp);
            do_inverse_pos_offsets(dh, start_pos, dh->num_dim_nodes[i], i);
            break;
          }
        }
        else
          printf("compute_proc_offsets(): unsupported comm type for block\n");

        break;

      case FD_DIST_LOCAL:
        local_dim[i] = true;
        do_local_range(dh, i, l2, u2);
        break;

      case FD_DIST_CYCLIC:
        if (rset->ctype == FD_COMM_SHIFT) 
          printf("compute_proc_offsets(): unsupported comm type for cyclic\n");
        break;

      default:
        break;
    }
  }

  compute_num_nodes(dh, sp->numdim);

}

/*-----------------------------------------------------------------------

  init_msg_info()  initialize the message info structures

*/

void 
init_msg_info(Dist_Globals *dh)
{
  int i;

  dh->num_nodes = 0;
  dh->proc = 0;

  for (i = 0; i < DC_MAXDIM; i++)
    dh->num_dim_nodes[i] = 0;

  bzero(dh->rs_parts, sizeof(RSD_PARTS) * MAX_PROC * DC_MAXDIM);

  dh->send_stmts = AST_NIL;
  dh->recv_stmts = AST_NIL;
  dh->buftype = AST_NIL;
  dh->bufpoint = AST_NIL;
  dh->send_sync = AST_NIL;
  dh->recv_sync = AST_NIL;
  dh->msg_id = AST_NIL;
}


/*-----------------------------------------------------------------------

  ast_get_sub()   Extract the AST for the subscript expression in dim

*/

AST_INDEX
ast_get_sub(AST_INDEX subs, int dim)
{
  int i = 0;

  if (!is_subscript(subs))
    die_with_message("ast_get_sub(): arg not subscript\n");

  subs = list_first(gen_SUBSCRIPT_get_rvalue_LIST(subs));
  while (i++ != dim)
  {
    subs = list_next(subs);
    if (subs == AST_NIL)
      die_with_message("ast_get_sub(): too few subscripts\n");
  }

  return subs;
}


/*******************/
/* local functions */
/*******************/

/*--------------------------------------------------------------------

  dc_rset_msgs()   Generate explicit communications for one RSD list

  Use the regular section information in the side array to produce 
  the communications needed before loops. RSD information was 
  generated by dc_comm() in rsd_sec.c.

*/

static void
dc_rset_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set_info *rsd_loop_list)
{
  int i, j, k;          /* loop variables */
  Boolean simple_mesg;
  Rsd_set *rset;     /* RSD info for one reference in rsd_loop_list */
  Rsd_set *shift[MAXREF];
  Rsd_set *pipe[MAXREF];
  Rsd_set *collec[MAXREF];
  Rsd_set *sendrecv[MAXREF];
  Rsd_set *irreg[MAXREF];     
  int shift_idx, pipe_idx, collec_idx, sendrecv_idx, irreg_idx;

  /*-----------------------------------------------------*/
  /* collect & classify all RSDs tagged at this RSD list */

  shift_idx = pipe_idx = collec_idx = sendrecv_idx = irreg_idx = 0;

  /* loop through RSDs for all variables */
  for (j = rsd_loop_list->num_ref - 1; j >= 0; j--)
  {
    /* loop through all RSDs for one variable */
    for (rset = rsd_loop_list->rsd_s[j]; rset; rset = rset->rsd_next)
    {
      switch (rset->ctype)
      {
      case FD_COMM_SHIFT:
        if (rset->mtype == FD_MESG_CARRIED_PART)
          pipe[pipe_idx++] = rset;
        else
          shift[shift_idx++] = rset;
        break;

      case FD_COMM_BCAST:
      case FD_COMM_TRANSPOSE:
      case FD_COMM_REDUCE:
        collec[collec_idx++] = rset;
        break;

      case FD_COMM_SEND_RECV:
        sendrecv[sendrecv_idx++] = rset;
        break;

      default:
        irreg[irreg_idx++] = rset;
        break;
      }
    }
  }

  /*------------------------------------------------*/
  /* generate messages for RSDs tagged at this loop */

  /*-------------------------------*/
  /* 0: do runtime processing */
  
  for (j = 0; j < irreg_idx; j++)
    dc_irreg_msgs(dh->irr, loop, irreg[j]->num_subs, irreg[j]->subs);

  /*-----------------------*/
  /* 1: do reductions */

  if (is_loop(loop) && (loop_level(loop) == 1))
    dc_reduction_msgs(dh, loop, LEVEL1);

  /*--------------------------------------*/
  /* 2: do collective communications */
  
  for (j = 0; j < collec_idx; j++)
    dc_collective_msgs(dh, loop, collec[j]);
  
  /*-----------------------------------------*/
  /* 3 : do point to point communication */

  for (j = 0; j < sendrecv_idx; j++)
    dc_send_recv_msgs(dh, loop, sendrecv[j]);

  /*--------------------------------*/
  /* 4: do parallel shifts */
  
  for (j = 0; j < shift_idx; j++)
  {
    rset = shift[j];

    simple_mesg = true;
    for (k = 0; k < MAXLOOP; k++)  /* check boundary conditions */
    {
      if ((rset->iterset->type[k] != Iter_simple) &&
          (rset->iterset->type[k] != Iter_pre_only) &&
          (rset->iterset->type[k] != Iter_post_only))
        simple_mesg = false;
    }
    if (simple_mesg)
      print_rsds(dh, loop, rset, Iter_simple);
    else
      dc_boundary_msgs(dh, loop, rset);
  }

  /*--------------------------------*/
  /* 5: do pipelined messages */

  /* if appropriate, strip-mine outer loop to increase   */
  /* granularity of pipelining (coarse-grain pipelining) */

  if (pipe_idx)
    loop = dc_cgp_loop(dh, loop, pipe_idx, pipe);

  /* insert messages outside of newly created iterator loop  */

  for (j = 0; j < pipe_idx; j++)
  {
    rset = pipe[j];

    simple_mesg = true;
    for (k = 0; k < MAXLOOP; k++)  /* check boundary conditions */
    {
      if ((rset->iterset->type[k] != Iter_simple) &&
          (rset->iterset->type[k] != Iter_pre_only) &&
          (rset->iterset->type[k] != Iter_post_only))
        simple_mesg = false;
    }
    if (simple_mesg)
      print_rsds(dh, loop, rset, Iter_simple);
    else
      dc_boundary_msgs(dh, loop, rset);
  }

  if (pipe_idx)
    dc_cgp_clear(dh);  /* finished coarse-grain pipelining */
}


/*--------------------------------------------------------------------

  insert_carried_all()  Insert message for MESG_CARRIED_ALL rsd

*/

static void
insert_carried_all(Dist_Globals *dh, AST_INDEX loop, Iter_type msg_type, Rsd_set *rset)
{
    AST_INDEX ifstmt, body, location, node;
    FortD_LI *fli;

    fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
    ifstmt = (fli == (FortD_LI *) NO_FD_INFO) ? AST_NIL : fli->guard;

    if (ifstmt != AST_NIL)
    {
      printf("insert_carried_all(): unexpected IF, no message generated\n");
      return;
    }

    if (dh->opts.vect_mesg_pipe)
    {
      location = dc_vector_pipe_recv(dh, rset, dh->loop_first);
      list_insert_before(location, dh->recv_stmts); 

      /* update dh->loop_first if necessary  */

      if (location == dh->loop_first)
        dh->loop_first = list_first(dh->recv_stmts);

      if (dh->head_send == AST_NIL)
        dh->head_send = dh->send_stmts;
      else
        (void) list_insert_first(dh->head_send, dh->send_stmts);

      if (dh->recv_sync != AST_NIL)
      {
        if (dh->head_recv == AST_NIL)
          dh->head_recv = dh->recv_sync;
        else
          (void) list_insert_last(dh->head_recv, dh->recv_sync);
      }
    }
    else
    {
      body = gen_DO_get_stmt_LIST(loop);               /* loop body */
      body = list_insert_first(body, dh->recv_stmts);  /* add recv  */
      body = list_insert_first(body, dh->send_stmts);  /* add send  */
      if (dh->recv_sync != AST_NIL)
        (void) list_insert_first(body, dh->recv_sync);
    }

    /* update location of first stmt in loop */
    dh->loop_first = list_first(gen_DO_get_stmt_LIST(loop));

    /* place synchronization for unbuffered messages */

    if (dh->send_sync != AST_NIL)
    {
      node = dc_ins_send_sync(dh, rset, dh->loop_first);
      (void) list_insert_after(node, dh->send_sync);
    }

}

/*--------------------------------------------------------------------

  insert_carried_part()  Insert message for MESG_CARRIED_PART rsd

*/

static void
insert_carried_part(Dist_Globals *dh, AST_INDEX loop, Iter_type msg_type)
{
    (void) list_insert_after(loop, dh->send_stmts);
    do_pos_recv_grd(dh, loop, msg_type);
}


/*---------------------------------------------------------------

  do_pos_recv_grd()   Insert recv statement

  find mask inside loop and use it to guard the call to crecv()

*/

static void
do_pos_recv_grd(Dist_Globals *dh, AST_INDEX loop, Iter_type msg_type)
{
  AST_INDEX ifstmt, guard, body, place_holder;
  FortD_LI *fli;

  fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
  ifstmt = (fli == (FortD_LI *) NO_FD_INFO) ? AST_NIL : fli->guard;

  if ((msg_type != Iter_simple) || (ifstmt == AST_NIL))
  {
    list_insert_before(loop, dh->recv_stmts);
    return;
  }

  /*------------------------------*/
  /* move IF stmt outside of loop */

  /* get body of IF, take it out  */
  guard = gen_IF_get_guard_LIST(ifstmt);
  guard = list_first(guard);
  body = gen_GUARD_get_stmt_LIST(guard);
  gen_GUARD_put_stmt_LIST(guard, list_create(AST_NIL));

  /* take out IF, replace with body of IF */
  tree_replace(ifstmt, body);

  /* set up place holder, then remove loop    */
  place_holder = list_prev(loop);
  list_remove_node(loop);

  /* make up new body consisting of min/max, crecv, buffer, & loop */

  body = list_create(loop);
  list_insert_before(loop, dh->recv_stmts);

  /* insert new body into IF, then put IF where place holder is   */
  gen_GUARD_put_stmt_LIST(guard, body);
  list_insert_after(place_holder, ifstmt);
}


/*---------------------------------------------------------------

  perform_shift_comm()     Do communication for simple shifts

     In this case the bounds are constant and the partitions are    
     are the same. Thus a simple division of the rsd can be done at 
     compile time and the send/receive statements can be put in     
                                                                    
     rsd_sect - part of regular section info from side array        
         calculated during analysis                                 
                                                                    
     Assume that all RSDs that reach here require communication,    
       those that don't have been discarded by dc_rsd_one_section() 
                                                                    
     Produce communication for each right-hand side array           
     reference. The code is placed in dh->send_stmts.               

*/

static void 
perform_shift_comm(Dist_Globals *dh, AST_INDEX loop, Rsd_section *rsd_sect, 
                   Rsd_set *rset, Iter_type msg_type)
{
  dc_compute_proc_range(dh->ped, dh->min, dh->max, rset, msg_type);
  init_msg_info(dh);
  compute_proc_offsets(dh, rset, rsd_sect);
  compute_procs(dh, rset->sp->numdim);
  (void) compute_send_data(dh, rset);
  compute_recv_data(dh, rset, msg_type, AST_NIL);
}


/*-----------------------------------------------------------------------

  do_negative_range()  

  Compute RSD(s) for nonlocal data that must be received.

  Chop off local section, then break up remaining sections
  into pieces, each owned by a single processor.

  Data is from LEFT since both bounds of RSD are negative.

*/

static void 
do_negative_range(Dist_Globals *dh, int i, int l2, int u2, int bk1)
{
  int j = dh->num_dim_nodes[i];
  int proc, diff;

  proc = dh->proc;

  if (l2 == u2)  /* catches l2 == u2 == 0 */
  {
    dh->rs_parts[proc][i].upper[j] = u2;
    dh->rs_parts[proc][i].lower[j] = l2;
    dh->num_dim_nodes[i] = j + 1;
    return;
  }

  /* calculate boundary condition for nonlocal data       */
  /* belonging to processor furthest LEFT, if it exists   */

  if (diff = abs(l2 - 1) % bk1)
  {
    if ((l2 + abs(l2) % bk1 - 1) < u2 - 1)
      dh->rs_parts[proc][i].upper[j] = l2 + diff - 1;
    else
      dh->rs_parts[proc][i].upper[j] = u2;

    dh->rs_parts[proc][i].lower[j] = l2;
    l2 =  dh->rs_parts[proc][i].upper[j] + 1;
/*   l2 += abs(l2) % bk1; seema 3/8/94 */
   j++;
  }

  /* calculate nonlocal data belonging to other processors */

  while (l2 < u2)
  {
    dh->rs_parts[proc][i].lower[j] = l2;

    if (l2 + bk1 - 1 >= u2)
      dh->rs_parts[proc][i].upper[j] = u2;
    else
      dh->rs_parts[proc][i].upper[j] = l2 + bk1;

    l2 += bk1;
    ++j;
  }

  dh->num_dim_nodes[i] = j;
}


/*-----------------------------------------------------------------------

  do_positive_range()  

  Compute RSD(s) for nonlocal data that must be received.

  Chop off local section, then break up remaining sections
  into pieces, each owned by a single processor.

  Data is from RIGHT since both bounds of RSD are positive.

*/

static void 
do_positive_range(Dist_Globals *dh, int i, int l2, int u2, int bk1, SNODE *sp)
{
  int dlower, dupper, offset, proc, diff;
  int j = dh->num_dim_nodes[i];

  proc = dh->proc;
  decomp_local_bounds(sp, i, &dupper, &dlower);

 /* check if any non local section exists */
  if (dupper >= u2 ) return;

  if (l2 == u2)   /* catches l2 == u2 == 0 */
  {
    dh->rs_parts[proc][i].lower[j] = l2;
    dh->rs_parts[proc][i].upper[j] = u2;
    dh->num_dim_nodes[i] = j + 1;
    return;
  }

  /* get the bounds of the decomposition */


  offset = dlower - 1;

  /* first bump lower bound up to closest location for nonlocal data */

  if (l2 <= dupper)
    l2 = dupper + 1;

  /* calculate boundary condition for nonlocal data       */
  /* belonging to processor furthest RIGHT, if it exists  */

  if (diff = (u2 - dupper) % bk1)
  {
    dh->rs_parts[proc][i].upper[j] = u2;

    if (l2 > (u2 - diff))
      dh->rs_parts[proc][i].lower[j] = l2;
    else
      dh->rs_parts[proc][i].lower[j] = u2 - diff;

    u2 = dh->rs_parts[proc][i].lower[j] - 1;

    j++;
  }

  /* calculate nonlocal data belonging to other processors */

  while (u2 >= l2)
  {
    dh->rs_parts[proc][i].upper[j] = u2;

    if (u2 - bk1 < l2)
      dh->rs_parts[proc][i].lower[j] = l2;
    else
      dh->rs_parts[proc][i].lower[j] = u2 - bk1 + 1;
    u2 -= bk1;
    ++j;
  }

  dh->num_dim_nodes[i] = j;
}



/*************************************************************/
/* both the ranges are positive and the distribution is      */
/* local: the range corresponds to the rsd section       */
/*************************************************************/
static void 
do_local_range(Dist_Globals *dh, int i, int l2, int u2)
{

  int j = dh->num_dim_nodes[i];
  int proc, k;

  proc = dh->proc;

  for (k = 0; k <= j; ++k)
  {
    dh->rs_parts[proc][i].lower[k] = l2;
    dh->rs_parts[proc][i].upper[k] = u2;
    dh->rs_parts[proc][i].inv_lower[k] = l2;
    dh->rs_parts[proc][i].inv_upper[k] = u2;
  }
}


/*************************************************************/
/* get the inverse rsds so that the sends can be computed    */
/*************************************************************/
static void 
do_inverse_negative(Dist_Globals *dh, int i, int start_pos, int end_pos, int bk1)
{
  int k;
  int proc;

  proc = dh->proc;

  for (k = start_pos; k < end_pos; ++k)
  {

    if (!dh->rs_parts[proc][i].lower[k])
      dh->rs_parts[proc][i].inv_lower[k] = bk1;
    else
      dh->rs_parts[proc][i].inv_lower[k] = bk1 -
        abs(dh->rs_parts[proc][i].lower[k]); 

/*        dh->rs_parts[proc][i].inv_lower[k] =
          abs(bk1*dh->rs_parts[proc][i].proc_offset[k]) - 
          abs(dh->rs_parts[proc][i].lower[k]);
*/
    if (!dh->rs_parts[proc][i].upper[k])
      dh->rs_parts[proc][i].inv_upper[k] = bk1;
    else
      dh->rs_parts[proc][i].inv_upper[k] = bk1 -
        abs(dh->rs_parts[proc][i].upper[k]); 

/*      dh->rs_parts[proc][i].inv_upper[k] = 
        abs(bk1*dh->rs_parts[proc][i].proc_offset[k]) - 
        abs(dh->rs_parts[proc][i].upper[k]);
*/
  }

}


/*************************************************************/
/* get the inverse processor offsets : this provides us with */
/* the information as to WHO to send the data            */
/*************************************************************/
static void 
do_inverse_neg_offsets(Dist_Globals *dh, int start_pos, int end_pos, int i)
{
  int j;
  int proc;

  proc = dh->proc;

  for (j = start_pos; j < end_pos; ++j)
  {
    if (dh->rs_parts[proc][i].proc_offset[j])
      dh->rs_parts[proc][i].inv_proc_offset[j] =
           -dh->rs_parts[proc][i].proc_offset[j];
  }
}


/*************************************************************/
/* get the inverse processor offsets : this provides us with */
/* the information as to WHO to send the data            */
/*************************************************************/
static void 
do_inverse_pos_offsets(Dist_Globals *dh, int start_pos, int end_pos, int i)
{
  int j;
  int proc;

  proc = dh->proc;

  for (j = start_pos; j < end_pos; ++j)
  {
    if (dh->rs_parts[proc][i].proc_offset[j])
      dh->rs_parts[proc][i].inv_proc_offset[j] =
            -dh->rs_parts[proc][i].proc_offset[j];
  }
}

/*************************************************************/
/* get the inverse rsds so that the WHAT to send can be      */
/* computed                          */
/*************************************************************/
static void 
do_inverse_positive(Dist_Globals *dh, int i, int start_pos, int end_pos, int bk1)
{
  int k;
  int proc;

  proc = dh->proc;

  for (k = start_pos; k < end_pos; ++k)
  {
    dh->rs_parts[proc][i].inv_lower[k] = 
           dh->rs_parts[proc][i].lower[k] - bk1;
    dh->rs_parts[proc][i].inv_upper[k] = 
           dh->rs_parts[proc][i].upper[k] - bk1;
  }
}

/*************************************************************/
/* get the processor offsets if they are positive        */
/*************************************************************/
static void 
do_positive_proc_offsets(Dist_Globals *dh, int i, int blocksize, int start_pos, 
                         int end_pos, SNODE *sp)
{
  int j, dupper, dlower, offset;
  int proc;

  proc = dh->proc;

  decomp_local_bounds(sp, i, &dupper, &dlower);
  offset = dlower - 1;

  for (j = start_pos; j < end_pos; ++j)
  {
    if (dh->rs_parts[proc][i].upper[j] <= dupper)
      dh->rs_parts[proc][i].proc_offset[j] = 0;

    else
    {
      dh->rs_parts[proc][i].proc_offset[j] =
        (dh->rs_parts[proc][i].upper[j] - offset) / blocksize - 1;

      if ((dh->rs_parts[proc][i].upper[j] - offset) % blocksize)
        ++dh->rs_parts[proc][i].proc_offset[j];
    }
  }
}

/********************************************************/
/* get the processor offsets if they are negative   */
/********************************************************/
static void 
do_negative_proc_offsets(Dist_Globals *dh, int i, int blocksize, int start_pos, 
                         int end_pos)
{
  int j;
  int proc;

  proc = dh->proc;

  for (j = start_pos; j < end_pos; ++j)
  {
    if (!dh->rs_parts[proc][i].upper[j] &&
        !dh->rs_parts[proc][i].lower[j])
      dh->rs_parts[proc][i].proc_offset[j] = -1;

    else
    {
      dh->rs_parts[proc][i].proc_offset[j] =
       (dh->rs_parts[proc][i].lower[j] - blocksize) / blocksize;

/*   if ((dh->rs_parts[proc][i].upper[j] - 1) % blocksize)
    --dh->rs_parts[proc][i].proc_offset[j];
*/
    }
  }
}

/*****************************************************************/
/* compute the number of nodes to communicate with       */
/*****************************************************************/
static void 
compute_num_nodes(Dist_Globals *dh, int num_dim)
{
  int i;
  int num_nodes = 1;
  Boolean is_comm = false;

  for (i = 0; i < num_dim; i++)
    if (dh->num_dim_nodes[i])
      is_comm = true;

  if (is_comm)
  {
    for (i = 0; i < num_dim; i++)
    {
      if (dh->num_dim_nodes[i])
        num_nodes = num_nodes * dh->num_dim_nodes[i];
    }
    dh->num_nodes = num_nodes;
  }
  else
    dh->num_nodes = 0;
}



/*-----------------------------------------------------------------------

  get_sym_bound()   Create AST for RSD symbolic upper/lower bound

  This really should be part of rsd.c, once we add symbolic RSDs.

*/

static AST_INDEX
get_sym_bound(Rsd_set *rset, int dim, Boolean upper)
{
  Subs_data *sdata;
  Loop_data *ldata;
  AST_INDEX node;

  sdata = rset->sinfo[0]->subs + dim;
  if ((sdata->stype == SUBS_ZIV) || (sdata->stype >= SUBS_SYM))
  {
    return sdata->sym;
  }
  else if (sdata->stype >= SUBS_SIV)
  {
    printf("get_sym_bound(): subscript too complex\n");
    return AST_NIL;
  }
  ldata = rset->sinfo[0]->loop_nest->loops + sdata->stype;

  if (upper)
  {
    if (ldata->up.type == Expr_constant)
    {
      printf("get_sym_bound(): unexpected constant loop upper bound\n");
    }
    node = tree_copy(ldata->up.ast);
  }
  else
  {
    if (ldata->lo.type == Expr_constant)
    {
      printf("get_sym_bound(): unexpected constant loop lower bound\n");
    }
    node = tree_copy(ldata->lo.ast);
  }

  if (sdata->constant > 0)
    node = pt_gen_add(node, pt_gen_int(sdata->constant));
  else if (sdata->constant < 0)
    node = pt_gen_sub(node, pt_gen_int(-sdata->constant));
  node = pt_simplify_expr(node);

  return node;
}

#ifdef OLD_CODE

/*********************************************************/
/* the function is responsible for filling in the    */
/* ranges if a particular dimension is local.        */
/*********************************************************/
static void 
finish_local_range(Dist_Globals *dh, int maxproc, int *local_dim)
{
  int i, j, upper, lower, inv_upper, inv_lower, proc;

  proc = dh->proc;

  for (j = 0; j < DC_MAXDIM; ++j)
  {
    if (local_dim[j] == true)
    {
      lower = dh->rs_parts[proc][j].lower[0];
      upper = dh->rs_parts[proc][j].upper[0];
      inv_upper = dh->rs_parts[proc][j].inv_upper[0];
      inv_lower = dh->rs_parts[proc][j].inv_lower[0];

      for (i = 1; i < maxproc; ++i)
      {
        dh->rs_parts[proc][j].lower[i] = lower;
        dh->rs_parts[proc][j].upper[i] = upper;
        dh->rs_parts[proc][j].inv_upper[i] = inv_upper;
        dh->rs_parts[proc][j].inv_lower[i] = inv_lower;

      }
    }
  }
}

#endif /* OLD_CODE */
