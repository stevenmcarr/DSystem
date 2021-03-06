/* $Id: log.C,v 1.14 1997/03/27 20:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/uj/log.h>
#include <libs/Memoria/include/header.h>
#include <libs/Memoria/uj/do_unroll.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/Memoria/include/label.h>
#include <libs/Memoria/include/LoopStats.h>

static int LoopNumber = 1;

extern Boolean mc_extended_cache;

static void print_predicted_info(model_loop *loop_data,
				 int        loop,
				 FILE       *logfile,
				 PedInfo    ped,
				 int        UnrollCount,
				 int        *UnrolledLoops,
				 int        *unroll_vector,
				 LoopStatsType *LoopStats,
				 char       **IVar)

  {
   StatsInfoType Stats;
   float rhoL_lp;

     fprintf(logfile,"Predicted Unroll-and-Jam Statistics for Perfect Nest %d\n",LoopNumber);
     fprintf(logfile,"========================================================\n\n");
     fprintf(logfile,"Nesting Depth        = %d\n",loop_data[loop].level);
     Stats.ped = ped;
     Stats.flops = 0.0;
     Stats.mops = 0.0;
     Stats.level = loop_data[loop].level;
     Stats.UseCache = false;
     Stats.loop_data = loop_data;
     Stats.loop = loop;
     if (mc_extended_cache)
       Stats.UGS = new UniformlyGeneratedSets(loop_data[loop].node,
					      loop_data[loop].level,
					      IVar);
     else
       Stats.UGS = NULL;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),(WK_EXPR_CLBACK)
		     ut_ComputeBalance,(WK_EXPR_CLBACK)NOFUNC,(Generic)&Stats);
     delete Stats.UGS;
     rhoL_lp = loop_data[loop].rho * ((config_type *)PED_MH_CONFIG(ped))->pipe_length;
     if (Stats.flops < rhoL_lp)
       {
	fprintf(logfile,"Interlock Factor     = %.4f\n",rhoL_lp - Stats.flops);
	LoopStats->InitialInterlock += rhoL_lp - Stats.flops;
	fprintf(logfile,"Balance with Interlock = %.4f\n",Stats.mops/rhoL_lp);
	LoopStats->InitialBalanceWithInterlock += Stats.mops/rhoL_lp;
	if (loop_data[loop].InterlockCausedUnroll)
	  {
	   fprintf(logfile,"Interlock Caused Unrolling\n");
	   LoopStats->InterlockCausedUnroll++;
	  }
       }
     fprintf(logfile,"Initial Loop Balance = %.4f\n",loop_data[loop].ibalance);
     LoopStats->PredictedInitialBalance += loop_data[loop].ibalance;
     fprintf(logfile,"Final Loop Balance   = %.4f\n",loop_data[loop].fbalance);
     LoopStats->PredictedFinalBalance += loop_data[loop].fbalance;
     fprintf(logfile,"FP Register Pressure = %d\n",
	     loop_data[loop].registers);
     LoopStats->PredictedFPRegisterPressure += loop_data[loop].registers;
     if (UnrollCount > 0)
       {
	fprintf(logfile,"Outer Unroll Level   = %d\n",
		loop_data[UnrolledLoops[0]].level);
	fprintf(logfile,"Outer Unroll Amount  = %d\n",
		unroll_vector[loop_data[UnrolledLoops[0]].level-1]);
	if (UnrollCount > 1)
	  {
	   fprintf(logfile,"Inner Unroll Level   = %d\n",
		   loop_data[UnrolledLoops[1]].level);
	   fprintf(logfile,"Inner Unroll Amount  = %d\n",
		   unroll_vector[loop_data[UnrolledLoops[1]].level-1]);
	  }
       }
     fprintf(logfile,"Initial Prefetches/Iteration = %.4f\n",
	     loop_data[loop].initial_P_L);
     fprintf(logfile,"Initial Cycles/Iteration = %.4f\n",
	     loop_data[loop].initial_L_L);
     fprintf(logfile,"Initial Prefetch Bandwidth Requirement = %.4f\n",
	     loop_data[loop].initial_P_L/loop_data[loop].initial_L_L);
     fprintf(logfile,"Final Prefetches/Iteration = %.4f\n",loop_data[loop].P_L);
     fprintf(logfile,"Final Cycles/Iteration = %.4f\n",loop_data[loop].L_L);
     fprintf(logfile,"Final Prefetch Bandwidth Requirement = %.4f\n",
	     loop_data[loop].P_L/loop_data[loop].L_L);
     LoopStats->PredictedP_L += loop_data[loop].P_L;
     fprintf(logfile,"\n\n\n");
  }


static void print_actual_info(model_loop *loop_data,
			      int        loop,
			      FILE       *logfile,
			      int        UnrollCount,
			      int        *UnrolledLoops,
			      int        *unroll_vector,
			      PedInfo    ped,
			      SymDescriptor symtab,
			      arena_type *ar,
			      LoopStatsType *LoopStats,
			      char       **IVar)

  {
   StatsInfoType Stats;
   copy_info_type copy_info;
   int val1 = 0,val2 = 0;
   float rhoL_lp;

     copy_info.val = unroll_vector[loop_data[UnrolledLoops[0]].level-1];
     copy_info.ar = ar;
     copy_info.symtab = symtab;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     (WK_EXPR_CLBACK)ut_init_copies,(WK_EXPR_CLBACK)
		     NOFUNC,(Generic)&copy_info);
     val1 = unroll_vector[loop_data[UnrolledLoops[0]].level-1];
     mh_replicate_body(gen_DO_get_stmt_LIST(loop_data[loop].node),val1,
		       loop_data[UnrolledLoops[0]].level,
		       gen_get_text(gen_INDUCTIVE_get_name(
		        gen_DO_get_control(loop_data[UnrolledLoops[0]].node))),
		       AST_NIL,ped,symtab,false,
		       get_stmt_info_ptr(loop_data[loop].node)->loop_num,
		       loop_data[loop].node,
		       gen_get_text(gen_INDUCTIVE_get_name(
		           gen_DO_get_control(loop_data[loop].node))),
		       loop_data[loop].level,ar);
     if (UnrollCount > 1)
       {
	copy_info.val = unroll_vector[loop_data[UnrolledLoops[1]].level-1];
	walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
			(WK_EXPR_CLBACK) ut_init_copies,
			(WK_EXPR_CLBACK)NOFUNC,(Generic)&copy_info);
	val2 = unroll_vector[loop_data[UnrolledLoops[1]].level-1];
	mh_replicate_body(gen_DO_get_stmt_LIST(loop_data[loop].node),val2,
			  loop_data[UnrolledLoops[1]].level,
			  gen_get_text(gen_INDUCTIVE_get_name(
		        gen_DO_get_control(loop_data[UnrolledLoops[1]].node))),
			  AST_NIL,ped,symtab,false,
			  get_stmt_info_ptr(loop_data[loop].node)->loop_num,
			  loop_data[loop].node,
			  gen_get_text(gen_INDUCTIVE_get_name(
		              gen_DO_get_control(loop_data[loop].node))),
			  loop_data[loop].level,ar);
       }
     Stats.ped = ped;
     Stats.flops = 0.0;
     Stats.mops = 0.0;
     Stats.level = loop_data[loop].level;
     Stats.UseCache = false;
     Stats.loop_data = loop_data;
     Stats.loop = loop;
     if (mc_extended_cache)
       Stats.UGS = new UniformlyGeneratedSets(loop_data[loop].node,
					      loop_data[loop].level,
					      IVar);
     else
       Stats.UGS = NULL;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),(WK_EXPR_CLBACK)
		     ut_ComputeBalance,(WK_EXPR_CLBACK)NOFUNC,(Generic)&Stats);
     delete Stats.UGS;
     fprintf(logfile,"Actual Unroll-and-Jam Statistics for Perfect Nest %d\n",
	     LoopNumber++);
     fprintf(logfile,"========================================================\n\n");
     rhoL_lp =  loop_data[loop].rho * ((config_type *)PED_MH_CONFIG(ped))->pipe_length;
     if (Stats.flops < rhoL_lp)
       {
	fprintf(logfile,"Final Interlock   = %.4f\n",rhoL_lp - Stats.flops);
	LoopStats->FinalInterlock += rhoL_lp - Stats.flops;
	fprintf(logfile,"Balance with Interlock = %.4f\n",Stats.mops/rhoL_lp);
	LoopStats->FinalBalanceWithInterlock += Stats.mops/rhoL_lp;
       }
     loop_data[loop].fbalance = Stats.mops/Stats.flops;
     fprintf(logfile,"Final Loop Balance  = %.4f\n",loop_data[loop].fbalance);
     LoopStats->ActualFinalBalance += loop_data[loop].fbalance;
     memory_scalar_replacement(ped,loop_data[loop].node,loop_data[loop].level,
			       symtab,ar,LoopStats);
     fprintf(logfile,"\n\n\n");
  }

static void CheckForTriangular(model_loop *loop_data,
			       int        loop,
			       LoopStatsType *LoopStats,
			       FILE       *logfile)

  {
   if (loop_data[loop].tri_loop != -1 ||
       loop_data[loop].trap_loop != -1)
     {
      LoopStats->Normalized++;
      fprintf(logfile,"Normalization may have caused a problem\n");
     }
   else
     if (loop_data[loop].parent != -1)
       CheckForTriangular(loop_data,loop_data[loop].parent,LoopStats,logfile);
  }

static void print_NotUnrolledInfo(model_loop *loop_data,
				  int        loop,
				  FILE       *logfile,
				  PedInfo    ped,
				  int        UnrollCount,
				  int        *UnrolledLoops,
				  int        *unroll_vector,
				  LoopStatsType *LoopStats,
				  SymDescriptor symtab,
				  arena_type    *ar,
				  char          **IVar)

  {
   StatsInfoType Stats;
   int Temp;
   float rhoL_lp;

     fprintf(logfile,"Statistics for Not Unrolled Loop %d\n",LoopStats->NotUnrolled);
     fprintf(logfile,"===================================\n\n");
     fprintf(logfile,"Nesting Depth        = %d\n",loop_data[loop].level);
     Stats.ped = ped;
     Stats.flops = 0.0;
     Stats.mops = 0.0;
     Stats.level = loop_data[loop].level;
     Stats.UseCache = false;
     Stats.loop_data = loop_data;
     Stats.loop = loop;
     if (mc_extended_cache)
       Stats.UGS = new UniformlyGeneratedSets(loop_data[loop].node,
					      loop_data[loop].level,
					      IVar);
     else
       Stats.UGS = NULL;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),(WK_EXPR_CLBACK)
		     ut_ComputeBalance,(WK_EXPR_CLBACK)NOFUNC,(Generic)&Stats);
     delete Stats.UGS;
     rhoL_lp =  loop_data[loop].rho * ((config_type *)PED_MH_CONFIG(ped))->pipe_length;
     if (Stats.flops < rhoL_lp)
       {
	fprintf(logfile,"Interlock Factor     = %.4f\n",rhoL_lp - Stats.flops);
	LoopStats->NotUnrolledInterlock += rhoL_lp - Stats.flops;
	fprintf(logfile,"Balance with Interlock = %.4f\n",Stats.mops/rhoL_lp);
	LoopStats->NotUnrolledBalanceWithInterlock += Stats.mops/rhoL_lp;
       }
     if (Stats.flops > 0)
       loop_data[loop].ibalance = Stats.mops/Stats.flops;
     else 
       loop_data[loop].ibalance = 0;
     fprintf(logfile,"Loop Balance   = %.4f\n",loop_data[loop].ibalance);
     LoopStats->NotUnrolledBalance += loop_data[loop].ibalance;
     Temp = LoopStats->ActualFPRegisterPressure;
     LoopStats->ActualFPRegisterPressure = 0;
     memory_scalar_replacement(ped,loop_data[loop].node,loop_data[loop].level,
			       symtab,ar,LoopStats);
     LoopStats->NotUnrolledFPRegisterPressure += LoopStats->ActualFPRegisterPressure;
     LoopStats->ActualFPRegisterPressure = Temp;
     fprintf(logfile,"\nReasons for Failure\n");
     fprintf(logfile,"===================\n");
     if (loop_data[loop].Distribute)
       {
	fprintf(logfile,"\tDistribution\n");
	LoopStats->Distribute++;
       }
     if (loop_data[loop].Interchange)
       {
	fprintf(logfile,"\tInterchange\n");
	LoopStats->Interchange++;
       }
     if (loop_data[loop].NoImprovement)
       {
	fprintf(logfile,"\tNo Improvement Possible\n");
	LoopStats->NoImprovement++;
	CheckForTriangular(loop_data,loop,LoopStats,logfile);
       }
     if (loop_data[loop].ibalance <= ((config_type *)PED_MH_CONFIG(ped))->beta_m)
       {
	fprintf(logfile,"\tAlready Balanced\n");
	LoopStats->AlreadyBalanced++;
       }
     fprintf(logfile,"\n\n\n");
  }


static void print_SingleDepthInfo(model_loop *loop_data,
				  int        loop,
				  FILE       *logfile,
				  PedInfo    ped,
				  int        UnrollCount,
				  int        *UnrolledLoops,
				  int        *unroll_vector,
				  LoopStatsType *LoopStats,
				  SymDescriptor symtab,
				  arena_type    *ar,
				  char       **IVar)
  
  {
   StatsInfoType Stats;
   int Temp;
   float rhoL_lp;

     fprintf(logfile,"Statistics for Single Depth Loop %d\n",LoopStats->SingleDepth);
     fprintf(logfile,"===================================\n\n");
     fprintf(logfile,"Nesting Depth        = %d\n",loop_data[loop].level);
     Stats.ped = ped;
     Stats.flops = 0.0;
     Stats.mops = 0.0;
     Stats.level = loop_data[loop].level;
     Stats.UseCache = false;
     Stats.loop_data = loop_data;
     Stats.loop = loop;
     if (mc_extended_cache)
       Stats.UGS = new UniformlyGeneratedSets(loop_data[loop].node,
					      loop_data[loop].level,
					      IVar);
     else
       Stats.UGS = NULL;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),(WK_EXPR_CLBACK)
		     ut_ComputeBalance,(WK_EXPR_CLBACK)NOFUNC,(Generic)&Stats);
     delete Stats.UGS;
     rhoL_lp = loop_data[loop].rho * ((config_type *)PED_MH_CONFIG(ped))->pipe_length;
     if (Stats.flops < rhoL_lp)
       {
	fprintf(logfile,"Interlock Factor     = %.4f\n",rhoL_lp - Stats.flops);
	LoopStats->SingleDepthInterlock += rhoL_lp - Stats.flops;
	fprintf(logfile,"Balance with Interlock = %.4f\n",Stats.mops/rhoL_lp);
	LoopStats->SingleDepthBalanceWithInterlock += Stats.mops/rhoL_lp;
       }
     if (Stats.flops > 0)
       loop_data[loop].ibalance = Stats.mops/Stats.flops;
     else
       loop_data[loop].ibalance = 0;
     fprintf(logfile,"Loop Balance   = %.4f\n",loop_data[loop].ibalance);
     LoopStats->SingleDepthBalance += loop_data[loop].ibalance;
     Temp = LoopStats->ActualFPRegisterPressure;
     LoopStats->ActualFPRegisterPressure = 0;
     memory_scalar_replacement(ped,loop_data[loop].node,loop_data[loop].level,
			       symtab,ar,LoopStats);
     LoopStats->SingleDepthFPRegisterPressure += LoopStats->ActualFPRegisterPressure;
     LoopStats->ActualFPRegisterPressure = Temp;
     fprintf(logfile,"\n\n\n");
  }


static void walk_loops(model_loop *loop_data,
		       int        loop,
		       int        *unroll_vector,
		       int        *UnrolledLoops,
		       int        UnrollCount,
		       FILE       *logfile,
		       PedInfo    ped,
		       SymDescriptor symtab,
		       arena_type *ar,
		       LoopStatsType *LoopStats,
		       char          **IVar)

  {
   int i;
   
     IVar[loop_data[loop].level-1] = 
       gen_get_text(gen_INDUCTIVE_get_name(
                      gen_DO_get_control(loop_data[loop].node)));
     if (loop_data[loop].inner_loop != -1)
       {
	i = loop_data[loop].inner_loop;
	while(i != -1)
	  {
	   if (unroll_vector[loop_data[loop].level-1] > 0)
	     UnrolledLoops[UnrollCount++] = loop;
	   walk_loops(loop_data,i,unroll_vector,UnrolledLoops,UnrollCount,logfile,ped,
		      symtab,ar,LoopStats,IVar);
	   i = loop_data[i].next_loop;
	   if (i != -1)
	      unroll_vector = loop_data[i].unroll_vector;
	  }
       }
     else
       {
	if (loop_data[loop].level != 1)
	  {
	   if (UnrollCount > 0)
	     {
	      LoopStats->UnrolledLoops++;
	      print_predicted_info(loop_data,loop,logfile,ped,UnrollCount,
				   UnrolledLoops,unroll_vector,LoopStats,IVar);
	      // print_actual_info(loop_data,loop,logfile,UnrollCount,
			// 	UnrolledLoops,unroll_vector,ped,symtab,ar,
				// LoopStats,IVar);
	     }
	   else
	     {
	      LoopStats->NotUnrolled++;
	      print_NotUnrolledInfo(loop_data,loop,logfile,ped,UnrollCount,
				    UnrolledLoops,unroll_vector,LoopStats,symtab,ar,
				    IVar);
	     }
	  }
	else
	  {
	   LoopStats->SingleDepth++;
	   print_SingleDepthInfo(loop_data,loop,logfile,ped,UnrollCount,
				 UnrolledLoops,unroll_vector,LoopStats,symtab,ar,
				 IVar);
	  }
       }
  }
  
void mh_log_data(model_loop *loop_data,
		 FILE       *logfile,
		 PedInfo    ped,
		 SymDescriptor symtab,
		 arena_type *ar,
		 LoopStatsType *LoopStats)

  {
   int UnrollCount = 0;
   int UnrolledLoops[2];
   char *IVar[21];  // Max Fortran loop nesting depth

     walk_loops(loop_data,0,loop_data[0].unroll_vector,UnrolledLoops,
		UnrollCount,logfile,ped,symtab,ar,LoopStats,IVar);
  }        
