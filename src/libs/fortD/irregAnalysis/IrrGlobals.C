/* $Id: IrrGlobals.C,v 1.15 1997/03/11 14:28:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************
 * class IrrGlobals member functions
 */

/**********************************************************************
 * Revision History:
 * $Log: IrrGlobals.C,v $
 * Revision 1.15  1997/03/11 14:28:29  carr
 * newly checked in as revision 1.15
 *
Revision 1.15  94/03/21  13:43:23  patton
fixed comment problem
,

Revision 1.14  94/02/27  20:14:32  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.21  1994/02/27  19:42:11  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.20  1994/01/18  19:47:39  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.19  1993/10/04  15:36:19  reinhard
 * Changed inspector placement.
 *
 * Revision 1.18  1993/09/25  15:33:22  reinhard
 * Added dynamic memory handling.
 *
 * Revision 1.17  1993/09/02  18:48:03  reinhard
 * Disabled warning's indicating that the regular compiler has modified
 * ref's.
 */

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
//#include <include/fort/walk.h>
#include <libs/frontEnd/ast/AstIter.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/irregAnalysis/AST_SSA_Graph.h>
#include <libs/fortD/irregAnalysis/CommDFProblem.h>
#include <libs/fortD/irregAnalysis/CommDFUniv.h>
#include <libs/fortD/irregAnalysis/Exec.h>
#include <libs/fortD/irregAnalysis/Inspec.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/OnHomeTable.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/Slice.h>
#include <libs/fortD/irregAnalysis/ValDecomp.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(char *,    ftt_ast2str,       (FortTextTree ftt, AST_INDEX node));
EXTERN(Boolean,   collect_refs,      (S_desc_ht *refs, CfgInstance cfg,
				      Boolean do_all_arrays));
EXTERN(char *,    concatStrs,        (const char *str1,
				      const char *str2));
EXTERN(AST_INDEX, find_enclosing_DO, (AST_INDEX node));
EXTERN(const char *, find_pattern_insens, (const char *str, 
					   const char *pattern));
EXTERN(AST_INDEX, find_stmt_list,    (AST_INDEX node));
EXTERN(void,      insert_flags,      (AST_INDEX root_node,
				      Boolean *flags));
EXTERN(Boolean,   is_in_main,        (AST_INDEX node));
EXTERN(Boolean,   is_irreg_ref,      (AST_INDEX   ref_node,
				      CfgInstance cfg));
EXTERN(Boolean,   is_loop,           (AST_INDEX stmt));
EXTERN(Boolean,   match_pattern,     (const char *&str, 
				      const char *pattern));
EXTERN(Boolean,   match_boolean,     (const char *str));
EXTERN(char *,    prefix_str,        (char prefix, char *str));
EXTERN(AST_INDEX, prependLongComment, (const char *cmt_str,
				       AST_INDEX node));
EXTERN(const char *, ref2varName,    (AST_INDEX ref_node));
EXTERN(const char *, save_program,   (FortTree ft, FortTextTree ftt,
				      char *extension));
EXTERN(AST_INDEX, sn2arr_node,          (CfgInstance   cfg,
					 SymDescriptor symtab,
					 SsaNodeId     sn));


/**********************************************************************
 * Constructor
 */
IrrGlobals::IrrGlobals(FortTree     my_ft,
		       FortTextTree my_ftt,
		       Fd_opts      *my_fd_opts,
		       FortranDInfo *my_fd)
