//      Linear Algebra Data Reuse Model
//              C++ Code File
//	Do Analysis on Data Reuse Model
//	Be Ready for Next Step

#include <iostream.h>
#include <la.h>
#include <assert.h>
#include <pt_util.h>
#include <UniformlyGeneratedSets.h>
#include <Lambda/Lambda.h>
#include <mem_util.h>

extern "C" {
#include <stdlib.h>
};

int BlockSize;

int DimDiffOnly(la_vect, la_vect, int, int);


void DataReuseModel::DoAnalysis(int linesize)
{
 DataReuseModelEntry *e;

 BlockSize = linesize;
 for ( DRIter driter(this);
        e = driter(); )
   e->DoAnalysis(); 
  
}

float DataReuseModel::ComputePrefetch(int level1, int amount1, int level2, int amount2)
{
 float totalprefetch = 0;   // The number of blocks needed be prefetched
 DataReuseModelEntry *e;

 for ( DRIter driter(this);
        e = driter(); )
  totalprefetch += e->ComputePrefetch(level1, amount1, level2, amount2); 
 
 return totalprefetch;
 
}

float DataReuseModel::ComputePrefetch(int level, int amount)
{
 float totalprefetch = 0;
 DataReuseModelEntry *e;

 for ( DRIter driter(this);
        e = driter(); )
  totalprefetch += e->ComputePrefetch(level, amount); 
 
 return totalprefetch;

}

float DataReuseModelEntry::ComputePrefetch(int level1, int amount1, int level2, int amount2)
{
 return gsset->ComputePrefetch(level1,amount1, level2, amount2);
}

float DataReuseModelEntry::ComputePrefetch(int level, int amount)
{
 return gsset->ComputePrefetch(level, amount);
}

float GroupSpatialSet::ComputePrefetch(int level1, int amount1, int level2, int amount2)
{
 int i, j, k, l;
 int dim1, dim2;
 int Find;
 float prefetch_needed = 0;
 int gs_degree = 0;

 if (IsSelfTemporal) return 0;

 dim1 = dim2 = -1;
 // Find the dimension in subscript affected by level1 and level2
 Find = False;
 for( i=0; i<Subs && !Find; ++i)
    if( H[i][level1 - 1] != 0)
      {
       dim1 = i;
       Find = True;
      }

 Find = False;
 for( i=0; i<Subs && !Find; ++i)
    if( H[i][level2 - 1] != 0)
      {
       dim2 = i;
       Find = True;
      }

 if ( dim1 == dim2 && dim1>=0) 
    {
	cerr <<"The Current Model Can't Process This Case!";
        exit(1);
    }

 if( dim1 == 0 ) 
    gs_degree = amount1;
 else if ( dim2 == 0)
    gs_degree = amount2;
 
 int am;

 if( dim1 != 0 && dim1 != -1 ) am = amount1;
 if( dim2 != 0 && dim2 != -1 ) am = am*amount2;
 
 prefetch_needed += computeboard[0].Estimate(IsSelfSpatial, gs_degree, 0);

 for(i=1; i<Subs; ++i)
       if(i == dim1) prefetch_needed += computeboard[i].Estimate(IsSelfSpatial, gs_degree, amount1); 
       else if(i == dim2) prefetch_needed += computeboard[i].Estimate(IsSelfSpatial, gs_degree, amount2); 
       else prefetch_needed += computeboard[i].Estimate(IsSelfSpatial, gs_degree, 0);

 return prefetch_needed;

}

float GroupSpatialSet::ComputePrefetch(int level, int amount)
{
 int i;
 int dim;
 int Find;
 float prefetch_needed = 0;
 int gs_degree = 0;

 if (IsSelfTemporal) return 0;

 dim = -1;
 // Find the dimension in subscript affected by level
 Find = False;
 for( i=0; i<Subs && !Find; ++i)
    if( H[i][level - 1] != 0)
      {
       dim = i;
       Find = True;
      }

 if( dim == 0 ) 
   {
    gs_degree = amount;
    for(i = 0; i < Subs; ++i)
       prefetch_needed += computeboard[i].Estimate(IsSelfSpatial, gs_degree, 0);
   }
 else if ( dim == -1 )
   {
    for( i = 0; i<Subs; ++i)
        prefetch_needed += computeboard[i].Estimate(IsSelfSpatial, 0, 0);
   }
 else
   {
    for( i = 0; i<Subs; ++i)
        if( i != dim )
          prefetch_needed += computeboard[i].Estimate(IsSelfSpatial, 0, 0);
    prefetch_needed += computeboard[dim].Estimate(IsSelfSpatial, 0, amount); 
   }
 return prefetch_needed; 
 
}

