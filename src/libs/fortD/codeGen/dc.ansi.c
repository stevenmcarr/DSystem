/* $Id: dc.ansi.c,v 1.50 1997/03/11 14:28:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    dc.c    Main body of Fortran 77D compiler for MIMD 
            distributed-memory machines

    Author : Seema Hiranandani 

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libs/fortD/codeGen/private_dc.h>
#include <libs/frontEnd/prettyPrinter/ft2text.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/frontEnd/ast/ftExportSimple.h>

/*------------------------- extern definitions ------------------------*/

EXTERN(int, ft_export,(Context context, 
	FortTree ft, FortTextTree ftt, 
	FILE *outf, Company dialect));         /* ft_export.c  */

EXTERN(void, dc_analyze,(Dist_Globals *dh));   /* decomp.c     */
EXTERN(void, dc_partition,(Dist_Globals *dh)); /* bc.c         */
EXTERN(void, dc_loop_info,(Dist_Globals *dh)); /* dc_loops.c   */
EXTERN(void, dc_comm,(Dist_Globals *dh));      /* rsd_sec.c    */
EXTERN(void, dc_messages,(Dist_Globals *dh));  /* msg.c        */
EXTERN(void, dc_bounds,(Dist_Globals *dh));    /* bounds.c     */
EXTERN(void, dc_irreg,(Generic   irr, 
		       AST_INDEX irr_decls_node)); /* irr_msgs.C   */
EXTERN(void, dc_irreg_analyze,(Generic irr));       
EXTERN(void, dc_irreg_partition,(Generic irr));       
EXTERN(void, dc_irreg_cleanup,(Generic irr));
EXTERN(void, dc_do_distribution,(Dist_Globals *dh));/* dc_trans.c   */
EXTERN(void, dc_pipe_loops,(Dist_Globals *dh));/* dc_trans.c   */
EXTERN(void, rsd_vector_init,(DT_info *dt_info, 
        SideInfo *infoPtr, AST_INDEX root));   /* rsd.c        */
EXTERN(Carried_deps, *dg_carried_deps, 
        (DG_Instance *dg, SideInfo *infoPtr, 
         AST_INDEX loop));                    /* util_dg.c    */
EXTERN(int, dc_loop_part,());                 /* bc.c         */

EXTERN(void, SD_CleanupInstr,	 (Dist_Globals* dh));

/*------------------------- GLOBAL DEFINITIONS ------------------------*/

int ped_link_dc = 1;  /* Fortran D compiler linked in */


/*------------------------- LOCAL DEFINITIONS -------------------------*/

STATIC(void, dc_compile,(Dist_Globals *dh, Fd_opts *fd_opts));
STATIC(void, dc_remove_fortD,(Dist_Globals *dh));
STATIC(void, dc_gen_ph,(Dist_Globals *dh));
STATIC(void, dc_assignments,(Dist_Globals *dh));
STATIC(void, dc_storage,(Dist_Globals *dh));
STATIC(void, dc_forall,(Dist_Globals *dh));
STATIC(void, dc_compile_save,(Dist_Globals *dh));

