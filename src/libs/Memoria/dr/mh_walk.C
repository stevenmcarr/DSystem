/* $Id: mh_walk.C,v 1.5 1992/12/07 10:09:28 carr Exp $ */
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
#include <LoopStats.h>
#include <mh_config.h>
#include <dialogs/message.h>

static walk_info_type memory_walk_info;

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


static void InterchangeStats(AST_INDEX      stmt,
			     int            level,
			     walk_info_type *walk_info)
  {
   memory_interchange_stats(walk_info->ped,stmt,
			    LEVEL1,
			    &walk_info->LoopStats,
			    walk_info->symtab,
			    walk_info->ar);
   walk_expression(stmt,set_scratch,NOFUNC,
		   (Generic)NULL);
   walk_info->ar->arena_deallocate(LOOP_ARENA);
   if (NOT(walk_info->LoopStats.Perfect))
     walk_info->LoopStats.Imperfect++;
  }


static void Interchange(AST_INDEX      stmt,
			int            level,
			walk_info_type *walk_info)
  {
   memory_loop_interchange(walk_info->ped,stmt,
			   LEVEL1,walk_info->symtab,
			   walk_info->ar);
   walk_expression(stmt,set_scratch,NOFUNC,
		   (Generic)NULL);
   walk_info->ar->arena_deallocate(LOOP_ARENA);
  }


static void ScalarReplacement(AST_INDEX      stmt,
			      int            level,
			      walk_info_type *walk_info)
  {
   memory_scalar_replacement(walk_info->ped,stmt,
			     walk_info->symtab,
			     walk_info->ar);
   walk_expression(stmt,set_scratch,NOFUNC,
		   (Generic)NULL);
   walk_info->ar->arena_deallocate(LOOP_ARENA);
  }