void GroupSpatialSet::DoAnalysis()
{
 GroupSpatialEntry *e, *first_e;
 int stride;
 int i, j;

 if ( !IsSelfTemporal )
  {
   computeboard = new ComputeBoard[Subs];
   for( i=1; i<Subs; ++i)
     {
      computeboard[i].ComputeBoard(Strideof(i), i);
     }
   computeboard[0].ComputeBoard(0, 0);

// Gether GSEntry According to the difference on each dimension

   for ( i=1; i<Subs; ++i)
     {
	int done = False;
      for(GSSetIter gsiter(this); 
           e = gsiter() ;)
         {
	      if( !e->Marked ) 
		{
		 first_e = e; 
		 computeboard[i].TakeRows(e); 
		 e->Marked = 1;
		 done = True;
		}
		if (done) break;
         }
      for( gsiter.Reset();
	   e = gsiter();)
	{
         if( !e->Marked )
	     if( DimDiffOnly(first_e->Leader(), e->Leader(), i, Subs))
		{
        	 computeboard[i].TakeRows(e); 
		 e->Marked = 1;
		}
	}
     }
   for ( GSSetIter gsiter(this);
		e = gsiter();)
	if( !e->Marked )
	  computeboard[0].TakeRows(e);	

   for ( i = 0; i < Subs ; ++ i)
	{
	 cout << "ComputeBoard # " << i << endl;
	 computeboard[i].PrintOut();
         computeboard[i].GetReady();

	} 


/****
   for( GSSetIter gsiter(this);
        e = gsiter();)
     e->DoAnalysis();
 ****/
  }
}

void GroupSpatialEntry::DoAnalysis()
{
 gts->DoAnalysis(&NumGap, &TotalBlock, Gap);
}

void GroupTemporalSet::DoAnalysis(int *num_gap, int *block, int *&gap)
{
/****
 GroupTemporalEntry *e;
 int *const_array, *def_gap;
 int i, j, last_const, dist;

 j = SizeofGTSet();
 const_array = new int[j]; 
 def_gap = new int[j];

 (*block) = 1;
 (*num_gap) = 0;

 cout <<"Read in the 1st Dim: " ; 
 i = 0;
 for( GTSetIter gtiter(this);
       e = gtiter(); )
 {
  const_array[i] = e->getleader()[0]; 
  cout << const_array[i] << " " ;
  i ++;
 } 
 cout << endl;
 
 if ( i != j ) cout << " Erro in GRS::DoAnalysis!" << endl; 

 qsort ( const_array, j, sizeof(int), intcompare);
 cout << " The 1st Dim are sorted as: " ;     
 for ( i = 0; i < j; i++ )
     cout <<" " << const_array[i];
 cout << endl;

 last_const = const_array[0];
 for ( i = 0; i < j - 1; i ++ ) 
   {
     dist = abs(const_array[i+1] - const_array[i]); 
    if ( dist > BlockSize )
      {
       def_gap[*num_gap] = dist;
       (*num_gap) ++;
       (*block) += abs(const_array[i] - last_const);
       last_const = const_array[i+1]; 
       if( i + 1 == j - 1 )    // the last const causes a single cache line
	 (*block) += BlockSize;  // A single reference will carry a whole line 
      } 
   }
 
 if ( (*num_gap) > 0 )
   {
     gap = new int[*num_gap];
     for ( i = 0; i < *num_gap; i++)
 	    gap[i] = def_gap[i];
   }
*****/
}


int DimDiffOnly(la_vect v1, la_vect v2, int dim, int subs)
{
 int i;
 int cord = True;
 
 for( i = 1; i < subs && cord; ++ i)
    if( i!= dim )
	if( v1[i] != v2[i] ) 
		cord = False;

 return cord; 
}