STATIC(int, remove_fortD,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(int, gen_ph, (AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(int, convert_forall,(AST_INDEX stmt, int level, Dist_Globals *dh));
/*STATIC(int, conv_indices_stmt,());
STATIC(void, conv_subs,());
STATIC(void, conv_id,());
STATIC(int, conv_indices_expr,());
STATIC(void, insert_assign,());*/
STATIC(int, fix_storage,(AST_INDEX node, int level, Dist_Globals *dh));
STATIC(void, fix_decls,(Dist_Globals *dh, AST_INDEX node));
STATIC(void, add_storage,(Dist_Globals *dh, AST_INDEX start));
STATIC(void, ped_update,(Dist_Globals *dh));
STATIC(int, find_name,(AST_INDEX node, int level, char *buf));
STATIC(int, find_loop1,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(void, add_blank_line,(AST_INDEX node));
STATIC(int, zap_labels,(AST_INDEX stmt, int level, Dist_Globals *dh));

/*------------------------- local structures -------------------------*/

#define DC_MEM_BLOCKSIZE (64*1024)   /* block size for memory allocations */

#define MAXGVAR 256

struct conv_param
{
  Dist_Globals *dh;
  Iter_set *iset;
  Loop_list *l_list;
  Loop_data *ldata[LOOPTOTAL];

  int gbl_var_num;
  int gbl_loop_num;
  AST_INDEX loop_vars[LOOPTOTAL];
  AST_INDEX old_gbl_vars[MAXGVAR];
  AST_INDEX gbl_vars[MAXGVAR];
  AST_INDEX gbl_off[MAXGVAR];

  int loc_var_num;
  int loc_loop_num;
  AST_INDEX old_loc_vars[MAXGVAR];
  AST_INDEX loc_vars[MAXGVAR];
  AST_INDEX loc_off[MAXGVAR];
};

/*******************/
/* Global Routines */
/*******************/

/*--------------------------------------------------------------------

    dist_compiler()  

    Main entry point for the Fortran 77D compiler from PED
*/

int
dist_compiler(PedInfo ped)
{
  Dist_Globals dh;
  Fd_opts      fd_opts;

  bzero(&fd_opts, sizeof(Fd_opts));

  if (PED_EDIT_PERFORMED(ped))
  {
    message("Dependence analysis out of date. \nPlease reanalyze program.");
    return 1;
  }

  dc_init(&dh, &fd_opts);            /* initialize dc data structures    */
  dh.ped = ped;
  dh.root = PED_ROOT(ped);
  dh.numprocs = DEFAULT_PROC; 
  dh.in_ped = true;

  /* Prepare for changes */
  ped->TreeWillChange(PED_ED_HANDLE(ped), PED_ROOT(ped));
  dh.irr = dc_irreg_init(PED_FT(ped), PED_FTT(ped), fd_opts);
  (void) pedReinitialize(ped);
  ped_update(&dh);

  dc_compile(&dh, &fd_opts);    /* actually compile program */

  return 0;
}


/*--------------------------------------------------------------------

    dc_compile_proc()  

    Main entry point for the Fortran 77D compiler from fortd.
*/
void 
dc_compile_proc(PedInfo ped, AST_INDEX pnode, int nproc, Fd_opts *fd_opts, Generic di)
{
  Dist_Globals dh;

  dc_init(&dh, fd_opts);             /* initialize dc data structures */
  dh.ped = ped;
  dh.root = pnode;
  dh.numprocs = nproc;
  dh.in_ped = false;
  dh.irr = di;
  dh.declLog2Phys = dh.declPhys2Log = false;

  dc_compile(&dh, fd_opts); /* actually compile program */

  dc_compile_save(&dh);     /* save compiler output     */
}


/*--------------------------------------------------------------------

    dc_new_var()  Create new variable for use by compiler

*/

void
dc_new_var(Dist_Globals *dh, AST_INDEX node, enum FORM type)
{
  if (dh->vars.num < MAXNEWVAR)
  {
    dh->vars.name[dh->vars.num] = node;
    dh->vars.type[dh->vars.num++] = type;
  }
  else
    printf("dc_new_var(): Too many variables\n");

}


/*--------------------------------------------------------------------

  dc_init()  Initializes main Fortran D compiler data structure

*/

void
dc_init(Dist_Globals *dh, Fd_opts *fd_opts)
{
  int i;

  bzero(dh, sizeof(Dist_Globals));  /* init to 0, NULL, or false */

  /* select following optimization options */

  dh->opts.mesg_vectorize = !fd_opts->flags[Do_mesg_vect];
  dh->opts.mesg_coalesce = !fd_opts->flags[Do_mesg_coalesce];
  dh->opts.mesg_aggregate = !fd_opts->flags[Do_mesg_aggr];
  dh->opts.vect_mesg_pipe = !fd_opts->flags[Do_vector_mesg_pipe];
  dh->opts.unbuffered_send = !fd_opts->flags[Do_unbuffered_send];
  dh->opts.unbuffered_recv = !fd_opts->flags[Do_unbuffered_recv];
  dh->opts.iter_reorder = !fd_opts->flags[Do_iteration_reorder];
  dh->opts.fine_grain_pipe = !fd_opts->flags[Do_fine_grain_pipe];
  dh->opts.coarse_grain_pipe = !fd_opts->flags[Do_coarse_grain_pipe];
  dh->opts.relax_owner_computes = !fd_opts->flags[Do_relax_owner];
  dh->opts.loop_fuse = !fd_opts->flags[Do_loop_fusion]; 
  dh->opts.loop_dist = !fd_opts->flags[Do_loop_distribution]; 
  dh->opts.memory_order = !fd_opts->flags[Do_memory_order]; 

  dh->opts.loop_fuse = false;      /* broken, turn off for now */
  dh->opts.loop_dist = false;      /* broken, turn off for now */

  /* min size for using unbuffered sends   */
  dh->opts.min_unbuffered_send = 
    fd_opts->isend_size ? fd_opts->isend_size : MIN_UNBUFFERED;  

  /* min size for using unbuffered recvs   */
  dh->opts.min_unbuffered_recv = 
    fd_opts->irecv_size ? fd_opts->irecv_size : MIN_UNBUFFERED;  

  dh->opts.min_buf_size = MIN_BUF_SIZE;      /* min size for buf routines  */

  /* max size of mesgs for applying message aggregation */
  dh->opts.max_aggr_size = 
    fd_opts->ma_size ? fd_opts->ma_size : MAX_AGGR_SIZE; 

  /* default strip size for coarse-grain pipelining */
  dh->opts.pipe_size = 
    fd_opts->cgp_size ? fd_opts->cgp_size : COARSE_PIPE_SIZE;     

  /* choose target architecture, default is Intel NX/2 */
  if (!fd_opts->arch || !strcmp(fd_opts->arch, "nx"))
    dh->arch = FD_NX;
  else if (!strcmp(fd_opts->arch, "cmmd"))
    dh->arch = FD_CMMD;
  else
    printf("dc_init(): Unknown target architecture\n");

  dh->mtype = MSG_START_ID;      /* starting message ID */
  dh->msg_id_num = 1;  

  dh->recv_stmts = AST_NIL;
  dh->send_stmts = AST_NIL;
  dh->fortD_ph = AST_NIL;

  for (i = 0; i < LOOPTOTAL; ++i)
    dh->doloops[i] = AST_NIL;

  /* initialize names */

  dh->nproc = NPROC;
  dh->myproc = MYPROC;

  dh->bufs.i.name = IBUF_NAME;
  dh->bufs.r.name = RBUF_NAME;
  dh->bufs.dp.name = DPBUF_NAME;

  switch (dh->arch)
  {
    case FD_NX:
      dh->mypid = MYPID;
      dh->syscalls.nproc = "numnodes";
      dh->syscalls.myproc = "mynode";
      dh->syscalls.mypid = "mypid";
      dh->syscalls.csend = "csend";
      dh->syscalls.crecv = "crecv";
      dh->syscalls.bcast = dh->syscalls.csend;
      dh->syscalls.brecv = dh->syscalls.crecv;
      dh->syscalls.isend = "isend";
      dh->syscalls.irecv = "irecv";
      dh->syscalls.msgwait = "msgwait";
      dh->ret = NULL;
      break;

    case FD_CMMD:
      dh->mypid = NULL;
      dh->syscalls.nproc = "cmmd_partition_size";
      dh->syscalls.myproc = "cmmd_self_address";
      dh->syscalls.mypid = NULL;
      dh->syscalls.csend = "cmmd_send_noblock";
      dh->syscalls.crecv = "cmmd_receive";
      dh->syscalls.bcast = "cmmd_bc_to_nodes";
      dh->syscalls.brecv = "cmmd_receive_bc_from_node";
      dh->syscalls.isend = NULL;
      dh->syscalls.irecv = NULL;
      dh->syscalls.msgwait = NULL;
      dh->ret = "ret$";
      break;
  }

  dc_new_var(dh, pt_gen_ident(dh->nproc), INTTYPE);
  dc_new_var(dh, pt_gen_ident(dh->myproc), INTTYPE);

  if (dh->mypid)
    dc_new_var(dh, pt_gen_ident(dh->mypid), INTTYPE);

  dc_new_var(dh, pt_gen_ident(dh->syscalls.nproc), INTTYPE);

  dc_new_var(dh, pt_gen_ident(dh->syscalls.myproc), INTTYPE);

  if (dh->syscalls.mypid)
    dc_new_var(dh, pt_gen_ident(dh->syscalls.mypid), INTTYPE);

  if (dh->ret)
    dc_new_var(dh, pt_gen_ident(dh->ret), INTTYPE);

}

/*--------------------------------------------------------------------

    dc_finalize()  free all Fortran D compiler data structures

*/

void
dc_finalize(Dist_Globals *dh)
{
  dc_mem *mem;
  int bytes = 0;

  while (dh->mem)
  {
    mem = dh->mem;
    dh->mem = mem->next;
    bytes += mem->size;
    free_mem(mem->buf);
    free_mem(mem);
  }

  dh->mem = NULL;

  /* Disabled because this number covers only the code gen phase,
   * at best.  Local analysis, IP analysis, and instrumentation are
   * not included.
   * --Vikram Adve, 9/19/94.
   */
  /* if (!dh->in_ped)
   * printf("  Fortran D compiler used %d bytes\n", bytes);
   */
}


/*--------------------------------------------------------------------

    dc_get_mem()  Block allocate memory for Fortran D compiler

*/

char *
dc_get_mem(Dist_Globals *dh, int size)
{
  char *buf;
  dc_mem *mem;

  if (!dh->mem || (dh->mem->size + size > DC_MEM_BLOCKSIZE))
  {
    mem = dh->mem;
    dh->mem = (dc_mem *) get_mem(sizeof(dc_mem), "Fortran D compiler");
    dh->mem->buf = (char *) get_mem(DC_MEM_BLOCKSIZE, "Fortran D compiler");
    dh->mem->next = mem;
    dh->mem->size = 0;
    bzero(dh->mem->buf, DC_MEM_BLOCKSIZE);
  }

  buf = dh->mem->buf + dh->mem->size;
  dh->mem->size += size;
  return buf;
}


/******************/
/* Local Routines */
/******************/

/*--------------------------------------------------------------------

    dc_compile()      The Fortran 77D compiler

*/

static void 
  dc_compile(Dist_Globals *dh, Fd_opts *fd_opts)
{
  PedInfo ped = dh->ped;

  /* prepare for AST changes */
  if (dh->in_ped)
    ped->TreeWillChange(ped->ed_handle, dh->root);

  /* remove all redundant labels */
  /* 9/25/93 RvH: I need labels ... */
  /* walk_statements(dh->root, LEVEL1, zap_labels, NULL, dh); */

  /*---------------------------------*/
  /* analysis & transformation phase */

  dc_irreg_analyze(dh->irr);     /* analyze irregular computations */

  if (dh->opts.loop_fuse)
  {
    pt_fuse_all(ped, dh->root, false);  /* conservatively fuse loops  */
    dt_update_loopref(PED_DT_INFO(ped), PED_INFO(ped), dh->root); 
    ped_update(dh);
  }

  dc_analyze(dh);       /* analyze data decomposition       */

  dc_find_loops(dh);      /* find & record all loops */

  /* minimize granularity of pipelining via interchange */
  if (dh->opts.fine_grain_pipe)
  {
    dc_pipe_loops(dh); 
    dt_update_loopref(PED_DT_INFO(ped), PED_INFO(ped), dh->root); 
    dc_find_loops(dh);      /* find & record all loops */
    ped_update(dh);
  }

  dc_partition(dh);       /*  partition computation analysis    */

  dc_irreg_partition(dh->irr); /* partition irregular computations   */

  /* distribute loops to improve loop bounds reduction */
  if (dh->opts.loop_dist)
  {
    dc_do_distribution(dh);
    dt_update_loopref(PED_DT_INFO(ped), PED_INFO(ped), dh->root); 
    dc_find_loops(dh);    /* find & record all loops */
    ped_update(dh);
  }

  dc_loop_info(dh);       /* update information on loops & indices */

  ped_rsd_vector_init(ped, dh->root);  /* build Rsd_vector structs */

  dc_comm(dh);            /* determine needed data movement   */

  /*-----------------------*/
  /* code generation phase */

  dc_gen_ph(dh);          /* generate fortD_ph                */

  dc_assignments(dh);     /* introduce initializations        */
  ped_update(dh);

  dc_bounds(dh);          /* update loop bounds               */
  ped_update(dh);

  dc_indices(dh);         /* convert global <-> local indices */
  ped_update(dh);

  dc_messages(dh);        /* generate messages                */
  ped_update(dh);

  dc_storage(dh);         /* manage storage for nonlocal data */
  ped_update(dh);

  dc_forall(dh);          /* scalarize FORALL loops           */
  ped_update(dh);

  list_remove_node(dh->fortD_ph); /* remove place holder      */
  dc_irreg(dh->irr,       /* support irregular computations   */
	   dh->irr_decls_node);
  ped_update(dh);

  /* 12/3/93 RvH: Moved this down here, to not mess up irregular part */
  dc_remove_fortD(dh);    /* remove Fortran D statements      */
  ped_update(dh);

  dc_irreg_cleanup(dh->irr); /* cleanup irregular part        */

  SD_CleanupInstr(dh);

  dc_finalize(dh);
}


/*----------------------------------------------------------------------

  ped_update()      Update screen after AST changes if in PED

*/

static void
ped_update(Dist_Globals *dh)
{
  if (dh->in_ped)
    dh->ped->TreeChanged(dh->ped->ed_handle, dh->root);
}

/*----------------------------------------------------------------------*/
/*------------------------- Initializations ----------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------

  dc_gen_ph() 
  
  Generates place holder
*/

static void
dc_gen_ph(Dist_Globals *dh)
{
  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)gen_ph, NULL, (Generic)dh);
}


/*----------------------------------------------------------------------

  gen_ph()   Helper function for dc_gen_ph()
  
*/

static int
gen_ph(AST_INDEX stmt, int level, Dist_Globals *dh) 
{
  char *str;
  AST_INDEX node, prev;

  /* if first executable statment encountered, mark location */

  if ((dh->fortD_ph == AST_NIL) && executable_stmt(stmt))
  {
    for (node = stmt, prev = list_prev(node);
         (prev != AST_NIL) && is_comment(prev);
         node = prev, prev = list_prev(node))
      ;

    dh->fortD_ph = gen_PLACE_HOLDER();
    (void) list_insert_before(node, dh->fortD_ph);
  }

  return WALK_CONTINUE;
}
/*----------------------------------------------------------------------

  dc_remove_fortD() 
  
  Removes the statements unique to Fortran D.  Currently, these include
  decomposition, align, and distribute comments and n$proc parameters.

*/

static void
dc_remove_fortD(Dist_Globals *dh)
{
  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)remove_fortD, NULL, (Generic)dh);
}


