/* $Id: la.C,v 1.7 1998/08/05 19:31:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

//	Linear Algebra Data Reuse Model
//		C++ Code File
//     
//	Input:     Uniformly Generated Sets
//	Output:    Data Reuse Model
//		  	Reference are partitioned into groups
//			according to Group Spatial Data Reuse and
//			Group Temporal Data Reuse

#include <iostream.h>
#include <libs/Memoria/include/la.h>
#include <assert.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/UniformlyGeneratedSets.h>
#include <libs/support/Lambda/Lambda.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/include/GenericList.h>

#define sign(x) (((x)>0) ? 1: -1)

int Solve(la_matrix, la_vect, int, int, la_matrix*, int*);
int ChangeLeader(la_vect, la_vect, int,AST_INDEX,AST_INDEX);
int ChangeTrailer(la_vect, la_vect, int, AST_INDEX, AST_INDEX);
int ChangeLoadLeader(la_vect, la_vect, int, AST_INDEX, AST_INDEX);

extern Boolean ReuseModelDebugFlag;

DataReuseModel::DataReuseModel(UniformlyGeneratedSets *UGS) : SinglyLinkedList()
{
 UniformlyGeneratedSetsEntry *UGSEntry;
 UGSIterator UGSIter(*UGS);

 size = 0;
 while(UGSEntry = (UniformlyGeneratedSetsEntry *)UGSIter())
   {
    ++ size;
    // UGSEntry->PrintOut();
    DataReuseModelEntry * e = new DataReuseModelEntry(UGSEntry);
    SinglyLinkedList::Append(e); 
   }
 if (ReuseModelDebugFlag)
   {
     cout << "Total # of UGS is " << size << endl; 
     PrintOut();
   }
}

DataReuseModelEntry* DRIter::operator () ()
{
 DataReuseModelEntry *e = (DataReuseModelEntry *) SinglyLinkedListIterator ::Current();
 ++(*this);
 return e;
}


void DataReuseModel::PrintOut()
{
 DataReuseModelEntry *e;
 int i = 0;

 for ( DRIter driter(this);
        e = driter(); )
 {
  cout << "UGS #" << i << endl;
  e->PrintOut();
  ++ i;
 }
}

Boolean DataReuseModel::IsGroupSpatialLeader(AST_INDEX node)

{
  DRIter ReuseIter(*this);
  Boolean IsLeader = false;
  DataReuseModelEntry *ReuseEntry;

  while ((ReuseEntry = ReuseIter()) != NULL && NOT(IsLeader))
    IsLeader = ReuseEntry->IsGroupSpatialLeader(node);
  return IsLeader;
}

Boolean DataReuseModel::IsGroupSpatialLoadLeader(AST_INDEX node)

{
  DRIter ReuseIter(*this);
  Boolean IsLoadLeader = false;
  DataReuseModelEntry *ReuseEntry;

  while ((ReuseEntry = ReuseIter()) != NULL && NOT(IsLoadLeader))
    IsLoadLeader = ReuseEntry->IsGroupSpatialLoadLeader(node);
  return IsLoadLeader;
}

LocalityType DataReuseModel::GetNodeReuseType(AST_INDEX node)

{
  if (HasSelfTemporalReuse(node))
    return SELF_TEMPORAL;
  else if (HasGroupTemporalReuse(node))
    return GROUP_TEMPORAL;
  else if (HasGroupSpatialReuse(node))
    return GROUP_SPATIAL;
  else if (HasSelfSpatialReuse(node))
    return SELF_SPATIAL;
  else
    return NONE;
}

Boolean DataReuseModel::HasGroupSpatialReuse(AST_INDEX node)

{
  DRIter ReuseIter(*this);
  Boolean IsGroupSpatial = false;
  DataReuseModelEntry *ReuseEntry;

  while ((ReuseEntry = ReuseIter()) != NULL && NOT(IsGroupSpatial))
    IsGroupSpatial = ReuseEntry->HasGroupSpatialReuse(node);

  return IsGroupSpatial;
}


Boolean DataReuseModel::HasSelfSpatialReuse(AST_INDEX node)

{
  DRIter ReuseIter(*this);
  Boolean IsSelfSpatial = false;
  DataReuseModelEntry *ReuseEntry;

  while ((ReuseEntry = ReuseIter()) != NULL && NOT(IsSelfSpatial))
    IsSelfSpatial = ReuseEntry->HasSelfSpatialReuse(node);

  return IsSelfSpatial;
}

Boolean DataReuseModel::HasSelfTemporalReuse(AST_INDEX node)

{
  DRIter ReuseIter(*this);
  Boolean IsSelfTemporal = false;
  DataReuseModelEntry *ReuseEntry;

  while ((ReuseEntry = ReuseIter()) != NULL && NOT(IsSelfTemporal))
    IsSelfTemporal = ReuseEntry->HasSelfTemporalReuse(node);

  return IsSelfTemporal;
}

Boolean DataReuseModel::HasGroupTemporalReuse(AST_INDEX node)

{
  DRIter ReuseIter(*this);
  Boolean IsGroupTemporal = false;
  DataReuseModelEntry *ReuseEntry;

  while ((ReuseEntry = ReuseIter()) != NULL && NOT(IsGroupTemporal))
    IsGroupTemporal = ReuseEntry->HasGroupTemporalReuse(node);

  return IsGroupTemporal;
}

Boolean DataReuseModelEntry::HasGroupSpatialReuse(AST_INDEX node)
{
  GSSetIter GSIter(*gsset);
  Boolean IsGroupSpatial = false;
  GroupSpatialEntry *GSEntry;

  while ((GSEntry = GSIter()) != NULL && NOT(IsGroupSpatial))
    IsGroupSpatial = BOOL(GSEntry->GetNodeList()->Count() > 1 &&
			  GSEntry->Member(node) &&
			  GSEntry->LeaderNode() != node);
  return IsGroupSpatial;
}

Boolean DataReuseModelEntry::HasSelfSpatialReuse(AST_INDEX node)
{
  GSSetIter GSIter(*gsset);
  Boolean IsSelfSpatial = false;
  GroupSpatialEntry *GSEntry;

  while ((GSEntry = GSIter()) != NULL && NOT(IsSelfSpatial))
    IsSelfSpatial = GSEntry->Member(node);
  return BOOL(IsSelfSpatial && gsset->HasSelfSpatial());
}

Boolean DataReuseModelEntry::HasGroupTemporalReuse(AST_INDEX node)

{
  GSSetIter GSIter(*gsset);
  Boolean IsGroupTemporal = false;
  GroupSpatialEntry *GSEntry;

  while ((GSEntry = GSIter()) != NULL && NOT(IsGroupTemporal))
    if (GSEntry->Member(node))
      IsGroupTemporal = GSEntry->HasGroupTemporalReuse(node);
  return IsGroupTemporal;
} 

Boolean DataReuseModelEntry::HasSelfTemporalReuse(AST_INDEX node)
{
  GSSetIter GSIter(*gsset);
  Boolean IsSelfTemporal = false;
  GroupSpatialEntry *GSEntry;

  while ((GSEntry = GSIter()) != NULL && NOT(IsSelfTemporal))
    IsSelfTemporal = GSEntry->Member(node);
  return BOOL(IsSelfTemporal && gsset->HasSelfTemporal());
}

Boolean DataReuseModelEntry::IsGroupSpatialLeader(AST_INDEX node)

{
  GSSetIter GSIter(*gsset);
  Boolean IsLeader = false;
  GroupSpatialEntry *GSEntry;

  while ((GSEntry = GSIter()) != NULL && NOT(IsLeader))
    IsLeader = BOOL(GSEntry->LeaderNode() == node);
  return IsLeader;
}

Boolean DataReuseModelEntry::IsGroupSpatialLoadLeader(AST_INDEX node)

{
  GSSetIter GSIter(*gsset);
  Boolean IsLoadLeader = false;
  GroupSpatialEntry *GSEntry;

  while ((GSEntry = GSIter()) != NULL && NOT(IsLoadLeader))
    IsLoadLeader = BOOL(GSEntry->LoadLeaderNode() == node);
  return IsLoadLeader;
}

DataReuseModelEntry::DataReuseModelEntry(UniformlyGeneratedSetsEntry *ugse) : 
  SinglyLinkedListEntry()
{
 UGSEntryIterator ugseiter(*ugse);
 AST_INDEX node;
 int size;
 la_vect const_vect;

 size = (int)ugse->getSubs();
 const_vect = la_vecNew(size);

 gsset = new GroupSpatialSet((la_vect)ugse->getLocal(),
			     (int)ugse->getNestl(),
			     (int)ugse->getSubs(),
                             ugse->getName(),
			     (la_matrix)ugse->getH() );
 while((node = (AST_INDEX)ugseiter()) && node)
   {
     if (ReuseModelDebugFlag)
       {
	 char Text[80];
	 ut_GetSubscriptText( node, Text);
	 cout << "Put " << Text << " " << node << endl;
       }
 
    ugse->GetConstants( node, const_vect);
    gsset->PutintoGSEntry(node, const_vect); 
   }
}

GroupSpatialSet::GroupSpatialSet(la_vect lisp,
                                int nestl, int subscript, 
				char *name_in,
				 la_matrix inH) : SinglyLinkedList()
{
 size = 0;
 (void)strcpy(Name, name_in);
 if (ReuseModelDebugFlag)
   cout<<"Nestl = "<< nestl << endl;
 Nestl = nestl;
 if (ReuseModelDebugFlag)
   cout<<"Subs = " << subscript << endl;
 Subs = subscript;
 LocIterSpace = la_vecNew(Nestl);
 la_vecCopy(lisp, LocIterSpace, Nestl);
 H = la_matNew(Subs, Nestl);
 la_matCopy(inH, H, Subs, Nestl);
 IsSelfTemporal = IsSelfSpatial = False;
 FindSelfReuse();
}

void GroupSpatialSet::FindSelfReuse()
{
 la_matrix H_S, X;
 int i, j, numSol;
 la_vect zerovect;

 // PrintH();
 zerovect = la_vecNew(Subs);
 la_vecClear(zerovect, Subs);

 H_S = la_matNew(Subs, Nestl);
 la_matCopy(H, H_S, Subs, Nestl);
 
 for (j = 0; j<Nestl; ++j)
     H_S[0][j] = 0;

 if (ReuseModelDebugFlag)
   cout << "Testing for Self Spatial Reuse" << endl;
 if (Solve(H_S, zerovect, Subs, Nestl, &X, &numSol))
   {
     if (ReuseModelDebugFlag)
       {
	 cout << "Print Solution" << endl;
      	 for( i = 0; i < numSol; i++)
	   {
	     cout << "Solution " << i << ": " ;
	     for ( j = 0; j < Nestl; j++)
      	      cout <<"\t"<< X[i][j];
	     cout << endl << endl;
	   }
       }

     IsSelfSpatial = IsSolutionInLIS(X, numSol);
   }
 else
   IsSelfSpatial = False;

 if (ReuseModelDebugFlag)
   if(IsSelfSpatial)
     cout <<"\nIs Self Spatial" << endl;
   else
     cout <<"\nIs Not Self Spatial" << endl;
 if (ReuseModelDebugFlag)
   cout << "Testing for Self Temporal Reuse" << endl;
 if(IsSelfSpatial)
   {
    if ( Solve(H, zerovect, Subs, Nestl, &X, &numSol))
      {
	if (ReuseModelDebugFlag)
	  {
	      {
		cout << "Solution " << i << ": " ;
		for ( j = 0; j < Nestl; j++)
		  cout <<"\t"<< X[i][j];
		cout << endl << endl;
	      }
	  }
	IsSelfTemporal = IsSolutionInLIS(X, numSol);
      }
    else
      IsSelfTemporal = False; 
   }
 if (ReuseModelDebugFlag)
   if (IsSelfTemporal) 
     cout <<"\nIs Self Temporal" << endl;
   else
     cout <<"\nIs Not Self Temporal" << endl;
 
}

int GroupSpatialSet::IsSolutionInLIS(la_matrix sol, int num)
{
 int i, j;
 int intersect = False, cord;

 if (ReuseModelDebugFlag)
   {
     cout << "Self Reuse LIS: " ;
     for ( j = 0; j < Nestl; j++)
       cout <<"\t"<< LocIterSpace[j];
     cout << endl << endl;
   }


 //
 // For self reuse a solution of all zeroes is not valid because
 // the reuse must be carried across a loop iteration.
 //
 // We aren't concerned about actual distances here (although we may
 // want to be in the future (cache line size)). We are looking for 
 // a solution that is in the vector space that is not all 0's
 

 for( i = 0; i<num-1 && !intersect; ++ i)
   {

     Boolean AllZero = true;
     for (j = 0;j < Nestl && AllZero; j++)
       if (sol[i][j] + sol[num-1][j] != 0)
	 AllZero = false;

     if (NOT(AllZero))
       {
	 cord = True;
	 for( j = 0; j<Nestl && cord; j++)
	   {
	     if(LocIterSpace[j] == 0 && sol[i][j] + sol[num-1][j] != 0)
	       cord = False;
	     else if ( LocIterSpace[j] != 0 && 
		       (sol[i][j] + sol[num-1][j] == 0))  // don't need to check % here
	       cord = False;
	   } 
	 
	 
	 if (ReuseModelDebugFlag)
	   {
	     cout << "Solution " << i << ": " ;
	     for ( j = 0; j < Nestl; j++)
	       cout <<"\t"<< sol[i][j] + sol[num-1][j];
	     cout << endl << endl;
	   }
	     
	 intersect = cord;
       }
     else
       intersect = False;
   }

  if ( intersect ) return intersect;
  else
   {
     Boolean AllZero = true;
     for (j = 0;j < Nestl && AllZero; j++)
       if (sol[num-1][j] != 0)
	 AllZero = false;

     if (NOT(AllZero))
       {
	 cord = True;
	 for ( j = 0 ; j < Nestl && cord ; j++ )
	   {
	     if ( LocIterSpace[j] == 0 && sol[num-1][j] != 0 )
	       cord = False;
	     else if ( LocIterSpace[j] != 0 && sol[num-1][j] == 0)  // no % needed
	       cord = False; 
	   }
	 intersect = cord;
       }
     else
       intersect = False;

     return intersect;
   }
 
}

int GroupSpatialSet:: Strideof(int dim)
{
 int i;

 for(i=0; i<Nestl; ++i)
   if( H[dim][i] != 0) return ( H[dim][i] );
 return (0);
}

void GroupSpatialSet::PutintoGSEntry(AST_INDEX node, la_vect c_vect)
{
 GSSetIter gssetiter(this);
 GroupSpatialEntry *gsentry; 
 int Done = False;

   
 while((gsentry = (GroupSpatialEntry *)gssetiter()) && !Done)
   {
    if(gsentry->take(node, c_vect))
      Done = True;
   }
  
 if(!Done)
   {
    gsentry = new GroupSpatialEntry(LocIterSpace, Nestl, Subs, H, node, c_vect);
    ++ size;
    SinglyLinkedList::Append( (SinglyLinkedListEntry*)gsentry ) ;
   }
}

GroupSpatialEntry* GSSetIter::operator () ()
{
 GroupSpatialEntry *e = (GroupSpatialEntry *) SinglyLinkedListIterator::Current();
 ++(*this);
 return e;
}

void GroupSpatialSet::PrintGSSet()
{
 GroupSpatialEntry *e;
 int i = 0;
 char Text[80];

 cout << "\tReference Name " << Name << endl;
 cout << "\tTotal # of GS Sets = " << size << endl;
 cout << "\tH is " << endl;
 PrintH();
 cout << "\tLIS is " << endl;
 PrintLIS();
 if( IsSelfTemporal ) cout << "\tHas Self Temporal!!!" << endl;
 else if ( IsSelfSpatial ) cout << "\tHas Self Spatial!!!" << endl;
 
 if( !IsSelfTemporal )
      for( GSSetIter gsiter(this);
    	    e = gsiter(); )
    	{
	     cout << "\tGS Set #" << i <<  endl;
	     e->PrintOut();
	     i ++;
	}
}


void GroupSpatialSet::PrintH()
{
 int i,j;
    
      for (i = 0; i < Subs; i++)
        {
          for (j = 0; j < Nestl; j++)
            printf("\t\t%d",H[i][j]);
          printf("\n");
        }
}

void GroupSpatialSet::PrintLIS()
{
 int i;
    
  for (i = 0; i< Nestl; i++)
     printf("\t\t%d \n",LocIterSpace[i]);
  printf("\n");

}


GroupSpatialEntry::GroupSpatialEntry(la_vect loc, int nestl, 
				     int sub, la_matrix h, 
				     AST_INDEX node,
				     la_vect c_vect) : SinglyLinkedListEntry()
{
 Nestl = nestl;
 Subs = sub;
 LocIterSpace = la_vecNew(Nestl);
 la_vecCopy(loc, LocIterSpace, Nestl);
 ZeroSpace = la_vecNew(Nestl);
 la_vecClear(ZeroSpace, Nestl);
 H = la_matNew(Subs, Nestl);
 la_matCopy(h, H, Subs, Nestl);
 leader_v = la_vecNew(sub);
 la_vecCopy(c_vect, leader_v, sub);
 leader_n = node;
 trailer_v = la_vecNew(sub);
 load_leader_v = la_vecNew(sub);
 la_vecCopy(c_vect, trailer_v, sub);
 la_vecCopy(c_vect, load_leader_v, sub);
 trailer_n = node;
 load_leader_n = node;
 gts = new GroupTemporalSet(loc, nestl, sub, h); 
 gts->PutintoGTEntry(node, c_vect);
 NodeList = new GenericList;
 NodeList->Append((Generic)node);
 NumGap = 0;
 Marked = 0;
}

Boolean GroupSpatialEntry::Member(AST_INDEX node)

{
  GenericListIter GSEIter(*NodeList);
  GenericListEntry *GLEntry;
  Boolean Found = false;

  while((GLEntry = GSEIter()) != NULL && NOT(Found))
    Found = BOOL(node == GLEntry->GetValue());

  return Found;
}

Boolean GroupSpatialEntry::HasGroupTemporalReuse(AST_INDEX node)

{
  GTSetIter GTIter(this->gts);
  AST_INDEX node1;
  GroupTemporalEntry *GTEntry;
  Boolean IsGroupTemporal = false;

  while((GTEntry = GTIter()) != NULL && NOT(IsGroupTemporal))
    IsGroupTemporal = BOOL(GTEntry->GetNodeList()->Count() > 1 &&
			   GTEntry->LeaderNode() != node &&
			   GTEntry->Member(node));

  return IsGroupTemporal;
}

int GroupSpatialEntry::take(AST_INDEX n, la_vect in_c)
{
 int Accept = False;
 int i;
 
     if (ReuseModelDebugFlag)
       {
	 cout <<endl << " Leader is " << endl;
	 for ( i = 0; i<Subs; ++ i)
	   cout <<"\t" << leader_v[i] <<endl;
	 
	 cout <<endl << " In_C is " << endl;
	 for ( i = 0; i<Subs; ++ i)
	   cout <<"\t" << in_c[i] <<endl;
       }

 if( NodeshasGS(leader_v, in_c))
   {
    gts->PutintoGTEntry(n, in_c );
    NodeList->Append((Generic)n);
    if(ChangeLeader(leader_v, in_c, Subs,leader_n,n))
       leader_n = n;
    if(ChangeTrailer(trailer_v, in_c, Subs,trailer_n,n))
       trailer_n = n;
    if(ChangeLoadLeader(load_leader_v, in_c, Subs,load_leader_n,n))
       load_leader_n = n;
    Accept = True;
   } 
 
 return Accept;
}

int GroupSpatialEntry::NodeshasGS(la_vect vect1, la_vect vect2)
{
 int i,j,numSol;
 la_matrix H_S, X;
 int HasGroupSpatial; 
 la_vect b;

  H_S = la_matNew(Subs,Nestl);
  la_matCopy(H,H_S,Subs,Nestl);
  for (j = 0;j < Nestl; j++)
       H_S[0][j] = 0;
  b = la_vecNew(Subs);
  for(i=0; i<Subs; i++)
       b[i] = vect1[i] - vect2[i];
  b[0] = 0;

  if (ReuseModelDebugFlag)
    {
      cout << endl << "b is" << endl;
      for ( i = 0; i < Subs; ++ i)
	cout <<"\t" << b[i] << endl;
      cout << endl << endl;
      cout << "Test for GroupSpatial Reuse" << endl;
    }

  if (Solve(H_S, b, Subs, Nestl, &X, &numSol))
    { 
       if (SolutionIsValid(H_S, X[numSol-1], b))
         HasGroupSpatial = IsSolutionInLIS(X,X[numSol-1],numSol,Nestl,
                                           True);
       else
         HasGroupSpatial = False;
     la_matFree(X,numSol,Nestl);
    }
  else
       HasGroupSpatial = False;
  la_vecFree(b);
  la_matFree(H_S,Subs,Nestl);
  if (ReuseModelDebugFlag)
    if (HasGroupSpatial)
      cout << "Is Group Spatial" << endl;
    else
      cout << "Is Not Group Spatial" << endl;
  
  return(HasGroupSpatial);

}

int GroupSpatialEntry::SolutionIsValid(la_matrix A,
				       la_vect x,
				       la_vect b)
{
 la_vect temp;

 temp = la_vecNew(Subs);
 la_matVecMult(A,Subs,Nestl,x,temp);
 return((la_vecEq(b,temp,Subs)));

}

int GroupSpatialEntry::CheckIntersect(la_vect X1,
                                      la_vect X2,
                                          int n)
{
  int Intersect = True;
  int j;

  if (ReuseModelDebugFlag)
    {
      cout << "Solution: " ;
      for ( j = 0; j < n; j++)
	cout <<"\t"<< X1[j] + X2[j];
      cout << endl << endl;
    }
  for (j = 0; j < n-1 && Intersect; j++)
    if (LocIterSpace[j] == 0)
      Intersect = (int)(Intersect && (X1[j] + X2[j] == 0));
    else 
      Intersect = (int)(Intersect && ((X1[j] + X2[j]) % LocIterSpace[j] == 0)); 
  return(Intersect);
}


int GroupSpatialEntry::IsSolutionInLIS(la_matrix X,
                                       la_vect r_p,
                                       int row,
                                       int column,
                    	               int TestSol)
{
 int Intersect = False;
 double ratio;
 int i, j;

   if (ReuseModelDebugFlag)
     {
       cout << "Group Spatial Entry LIS: " ;
       for ( j = 0; j < column; j++)
	 cout <<"\t"<< LocIterSpace[j];
       cout << endl << endl;
       cout << "r_p is: ";
       for (i = 0; i < column; i++)
	 cout << r_p[i] << "  ";
       cout << endl << endl;
     }
   for (i = 0; i < row-1 && !(Intersect); i++)
     Intersect = CheckIntersect(X[i],r_p,column);
   if (TestSol && !(Intersect))
     return(CheckIntersect(r_p,ZeroSpace,column));
   else
     return(Intersect);
}

void GroupSpatialEntry::PrintOut()
{
 char Text[80];

 ut_GetSubscriptText( leader_n, Text);
 cout<< "\t\tLeader is " << Text << endl;

 ut_GetSubscriptText( trailer_n, Text);
 cout<< "\t\tTrailer is " << Text << endl;

 ut_GetSubscriptText( load_leader_n, Text);
 cout<< "\t\tLoad Leader is " << Text << endl;

 gts->PrintOut();
}
 
GroupTemporalSet::GroupTemporalSet(la_vect loc, int nestl, int sub, 
				   la_matrix h) : SinglyLinkedList()
{
 size = 0;
 Nestl = nestl;
 Subs = sub;
 LocIterSpace = la_vecNew(Nestl);
 la_vecCopy(loc, LocIterSpace, Nestl);
 H = la_matNew(Subs, Nestl);
 la_matCopy(h, H, Subs, Nestl);
}

void GroupTemporalSet::PutintoGTEntry(AST_INDEX n, la_vect c_vect)
{
 GTSetIter gtsetiter(this);
 GroupTemporalEntry *gtentry;
 int Done = False;

 while((gtentry = (GroupTemporalEntry *)gtsetiter()) && !Done)
   {
    if(gtentry->take(n,c_vect))
	Done = True;
   }

if(!Done)
  {
   gtentry = new GroupTemporalEntry(LocIterSpace, Nestl, Subs, H, n, c_vect);
   ++ size;
   SinglyLinkedList::Append( gtentry );
  } 
 
}
 
GroupTemporalEntry* GTSetIter::operator () ()
{
 GroupTemporalEntry *e = (GroupTemporalEntry *) SinglyLinkedListIterator::Current();
 ++(*this);
 return e;
}

void GroupTemporalSet::PrintOut()
{
 GroupTemporalEntry *e;
 int i = 0;
 char Text[80];

 cout << "\t\tTotal # of GT = " << size << endl;
 for( GTSetIter gtiter(this);
        e = gtiter(); )
   {
     cout << "\t\tGT Set #" << i  << endl;
     e->PrintOut();
     i ++;
   }
}


void GroupTemporalSet::FillArray(int* array, int start)
{
 GroupTemporalEntry *e;
 int i = start;

 for( GTSetIter gtiter(this);
         e = gtiter(); )
    {
     array[i] = e->getleader()[0]; 
     ++ i;
    } 
}

GroupTemporalEntry::GroupTemporalEntry(la_vect loc, int nestl, int sub,
				       la_matrix h, 
				       AST_INDEX n, la_vect in_c) : SinglyLinkedListEntry()
{
 size = 0;
 Nestl = nestl;
 Subs = sub;
 LocIterSpace = la_vecNew(Nestl);
 la_vecCopy(loc, LocIterSpace, Nestl);
 ZeroSpace = la_vecNew(Nestl);
 la_vecClear(ZeroSpace, Nestl);
 H = la_matNew(Subs, Nestl);
 la_matCopy(h, H, Subs, Nestl);
 leader_v = la_vecNew(sub);
 la_vecCopy(in_c, leader_v, sub); 
 leader_n = n;
 vectlst = new VectList(Subs); 
 vectlst->AddVect(n, in_c); 
 NodeList = new GenericList;
 NodeList->Append((Generic)n);
}

Boolean GroupTemporalEntry::Member(AST_INDEX node)

{
  GenericListIter GTEIter(*NodeList);
  GenericListEntry *GLEntry;
  Boolean Found = false;

  while((GLEntry = GTEIter()) != NULL && NOT(Found))
    Found = BOOL(node == GLEntry->GetValue());

  return Found;
}

int GroupTemporalEntry::take(AST_INDEX n, la_vect c_vect)
{
 int Accept = False;

 if( NodeshasGT(leader_v, c_vect) )
  {
   vectlst->AddVect(n, c_vect);
   NodeList->Append((Generic)n);
   Accept = True;
   if(ChangeLeader(leader_v, c_vect, Subs,leader_n,n))
     leader_n = n;
  }

 return Accept;
	
}

int GroupTemporalEntry::NodeshasGT(la_vect vect1, la_vect vect2)
{
 int i,j,numSol;
 la_matrix X;
 la_vect b;
 int HasGroupTemporal;

 b = la_vecNew(Subs);
 for(i=0; i<Subs; i++)
   b[i] = vect1[i] - vect2[i];
 if (Solve(H, b, Subs, Nestl, &X, &numSol))
  {
   if (SolutionIsValid(H,X[numSol-1],b))
     HasGroupTemporal = IsSolutionInLIS(X,X[numSol-1],numSol,Nestl, True);
   else
     HasGroupTemporal = False;
   la_matFree(X,numSol,Nestl);
  }
 else
   HasGroupTemporal = False;
 la_vecFree(b);
 if (ReuseModelDebugFlag)
   if (HasGroupTemporal)
     cout << "Is Group Temporal" << endl;
   else
     cout << "Is Not Group Temporal" << endl;
 return(HasGroupTemporal); 
 
}

 int GroupTemporalEntry::SolutionIsValid(la_matrix A,
                                       la_vect x,
                                       la_vect b)
{
 la_vect temp;

 temp = la_vecNew(Subs);
 la_matVecMult(A,Subs,Nestl,x,temp);
 return((la_vecEq(b,temp,Subs)));

}

int GroupTemporalEntry::CheckIntersect(la_vect X1,
                                      la_vect X2,
                                          int n)
{
  int Intersect = True;
  int j;

  if (ReuseModelDebugFlag)
    {
      cout << "Solution: " ;
      for ( j = 0; j < n; j++)
	cout <<"\t"<< X1[j] + X2[j];
      cout << endl << endl;
    }
  for (j = 0; j < n && Intersect; j++)
    if (LocIterSpace[j] == 0)
      Intersect = (int)(Intersect && (X1[j] + X2[j] == 0));
    else
      Intersect = (int)(Intersect && ((X1[j] + X2[j]) % LocIterSpace[j] == 0));
  
  return(Intersect);
}

int GroupTemporalEntry::IsSolutionInLIS(la_matrix X,
                                       la_vect r_p,
                                       int row,
                                       int column,
                                       int TestSol)
{
 int Intersect = False;
 double ratio;
 int i;

   if (ReuseModelDebugFlag)
     {
       cout << "Group Temporal Entry LIS: ";
       for (i = 0; i < column; i++)
	 cout << LocIterSpace[i] << "  ";
       cout << endl << endl;
       cout << "r_p is: ";
       for (i = 0; i < column; i++)
	 cout << r_p[i] << "  ";
       cout << endl << endl;
     }
   for (i = 0; i < row-1 && !(Intersect); i++)
     Intersect = CheckIntersect(X[i],r_p,column);
   if (TestSol && !(Intersect))
     return(CheckIntersect(r_p,ZeroSpace,column));
   else
     return(Intersect);
}

void GroupTemporalEntry::PrintOut()
{
 char Text[80];

 cout << "\t\t\t" << "Leader is "; 
 ut_GetSubscriptText( leader_n, Text);
 cout <<  Text << endl;

 vectlst->PrintOut();

}
 
VectListEntry::VectListEntry(AST_INDEX n, la_vect v, int sub) :  SinglyLinkedListEntry()
{
     const_vect = la_vecNew(sub);
     la_vecCopy(v, const_vect, sub);
     node = n;
}

void VectListEntry::PrintOut()
{
 char Text[80];
 
 ut_GetSubscriptText(node, Text);
 cout << "\t\t\t" << Text << endl;
}

VectList::VectList(int sub) :  SinglyLinkedList()
{
 Subs = sub;
}

void VectList::AddVect(AST_INDEX n, la_vect v)
{
 VectListEntry *e = new VectListEntry(n, v, Subs);
 SinglyLinkedList::Append(e);
}

void VectList::PrintOut()
{
 int i = 0;
 VectListEntry *e;
 char Text[80];

 for( VectListIter vlstiter(this);
        e = vlstiter(); )
  e->PrintOut();
}

VectListEntry* VectListIter::operator () ()
{
 VectListEntry *e = (VectListEntry *)SinglyLinkedListIterator::Current();
 ++(*this);
 return e;
}

int Solve(la_matrix B,
          la_vect   c,
          int       m, 
          int       n, 
          la_matrix *S, 
          int       *num)  

//
// This code is from Wei Li's compiler
//

{
  int *p, i, j, k, swap_elem, mult1, mult2, lcm, scaler;
  la_vect swap_row, total_scaler;
  la_matrix T, s;

  LA_MATRIX_T TT, TT1;

  p = (int *)malloc(sizeof(int)*n);
  for (i=0; i<n; i++)
    {
      p[i] = i;
    }

  T = la_matNew(m, n+1);
  for (i=0; i<m; i++)
    {
      for (j=0; j<n; j++)
        {
          T[i][j] = B[i][j];
        }
      T[i][n] = c[i];
    }

  TT = la_matrix_allocate();
  LA_MATRIX(TT) = T;
  LA_MATRIX_ROWSIZE(TT) = m;
  LA_MATRIX_COLSIZE(TT) = n+1;

  TT1 = la_base(TT);
  m = LA_MATRIX_ROWSIZE(TT1);
  T = LA_MATRIX(TT1);

  la_matrix_free(TT);

  s = la_matNew(n+1,n+1);

  total_scaler = la_vecNew(n+1);
  for (i=0; i<=n; i++)
    {
      total_scaler[i] = 1;
    }
  
  i = 0;
  while (i<m)
    {
      if (T[i][i] == 0)
        {
          /* pivot is zero */
          for (j=i+1; j<m; j++)
            {
              if (T[j][i] != 0)
                {
                  swap_row = T[i];
                  T[i] = T[j];
                  T[j] = swap_row;
                  break;
                }
            }

          if (j == m)
            {
              /* column is zero below diagonal */
              for (j=i+1; j<n; j++)
                {
                  if (T[i][j] != 0)
                    {
                      /* swap columns */
                      for (k=0; k<m; k++)
                        {
                          swap_elem = T[k][i];
                          T[k][i] = T[k][j];
                          T[k][j] = swap_elem;
                        }
                      swap_elem = p[i];
                      p[i] = p[j];
                      p[j] = swap_elem;
                      break;
                    }
                }

              if (j == n)
                {
                  /* row is zero */
                  T[i] = T[m-1];
                  m--;
                  continue;
                }
            }
        }
      
      /* pivot is non zero */
      for (j=i+1; j<m; j++)
        {
          if (T[j][i] != 0)
            {
              lcm = sign(T[i][i]) * sign(T[j][i])
                * la_lcm(T[i][i], T[j][i]);
              mult1 = lcm/T[i][i];
              mult2 = lcm/T[j][i];
              for (k=i+1; k<=n; k++)
                {
                  T[j][k] = T[j][k]*mult2 - T[i][k]*mult1;
                }
            }
        }
      
      i++;
    }

  if (m < n)
    {
      for (i=0; i<m; i++)
        {
          for (j=m; j<n; j++)
            {
              T[i][j] = -T[i][j];
            }
        }
      *num = (n-m) + 1;
    }
  else if (m > n)
    {
      for (i=n; i<m; i++)
        {
          if (T[i][n] != 0)
            {
              return 0;
            }
        }
      *num = 1;
    }
  else
    {
      *num = 1;
    }

  for (i=m-1; i>=0; i--)
    {
      for (j=m; j<=n; j++)
        {
          /* scale solution */
          scaler = T[i][i]/la_gcd(T[i][i], T[i][j]);
          total_scaler[j] *= scaler;
          for (k=0; k<m; k++)
            {
              T[k][j] *= scaler;
            }

          /* divide to get answer */
          T[i][j] = T[i][j]/T[i][i];

          /* substitute answer in other eqs */
          for (k=0; k<i; k++)
            {
              T[k][j] -= T[k][i]*T[i][j];
            }
        }
    }


  for (i=0; i<(*num); i++)
    {
      for (j=0; j<m; j++)
        s[i][p[j]] = T[j][i+m];
      for (j=m; j<n; j++)
        s[i][p[j]] = ((i==(j-m)) && (i!=(*num - 1))) * total_scaler[i+m];
    }

  *S = la_matNew(*num,0);
  for (i=0; i<*num; i++)
    {
      (*S)[i] = s[i];
    }

  return 1;
}

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

      

