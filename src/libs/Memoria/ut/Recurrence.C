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
  walk_expression(gen_DO_get_stmt_LIST(loop),(WK_EXPR_CLBACK)CountNodes,
		  NOFUNC,(Generic)&size);
  Nodes = new AST_INDEX[size];
  ASTMap = new ASTToIntMap;
  ASTMap->MapCreate(size);
  T = new int[size*size*size];
  NumberOfNodes = 0; // This has to be recounted for indexing purposes when
                     // initializing the Nodes array
  walk_expression(gen_DO_get_stmt_LIST(loop),(WK_EXPR_CLBACK)InitializeNodeMapping,
		  NOFUNC,(Generic)this);
  ComputeT();
}

static int CountNodes(AST_INDEX node,
		      int *size)
  
{
  (*size)++;
  return (WALK_CONTINUE);
}

static int InitializeNodeMapping(AST_INDEX node,
				 Recurrence *R)
{
  R->PutNode(node);
  R->GetASTMap()->MapAddEntry(node,R->GetNumberOfNodes());
  R->IncrementNumberOfNodes();

  return (WALK_CONTINUE);
}

void Recurrence::ComputeT()

{
  int i,j,k,l;
  AST_INDEX name;
  DG_Edge   *dg;
  int       vector;
  EDGE_INDEX edge;

  for (k = 0; k < NumberOfNodes; k++)
    for (i = 0; i < NumberOfNodes; i++)
      for (j = 0; j < NumberOfNodes; j++)
	T[ONED(i,j,k,NumberOfNodes)] = INFINITY;
  dg = dg_get_edge_structure( PED_DG(ped));

  // First we record that a node is connected to its parent in the AST
  // with a 0 distance edge. Then, we record all of the innermost loop
  // carried and loop independent edges in the recurrence matrix

  for (i = 0; i < NumberOfNodes; i++)
    if (ut_get_stmt(Nodes[i]) != Nodes[i])
      {
	if (NOT(is_f77_statement(Nodes[i])))
	  {
	    j = ASTMap->MapToValue(tree_out(Nodes[i]));
	    T[ONED(i,j,0,NumberOfNodes)] = 0; // edge has distance zero in AST
	  }
	else if (is_subscript(Nodes[i]))
	  {
	    AST_INDEX name = gen_SUBSCRIPT_get_name(Nodes[i]);
	    vector = get_info(ped,name,type_levelv);
	    for (edge = dg_first_src_ref(PED_DG(ped),vector);
		 edge != END_OF_LIST;
		 edge = dg_next_src_ref( PED_DG(ped),edge))
	      if ((dg[edge].type == dg_true || dg[edge].type == dg_anti ||
		   dg[edge].type == dg_output) &&
		  (dg[edge].level = level || dg[edge].level == LOOP_INDEPENDENT))
		{
		  j = ASTMap->MapToValue(dg[edge].sink);
		  T[ONED(i,j,0,NumberOfNodes)] = 
		    gen_get_dt_DIS(&dg[edge],dg[edge].level);
		}
	  }
      }
	  
 
  for (k = 1; k < NumberOfNodes; k++)
    for (i = 0; i < NumberOfNodes; i++)
      for (j = 0; j < NumberOfNodes; j++)
	for (l = 0; l < NumberOfNodes; l++)
	  {
	    int Distance = T[ONED(i,l,0,NumberOfNodes)] + T[ONED(i,j,k-1,NumberOfNodes)];

	    T[ONED(i,j,k,NumberOfNodes)] = (T[ONED(i,j,k,NumberOfNodes)] > Distance) ? 
	                                     Distance : 
	                                     T[ONED(i,j,k,NumberOfNodes)];
	  }
}

Boolean Recurrence::IsReferenceOnRecurrence(AST_INDEX node)

{
  int i = ASTMap->MapToValue(node);
  int k;

  for (k = 0; k < NumberOfNodes; k++)
    if (T[ONED(i,i,k,NumberOfNodes)] > 0 && 
	T[ONED(i,i,k,NumberOfNodes)] < INFINITY)
      return true;
  return false;
}
    