:
 cfg           (NULL),
 comm_df       (NULL),
 do_irreg      (false),
 exec_loops    (new AST_Set()),
 execs         (new Exec_ht(this)),
 fd            (my_fd),
 fd_opts       (my_fd_opts),
 ft            (my_ft),
 ftt           (my_ftt),
 flags         (fd_opts->flags),
 st            (new IrrSymTab()),
 init_node     (AST_NIL),
 insps         (new Inspec_ht(this)),
 mall          (new Mall()),
 on_home_table (new OnHomeTable(ftt)),
 refs          (new S_desc_ht(this)), 
 root_node     (ft_Root(ft)),
 ssa_graph     (NULL),
 hidden_ValDecompInfo ((Generic) new ValDecompInfo(st, fd)),
 is_main       (is_in_main(root_node))
{
  Boolean with_values = false;

  for (int i = 0; i < MallType_cnt; i++)
  {
    need_mall[i] = false;
  }

  if (!flags[Skip_irreg])            // Skip irregular phase ?
  {
    // Module passes type checking ?
    flags[Skip_irreg] = (Boolean) !ssa_init(with_values);
  }

  init_options();                    // Read in options from program text
  st->putFd(fd);
}


/**********************************************************************
 * Destructor
 */
IrrGlobals::~IrrGlobals()
{
  delete comm_df;
  delete execs;
  delete insps;
  delete mall;
  delete on_home_table;
  delete refs;
  delete st;
  delete (ValDecompInfo*) hidden_ValDecompInfo;
}


/**********************************************************************
 * getDo_all_arrays()
 */
Boolean
IrrGlobals::getDo_all_arrays() const
{
  return flags[Do_all_arrays];
}


/**********************************************************************
 * getSplit_comm()
 */
Boolean
IrrGlobals::getSplit_comm() const
{
  return flags[Split_comm];
}


/**********************************************************************
 * getSave_irreg()
 */
Boolean
IrrGlobals::getSave_irreg() const
{
  return flags[Save_irreg];
}


/**********************************************************************
 * getVal()
 */
Values
IrrGlobals::getVal() const
{
  return cfgval_get_values(cfg);
}


/**********************************************************************
 * collect()
 */
void
IrrGlobals::collect()
{
  Boolean with_values = true;

  if ((!flags[Skip_irreg])         // Skip irregular phase ?
      && (ssa_init(with_values)))  // Module passes type checking ?
  {
    if (collect_refs())            // Collect all "irregular" refs
    {
      do_irreg = true;
      collect_exec_loops();        // Collect all exec loops
    }
  }
}


/**********************************************************************
 * init_options() Processes comments of the form
 *                          C     --<< split_comm: false >>--
 *                          C     --<< do_all_arrays: true >>--
 *
 * NOTE: this routine converts everything to lower case before it
 *       matches patterns.
 */
void
IrrGlobals::init_options()
{
  char       *pattern_str;
  const char *cmt, *def;
  int        i;
  AST_INDEX node;
  AstIter   iter(root_node);

  // Scan Fortran source for <do_all_arrays>, etc., flags
  // These flags can also be set as a command line option
  while ((node = iter()) != AST_NIL)
  {
    if (is_comment(node))
    {
      cmt = gen_get_text(gen_COMMENT_get_text(node));
      if (match_pattern(cmt, "--<< "))
      {
	// Loop through different flags, try to match each of them.
	for (i = 0; i < Fd_flags_cnt; i++)
	{
	  pattern_str = concatStrs(fd_flags_names[i], ": ");
	  if (def = find_pattern_insens(cmt, pattern_str))
	  {
	    flags[i] = match_boolean(def);
	  }
	  delete pattern_str;
	}
      }
    }
  }
}


/**********************************************************************
 * analyze()  ANALYSIS PHASE  (No modifications of AST yet)
 */
void
IrrGlobals::analyze()
{
  S_desc_ht_Iter iter(refs);
  S_desc         *ref;

  //ssa_init();

  // Scan for value-based decompositions
  // (Very grubby scheme, need to integrate better w/ regular compiler)
  //ValDecomp_collect_directives(valdecomp_ht);

  // Generate Slices for References
  for (iter.reset(); ref = iter();)
  {
    ref->genSlices();
  }

  // Analyze communication requirements (via dataflow)
  // and collect References
  comm_df = new CommDFProblem(cfg, refs, st, flags[Split_comm],
			      flags[Gen_high_level]);
  comm_df->solve();
  
  // Placement of inspector for all refs
  comm_df->forall_annots(&CommDFAnnot::place_Inspecs);

  for (iter.reset(); ref = iter();)
  {
    // Generate Executors for References
    ref->genExec();
    
    // Generate Inspectors for each Slice of each Reference
    ref->genInspec();
  }
  
  // 5/26/93 RvH: the following bombs.
  //delete ssa_graph;  // This cannot be done after AST gets modified
}