static int NewReferenceIsEarlier(AST_INDEX Old,
			  	 AST_INDEX New)

{
  AST_INDEX OldStmt = ut_get_stmt(Old);
  AST_INDEX NewStmt = ut_get_stmt(New);
  stmt_info_type *OldSptr = get_stmt_info_ptr(OldStmt);
  stmt_info_type *NewSptr = get_stmt_info_ptr(NewStmt);

  if (NewSptr->stmt_num < OldSptr->stmt_num)
    return True;

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
	return True;
    }

  return False;
}

static int NewReferenceIsLater(AST_INDEX Old,
			       AST_INDEX New)

{
  AST_INDEX OldStmt = ut_get_stmt(Old);
  AST_INDEX NewStmt = ut_get_stmt(New);
  stmt_info_type *OldSptr = get_stmt_info_ptr(OldStmt);
  stmt_info_type *NewSptr = get_stmt_info_ptr(NewStmt);

  if (NewSptr->stmt_num > OldSptr->stmt_num)
    return True;

  if (NewSptr->stmt_num == OldSptr->stmt_num)
    {
      StmtOrderInfoType StmtOrderInfo;

      StmtOrderInfo.Old = Old;
      StmtOrderInfo.New = New;
      StmtOrderInfo.Found = 0;

      if (is_assignment(NewStmt))
	walk_expression(gen_ASSIGNMENT_get_rvalue(NewStmt),
			(WK_EXPR_CLBACK)CheckNodes,
			(WK_EXPR_CLBACK)NOFUNC,
			(Generic)&StmtOrderInfo);
      else
        walk_expression(NewStmt,(WK_EXPR_CLBACK)CheckNodes,(WK_EXPR_CLBACK)NOFUNC,
		        (Generic)&StmtOrderInfo);
      if (StmtOrderInfo.Found == 2)
	return True;
    }

  return False;
}