/*----------------------------------------------------------------------

  remove_fortD()   Helper function for dc_remove_fortD()
  
*/

static int
remove_fortD(AST_INDEX stmt, int level, Dist_Globals *dh) 
{
  char *str;
  AST_INDEX node, prev;

  if (is_comment(stmt))
  {
    str = gen_get_text(gen_COMMENT_get_text(stmt));

    /* check whether CALL is a Fortran D specification */
    if (*str &&
      !strncmp("decomposition", str, DECOMP_LEN) ||
      !strncmp("Decomposition", str, DECOMP_LEN) ||
      !strncmp("DECOMPOSITION", str, DECOMP_LEN) ||
      !strncmp("align", str, ALIGN_LEN) ||
      !strncmp("Align", str, ALIGN_LEN) ||
      !strncmp("ALIGN", str, ALIGN_LEN) ||
      !strncmp("distribute", str, DIST_LEN) ||
      !strncmp("Distribute", str, DIST_LEN) ||
      !strncmp("DISTRIBUTE", str, DIST_LEN))
    {
      /* Assume all call nodes are in a list since they are in a block. */

      list_remove_node(stmt); /*  Update the list. */
      tree_free(stmt);        /* Release the node. */

      /* Tell walk_statements() to start again at the next 
       * node since "stmt" is no longer in the tree. */

      return (WALK_FROM_OLD_NEXT);
    }
  }
  else if (is_parameter(stmt))
  {
    node = gen_PARAMETER_get_param_elt_LIST(stmt);
    node = list_first(node);
    node = gen_PARAM_ELT_get_name(node);
    str = gen_get_text(node);

    /* look for assignment to n$proc (or nproc) */

    if (!strcmp("n$proc", str) || !strcmp("nproc", str) ||
        !strcmp("N$PROC", str) || !strcmp("NPROC", str))
    { 
      list_remove_node(stmt);   /* Update the list. */
      tree_free(stmt);          /* Release the node */

      /* Tell walk_statements() to start again at the next 
       * node since "stmt" is no longer in the tree. */

      return (WALK_FROM_OLD_NEXT); 
    }
  }

  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

  dc_assignments()  Inserts initialization statements needed by 
                    Fortran D programs - n$proc, my$p, my$pid

*/

static void
dc_assignments(Dist_Globals *dh)
{
  AST_INDEX node; 

  add_blank_line(dh->fortD_ph);
  (void) list_insert_before(dh->fortD_ph, 
           pt_gen_comment("--<< Fortran D initializations >>--")); 

  /* determine type of scope, generate assignments only if top level */

  node = dh->fortD_ph;
  while (true)
  {
    node = tree_out(node);
    if (node == AST_NIL)
    {
      printf("dc_assignments(): Unknown scope\n");
      return;
    }
    if (is_subroutine(node) || is_function(node))
      return;
    if (is_program(node) || is_global(node))
      break;
  }

  if (dh->syscalls.nproc)  /* first insert "n$proc = numnodes()" */
  {
    node = pt_gen_invoke(dh->syscalls.nproc, AST_NIL);
    node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->nproc), node);
    (void) list_insert_before(dh->fortD_ph, node); 

    /* insert check to ensure right number of processors allocated */

    node = gen_BINARY_NE(pt_gen_ident(dh->nproc), pt_gen_int(dh->numprocs));
    node = gen_LOGICAL_IF(AST_NIL, node,
             list_create(gen_STOP(AST_NIL,AST_NIL)));
    (void) list_insert_before(dh->fortD_ph, node); 
  }

  if (dh->syscalls.myproc)  /* then insert "my$p = phys2log(mynode())" */
  {
    /* node = pt_gen_invoke(dh->syscalls.myproc, AST_NIL);	*/
    /* node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->myproc), node);	*/
    /* (void) list_insert_before(dh->fortD_ph, node);	*/
    (void) list_insert_before(dh->fortD_ph, ast_set_logical_myproc(dh));
  }

  if (dh->syscalls.mypid)  /* and insert "my$pid = mypid()" */
  {
    node = pt_gen_invoke(dh->syscalls.mypid, AST_NIL);
    node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->mypid), node);
    (void) list_insert_before(dh->fortD_ph, node);
  }

}