/**********************************************************************
 *  gen_code()   CODE GENERATION PHASE  (Modifications of AST)
 */
void
IrrGlobals::gen_code()
{
  S_desc  *ref;
  Inspec  *insp;
  Exec    *exec;
  
  // 1. Add comment showing which options are in effect
  insert_flags(root_node, flags);

  // 2. If requested, save intermediate file
  //    Save it now if the regular compiler has already generated code,
  //    otherwise save it after code generation.
  if (flags[Save_irreg] && !flags[Code_before_reg])
  {
    (void) save_program(ft, ftt, ".irreg.f");
  }

  // Add 'implicit none'
  (void) prependLongComment("--<<F77:implicit none",
			    list_first(find_stmt_list(root_node)));

  // 3. Make the slices recalculate size of trace array, etc.
  // This is needed, for example, when loop distribution has been
  // performed by regular compiler after the analysis phase of
  // the irregular compiler.
  ssa_init();
  for (S_desc_ht_Iter iter_ref(refs); ref = iter_ref();)
  {
    ref->tree_changed();
  }

  // 4. Install inspectors
  // [This cannot be done after 5.]
  // insps->forall(&Inspec::install);
  if (!flags[Gen_high_level])
  {
    for (AST_ht_Iter iter_insp(insps);
	 insp = (Inspec *) iter_insp();
	 insp->install());
  }

  // 5. Generate communication stmts
  // [This cannot be done after 6.]
  comm_df->gen_comm();
  
  // 6. Update ON_HOME directives in program text & their table
  if (!flags[Gen_high_level])
    on_home_table->fixup_program();

  // Initialize reduction buffers
  comm_df->forall_annots(&CommDFAnnot::gen_red_init_stmts);
  
  // 7. Generate executors
  //execs->forall(&Exec::install);
  if (!flags[Gen_high_level])
  {
    for (AST_ht_Iter iter_exec(execs);
	 exec = (Exec *) iter_exec();
	 exec->install());
  }

  // 8. Generate code for remapping of value based decompositions
  ((ValDecompInfo*) hidden_ValDecompInfo)->gencode_redists();

  // 9. If requested, save intermediate file
  //    Save it now if the regular compiler has not generated code yet,
  //    otherwise save it at the beginning of code generation.
  if (flags[Save_irreg])
  {
    (void) save_program(ft, ftt, ".no_mall.f");
  }

  // 10. Handle dynamic array allocation
  if (!flags[Gen_high_level])
  {
    //do_mall();
  }

  // 10. If requested, save intermediate file
  //    Save it now if the regular compiler has not generated code yet,
  //    otherwise save it at the beginning of code generation.
  if (flags[Save_irreg] && flags[Code_before_reg])
  {
    (void) save_program(ft, ftt, ".irreg.f");
  }

  // 9. Check the generated module
  (void) check_module();
}


/**********************************************************************
 * find_ex_stmt()  Find first executable node
 */
AST_INDEX
IrrGlobals::find_ex_stmt() const
{
  AST_INDEX node;

  for (node = list_first(find_stmt_list(root_node));
       !is_executable_stmt(node);
       node = list_next(node));
  
  return node;
}


/**********************************************************************
 * do_mall()  Handle allocation of dynamic arrays
 */