static AST_INDEX UnrollAndJam(AST_INDEX      stmt,
			      int            level,
			      walk_info_type *walk_info)
  {
   AST_INDEX new_stmt;

     fst_InitField(walk_info->symtab,EXPAND_LVL,0,0);
     walk_expression(stmt,set_scratch,NOFUNC,(Generic)NULL);
     new_stmt = memory_unroll_and_jam(walk_info->ped,stmt,level,2,
				      walk_info->symtab,walk_info->ar);
     walk_info->ar->arena_deallocate(LOOP_ARENA);
     fst_KillField(walk_info->symtab,EXPAND_LVL);
     return(new_stmt);
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

   if (is_do(stmt) && level == LEVEL1)
     {
      walk_info->LoopStats.Nests++;
      walk_info->LoopStats.Perfect = true;
      walk_expression(stmt,set_scratch,NOFUNC,(Generic)NULL);
      switch(walk_info->selection) {
	case INTERSTATS:     InterchangeStats(stmt,level,walk_info);
	                     break;
	case INTERCHANGE:    Interchange(stmt,level,walk_info);
	                     break;
	case SCALAR_REP:     ScalarReplacement(stmt,level,walk_info);
	                     break;
	case UNROLL_AND_JAM: (void)UnrollAndJam(stmt,level,walk_info);
	                     break;
	case UNROLL_SCALAR:  new_stmt = UnrollAndJam(stmt,level,walk_info);
                             while (new_stmt != stmt)
			       {
				if (is_do(new_stmt))
				  {
				   ScalarReplacement(new_stmt,level,walk_info);
				   walk_info->ar->arena_deallocate(LOOP_ARENA);
				  }
				new_stmt = list_next(new_stmt);
			       }
			     ScalarReplacement(stmt,level,walk_info);
	                     break;
	case LI_SCALAR:      Interchange(stmt,level,walk_info);
			     ScalarReplacement(stmt,level,walk_info);
	                     break;
	case LI_UNROLL:      Interchange(stmt,level,walk_info);
			     (void)UnrollAndJam(stmt,level,walk_info);
	                     break;
	case MEM_ALL:        Interchange(stmt,level,walk_info);
			     new_stmt = UnrollAndJam(stmt,level,walk_info);
                             while (new_stmt != stmt)
			       {
				if (is_do(new_stmt))
				  {
				   ScalarReplacement(new_stmt,level,walk_info);
				   walk_info->ar->arena_deallocate(LOOP_ARENA);
				  }
				new_stmt = list_next(new_stmt);
			       }
			     ScalarReplacement(stmt,level,walk_info);
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

/****************************************************************************/

static void print_RefGroups(FILE *logfile, char *title,
	int s1, int s2, int s3, int m1, int m2, int m3,
	int g1, int g2, int g3, int os)
{
	int r1, r2, r3, r4;
	double a1, a2, a3, a4;

	r1 = s1+m1;
	r2 = s2+m2;
	r3 = s3+m3;
    r4 = r1+r2+r3;

	a1 = (r1 != 0) ? double(s1+g1)/r1 : double(0);
	a2 = (r2 != 0) ? double(s2+g2)/r2 : double(0);
	a3 = (r3 != 0) ? double(s3+g3)/r3 : double(0);
    a4 = (r4 != 0) ? double(s1+g1+s2+g2+s3+g3)/r4 : double(0);

	fprintf(logfile,title);
	fprintf(logfile,"=======================\n\n");
	fprintf(logfile,"\t\t\tInvariant   Spatial    None    Total\n");
	fprintf(logfile,"\t\t\t=========   =======    ====    =====\n");
	fprintf(logfile,"\tTotal RG   \t%6d    %6d    %6d    %6d\n",
		 r1,r2,r3,r4);
	fprintf(logfile,"\tAvg Size   \t%6.2f    %6.2f    %6.2f    %6.2f\n\n",
		 a1,a2,a3,a4);
	fprintf(logfile,"\tSingle RG  \t%6d    %6d    %6d    %6d\n",
		 s1,s2,s3,s1+s2+s3);
	fprintf(logfile,"\tMulti RG   \t%6d    %6d    %6d    %6d\n",
		 m1,m2,m3,m1+m2+m3);
	fprintf(logfile,"\tRefs in RG \t%6d    %6d    %6d    %6d\n",
		 g1,g2,g3,g1+g2+g3);
	fprintf(logfile,"\tOther Spatial:   %d\n\n\n",os);
}


static void memory_stats_dump(FILE *logfile, walk_info_type *w)
{
 int i;
 float TotalFinalRatio = 0.0,
       TotalMemoryRatio = 0.0;
 float WeightedTotalFinalRatio = 0.0,
       WeightedTotalMemoryRatio = 0.0;
 int NumberLoops = 0;
 int WeightedNumberLoops = 0;

	fprintf(logfile,
		"\n\nTotal Loops: %d\n",w->LoopStats.TotalLoops);
	fprintf(logfile,
		"Loop Nests: %d\n",w->LoopStats.Nests);
	fprintf(logfile,
		"Imperfect Nests: %d\n",w->LoopStats.Imperfect);

	fprintf(logfile,
		"Permutable Sets of Loop Nests: %d\n",
		w->LoopStats.InMemoryOrder +
		w->LoopStats.InterchangedIntoMemoryOrder +
		w->LoopStats.NotInMemoryOrder);

	fprintf(logfile,
		"\n\nStats for Permutable Sets of Loops\n\n");
	fprintf(logfile,
		"\tIn Memory Order:                                %d\n",
		w->LoopStats.InMemoryOrder);
	fprintf(logfile,
		"\tInterchanged Into Memory Order:                 %d\n",
		w->LoopStats.InterchangedIntoMemoryOrder);
	fprintf(logfile,
		"\tUnable to Achieve Memory Order:                 %d\n",
		w->LoopStats.NotInMemoryOrder);

	fprintf(logfile,
		"\tTime Step Loop Prevented Memory Order: %d\n",
		w->LoopStats.TimeStepPreventedMemoryOrder);
	fprintf(logfile,
		"\tNearby Permutation Attained: %d\n",
		w->LoopStats.NearbyPermutationAttained);

	fprintf(logfile,
		"\tInner Loop Correct:                             %d\n",
		w->LoopStats.InnerLoopAlreadyCorrect);
	fprintf(logfile,
		"\tInterchanged to Obtain Correct Inner Loop:      %d\n",
		w->LoopStats.ObtainedInnerLoop);
	fprintf(logfile,
		"\tUnable to Achieve Correct Inner Loop:           %d\n",
		w->LoopStats.WrongInnerLoop);

	fprintf(logfile,
		"\tDesired Inner Loop was a Time Step Loop : %d\n",
		w->LoopStats.DesiredInnerTimeStep);
	fprintf(logfile,
		"\tNext Best Inner Loop Attained: %d\n",
		w->LoopStats.NextInnerLoop);
	fprintf(logfile,
		"\tInterchange Unsafe:                             %d\n",
		w->LoopStats.UnsafeInterchange);
	fprintf(logfile,
		"\tDistribution Unsafe:                            %d\n",
		w->LoopStats.DistributionUnsafe);
	fprintf(logfile,
		"\tNeeds Scalar Expansion:                         %d\n",
		w->LoopStats.NeedsScalarExpansion);
	fprintf(logfile,
		"\tToo Complex:                                    %d\n\n\n",
		w->LoopStats.TooComplex);

		print_RefGroups(logfile,"Original Loop RefGroups\n",
		w->LoopStats.OriginalLocalityMatrix.SingleGroups.Invariant,
		w->LoopStats.OriginalLocalityMatrix.SingleGroups.Spatial,
		w->LoopStats.OriginalLocalityMatrix.SingleGroups.None,
		w->LoopStats.OriginalLocalityMatrix.MultiGroups.Invariant,
		w->LoopStats.OriginalLocalityMatrix.MultiGroups.Spatial,
		w->LoopStats.OriginalLocalityMatrix.MultiGroups.None,
		w->LoopStats.OriginalLocalityMatrix.MultiRefs.Invariant,
		w->LoopStats.OriginalLocalityMatrix.MultiRefs.Spatial,
		w->LoopStats.OriginalLocalityMatrix.MultiRefs.None,
		w->LoopStats.OriginalOtherSpatialGroups);

		print_RefGroups(logfile,"Final Loop RefGroups\n",
		w->LoopStats.FinalLocalityMatrix.SingleGroups.Invariant,
		w->LoopStats.FinalLocalityMatrix.SingleGroups.Spatial,
		w->LoopStats.FinalLocalityMatrix.SingleGroups.None,
		w->LoopStats.FinalLocalityMatrix.MultiGroups.Invariant,
		w->LoopStats.FinalLocalityMatrix.MultiGroups.Spatial,
		w->LoopStats.FinalLocalityMatrix.MultiGroups.None,
		w->LoopStats.FinalLocalityMatrix.MultiRefs.Invariant,
		w->LoopStats.FinalLocalityMatrix.MultiRefs.Spatial,
		w->LoopStats.FinalLocalityMatrix.MultiRefs.None,
		w->LoopStats.FinalOtherSpatialGroups);

		print_RefGroups(logfile,"Memory Order RefGroups\n",
		w->LoopStats.MemoryLocalityMatrix.SingleGroups.Invariant,
		w->LoopStats.MemoryLocalityMatrix.SingleGroups.Spatial,
		w->LoopStats.MemoryLocalityMatrix.SingleGroups.None,
		w->LoopStats.MemoryLocalityMatrix.MultiGroups.Invariant,
		w->LoopStats.MemoryLocalityMatrix.MultiGroups.Spatial,
		w->LoopStats.MemoryLocalityMatrix.MultiGroups.None,
		w->LoopStats.MemoryLocalityMatrix.MultiRefs.Invariant,
		w->LoopStats.MemoryLocalityMatrix.MultiRefs.Spatial,
		w->LoopStats.MemoryLocalityMatrix.MultiRefs.None,
		w->LoopStats.MemoryOtherSpatialGroups);

	fprintf(logfile,"Depth     1      2      3      4      5 ");
	fprintf(logfile,"     6      7      8      9      10\n");
	fprintf(logfile,"         ===    ===    ===    ===    ===");
	fprintf(logfile,"    ===    ===    ===    ===    ===\n");
        fprintf(logfile,"Loops ");
	for (i = 0; i < NESTING_DEPTH; i++)
	  {
	   fprintf(logfile,"%6d ",w->LoopStats.NestingDepth[i]);
	   NumberLoops += w->LoopStats.NestingDepth[i];
	   WeightedNumberLoops += w->LoopStats.NestingDepth[i] * (i+1);
	  }
        fprintf(logfile,"\nFinal ");
	for (i = 0; i < NESTING_DEPTH; i++)
	  if (w->LoopStats.NestingDepth[i] > 0)
	    if (w->LoopStats.FinalRatio[i] > 0.0)
	      {
	       fprintf(logfile,"%6.2f ",w->LoopStats.FinalRatio[i] /
		       ((float) w->LoopStats.NestingDepth[i]));
	       TotalFinalRatio += w->LoopStats.FinalRatio[i];
	       WeightedTotalFinalRatio += 
			(i+1) * w->LoopStats.FinalRatio[i];
	      }
	    else
	      {
	       fprintf(logfile,"%6.2f ",1.00);
	       TotalFinalRatio += ((float) w->LoopStats.NestingDepth[i]);
	       WeightedTotalFinalRatio += 
			(i+1) * ((float) w->LoopStats.NestingDepth[i]);
	      }
	  else
	    fprintf(logfile,"%6.2f ",1.00);
        fprintf(logfile,"\nMem   ");
	for (i = 0; i < NESTING_DEPTH; i++)
	  if (w->LoopStats.NestingDepth[i] > 0)
	    if (w->LoopStats.MemoryRatio[i] > 0.0)
	      {
	       fprintf(logfile,"%6.2f ",w->LoopStats.MemoryRatio[i] /
		       ((float) w->LoopStats.NestingDepth[i]));
	       TotalMemoryRatio += w->LoopStats.MemoryRatio[i];
	       WeightedTotalMemoryRatio += 
			(i+1) * w->LoopStats.MemoryRatio[i];
	      }
	    else
	      {
	       fprintf(logfile,"%6.2f ",1.00);
	       TotalMemoryRatio += ((float) w->LoopStats.NestingDepth[i]);
	       WeightedTotalMemoryRatio += 
			(i+1) * ((float) w->LoopStats.NestingDepth[i]);
	      }
	  else
	    fprintf(logfile,"%6.2f ",1.00);
         if (NumberLoops > 0)
	   {
	    fprintf(logfile,"\n\nAverage Final Improvement = %.2f\n",
		    TotalFinalRatio/((float) NumberLoops));
	    fprintf(logfile,"Average Memory Improvement = %.2f\n",
		    TotalMemoryRatio/((float) NumberLoops));
	    fprintf(logfile,"\n\nAverage Weighted Final Improvement = %.2f\n",
		    WeightedTotalFinalRatio/((float) WeightedNumberLoops));
	    fprintf(logfile,"Average Weighted Memory Improvement = %.2f\n",
		    WeightedTotalMemoryRatio/((float) WeightedNumberLoops));
	   }
	 else
	   {
	    fprintf(logfile,"\n\nAverage Final Improvement = %.2f\n",1.00);
	    fprintf(logfile,"Average Memory Improvement = %.2f\n",1.00);
	   }
         fprintf(logfile,"\n\n\n");
	
}


void mh_walk_ast(int          selection,
                 PedInfo      ped,
                 AST_INDEX    root,
		 FortTree     ft,
		 Context      mod_context,
		 arena_type   *ar)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


  {
   walk_info_type walk_info;
   static int file = 0,i;
   char fn[DB_PATH_LENGTH];
   
     show_message2("The Memory Optimizer is in progress");

     if (((config_type *)PED_MH_CONFIG(ped))->logging > 0 ||
	 selection == INTERSTATS)
       {
	/* what should memory_folder be? */
	sprintf(fn,"%s.STATSLOG", ctxLocation(mod_context));
	((config_type *)PED_MH_CONFIG(ped))->logfile = fopen(fn,"w");
       }

     walk_info.selection = selection;
     walk_info.ped = ped;
     walk_info.ft = ft;
     walk_info.ar = ar;
     walk_info.LoopStats.TotalLoops = 0;
     walk_info.LoopStats.NotInMemoryOrder = 0;
     walk_info.LoopStats.InMemoryOrder = 0;
     walk_info.LoopStats.NearbyPermutationAttained = 0;
     walk_info.LoopStats.InterchangedIntoMemoryOrder = 0;
     walk_info.LoopStats.TimeStepPreventedMemoryOrder = 0;
     walk_info.LoopStats.ObtainedInnerLoop = 0;
     walk_info.LoopStats.InnerLoopAlreadyCorrect = 0;
     walk_info.LoopStats.WrongInnerLoop = 0;
     walk_info.LoopStats.DesiredInnerTimeStep = 0;
     walk_info.LoopStats.NextInnerLoop = 0;
     walk_info.LoopStats.UnsafeInterchange = 0;
     walk_info.LoopStats.DistributionUnsafe = 0;
     walk_info.LoopStats.NeedsScalarExpansion = 0;
     walk_info.LoopStats.TimeStepPreventedMemoryOrder = 0;
     walk_info.LoopStats.TooComplex = 0;
     walk_info.LoopStats.Nests = 0;
     walk_info.LoopStats.Imperfect = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.Invariant = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.Spatial = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.None = 0;
     walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.Invariant = 0;
     walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.Spatial = 0;
     walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.None = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.Invariant = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.Spatial = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.None = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.Invariant = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.Spatial = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.None = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.Invariant = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.Spatial = 0;
     walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.None = 0;
     walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.Invariant = 0;
     walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.Spatial = 0;
     walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.None = 0;
     walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.Invariant = 0;
     walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.Spatial = 0;
     walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.None = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.Invariant = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.Spatial = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.None = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.Invariant = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.Spatial = 0;
     walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.None = 0;
     walk_info.LoopStats.OriginalOtherSpatialGroups = 0;
     walk_info.LoopStats.FinalOtherSpatialGroups = 0;
     walk_info.LoopStats.MemoryOtherSpatialGroups = 0;
     for (i = 0; i < NESTING_DEPTH; i++)
       {
	walk_info.LoopStats.FinalRatio[i] = 0.0;
	walk_info.LoopStats.MemoryRatio[i] = 0.0;
        walk_info.LoopStats.NestingDepth[i] = 0;
       }

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

       memory_walk_info.LoopStats.TotalLoops 
           += walk_info.LoopStats.TotalLoops;
       memory_walk_info.LoopStats.NotInMemoryOrder 
           += walk_info.LoopStats.NotInMemoryOrder;
       memory_walk_info.LoopStats.InMemoryOrder 
           += walk_info.LoopStats.InMemoryOrder;
       memory_walk_info.LoopStats.NearbyPermutationAttained
           += walk_info.LoopStats.NearbyPermutationAttained;
       memory_walk_info.LoopStats.NearbyPermutationAttained 
           += walk_info.LoopStats.NearbyPermutationAttained;
       memory_walk_info.LoopStats.InterchangedIntoMemoryOrder 
           += walk_info.LoopStats.InterchangedIntoMemoryOrder;
       memory_walk_info.LoopStats.TimeStepPreventedMemoryOrder 
           += walk_info.LoopStats.TimeStepPreventedMemoryOrder;
       memory_walk_info.LoopStats.ObtainedInnerLoop 
           += walk_info.LoopStats.ObtainedInnerLoop;
       memory_walk_info.LoopStats.InnerLoopAlreadyCorrect 
           += walk_info.LoopStats.InnerLoopAlreadyCorrect;
       memory_walk_info.LoopStats.WrongInnerLoop 
           += walk_info.LoopStats.WrongInnerLoop;
       memory_walk_info.LoopStats.DesiredInnerTimeStep 
           += walk_info.LoopStats.DesiredInnerTimeStep;
       memory_walk_info.LoopStats.NextInnerLoop 
           += walk_info.LoopStats.NextInnerLoop;
       memory_walk_info.LoopStats.UnsafeInterchange 
           += walk_info.LoopStats.UnsafeInterchange;
       memory_walk_info.LoopStats.DistributionUnsafe 
           += walk_info.LoopStats.DistributionUnsafe;
       memory_walk_info.LoopStats.NeedsScalarExpansion 
           += walk_info.LoopStats.NeedsScalarExpansion;
       memory_walk_info.LoopStats.TimeStepPreventedMemoryOrder
           += walk_info.LoopStats.TimeStepPreventedMemoryOrder;
       memory_walk_info.LoopStats.TooComplex 
           += walk_info.LoopStats.TooComplex;
       memory_walk_info.LoopStats.Nests 
           += walk_info.LoopStats.Nests;
       memory_walk_info.LoopStats.Imperfect 
           += walk_info.LoopStats.Imperfect;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.Invariant
          += walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.Invariant;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.Spatial
           += walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.Spatial;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.None
           += walk_info.LoopStats.OriginalLocalityMatrix.SingleGroups.None;
       memory_walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.Invariant
          += walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.Invariant;
       memory_walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.Spatial
           += walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.Spatial;
       memory_walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.None
           += walk_info.LoopStats.FinalLocalityMatrix.SingleGroups.None;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.Invariant
          += walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.Invariant;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.Spatial
           += walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.Spatial;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.None
           += walk_info.LoopStats.MemoryLocalityMatrix.SingleGroups.None;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.Invariant
          += walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.Invariant;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.Spatial
           += walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.Spatial;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.None
           += walk_info.LoopStats.OriginalLocalityMatrix.MultiGroups.None;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.Invariant 
           += walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.Invariant;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.Spatial
           += walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.Spatial;
       memory_walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.None 
           += walk_info.LoopStats.OriginalLocalityMatrix.MultiRefs.None;
       memory_walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.Invariant
          += walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.Invariant;
       memory_walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.Spatial
           += walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.Spatial;
       memory_walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.None
           += walk_info.LoopStats.FinalLocalityMatrix.MultiGroups.None;
       memory_walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.Invariant 
           += walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.Invariant;
       memory_walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.Spatial
           += walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.Spatial;
       memory_walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.None 
           += walk_info.LoopStats.FinalLocalityMatrix.MultiRefs.None;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.Invariant
          += walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.Invariant;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.Spatial
           += walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.Spatial;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.None
           += walk_info.LoopStats.MemoryLocalityMatrix.MultiGroups.None;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.Invariant 
           += walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.Invariant;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.Spatial
           += walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.Spatial;
       memory_walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.None 
           += walk_info.LoopStats.MemoryLocalityMatrix.MultiRefs.None;
       memory_walk_info.LoopStats.OriginalOtherSpatialGroups 
           += walk_info.LoopStats.OriginalOtherSpatialGroups;
       memory_walk_info.LoopStats.FinalOtherSpatialGroups 
           += walk_info.LoopStats.FinalOtherSpatialGroups;
       memory_walk_info.LoopStats.MemoryOtherSpatialGroups 
           += walk_info.LoopStats.MemoryOtherSpatialGroups;
     for (i = 0; i < NESTING_DEPTH; i++)
       {
	memory_walk_info.LoopStats.FinalRatio[i] 
           += walk_info.LoopStats.FinalRatio[i];
	memory_walk_info.LoopStats.MemoryRatio[i] 
           += walk_info.LoopStats.MemoryRatio[i];
        memory_walk_info.LoopStats.NestingDepth[i]
            += walk_info.LoopStats.NestingDepth[i];
       }

	memory_stats_dump(((config_type *)PED_MH_CONFIG(ped))->logfile, &walk_info);
       }
     if (((config_type *)PED_MH_CONFIG(ped))->logging > 0 ||
	 selection == INTERSTATS)
       fclose(((config_type *)PED_MH_CONFIG(ped))->logfile);
     hide_message2();
  }


/****************************************************************************/

void ApplyMemoryCompiler(int         selection,
			 PedInfo     ped, 
			 AST_INDEX   root, 
			 FortTree    ft, 
			 Context     mod_context)
  {
   arena_type   ar(1);
   
   if (!PED_MH_CONFIG(ped))
     {
      PED_MH_CONFIG(ped) = (int) get_mem(sizeof(config_type), "memory_stats");
      mh_get_config(PED_MH_CONFIG(ped));
     }
   
   printf("Analyzing %s...\n", ctxLocation(mod_context));
   
   mh_walk_ast(selection,ped,root,ft, mod_context, &ar);
  }

/****************************************************************************/

void memory_stats_total(char *program)

  {
   char fn[DB_PATH_LENGTH];
   FILE *logfile;

     sprintf(fn,"%s.STATSLOG",program);
     logfile = fopen(fn,"w");
     fprintf(logfile,"\n\nTotal Statistics for Program %s\n",program);
     memory_stats_dump(logfile,&memory_walk_info);
     fclose(logfile);
  }