/*----------------------------------------------------------------------*/
/*------------------------ Storage Management --------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------

  dc_storage()   Determine overlap regions needed for nonlocal accesses

*/

static void
dc_storage(Dist_Globals *dh)
{
  AST_INDEX node, prev;

  /* Walk the statements for declarations and update all of the 
   * loop bounds for distributed dimensions of arrays according 
   * to the minimum and maximum overlap offsets accessed.       */

  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)fix_storage, NULL, (Generic)dh);

  /* find last type statement & insert additional decls there */

  for (node = dh->fortD_ph, prev = list_prev(node);
       (prev != AST_NIL) && !is_type_statement(prev) && !is_common(prev);
       node = prev, prev = list_prev(node))
    ;

  add_storage(dh, node);

  /* Fortran D/irreg declarations go after this node */
  dh->irr_decls_node = node;
}


/*----------------------------------------------------------------------

  fix_storage()  Change storage allocation for each distributed array

*/

static int
fix_storage(AST_INDEX node, int level, Dist_Globals *dh)   
{
  if (is_type_statement(node))
    fix_decls(dh, gen_TYPE_STATEMENT_get_array_decl_len_LIST(node));

  else if (is_common(node))
  {
    node = gen_COMMON_get_common_elt_LIST(node);
    for (node = list_first(node); node != AST_NIL; node = list_next(node))
      fix_decls(dh, gen_COMMON_ELT_get_common_vars_LIST(node));
   }

  return WALK_CONTINUE;
}