void
IrrGlobals::do_mall()
{
  AST_INDEX node;
  int       type;
  char      *init_name;

  // Add "call PARTI_setup()"
  if (do_irreg && is_main)
  {
    node = pt_gen_call("PARTI_setup", list_create(AST_NIL));
    add_init_stmt(node);
  }

  // Generate "call iputsize(1000)" for each needed type
  for (type = 0; type < MallType_cnt; type++)
  {
    if (need_mall[type])
    {
      init_name = prefix_str(MallType_prefixes[type], "putsize");
      node      = mall_size_node(type);
      node      = list_create(node);
      node      = pt_gen_call(init_name, node);
      add_init_stmt(node);
    }
  }
    
  // Do the actual conversion
  mall->convert(ft, ftt, st);
}


/**********************************************************************
 * add_init_stmt()
 */
void
IrrGlobals::add_init_stmt(AST_INDEX node)
{
  if (init_node == AST_NIL)
  {
    init_node = find_ex_stmt();
  }

  (void) list_insert_before(init_node, node);
}


/**********************************************************************
 * mall_size_node()  Pick work array size
 */
AST_INDEX
IrrGlobals::mall_size_node(int type)
{
  AST_INDEX node;
  int       size;

  size = fd_opts->wrk_array_sizes[type];
  if (!size)
  {
    // 9/20/93 RvH: Just a big constant for now; should probably call
    //              some system routine to figure out available space
    size = 500000;
  }
  node = pt_gen_int(size);

  return node;
}


/**********************************************************************
 * Ast2Str()  Unparse AST index
 */
char *
IrrGlobals::Ast2Str(AST_INDEX node) const
{
  return ftt_ast2str(ftt, node);
}


/**********************************************************************
 * check_module()  Check module for correctness.
 *
 * 5/26/93 RvH: Currently this is done by recomputing the symbol table.
 */
Boolean
IrrGlobals::check_module()
{
  //FortTree ft = FortTree(PED_FT(ped));
  const char *err_file_name;

  ftt_TreeChanged(ftt, ft_Root(ft));

#if 0
  // Save error file before type checking, in case type checking bombs
  err_file_name = save_program(ft, ftt, ".error.f");
#endif

  ft_States module_ok = ft_SymRecompute(ft);  // Recompute symbol table

  if (module_ok != ft_CORRECT)
  {
    cerr << "WARNING: IrrGlobals::check_module(): " <<
      "Module contains errors; run \"checker -M " <<
	err_file_name << "\".\n";
  }
  else
  {
    st->init(ft);           // Initialize symbol table interface
  }

  return BOOL(module_ok == ft_CORRECT);
}


/**********************************************************************
 * ssa_init()  Compute cfg/ssa info.
 */
Boolean
IrrGlobals::ssa_init(Boolean with_values)
{
  Boolean module_ok = check_module();  // Check correctness of module

  if (module_ok) {
    // delete ssa_graph;   
    // 4/19/93 RvH: "delete ssa_graph" bombs if the AST has been
    // messed with since the ssa_graph has been created.
    // Therefore a method of IrrGlobals, ssa_delete(), has to be called
    // before AST is modified again.

    ssa_graph = new AST_SSA_Graph(ft, with_values);
    cfg       = (CfgInstance) ssa_graph->getGraph();
    ((ValDecompInfo*) hidden_ValDecompInfo)->putCfg(cfg);
  }

  return module_ok;
}


/**********************************************************************
 * ssa_delete()  Destroy cfg/ssa info.
 */
void
IrrGlobals::ssa_delete()
{
  delete ssa_graph;   
  cfg = NULL;
}


/**********************************************************************
 * query_irreg_ref()  Query <refs> for <node>
 */
Boolean
IrrGlobals::query_irreg_ref(AST_INDEX node) const
{
  Boolean is_irregular = refs->query_entry(node);
  //char    *ref_str;

  if ((!flags[Skip_irreg]) && (!is_irregular))
  {
    // We get here when the regular compiler has introduced / modified
    // irregular subscripts.

    //ftt_TreeChanged(ftt, ft_Root(ft));
    //ref_str = Ast2Str(node);
    //ftt_TreeWillChange(ftt, ft_Root(ft));
    //cerr << "WARNING: query_irreg_ref(): \"" << ref_str <<
    //  "\", node = " << node << ", not in <refs>.\n";
    //delete ref_str;
  }

  return is_irregular;
}


