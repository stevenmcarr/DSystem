#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/annotate/AddressEquivalenceClassSet.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>

static int CheckNodes(AST_INDEX node,
		      StmtOrderInfoType *StmtOrderInfo)

{
  if (node == StmtOrderInfo->New)
    {
      StmtOrderInfo->Found = 2;
      return (WALK_ABORT);
    }
  else if (node == StmtOrderInfo->Old)
    {
      StmtOrderInfo->Found = 1;
      return (WALK_ABORT);
    }
  return(WALK_CONTINUE);
}

      

static Boolean NewReferenceIsEarlier(AST_INDEX Old,
				     AST_INDEX New)

{
  AST_INDEX OldStmt = ut_get_stmt(Old);
  AST_INDEX NewStmt = ut_get_stmt(New);
  stmt_info_type *OldSptr = get_stmt_info_ptr(OldStmt);
  stmt_info_type *NewSptr = get_stmt_info_ptr(NewStmt);

  if (NewSptr->stmt_num < OldSptr->stmt_num)
    return true;

  if (NewSptr->stmt_num == OldSptr->stmt_num)
    {
      StmtOrderInfoType StmtOrderInfo;

      StmtOrderInfo.Old = Old;
      StmtOrderInfo.New = New;
      StmtOrderInfo.Found = 0;

      if (is_assignment(NewStmt))
	{
	  walk_expression(gen_ASSIGNMENT_get_rvalue(NewStmt),
			  (WK_EXPR_CLBACK)CheckNodes,
			  (WK_EXPR_CLBACK)NOFUNC,
			  (Generic)&StmtOrderInfo);
	  if (StmtOrderInfo.Found == 0)
	    walk_expression(gen_ASSIGNMENT_get_lvalue(NewStmt),
			    (WK_EXPR_CLBACK)CheckNodes,
			    (WK_EXPR_CLBACK)NOFUNC,
			    (Generic)&StmtOrderInfo);
	}
      else
	walk_expression(NewStmt,(WK_EXPR_CLBACK)CheckNodes,(WK_EXPR_CLBACK)NOFUNC,
			(Generic)&StmtOrderInfo);
      if (StmtOrderInfo.Found == 2)
	return true;
    }

  return false;
}

static int BuildEquivalenceClasses(AST_INDEX node,
				   AddressEquivalenceClassSet *AECS)

{
  if (is_subscript(node))
    AECS->AddNode(node);
  return(WALK_CONTINUE);
      
}

AddressEquivalenceClassSet::AddressEquivalenceClassSet(AST_INDEX loop,
						       int NL,
						       char **IV) : GenericList()

  { 
   int i;
   AST_INDEX node;
   
     NestingLevel = NL;
     Size = 0;
     IndexVars = new char*[NestingLevel];
     for (i = 0; i < NestingLevel; i++)
       IndexVars[i] = new char[80];
     for (i = 0; i < NestingLevel; i++)
       (void)strcpy(IndexVars[i],IV[i]);
       Offsets = new ASTToIntMap;
     walk_expression(gen_DO_get_stmt_LIST(loop),
		     (WK_EXPR_CLBACK)BuildEquivalenceClasses,
		     (WK_EXPR_CLBACK)NOFUNC,(Generic)this);
     ComputeAddressOffsets();
  }

void AddressEquivalenceClassSet::ComputeAddressOffsets()

{
  AddressEquivalenceClass *Class;
  AST_INDEX node;
  
  Offsets->MapCreate(Size);

  
  for (AddressEquivSetIterator AECSIter(*this);
       Class = AECSIter();)
    {
      la_vect C_L = la_vecNew(Class->GetSubscripts());
      Class->GetConstants(Class->GetLeader(),C_L);
      for (AddressClassIterator ACIter(*Class);
	   node = ACIter();)
	{
	  la_vect C_n = la_vecNew(Class->GetSubscripts());
	  Class->GetConstants(node,C_n);
	  Offsets->MapAddEntry(node,C_n[0] - C_L[0]);
	  la_vecFree(C_n);
	}
      la_vecFree(C_L);
    }
}
  
int AddressEquivalenceClassSet::GetOffset(AST_INDEX node)

{
  return (Offsets->MapToValue(node));
}

void AddressEquivalenceClassSet::AddNode(AST_INDEX node)

  {
   AddressEquivalenceClass *EquivalenceClass;
   Boolean uniform = true;
   int **nodeH,i,j,Subscripts;

     Size++;
     Subscripts = list_length(gen_SUBSCRIPT_get_rvalue_LIST(node));
     nodeH = la_matNew(Subscripts,NestingLevel);
     GetH(node,nodeH,&uniform);
     if (uniform && (EquivalenceClass = GetAddressEquivalenceClass(node,nodeH)) != NULL)
       EquivalenceClass->AddEntry(node);
     else
       EquivalenceClass = Append(nodeH,node,Subscripts,uniform);
     EquivalenceClass->CheckLeader(node);
  }