/*----------------------------------------------------------------------

  fix_decls()  Change storage allocation for each distributed array
               in list of variable declarations
*/

static void
fix_decls(Dist_Globals *dh, AST_INDEX node)   
{
  AST_INDEX curr, dimlist, upperb, lowerb, name_ast;
  char *name;
  SNODE *sp;
  int i, numdim;

  /* loop through all decls in statement */

  for (node = list_first(node); node != AST_NIL; node = list_next(node))
  {
    if (!is_array_decl_len(node))  /* only check arrays */
      continue;

    name_ast = gen_ARRAY_DECL_LEN_get_name(node);
    name = gen_get_text(name_ast);
    dimlist = gen_ARRAY_DECL_LEN_get_dim_LIST(node);

    if (dimlist == AST_NIL)   /* if not actually an array */
      continue;

    numdim = list_length(dimlist);
    sp = findadd2(name_ast, 0, 0, dh);

    if (!sp || !sp->decomp)     /* non-distributed array */
      continue;

    /* modify bounds for each distributed dimension */

    for (i = 0, curr = list_first(dimlist); 
           i < numdim; i++, curr = list_next(curr))
    {
      if ((sp_is_part(sp, i) != FD_DIST_DC_NONE) && 
          (sp_is_part(sp, i) != FD_DIST_LOCAL))
      {
        /*** FOR DISTRIBUTED DIMENSIONS ONLY                ***/
        /*** if the either bound of the array is a constant ***/
        /***    its value was stored in the symbol table    ***/
        /***    and its new value is written back           ***/

        upperb = gen_DIM_get_upper(curr);
        lowerb = gen_DIM_get_lower(curr);

        /* 10/8/93 RvH: Want to allow symbolics */
        /* if (is_constant(upperb)) */
          gen_DIM_put_upper(curr, pt_gen_int(sp_max_access(sp,i)));
        /* else
          die_with_message("non-constant size for array %s", name); */

        /* 10/8/93 RvH: Want to allow symbolics */
        if ((lowerb != AST_NIL) /* && is_constant(lowerb) */)
          gen_DIM_put_lower(curr, pt_gen_int(sp_min_access(sp,i)));
        else if ((lowerb == AST_NIL) && (sp_min_access(sp, i) != 1))
          gen_DIM_put_lower(curr, pt_gen_int(sp_min_access(sp,i)));
      }
    } 
  }
}


