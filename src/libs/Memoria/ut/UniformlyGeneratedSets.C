#include <assert.h>
#include <general.h>
#include <UniformlyGeneratedSets.h>
#include <pt_util.h>
#include <Lambda/Lambda.h>

Boolean UniformlyGeneratedSetsEntry::SameUniformlyGeneratedSet(la_matrix nodeH)

  {
   int i,j;

     if (NOT(Uniform)) 
       return (false);
     for (i = 0; i < Subscripts; i++)
       for (j = 0; j < NestingLevel; j++)
         if (H[i][j] != nodeH[i][j])
	   return(false);
     return(true);
  }

void UniformlyGeneratedSetsEntry::GetConstants(AST_INDEX node1,
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
					       

#define sign(x) (((x)>0) ? 1 : -1)


int UniformlyGeneratedSetsEntry::Solve(la_matrix B,
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

Boolean UniformlyGeneratedSetsEntry::NullSpaceIsZero(la_matrix X,
						     int row,
						     int column)

{
  int i;

  for (i = 0; i < row-1; i++)
    if (!la_vecIsZero(X[i],column))
      return(false);
  return(true);
}


Boolean UniformlyGeneratedSetsEntry::SolutionIsValid(la_matrix A,
						     la_vect x,
						     la_vect b)
{
  la_vect temp;

  temp = la_vecNew(Subscripts);
  la_matVecMult(A,Subscripts,NestingLevel,x,temp);
  return(BOOL(la_vecEq(b,temp,Subscripts)));
}

Boolean UniformlyGeneratedSetsEntry::CheckIntersect(la_vect X1,
						    la_vect X2,
						    int n)
{
  Boolean Intersect = true;
  int j;

  for (j = 0; j < n-1 && Intersect; j++)
    if (LocalizedIterationSpace[j] == 0)
      Intersect = BOOL(Intersect && (X1[j] + X2[j] == 0));
    else 
      Intersect = BOOL(Intersect && (X1[j] + X2[j] != 0)); 
  return(Intersect);
}

Boolean UniformlyGeneratedSetsEntry::IsSolutionInLIS(la_matrix X,
						     la_vect r_p,
						     int row,
						     int column,
						     Boolean TestSol)

  {
   Boolean Intersect = false;
   double ratio;
   int i;

   for (i = 0; i < row-1 && NOT(Intersect); i++)
     Intersect = CheckIntersect(X[i],r_p,column);
   if (TestSol && NOT(Intersect))
     return(CheckIntersect(r_p,ZeroSpace,column));
   else
     return(Intersect);
  }

Boolean 
UniformlyGeneratedSetsEntry::NodesHaveGroupSpatialReuse(AST_INDEX node1,
							AST_INDEX node2)
  {
   int i,j,numSol;
   la_matrix H_S,X;
   la_vect C1,C2,b;
   Boolean HasGroupSpatial;

     H_S = la_matNew(Subscripts,NestingLevel); 
     C1 = la_vecNew(Subscripts);
     C2 = la_vecNew(Subscripts);
     la_matCopy(H,H_S,Subscripts,NestingLevel);
     for (j = 0;j < NestingLevel; j++)
       H_S[0][j] = 0;
     GetConstants(node1,C1);
     GetConstants(node2,C2);
     C1[0] = 0;
     C2[0] = 0;
     b = la_vecNew(Subscripts);
     for(i=0; i<Subscripts; i++)
       b[i] = C1[i] - C2[i];
     if (Solve(H_S, b, Subscripts, NestingLevel, &X, &numSol))
       if (SolutionIsValid(H,X[numSol-1],b))
	 HasGroupSpatial = IsSolutionInLIS(X,X[numSol-1],numSol,NestingLevel,
					   true);
       else
	 HasGroupSpatial = false;
     else
       HasGroupSpatial = false;
     la_vecFree(b);
     la_matFree(X,Subscripts,NestingLevel);
     la_matFree(H_S,Subscripts,NestingLevel);
     la_vecFree(C1);
     la_vecFree(C2);
     return(HasGroupSpatial);
  }

Boolean 
UniformlyGeneratedSetsEntry::NodesHaveGroupTemporalReuse(AST_INDEX node1,
							 AST_INDEX node2)
  {
   int i,j,numSol;
   la_matrix X;
   la_vect C1,C2,b;
   Boolean HasGroupTemporal;

     C1 = la_vecNew(Subscripts);
     C2 = la_vecNew(Subscripts);
     GetConstants(node1,C1);
     GetConstants(node2,C2);
     b = la_vecNew(Subscripts);
     for(i=0; i<Subscripts; i++)
       b[i] = C1[i] - C2[i];
     if (Solve(H, b, Subscripts, NestingLevel, &X, &numSol))
       if (SolutionIsValid(H,X[numSol-1],b))
	 HasGroupTemporal = IsSolutionInLIS(X,X[numSol-1],numSol,NestingLevel,
					    true);
       else
	 HasGroupTemporal = false;
     else
       HasGroupTemporal = false;
     la_vecFree(b);
     la_matFree(X,Subscripts,NestingLevel);
     la_vecFree(C1);
     la_vecFree(C2);
     return(HasGroupTemporal);
  }

Boolean 
UniformlyGeneratedSetsEntry::SingleNodeHasSelfTemporalReuse()
  {
   int i,j,numSol;
   la_matrix X;
   Boolean HasSelfTemporal = false;

     if (NOT(Uniform)) 
       return (false);   
     if (Solve(H, ZeroSpace, Subscripts, NestingLevel, &X, &numSol))
       if (NullSpaceIsZero(X,numSol,NestingLevel))
	 HasSelfTemporal = false; 
       else
	 HasSelfTemporal = IsSolutionInLIS(X,ZeroSpace,numSol,NestingLevel,
					   false);
     else
       HasSelfTemporal = false;
     la_matFree(X,Subscripts,NestingLevel);
     return(HasSelfTemporal);
  }

Boolean 
UniformlyGeneratedSetsEntry::SingleNodeHasSelfSpatialReuse()
						     
  {
   int i,j,numSol;
   la_matrix H_S,X;
   la_vect b;
   Boolean HasSelfSpatial = false;

     if (NOT(Uniform)) 
       return (false);
     H_S = la_matNew(Subscripts,NestingLevel);
     la_matCopy(H,H_S,Subscripts,NestingLevel);
     for (j = 0;j < NestingLevel; j++)
       H_S[0][j] = 0;
     b = la_vecNew(Subscripts);
     if (Solve(H_S, ZeroSpace, Subscripts, NestingLevel, &X, &numSol))
       if (NullSpaceIsZero(X,numSol,NestingLevel))
	 HasSelfSpatial = false; 
       else
	 HasSelfSpatial = IsSolutionInLIS(X,ZeroSpace,numSol,NestingLevel,
					  false);
     else
       HasSelfSpatial = false;
     la_vecFree(b);
     la_matFree(X,Subscripts,NestingLevel);
     la_matFree(H_S,Subscripts,NestingLevel);
     return(HasSelfSpatial);
  }


Boolean 
UniformlyGeneratedSetsEntry::SingleNodeHasGroupSpatialReuse(AST_INDEX node1)

  {
   AST_INDEX node2;
   Boolean HasGroupSpatial = false;
   
     if (NOT(Uniform)) 
       return (false);
       for (UGSEntryIterator UGSEIter(*this);
	    node2 = UGSEIter() && NOT(HasGroupSpatial);)
         if (node1 != node2)
	   HasGroupSpatial = NodesHaveGroupSpatialReuse(node1,node2);
     return (HasGroupSpatial);
  }

Boolean 
UniformlyGeneratedSetsEntry::SingleNodeHasGroupTemporalReuse(AST_INDEX node1)

  {
   AST_INDEX node2;
   Boolean HasGroupTemporal = false;
   
     if (NOT(Uniform)) 
       return (false);
       for (UGSEntryIterator UGSEIter(*this);
	    node2 = UGSEIter() && NOT(HasGroupTemporal);)
         if (node1 != node2)
	   HasGroupTemporal = NodesHaveGroupTemporalReuse(node1,node2);
     return (HasGroupTemporal);
  }

void UniformlyGeneratedSets::Append(la_matrix nodeH, AST_INDEX node,
				    int NumSubs, Boolean uniform)

  {
   UniformlyGeneratedSetsEntry *e;
   
     e = new UniformlyGeneratedSetsEntry(nodeH,NestingLevel,NumSubs,
					 LocalizedIterationSpace,
					 uniform);
     (*e) += node;
     (*this) += (int)e;
  }

int UniformlyGeneratedSets::GetIndex(char *ivar)

  {
   int i;

     for (i = 0; i < NestingLevel; i++)
       if (strcmp(ivar,IndexVars[i]) == 0)
         return(i);
     return(-1);
  }


void UniformlyGeneratedSets::ComputeH(AST_INDEX node,la_matrix nodeH,
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
   

void UniformlyGeneratedSets::GetH(AST_INDEX node,
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
       

void UniformlyGeneratedSets::AddNode(AST_INDEX node)

  {
   UniformlyGeneratedSetsEntry *UGSEntry;
   Boolean uniform = true;
   int **nodeH,i,j,Subscripts;

     Subscripts = list_length(gen_SUBSCRIPT_get_rvalue_LIST(node));
     nodeH = new int*[Subscripts];
     for (i = 0; i < Subscripts; i++)
       {
	 nodeH[i] = new int[NestingLevel];
	 for (j = 0; j < NestingLevel; j++)
	   nodeH[i][j] = 0;
       }
     GetH(node,nodeH,&uniform);
     if (uniform && (UGSEntry = GetUniformlyGeneratedSet(nodeH)) != NULL)
       (*UGSEntry) += node;
     else
       Append(nodeH,node,Subscripts,uniform);
  }
   
UniformlyGeneratedSetsEntry *
UniformlyGeneratedSets::GetUniformlyGeneratedSet(AST_INDEX node)

  {
   UniformlyGeneratedSetsEntry *UGSEntry;
   UGSIterator UGSIter(*this);

     while(UGSEntry = (UniformlyGeneratedSetsEntry *)UGSIter())
       if (UGSEntry->QueryEntry(node))
	 return(UGSEntry);
     return(NULL);
  }

UniformlyGeneratedSetsEntry *
UniformlyGeneratedSets::GetUniformlyGeneratedSet(la_matrix nodeH)

  {
   UniformlyGeneratedSetsEntry *UGSEntry;
   UGSIterator UGSIter(*this);

     while(UGSEntry = (UniformlyGeneratedSetsEntry* )UGSIter())
       if (UGSEntry->SameUniformlyGeneratedSet(nodeH))
	 return(UGSEntry);
     return(NULL);
  }


Boolean UniformlyGeneratedSets::NodeHasGroupSpatialReuse(AST_INDEX node)

  {
   UniformlyGeneratedSetsEntry *E;

     if ((E = GetUniformlyGeneratedSet(node)) == NULL)
       return(false);
     else
       return(E->SingleNodeHasGroupSpatialReuse(node));
  }


Boolean UniformlyGeneratedSets::NodeHasGroupTemporalReuse(AST_INDEX node)

  {
   UniformlyGeneratedSetsEntry *E;

     if ((E = GetUniformlyGeneratedSet(node)) == NULL)
       return(false);
     else
       return(E->SingleNodeHasGroupTemporalReuse(node));
  }


Boolean UniformlyGeneratedSets::NodeHasSelfSpatialReuse(AST_INDEX node)

  {
   UniformlyGeneratedSetsEntry *E;

     if ((E = GetUniformlyGeneratedSet(node)) == NULL)
       return(false);
     else
       return(E->SingleNodeHasSelfSpatialReuse());
  }



Boolean UniformlyGeneratedSets::NodeHasSelfTemporalReuse(AST_INDEX node)

  {
   UniformlyGeneratedSetsEntry *E;

     if ((E = GetUniformlyGeneratedSet(node)) == NULL)
       return(false);
     else
       return(E->SingleNodeHasSelfTemporalReuse());
  }