/**********************************************************************
 * needs_reg_comm()  True if a reference <node> might need regular
 *                   communication.
 */
Boolean
IrrGlobals::needs_reg_comm(AST_INDEX node)
{
  const char *name;
  Boolean    result = false;

  if (!refs->query_entry(node))        // Is <node> irregular ?
  {
    name   = ref2varName(node);
    result = (Boolean) !st->is_local(name);    // Is <node> a local reference ?
  }

  return result;
}



/*********************************************************************
 * collect_refs()  Collect all "irregular" array references
 */
Boolean
IrrGlobals::collect_refs()
{
  SymDescriptor symtab       = cfg_get_inst_symtab(cfg);
  fst_index_t   max_index    = fst_MaxIndex(symtab);
  Boolean       *irreg_index = new Boolean[max_index];
  fst_index_t   index;
  SsaNodeId     sn;
  AST_INDEX     ref_node;
  irr_type      kind;
  Boolean       is_irreg = false;      // Any irreg refs to process ?

  // Initially assume that there are no irregularly referenced arrays
  for (index = 0;  index < max_index;  index++) {
    irreg_index[index] = false;
  }

  // Collect "irregular" arrays,
  // which are referenced irregularly somewhere
  for(sn = ssa_get_first_node(cfg);
      (sn = ssa_get_next_node(cfg, sn)) != SSA_NIL;)
  {
    // For now: collect all array refs which are distributed
    // AND have any subscript that
    // is not either constant or an induction variable
    ref_node = sn2arr_node(cfg, symtab, sn);
    if ((ref_node != AST_NIL)
	&& (flags[Do_all_arrays]
	    || st->is_irreg_and_distrib(ref_node, cfg))) 
	//&& st->is_distributed(ref_node)
	//&& (flags[Do_all_arrays] || is_irreg_ref(ref_node, cfg))) 
    {
      index = ssa_node_name(cfg, sn);
      irreg_index[index] = true;
      is_irreg = true;
    }
  }
      
  // Collect all references to "irregular" arrays
  // and generate an S_desc for them
  for(sn = ssa_get_first_node(cfg);
      (sn = ssa_get_next_node(cfg, sn)) != SSA_NIL;)
  {
    if (((index = ssa_node_name(cfg, sn)) >= 0)
	&& (index < max_index)
	&& irreg_index[index]
	&& ((ref_node = ssa_node_to_ast(cfg, sn)) != AST_NIL)
	//&& (ssa_node_type(cfg, sn) != SSA_IP_IN))
	&& (is_subscript(ref_node)))
    {
      kind = ssa_is_use(cfg, sn) ? rhs : lhs;
      (void) refs->gen_entry_by_AST(ref_node, kind);
    }
  }

  delete irreg_index;

  //ssa_delete();
  
  return is_irreg;
}


/**********************************************************************
 * register_loops()
 */
void 
IrrGlobals::register_loops(AST_INDEX loop_node)
{
  AST_INDEX node;
  AstIter   iter(loop_node);

  while ((node = iter()) != AST_NIL)
  {
    if (is_loop(node))
    {
      *exec_loops += node;
    }
  }
}


/*********************************************************************
 * collect_exec_loops()  Collect all exec loops.
 *
 * For now this is done to prevent interchange in the regular compiler.
 */
void
IrrGlobals::collect_exec_loops()
{
  AST_INDEX exec_node;
  S_desc    *ref;

  for (S_desc_ht_Iter iter(refs); ref = iter();)
  {
    exec_node = ref->getOutmost_loop_node();
    if (exec_node != AST_NIL)
    {
      register_loops(exec_node);
    }
  }
}
