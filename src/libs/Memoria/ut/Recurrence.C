#include <iostream.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/ut/Recurrence.h>

// access a one-d array with three subscripts. I don't want to do this but the 
// Alpha seems to screw up 3-d arrays

#define ONED(i,j,k,n) (((k * n) + j)*n + i)

Recurrence::Recurrence(AST_INDEX l,
			 PedInfo p,
			 int lev)

{
  int i,j,size=0;

  loop = l;
  ped = p;
  level = lev;
  walk_statements(gen_DO_get_stmt_LIST(loop),lev,(WK_STMT_CLBACK)CountStatements,
		 NOFUNC,(Generic)&size);
  Stmts = new AST_INDEX[size];
  T = new int[size*size*size];
  NumberOfStmts = 0; // This has to be recounted for indexing purposes when
                     // initialized the Stmts array
  walk_statements(gen_DO_get_stmt_LIST(loop),lev,(WK_STMT_CLBACK)InitializeStmtMapping,
		  NOFUNC,(Generic)this);
  ComputeT();
}

static int CountStatements(AST_INDEX stmt,
			   int level,
			   int *size)
  
{
  (*size)++;
  return (WALK_CONTINUE);
}

static int InitializeStmtMapping(AST_INDEX stmt,
				 int level,
				 Recurrence *R)
{
  R->PutStmt(stmt);
  get_stmt_info_ptr(stmt)->stmt_num = R->GetNumberOfStmts();
  R->IncrementNumberOfStmts();

  return (WALK_CONTINUE);
}

void Recurrence::ComputeT()

{
  int i,j,k,l;
  AST_INDEX name;
  DG_Edge   *dg;
  int       vector;
  EDGE_INDEX edge;

  for (k = 0; k < NumberOfStmts; k++)
    for (i = 0; i < NumberOfStmts; i++)
      for (j = 0; j < NumberOfStmts; j++)
	T[ONED(i,j,k,NumberOfStmts)] = INFINITY;
  dg = dg_get_edge_structure( PED_DG(ped));
  for (i = 0; i < NumberOfStmts; i++)
     {
       vector = get_info(ped,Stmts[i],type_levelv);
       for (edge = dg_first_src_stmt( PED_DG(ped),vector,level);
	    edge != END_OF_LIST;
	    edge = dg_next_src_stmt( PED_DG(ped),edge))
	 if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	     dg[edge].type == dg_output) 
	   {
	     j = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))->stmt_num;
	     T[ONED(i,j,0,NumberOfStmts)] = gen_get_dt_DIS(&dg[edge],dg[edge].level);
	   }
       for (edge = dg_first_src_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	    edge != END_OF_LIST;
	    edge = dg_next_src_stmt( PED_DG(ped),edge))
	 if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	     dg[edge].type == dg_output) 
	   {
	     j = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))->stmt_num;
	     T[ONED(i,j,0,NumberOfStmts)] = 0;
	   }
      }
	  
 
  for (k = 1; k < NumberOfStmts; k++)
    for (i = 0; i < NumberOfStmts; i++)
      for (j = 0; j < NumberOfStmts; j++)
	for (l = 0; l < NumberOfStmts; l++)
	  {
	    int Distance = T[ONED(i,l,0,NumberOfStmts)] + T[ONED(i,j,k-1,NumberOfStmts)];

	    T[ONED(i,j,k,NumberOfStmts)] = (T[ONED(i,j,k,NumberOfStmts)] > Distance) ? 
	                                     Distance : 
	                                     T[ONED(i,j,k,NumberOfStmts)];
	  }
}

Boolean Recurrence::IsReferenceOnRecurrence(AST_INDEX stmt)

{
  int i = get_stmt_info_ptr(stmt)->stmt_num;
  int k;

  for (k = 0; k < NumberOfStmts; k++)
    if (T[ONED(i,i,k,NumberOfStmts)] > 0 && 
	T[ONED(i,i,k,NumberOfStmts)] < INFINITY)
      return true;
  return false;
}
    