/*----------------------------------------------------------------------

  add_storage()  Add storage allocation for buffers, other variables

*/

static void
add_storage(Dist_Globals *dh, AST_INDEX start)
{
  int i, btype;
  AST_INDEX node, tnode, lnode, name, dim;
  char strbuf[MAXCOMMENT];
  Buf_info *bufs;
  enum FORM type;

  add_blank_line(start); 
  (void) list_insert_before(start, 
           pt_gen_comment("--<< Fortran D variable declarations >>--")); 

  /*--------------------------------------------------*/
  /* do common block declaration for global variables */

  node = pt_gen_ident(dh->nproc);
  node = gen_ARRAY_DECL_LEN(node,AST_NIL,AST_NIL,AST_NIL);
  lnode = list_create(node);

  node = pt_gen_ident(dh->myproc);
  node = gen_ARRAY_DECL_LEN(node,AST_NIL,AST_NIL,AST_NIL);
  lnode = list_insert_last(lnode,node);

  if (dh->mypid)
  {
    node = pt_gen_ident(dh->mypid);
    node = gen_ARRAY_DECL_LEN(node,AST_NIL,AST_NIL,AST_NIL);
    lnode = list_insert_last(lnode,node);
  }

  node = gen_COMMON_ELT(pt_gen_ident("/FortD/"), lnode);
  node = gen_COMMON(AST_NIL,list_create(node));
  (void) list_insert_before(start, node); 

  /*------------------------------------------------------------*/
  /* do 3 types of buffers: integer, real, and double precision */

  for (btype = 0; btype < 3; btype++)
  {
    /*-----------------------------*/
    /* set type of buffer/variable */

    switch (btype)
    {
      case 0:  /* integer */
        bufs = &dh->bufs.i;
        type = INTTYPE;
        node = gen_INTEGER();
        break;

      case 1:  /* real    */
        bufs = &dh->bufs.r;
        type = REAL;
        node = gen_REAL();
        break;

      case 2:  /* double  */
        bufs = &dh->bufs.dp;
        type = DOUBLE_P;
        node = gen_DOUBLE_PRECISION();
        break;
    }

    tnode = gen_TYPE_LEN(node,AST_NIL);
    lnode = AST_NIL;

    /*---------------------------------------*/
    /* add declarations for buffer variables */

    for (i = 0; i <= bufs->num; i++)
    {
      if (bufs->size[i])
      {
        if (!i)
          strcpy(strbuf, bufs->name);
        else
          sprintf(strbuf, "%s%d", bufs->name, i);

        name = pt_gen_ident(strbuf);
        dim = list_create(gen_DIM(AST_NIL,pt_gen_int(bufs->size[i])));
        node = gen_ARRAY_DECL_LEN(name,AST_NIL,dim,AST_NIL);

        lnode = (lnode == AST_NIL) ? list_create(node) :
                                     list_insert_last(lnode,node);
      }
    }

    /*---------------------------------------------------*/
    /* add declarations for scalar variables & functions */

    for (i = 0; i < dh->vars.num; i++)
    {
      if (dh->vars.type[i] == type)
      {
        /* typechecker requires scalars to be zero-dim arrays */
        node = gen_ARRAY_DECL_LEN(dh->vars.name[i],AST_NIL,AST_NIL,AST_NIL);

        if (lnode == AST_NIL)
          lnode = list_create(node);
        else
          lnode = list_insert_last(lnode,node);
      }
    }

    /*----------------------------------------------------*/
    /* insert list of buffer & scalar variables collected */

    if (lnode != AST_NIL)
    {
      node = gen_TYPE_STATEMENT(AST_NIL,tnode,lnode);
      ft_SetComma(node, false);
      (void) list_insert_before(start, node); 
    }
  }
}


