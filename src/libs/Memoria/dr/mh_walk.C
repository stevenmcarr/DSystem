/* $Id: mh_walk.C,v 1.4 1992/10/03 15:48:25 rn Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <mh.h>
#include <Arena.h>
#include <memory_menu.h>
#include <mh_walk.h>
#include <fort/gi.h>
#include <header.h>


static int set_scratch(AST_INDEX node,
		       int       dummy)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   set_scratch_to_NULL(node);
   return(WALK_CONTINUE);
  }

static int remove_do_labels(AST_INDEX stmt,
			    int       level,
			    int       dummy)

  {
   if (is_do(stmt))
     gen_DO_put_lbl_ref(stmt,AST_NIL);
   return(WALK_CONTINUE);
  }

static int pre_walk(AST_INDEX      stmt,
		    int            level,
		    walk_info_type *walk_info)

  {
   if (is_program(stmt))
     {
      walk_info->symtab = ft_SymGetTable(walk_info->ft,
				     gen_get_text(gen_PROGRAM_get_name(stmt)));
      fst_InitField(walk_info->symtab,NEW_VAR,false,0);
     }
   else if (is_subroutine(stmt))
     {
      walk_info->symtab = ft_SymGetTable(walk_info->ft,
				  gen_get_text(gen_SUBROUTINE_get_name(stmt)));
      fst_InitField(walk_info->symtab,NEW_VAR,false,0);
     }
   else if (is_function(stmt))
     {
      walk_info->symtab = ft_SymGetTable(walk_info->ft,
				  gen_get_text(gen_FUNCTION_get_name(stmt)));
      fst_InitField(walk_info->symtab,NEW_VAR,false,0);
     }
   return(WALK_CONTINUE);
  }

static int post_walk(AST_INDEX      stmt,
		     int            level,
		     walk_info_type *walk_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX new_stmt;
   int       logval;
   FILE      *logfile;
   Boolean   perfect = true;

   if (is_do(stmt) && level == LEVEL1)
     {
      walk_info->nests++;
      walk_expression(stmt,set_scratch,NOFUNC,(Generic)NULL);
      logval = ((config_type *)PED_MH_CONFIG(walk_info->ped))->logging;
      logfile = ((config_type *)PED_MH_CONFIG(walk_info->ped))->logfile;
      switch(walk_info->selection) {
#ifdef WAITING_FOR_STEVE
	case INTERSTATS:     memory_interchange_stats(walk_info->ped,stmt,
						     LEVEL1,
						     &perfect,
						     &walk_info->total_loops,
						     walk_info->symtab,
						     walk_info->ar);
			     walk_expression(stmt,set_scratch,NOFUNC,
					     (Generic)NULL);
	                     walk_info->ar->arena_deallocate(LOOP_ARENA);
			     if (NOT(perfect))
			       walk_info->imperfect++;
	                     break;
#endif
	case INTERCHANGE:    memory_loop_interchange(walk_info->ped,stmt,
						     LEVEL1,walk_info->symtab,
						     walk_info->ar);
			     walk_expression(stmt,set_scratch,NOFUNC,
					     (Generic)NULL);
	                     walk_info->ar->arena_deallocate(LOOP_ARENA);
	                     break;
	case SCALAR_REP:     if (logval > LOG_UNROLL && logfile != NULL)
			       fprintf(logfile,"SCALAR_REPLACEMENT ON LOOP\n");
	                     memory_scalar_replacement(walk_info->ped,stmt,
						       walk_info->symtab,
						       walk_info->ar);
			     walk_expression(stmt,set_scratch,NOFUNC,
					     (Generic)NULL);
	                     walk_info->ar->arena_deallocate(LOOP_ARENA);
	                     break;
	case UNROLL_AND_JAM: fst_InitField(walk_info->symtab,EXPAND_LVL,0,0);
	                     if ((logval == LOG_UNROLL || logval == LOG_ALL) &&
				 logfile != NULL)
			       fprintf(logfile,"UNROLL-AND-JAM ON LOOP\n");
			     walk_expression(stmt,set_scratch,NOFUNC,
					     (Generic)NULL);
	                     (void)memory_unroll_and_jam(walk_info->ped,stmt,
							 level,2,
							 walk_info->symtab,
							 walk_info->ar);
	                     walk_info->ar->arena_deallocate(LOOP_ARENA);
	                     fst_KillField(walk_info->symtab,EXPAND_LVL);
	                     break;
	case MEM_ALL:        memory_loop_interchange(walk_info->ped,stmt,
						     LEVEL1,walk_info->symtab,
						     walk_info->ar); 
			     walk_expression(stmt,set_scratch,NOFUNC,
					     (Generic)NULL);
	                     walk_info->ar->arena_deallocate(LOOP_ARENA);
	                     fst_InitField(walk_info->symtab,EXPAND_LVL,0,0);
	                     if ((logval == LOG_UNROLL || logval == LOG_ALL) &&
				 logfile != NULL)
			       fprintf(logfile,"UNROLL-AND-JAM ON LOOP\n");
	                     new_stmt = memory_unroll_and_jam(walk_info->ped,
							      stmt, level,2,
							     walk_info->symtab,
							      walk_info->ar);
			     walk_expression(stmt,set_scratch,NOFUNC,
					     (Generic)NULL);
	                     walk_info->ar->arena_deallocate(LOOP_ARENA);
	                     fst_KillField(walk_info->symtab,EXPAND_LVL);
	                     if (logval > LOG_UNROLL && logfile != NULL)
			       fprintf(logfile,"SCALAR_REPLACEMENT ON LOOP\n");
                             while (new_stmt != stmt)
			       {
				if (is_do(new_stmt))
				  memory_scalar_replacement(walk_info->ped,
							    new_stmt,
							    walk_info->symtab,
							    walk_info->ar);
	                         walk_info->ar->arena_deallocate(LOOP_ARENA);
				new_stmt = list_next(new_stmt);
			       }
	                     memory_scalar_replacement(walk_info->ped,stmt,
						       walk_info->symtab,
						       walk_info->ar);
			     walk_expression(stmt,set_scratch,NOFUNC,
					     (Generic)NULL);
	                     walk_info->ar->arena_deallocate(LOOP_ARENA);
	                     break;
       }
      return(WALK_FROM_OLD_NEXT);
     } 
   return(WALK_CONTINUE);
  }

static int build_label_symtab(AST_INDEX   stmt,
                              int         level,
                              walk_info_type *walk_info)

  {
   AST_INDEX label_ref;

     if (is_program(stmt))
       {
        walk_info->symtab = ft_SymGetTable(walk_info->ft,
				     gen_get_text(gen_PROGRAM_get_name(stmt)));
	fst_InitField(walk_info->symtab,REFD,false,0);
       }  
     else if (is_subroutine(stmt))
       {
	walk_info->symtab = ft_SymGetTable(walk_info->ft,
				  gen_get_text(gen_SUBROUTINE_get_name(stmt)));
	fst_InitField(walk_info->symtab,REFD,false,0);
       }
     else if (is_function(stmt))
       {
	walk_info->symtab = ft_SymGetTable(walk_info->ft,
				  gen_get_text(gen_FUNCTION_get_name(stmt)));
	fst_InitField(walk_info->symtab,REFD,false,0);
       }
     else if (is_goto(stmt))
       fst_PutField(walk_info->symtab,gen_get_text(gen_GOTO_get_lbl_ref(stmt)),
		    REFD,true);
     else if (is_arithmetic_if(stmt))
       {
	fst_PutField(walk_info->symtab,
		     gen_get_text(gen_ARITHMETIC_IF_get_lbl_ref1(stmt)),REFD,
		     true);
	fst_PutField(walk_info->symtab,
		     gen_get_text(gen_ARITHMETIC_IF_get_lbl_ref2(stmt)),REFD,
		     true);
	fst_PutField(walk_info->symtab,
		     gen_get_text(gen_ARITHMETIC_IF_get_lbl_ref3(stmt)),REFD,
		     true);
       }
     else if (is_computed_goto(stmt))
       for (label_ref = list_first(gen_COMPUTED_GOTO_get_lbl_ref_LIST(stmt));
	    label_ref != AST_NIL;
	    label_ref = list_next(label_ref))
         fst_PutField(walk_info->symtab,gen_get_text(label_ref),REFD,true);
     return(WALK_CONTINUE);
  }


static int remove_bogus_labels(AST_INDEX     stmt,
                               int           level,
                               SymDescriptor symtab)

  {
   AST_INDEX label;

     if (labelled_stmt(stmt) && !is_format(stmt))
       if ((label = gen_get_label(stmt)) != AST_NIL)
         if (!fst_GetField(symtab,gen_get_text(label),REFD))
	   if (is_continue(stmt))
	     {
	      (void)list_remove_node(stmt);
	      return(WALK_FROM_OLD_NEXT);
	     }
	   else
            pt_tree_replace(label,AST_NIL);
     return(WALK_CONTINUE);
  }

static int check_labels(AST_INDEX   stmt,
                        int         level,
                        walk_info_type *walk_info)

  {
   if (is_program(stmt))
     {
      walk_statements(gen_PROGRAM_get_stmt_LIST(stmt),level,
		      remove_bogus_labels,NOFUNC,(Generic)walk_info->symtab);
      fst_KillField(walk_info->symtab,REFD);
     }
   else if (is_subroutine(stmt))
     {
      walk_statements(gen_SUBROUTINE_get_stmt_LIST(stmt),level,
		      remove_bogus_labels,NOFUNC,(Generic)walk_info->symtab);
      fst_KillField(walk_info->symtab,REFD);
     }
   else if (is_function(stmt))
     {
      walk_statements(gen_FUNCTION_get_stmt_LIST(stmt),level,
		      remove_bogus_labels,NOFUNC,(Generic)walk_info->symtab);
      fst_KillField(walk_info->symtab,REFD);
     }
   return(WALK_CONTINUE);
  }

static void check_decl(SymDescriptor  symtab,
                       fst_index_t    index,
                       decl_list_type *decl_lists)
  
  {
   if (fst_GetFieldByIndex(symtab,index,NEW_VAR))
     {
      switch(fst_GetFieldByIndex(symtab,index,SYMTAB_TYPE)) {
	case TYPE_DOUBLE_PRECISION:
	  list_insert_last(decl_lists->dbl_prec_list,gen_ARRAY_DECL_LEN(
                           pt_gen_ident((char *)fst_GetFieldByIndex(symtab,
								    index,
							    SYMTAB_NAME)),
		           AST_NIL,AST_NIL,AST_NIL));
	  break;
	case TYPE_REAL:
	  list_insert_last(decl_lists->real_list,gen_ARRAY_DECL_LEN(
                           pt_gen_ident((char*)fst_GetFieldByIndex(symtab,
								   index,
							    SYMTAB_NAME)),
		           AST_NIL,AST_NIL,AST_NIL));
	  break;
	case TYPE_COMPLEX:
	  list_insert_last(decl_lists->cmplx_list,gen_ARRAY_DECL_LEN(
                           pt_gen_ident((char*)fst_GetFieldByIndex(symtab,
								   index,
							    SYMTAB_NAME)),
			   AST_NIL,AST_NIL,AST_NIL));
	  break;
	default:;
       }
     }
  }

static void make_decls(SymDescriptor symtab,
                       AST_INDEX     stmt_list)

  {
   decl_list_type decl_lists;
   AST_INDEX      type_stmt;
   
     decl_lists.dbl_prec_list = list_create(AST_NIL);
     decl_lists.real_list = list_create(AST_NIL);
     decl_lists.cmplx_list = list_create(AST_NIL);
     fst_ForAll(symtab,check_decl,(Generic)&decl_lists);
     if (!list_empty(decl_lists.dbl_prec_list))
       {
	type_stmt = gen_TYPE_STATEMENT(AST_NIL,gen_TYPE_LEN(gen_REAL(),
                                       pt_gen_int(SIZE_PER_DB_PREC)),
				       decl_lists.dbl_prec_list);
	ft_SetComma(type_stmt,false);
	list_insert_first(stmt_list,type_stmt);
       }
     else
       tree_free(decl_lists.dbl_prec_list);
     if (!list_empty(decl_lists.real_list))
       {
	type_stmt = gen_TYPE_STATEMENT(AST_NIL,gen_TYPE_LEN(gen_REAL(),
                                       pt_gen_int(SIZE_PER_REAL)),
				       decl_lists.real_list);
	ft_SetComma(type_stmt,false);
	list_insert_first(stmt_list,type_stmt);
       }
     else
       tree_free(decl_lists.real_list);
     if (!list_empty(decl_lists.cmplx_list))
       {
	type_stmt = gen_TYPE_STATEMENT(AST_NIL,gen_TYPE_LEN(gen_COMPLEX(),
					       pt_gen_int(SIZE_PER_COMPLEX)),
				       decl_lists.cmplx_list);
	ft_SetComma(type_stmt,false);
	list_insert_first(stmt_list,type_stmt);
       }
     else
       tree_free(decl_lists.cmplx_list);
				       
  }

static int get_symtab_for_decls(AST_INDEX     stmt,
                                int           level,
                                FortTree      ft)

  {
   SymDescriptor symtab = NULL;

     if (is_program(stmt))
       {
	symtab = ft_SymGetTable(ft,gen_get_text(gen_PROGRAM_get_name(stmt)));
	make_decls(symtab,gen_PROGRAM_get_stmt_LIST(stmt));
	fst_KillField(symtab,NEW_VAR);
	return(WALK_FROM_OLD_NEXT);
       }
     else if (is_subroutine(stmt))
       {
	symtab=ft_SymGetTable(ft,gen_get_text(gen_SUBROUTINE_get_name(stmt)));
	make_decls(symtab,gen_SUBROUTINE_get_stmt_LIST(stmt));
	fst_KillField(symtab,NEW_VAR);
	return(WALK_FROM_OLD_NEXT);
       }
     else if (is_function(stmt))
       {
	symtab = ft_SymGetTable(ft,gen_get_text(gen_FUNCTION_get_name(stmt)));
	make_decls(symtab,gen_FUNCTION_get_stmt_LIST(stmt));
	fst_KillField(symtab,NEW_VAR);
	return(WALK_FROM_OLD_NEXT);
       }
     else
       return(WALK_CONTINUE);
  }


void mh_walk_ast(int          selection,
                 PedInfo      ped,
                 AST_INDEX    root,
		 FortTree     ft,
		 arena_type   *ar)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


  {
   walk_info_type walk_info;
   static int file = 0;
   char fn[80];
   
     if (((config_type *)PED_MH_CONFIG(ped))->logging > 0 ||
	 selection == INTERSTATS)
       {
	sprintf(fn,"%s/logfile%d",getenv("HOME"),file);
	((config_type *)PED_MH_CONFIG(ped))->logfile = fopen(fn,"w");
       }
     walk_info.selection = selection;
     walk_info.ped = ped;
     walk_info.ft = ft;
     walk_info.ar = ar;
     walk_info.imperfect = 0;
     walk_info.total_loops = 0;
     walk_info.nests = 0;
     walk_statements(root,LEVEL1,remove_do_labels,NOFUNC,(Generic)NULL);
     walk_statements(root,LEVEL1,build_label_symtab,check_labels,
		     (Generic)&walk_info);
     walk_statements(root,LEVEL1,pre_walk,post_walk,
		     (Generic)&walk_info);
     walk_statements(root,LEVEL1,build_label_symtab,check_labels,
		     (Generic)&walk_info);
     walk_statements(root,LEVEL1,get_symtab_for_decls,NOFUNC,
		     (Generic)ft);
     if (selection == INTERSTATS)
       {
	fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
		"\n\nTotal Loops: %d\n",walk_info.total_loops);
	fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
		"Loop Nests: %d\n",walk_info.nests);
	fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
		"Imperfect Nests: %d\n",walk_info.imperfect);
       }
     if (((config_type *)PED_MH_CONFIG(ped))->logging > 0 ||
	 selection == INTERSTATS)
       fclose(((config_type *)PED_MH_CONFIG(ped))->logfile);
  }