void AddressEquivalenceClassSet::GetH(AST_INDEX node,
				      la_matrix nodeH,
				      Boolean *uniform)

  {
   AST_INDEX sublist,sub,Inode;
   int SubPos = 0;

     sublist = gen_SUBSCRIPT_get_rvalue_LIST(node);
     for (sub = list_first(sublist);
	  sub != AST_NIL;
	  sub = list_next(sub))
       {
	for (AstIter AIter(sub,false); (Inode = AIter()) != AST_NIL;)
	  ComputeH(Inode,nodeH,uniform,sub,SubPos);
	SubPos++;
       }
  }


void AddressEquivalenceClassSet::ComputeH(AST_INDEX node,la_matrix nodeH,
					  Boolean *uniform, AST_INDEX expr,
					  int SubPos)
  {
   Boolean linear;
   int coeff,index;

     if (is_identifier(node))
       {
	pt_get_coeff(expr,gen_get_text(node),&linear,&coeff);
	*(uniform) = BOOL(*(uniform) && linear);
	index = GetIndex(gen_get_text(node));
	if (*uniform)
	  nodeH[SubPos][index] = coeff;
	else
	  nodeH[SubPos][index] = MININT;
       }
  }

int AddressEquivalenceClassSet::GetIndex(char *ivar)

  {
   int i;

     for (i = 0; i < NestingLevel; i++)
       if (strcmp(ivar,IndexVars[i]) == 0)
         return(i);
     return(-1);
  }
   
AddressEquivalenceClass* AddressEquivalenceClassSet::Append(la_matrix nodeH,
							    AST_INDEX node,
							    int NumSubs, Boolean uniform)

  {
   AddressEquivalenceClass *Class;
   
     Class = new AddressEquivalenceClass(gen_get_text(gen_SUBSCRIPT_get_name(node)),
					 nodeH,NestingLevel,NumSubs,uniform);
     Class->AddEntry(node);
     (*this) += (Generic)Class;
     return Class;
  }

AddressEquivalenceClass *
AddressEquivalenceClassSet::GetAddressEquivalenceClass(AST_INDEX node,
						      la_matrix nodeH)

  {
   AddressEquivalenceClass *Class;

   for (AddressEquivSetIterator AECSIter(*this);
	Class = AECSIter();)
     if (Class->SameEquivalenceClass(node,nodeH))
       return(Class);
   return(NULL);
  }


AddressEquivalenceClass* AddressEquivSetIterator::operator() ()
{
     GenericListEntry *e;

     e = GenericListIter::operator()();
     if ( e != NULL )
       return (AddressEquivalenceClass*)(e->GetValue());
     else return NULL;
}

AST_INDEX AddressEquivalenceClassSet::GetLeader(AST_INDEX node)

{
  Boolean uniform;

  int Subscripts = list_length(gen_SUBSCRIPT_get_rvalue_LIST(node));
  la_matrix nodeH = la_matNew(Subscripts,NestingLevel);
  GetH(node,nodeH,&uniform);
  return (GetAddressEquivalenceClass(node,nodeH)->GetLeader());
  la_matFree(nodeH,Subscripts,NestingLevel);
}


AddressEquivalenceClass::AddressEquivalenceClass(char *EntryName,
						 la_matrix nodeH, 
						 int level,
						 int subs,
						 Boolean uniform) : GenericList()
{
  int i,j;
  
  (void)strcpy(name,EntryName);
  NestingLevel = level;
  Subscripts = subs;
  Uniform = uniform;
  H = la_matNew(Subscripts,NestingLevel);
  la_matCopy(nodeH,H,Subscripts,NestingLevel);
  C_L = la_vecNew(Subscripts);
}

Boolean AddressEquivalenceClass::SameEquivalenceClass(AST_INDEX node,
						      la_matrix nodeH)

  {
   int i,j;
   la_vect C,C_L;

     if (NOT(Uniform) || strcmp(name,gen_get_text(gen_SUBSCRIPT_get_name(node)))) 
       return (false);

     for (i = 0; i < Subscripts; i++)
       for (j = 0; j < NestingLevel; j++)
         if (H[i][j] != nodeH[i][j])
	   return(false);

     C = la_vecNew(Subscripts);
     GetConstants(node,C);

     C_L = la_vecNew(Subscripts);
     GetConstants(Leader,C_L);
     for (i = 1; i < Subscripts; i++) // ignore the first subscript position
         if (C[i] != C_L[i])
	   return(false);
     
     return(true);
  }

void AddressEquivalenceClass::GetConstants(AST_INDEX node1,
					   la_vect C)

  {
   AST_INDEX sublist,sub;
   int i;

     sublist = gen_SUBSCRIPT_get_rvalue_LIST(node1);
     for (sub = list_first(sublist), i = 0;
	  sub != AST_NIL;
	  sub = list_next(sub), i++)
       pt_get_constant(sub,&C[i]);
  }

void AddressEquivalenceClass::CheckLeader(AST_INDEX node)

{
  Boolean Change = false;
  Boolean Done = false;
  int i = Subscripts - 1 ;
  Boolean AllEqual = true;
  la_vect C_n;

  if (Leader != NULL)
    if (NewReferenceIsEarlier(Leader,node))
      Leader = node;
    else;
  else
    Leader = node;
}

AST_INDEX AddressClassIterator::operator() ()
{
GenericListEntry *e;

     e = GenericListIter::operator()();
     if( e != NULL )
         return (AST_INDEX)( e->GetValue());
     else return NULL;
}