/*----------------------------------------------------------------------*/
/*------------------------ Forall Translation --------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------

  dc_forall()    Convert FORALL loops to DO loops

*/

static void
dc_forall(Dist_Globals *dh)
{
  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)convert_forall, NULL, (Generic)dh);
}


/*----------------------------------------------------------------------

  convert_forall()      Helper function for dc_forall()

*/

static int
convert_forall(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  AST_INDEX n, place_holder;
  Carried_deps *deps;

  if (is_do_all(stmt))
  {
      deps = dg_carried_deps((DG_Instance*)dh->ped, (SideInfo*)stmt, 0); 
                             /*too few args without 0*/

      if (deps->true_num)
      {
        /* should convert to anti dependences via renaming... */
        printf("Warning: FORALL loop carried true dependences\n");
      }
      if (deps->out_num)
      {
        /* should insert code to ensure last value used... */
        printf("Warning: FORALL loop carried output dependences\n");
      }

      place_holder = gen_PLACE_HOLDER();
      list_insert_before(stmt, place_holder);
      list_remove_node(stmt);

      n = gen_DO(tree_copy(gen_DO_ALL_get_lbl_def(stmt)),
                 tree_copy(gen_DO_ALL_get_close_lbl_def(stmt)),
                 tree_copy(gen_DO_ALL_get_lbl_ref(stmt)),
                 tree_copy(gen_DO_ALL_get_control(stmt)),
                 tree_copy(gen_DO_ALL_get_stmt_LIST(stmt)));

      list_insert_before(place_holder, n);
      list_remove_node(place_holder);

      /* tree_free(place_holder); */
      /* tree_free(stmt);         */

      walk_statements(gen_DO_get_stmt_LIST(n), 
         level, (WK_STMT_CLBACK)convert_forall, NULL, (Generic)dh);
  }

  return (WALK_CONTINUE);
}


/*----------------------------------------------------------------------*/
/*------------------------ Misc --------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------

  dc_compile_save()      Save results of F77D compile

*/

static void
dc_compile_save(Dist_Globals *dh)
{
  Company dialect;
  FILE *outFP;
  PedInfo ped;
  char buf[MAX_NAME];

  ped = dh->ped;
  dialect = None;

  strcpy(buf, "main");  /* default name if unnamed program */
  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)find_name, NULL, (Generic)buf);
  strcat(buf, ".fortd.f");

  if (!(outFP = fopen(buf, "w")))
  {
    printf("Unable to create %s!\n", buf);
    return;
  }

  {
   /* Patch to print out only the compiled procedure, instead of the
    * entire input file.  Added by John MC, 8/31/94.
    */
  	FortTree ft = (FortTree) PED_FT(ped);
	AST_INDEX root = ft_Root(ft);
	AST_INDEX stmtlist, stmt, nextstmt;
	assert(gen_get_node_type(root) == GEN_GLOBAL);
	stmtlist = gen_get_stmt_list(root);

	for(stmt = list_first(stmtlist); stmt != AST_NIL; stmt = nextstmt) {
		nextstmt = list_next(stmt);
		if (stmt != dh->root) { 
			list_remove_node(stmt);
			tree_free(stmt);
		}
	}
  }

  ftt_TreeChanged(ped->ftt, ft_Root((FortTree)ped->ft));

  ftExportSimple((FortTree)PED_FT(ped), PED_FTT(ped), outFP);
  
  fclose(outFP);

}

/*----------------------------------------------------------------------

  find_name()      Helper function for dc_compile_save()

*/