int ChangeLeader(la_vect leader, la_vect new_v, int size, 
		 AST_INDEX leader_n, AST_INDEX new_node)
{
 int Change = False;
 int Done = False;
 int i = size - 1 ;
 Boolean AllEqual = true;

 while( i >= 0 &&  !Done)
   {
    if(leader[i] > new_v[i])
      {	
        Done = True;
        AllEqual = false;
      }
    else if ( leader[i] < new_v[i] )
       {
         Done = True;
         Change = True;
         AllEqual = false;
       } 
    -- i;
   }

  if (AllEqual)
    Change = NewReferenceIsEarlier(leader_n,new_node);
  
  if(Change)
    for( i = 0; i<size; i++)
      leader[i] = new_v[i];

 return Change; 
  
}

int ChangeTrailer(la_vect trailer, la_vect new_v, int size,
		  AST_INDEX trailer_n, AST_INDEX new_node)
{

//
// Note that trailer nodes are only going to be loads
// This information is used determine which loads should bring
// in two cache lines.
//

 int Change = False;
 int Done = False;
 int i = size - 1 ;
 Boolean AllEqual = true;

 AST_INDEX TrailerStmt = ut_get_stmt(trailer_n);

 if (is_assignment(TrailerStmt))
   if (gen_ASSIGNMENT_get_lvalue(TrailerStmt) == new_node)
     return False;

 while( i >= 0 &&  !Done)
   {
    if(trailer[i] < new_v[i])
      {	
        Done = True;
        AllEqual = false;
      }
    else if ( trailer[i] > new_v[i] )
       {
         Done = True;
         Change = True;
         AllEqual = false;
       } 
    -- i;
   }
  
  if (AllEqual)
    Change = NewReferenceIsLater(trailer_n,new_node);
  
  if(Change)
    for( i = 0; i<size; i++)
      trailer[i] = new_v[i];

 return Change; 
  
}


int ChangeLoadLeader(la_vect load_leader, la_vect new_v, int size,
		     AST_INDEX load_leader_n, AST_INDEX new_node)
{

 int Change = False;
 int Done = False;
 int i = size - 1 ;
 Boolean AllEqual = true;

 AST_INDEX LoadLeaderStmt = ut_get_stmt(load_leader_n);

 if (is_assignment(LoadLeaderStmt))
   if (gen_ASSIGNMENT_get_lvalue(LoadLeaderStmt) == new_node)
     return False;

 while( i >= 0 &&  !Done)
   {
    if(load_leader[i] > new_v[i])
      {	
        Done = True;
        AllEqual = false;
      }
    else if ( load_leader[i] < new_v[i] )
       {
         Done = True;
         Change = True;
         AllEqual = false;
       } 
    -- i;
   }
  
  if (AllEqual)
    Change = NewReferenceIsLater(load_leader_n,new_node);
  
  if(Change)
    for( i = 0; i<size; i++)
      load_leader[i] = new_v[i];

 return Change; 
  
}