static int
find_name(AST_INDEX node, int level, char *buf)
{
  if (is_program(node))
  {
    node = gen_PROGRAM_get_name(node);
  }
  else if (is_subroutine(node))
  {
    node = gen_SUBROUTINE_get_name(node);
  }
  else if (is_function(node))
  {
    node = gen_FUNCTION_get_name(node);
  }
  else
    return WALK_CONTINUE;

  strcpy(buf, gen_get_text(node));
  return WALK_ABORT;
}


/*--------------------------------------------------------------------

    dc_find_loops()

    finds & stores location & number of loops in program
    also stores loop bounds for later use in message generation

*/

void
dc_find_loops(Dist_Globals *dh)
{
  dh->numdoloops = 0;
  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)find_loop1, NULL, (Generic)dh);
}


/*--------------------------------------------------------------------

    find_loop1()    Helper function for find_loops()

*/

static int
find_loop1(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  if (is_loop(stmt))
    dh->doloops[dh->numdoloops++] = stmt;

  return WALK_CONTINUE;
}

/*--------------------------------------------------------------------

    add_blank_line()    Insert blank line before stmt if needed.

*/

static void
add_blank_line(AST_INDEX node)
{
  AST_INDEX prev;

  prev = list_prev(node);
  if (!is_comment(prev) || (gen_COMMENT_get_text(prev) != AST_NIL))
    (void) list_insert_before(node, pt_gen_comment("")); 

}


/*--------------------------------------------------------------------

    zap_labels()    Zap all labels except for FORMAT stmts.  

    Add check for safety later when symbol table integrated using:
         if (!fst_GetField(symtab,gen_get_text(label),REFD))

*/

static int
zap_labels(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  AST_INDEX label;

  if (is_loop(stmt))
  {
    if ((label = gen_DO_get_lbl_def(stmt)) != AST_NIL)
      pt_tree_replace(label,AST_NIL);

    if ((label = gen_DO_get_close_lbl_def(stmt)) != AST_NIL)
      pt_tree_replace(label,AST_NIL);

    if ((label = gen_DO_get_lbl_ref(stmt)) != AST_NIL)
      pt_tree_replace(label,AST_NIL);
  }
  else if (labelled_stmt(stmt) && !is_format(stmt))
  {
    if ((label = gen_get_label(stmt)) != AST_NIL)
    {
	  if (is_continue(stmt))
	  {
	    (void) list_remove_node(stmt);
	    return WALK_FROM_OLD_NEXT;
	  }
      pt_tree_replace(label,AST_NIL);
    }
  }  
  return WALK_CONTINUE;
}

/*------------------------------------------------------------------------*
 * Functions to map logical <--> physical processor numbers.
 *
 *   Allows an arbitrary mapping of logical and physical processors. The
 * mapping is chosen when linking the node program, by linking in the
 * appropriate versions of two runtime functions:
 * 		integer phys2log(id)  :  physical --> logical  conversion
 *		integer log2phys(id)  :  logical --> physical conversion 
 *   Default definitions of these functions are provided in the runtime
 * library libFDruntime.a, and they implement a gray coded mapping, i.e.,
 * logical processors are adjacent on a binary hypercube.
 *   This compiler does *not* have to be re-run to change the mapping.
 *
 * USAGE:
 *   The idea is to use logical processor numbers everywhere within the
 * application, i.e., for index calculations, guards, and all such logic,
 * and physical processor numbers only in the processor number argument
 * to message passing routines.
 *
 * Compiler interface is via three functions:
 *   ast_set_logical_myproc(dh)	-- To set the logical "my$id" on each processor
 *   ast_get_logical_myproc(dh)	-- The logical processor number
 *   ast_map_logical_to_physical(dh, logical-number)
 * 				-- To convert a logical# to physical#
 *
 *   All three functions return an AST_INDEX of an integer-valued expression.
 * 
 * VSA, 4/94.
 */

#define PHYS_TO_LOG	"phys2log"
#define LOG_TO_PHYS	"log2phys"

AST_INDEX ast_set_logical_myproc(Dist_Globals *dh)
{
    AST_INDEX node;
    if ( !dh->declPhys2Log) {
	dc_new_var(dh, pt_gen_ident(PHYS_TO_LOG), INTTYPE);
	dh->declPhys2Log = true;
    }
    node = pt_gen_invoke(dh->syscalls.myproc, AST_NIL);
    node = pt_gen_invoke(PHYS_TO_LOG, node);
    return gen_ASSIGNMENT(AST_NIL, pt_gen_ident(dh->myproc), node);
}

AST_INDEX ast_get_logical_myproc(Dist_Globals *dh)
{
    return pt_gen_ident(dh->myproc);
}

AST_INDEX ast_map_logical_to_physical(Dist_Globals *dh,
				      AST_INDEX logical_proc_ast)
{
    if ( !dh->declLog2Phys) {
	dc_new_var(dh, pt_gen_ident(LOG_TO_PHYS), INTTYPE);
	dh->declLog2Phys = true;
    }
    return pt_gen_invoke(LOG_TO_PHYS, logical_proc_ast);
}
/*------------------------------------------------------------------------*/

/* eof */
