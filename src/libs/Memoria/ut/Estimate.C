/* $Id: Estimate.C,v 1.3 1997/04/07 13:40:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

// Estimate.C
// C++ code
// Estimate the cache miss per iteration

#include <iostream.h>
#include <libs/Memoria/include/la.h>


#ifndef Estimate_h
#include <libs/Memoria/include/Estimate.h>
#endif

extern "C"{
#include <math.h>
};


static int intcompare(const void *i, const void *j)
{
 return ( *(int*)i -  *(int*)j );
}

ComputeBoard::ComputeBoard(int n, int d)
{
 Initialize(n,d);
}

void ComputeBoard::Initialize(int n, int d)
{
 stride = n;
 dimension = d;
 rows = new Rows();
 Num_Rows = 0;
 Num_CacheBlocks = 0;
 Min_Const = Max_Const = 0;
 matrix_row = matrix_col = 0;
}

void ComputeBoard::TakeRows( GroupSpatialEntry* e)
{
  (*rows)+= e;
  int constant = e->Leader()[dimension];
  if( constant < Min_Const )
  	Min_Const = constant;
  else if ( constant > Max_Const )
	Max_Const = constant; 
  Num_Rows ++;
}

void ComputeBoard::PrintOut()
{
 cout <<"\tDimension is : " << dimension << endl;
 cout <<"\tStride is : " << stride << endl;

 rows->PrintOut();
}
	

void ComputeBoard::GetReady()
{

 if( dimension == 0 ) // the first computeboard contains GS sets not
	              // very strongly connected. 
    GetReady1stD();
 else
   {
    if ( Num_Rows >0 ) GetReadyRestD();
   }
}

void ComputeBoard::GetReady1stD()
{
 SomeRow *r;
 int i, j;

 block_array = new CacheBlock[Num_Rows];
 i = 0;
 for( RowsIter rowsiter(rows);
         r = rowsiter();)
    {
     block_array[i].TakeRows(r->e);
     block_array[i].Analysis();
     i ++ ;
    }
 if( i != Num_Rows ) cerr<< "Wrong in ComputeBoard # 0 !!" << endl;
 Num_CacheBlocks = Num_Rows;
}

void ComputeBoard::GetReadyRestD()
{
 SomeRow *r;
 GroupSpatialEntry **gse_array;
 struct Entry_Info *tmp_array;
 int i, j, k, m, n;

    int array_size = abs(Max_Const - Min_Const + 1);
    gse_array = (GroupSpatialEntry**)malloc(array_size*sizeof(GroupSpatialEntry*)); 
    for( i = 0; i<array_size; ++ i)
         gse_array[i] = NULL;

    for( RowsIter rowsiter(rows);
	 r = rowsiter(); )
       {
	 i = r->e->Leader()[dimension]; 
         gse_array[i - Min_Const ] = r->e;  
       }
    
    entry_info_array = new Entry_Info[Num_Rows];
    j = 0;
    for( i=0; i<array_size; ++i)
	{
	 if(gse_array[i] != NULL)
	   {
	    entry_info_array[j].e = gse_array[i];
	    entry_info_array[j].leader_const = gse_array[i]->Leader()[dimension]; 
	    entry_info_array[j].Have_Follower = False;
	    entry_info_array[j].Have_Ceiling = False;
	    entry_info_array[j].Follower = 0;
	    entry_info_array[j].Ceiling =0;
	    entry_info_array[j].Dist_fr_Follower = 0;
	    entry_info_array[j].Dist_to_Ceiling = 0;
            j ++;
	   }
	}
     
     free(gse_array);

     if( j != Num_Rows ) cerr << "Wrong in Initialize entry_info_array!" << endl;
    if( stride < 0 ) // We need flip the entry_info_array 
     	{
    	 tmp_array = new Entry_Info[Num_Rows];
	 for(i = Num_Rows - 1; i>= 0; i--)
             tmp_array[Num_Rows - i - 1] = entry_info_array[i]; 
         entry_info_array = tmp_array;
	} 

  // cout<<" Basic Leader Const: " ;
  //  for(i=0; i<Num_Rows; ++i)
  //   cout<<" " << entry_info_array[i].leader_const;
  // cout << endl;
	 
  if(stride != 0) Process_Info_Array();

  if(stride != 0)
    matrix_row = (int)ceil((double)(Max_Const - Min_Const)/(double)stride);
  else matrix_row = 1;

  if(matrix_row == 0) matrix_row = 1;

  int c1, c2, ua, amount;
  CacheBlock *init_cacheblock;
  int newnum = 0;  
  for( i = 1; i <= Num_Rows; i++) 
     newnum +=i;
  init_cacheblock = new CacheBlock[newnum];

  int need_newcacheblock;
  matrix_col = 0; 
     for( i=0; i<Num_Rows; ++i )
        init_cacheblock[i].TakeRows(entry_info_array[i].e); 
     matrix_col += Num_Rows;

  if(stride != 0)
    for( i=0; i<Num_Rows; ++i)
       {
	need_newcacheblock = False;
        c1 = entry_info_array[i].leader_const;
          for( k=i; k<Num_Rows; ++k ) // Choose Unroll Amount
            {
	     need_newcacheblock = False;
             ua =(int)ceil((double)(entry_info_array[k].leader_const - c1 )/(double)stride); 
	     for(m=1; m<=ua; ++m)           // Simulate Unroll
               {
		c2 = c1 + stride*m;
                for(j = i; j<Num_Rows; ++j)
                   { 
                    if(entry_info_array[j].leader_const == c2  )
	            {
	             need_newcacheblock = True;
	             init_cacheblock[matrix_col].TakeRows(entry_info_array[j].e);
                     init_cacheblock[matrix_col].end = j;
                    }
                   }
              }
              if (need_newcacheblock) 
	        {
	         init_cacheblock[matrix_col].start = i;
                 init_cacheblock[matrix_col].TakeRows(entry_info_array[i].e);
	         init_cacheblock[matrix_col].least_unroll_amount = ua; 
	         matrix_col ++;
	        }
	  }
       }

      Num_CacheBlocks = matrix_col;
      block_array = new CacheBlock[matrix_col];
      for( i=0; i<matrix_col; ++i)
         {
	  block_array[i] = init_cacheblock[i];
	  block_array[i].Analysis();
	 }
      
      free( init_cacheblock );
 

 BuildCoefMatrix();

}

void ComputeBoard::Process_Info_Array()
{
 int i, j, k, Done;
 int c1, c2;
 
 for( i = 0; i < Num_Rows; ++i)
    {
     Done = False;
     c1 = entry_info_array[i].leader_const;
     for( j = i+1; j<Num_Rows && !Done; ++j)
        {
         c2 = entry_info_array[j].leader_const;
         if( abs(c2-c1) % stride == 0 && !entry_info_array[j].Have_Follower)
           {
	    entry_info_array[j].Have_Follower = True;
            entry_info_array[j].Dist_fr_Follower = abs(c2-c1)/stride;
	    entry_info_array[j].Follower = c1;
	    entry_info_array[i].Have_Ceiling = True;
            entry_info_array[i].Dist_to_Ceiling = abs(c2-c1)/stride;
	    entry_info_array[i].Ceiling = c2;
            Done = True;
           }
        }	  
    } 
}


void ComputeBoard::BuildCoefMatrix()
{
 int i, j;

 // cout << "\tCoefficient Matrix " <<  endl;
 // cout << " Row = " << matrix_row << " Column = " << matrix_col << endl;

 
 coef_mat = la_matNew(matrix_row, matrix_col);
 for( i = 0; i<matrix_row  ; ++ i)
   {
   //  cout <<endl <<"\t\t" ;
    for ( j = 0; j < matrix_col; ++j)
        {
          coef_mat[i][j] = ComputeCoefficient(i+1, j);
          // cout << coef_mat[i][j] << " "; 
        }
   } 
 cout << endl;
 
}

int ComputeBoard::ComputeCoefficient(int unroll_amount, int cache_block_index)
{
 int index;
 index = cache_block_index;

 if(index < Num_Rows)
   {
    if( !entry_info_array[index].Have_Follower )
      {
       if( entry_info_array[index].Have_Ceiling )
          if( unroll_amount < entry_info_array[index].Dist_to_Ceiling )
		return ( unroll_amount + 1);
          else  return ( entry_info_array[index].Dist_to_Ceiling );
        
       else    // Have no either Follower or Ceiling 
		return ( unroll_amount + 1 );
      }
    else
      {
       if( entry_info_array[index].Have_Ceiling )
          {
           // Have both Follower and Ceiling
	   int  v, w, x, y, z;
	   v = unroll_amount + 1;
	   w = entry_info_array[index].Dist_to_Ceiling;
	   if ( v > w ) v = w; 
           x = entry_info_array[index].Dist_fr_Follower;
           if ( unroll_amount >= x) 
	     y = unroll_amount - x + 1;
           z = v - y;
           if ( z > 0 ) return z; else return 0; 
          }
       else
	  // Have only Follower
          if( unroll_amount < entry_info_array[index].Dist_fr_Follower )
                return ( unroll_amount + 1 );
          else
                return (entry_info_array[index].Dist_fr_Follower);   // Caught by the Follower
      }
   }
 else
   {
    int a, b, c, d, e;

    if( entry_info_array[ block_array[index].start].Have_Follower)
      if( entry_info_array[ block_array[index].end].Have_Ceiling) 
	{
	 a = entry_info_array[block_array[index].start].leader_const + stride*unroll_amount;
	 b = a - entry_info_array[block_array[index].end].leader_const + 1;
         if( b >= entry_info_array[ block_array[index].end].Dist_to_Ceiling) b = entry_info_array[ block_array[index].end].Dist_to_Ceiling;
         else if(b < 0) return 0;

         c = entry_info_array[block_array[index].start].Follower + stride*unroll_amount;
         d = c - entry_info_array[block_array[index].end].leader_const + 1;
         if (d>0) e= b - d; else return b;
	 if(e > 0) return e; else return 0;
	}
      else  // Have only Follower 
	{
	 a = entry_info_array[block_array[index].start].leader_const + stride*unroll_amount;
	 b = a - entry_info_array[block_array[index].end].leader_const + 1 ;
         if(b < 0) return 0;
         c = entry_info_array[block_array[index].start].Follower + stride*unroll_amount;
         d = c - entry_info_array[block_array[index].end].leader_const + 1;
         if (d>0) e= b - d; else return b;

	 if(e > 0) return e; else return 0;
	}
    else  // Have no Follower 	  
      if( entry_info_array[ block_array[index].end].Have_Ceiling) 
	{ // Have only Ceiling

	 a = entry_info_array[block_array[index].start].leader_const + stride*unroll_amount;
	 b = a - entry_info_array[block_array[index].end].leader_const +1;
         if(b<0) return 0;
	 d = entry_info_array[block_array[index].end].Dist_to_Ceiling; 
	 if( b>=d) return d; else return b;
	}
      else // Have no either Follower or Ceiling 
	{
         a = unroll_amount - block_array[index].least_unroll_amount; 
         if( a>=0) return a+1; else return 0;
	}
   }
 
}

float ComputeBoard::Estimate(int IsSP, int spatial_degree, int unroll_amount)
{
 int i, j, coef;
 float prefetch;

 prefetch = 0;

 if (Num_Rows == 0) return 0;

 if(dimension == 0)
  {
   for(i = 0; i<Num_CacheBlocks; ++i)
      {
       prefetch += block_array[i].Estimate(IsSP, spatial_degree);
      }      
   return prefetch;
  }
 else 
  {
   if( unroll_amount == 0) 
     {
      for(j = 0; j<Num_Rows; ++j)
         prefetch += block_array[j].Estimate(IsSP, spatial_degree); 
     }
   else if(unroll_amount <= matrix_row && unroll_amount >0)
     {
      for(j = 0; j<matrix_col; ++j)
        {
         coef = coef_mat[unroll_amount-1][j];
         if(coef != 0)
           prefetch += coef * block_array[j].Estimate(IsSP, spatial_degree); 
        } 
     }
   else
     {
      for( i=0; i<matrix_col; ++i)
         {
          coef = coef_mat[matrix_row-1][i];
          if(coef != 0)
            {    
             if( i < Num_Rows )
               if(!entry_info_array[i].Have_Follower && 
		  !entry_info_array[i].Have_Ceiling)
                 prefetch += (unroll_amount - matrix_row + coef) * 
			        block_array[i].Estimate(IsSP, spatial_degree);
		else
		  prefetch = coef*block_array[i].Estimate(IsSP, spatial_degree);
             else
             {
              int start_index; int end_index;
              start_index = block_array[i].start;
	      end_index = block_array[i].end;
              if(!entry_info_array[start_index].Have_Follower && 
                 !entry_info_array[end_index].Have_Ceiling)
                 prefetch += (unroll_amount - matrix_row + coef)*
				block_array[i].Estimate(IsSP, spatial_degree); 
              else
                 prefetch += coef*block_array[i].Estimate(IsSP, spatial_degree);
             }
            }
          }
        }
    }
 return prefetch;
}

CacheBlock::CacheBlock()
{
 NumGap = TotalBlock = Num_Rows = 0;
 rows = new Rows;
}

void CacheBlock::TakeRows( GroupSpatialEntry* e)
{
  (*rows)+= e;
  Num_Rows ++;
}

void CacheBlock::Analysis()
{
 int i, j, size, num_lines;
 SomeRow *r;
 int *const_array, *def_gap;
 int dist;
 int start, end;

 size = rows->const_array_size(); 
 const_array = new int[size];
 def_gap = new int[size];
 rows->FillArray(const_array, size);

 
 //cout << "The 1st Dim are as: " ;
 //for ( i = 0; i < size; i++ )
 //    cout <<" " << const_array[i];
 //cout << endl; 

 qsort ( const_array, size, sizeof(int), intcompare);

 //cout << "The 1st Dim are sorted as: " ;
 //for ( i = 0; i < size; i++ )
 //    cout <<" " << const_array[i];
 //cout << endl; 

 // Initialize
 NumGap = TotalBlock = 0;

 start = 0;
 for ( i = 0; i < size - 1; i ++ )
   {
     dist = const_array[i+1] - const_array[i];
    if ( dist > BlockSize )
      {
       end = i;
       num_lines = (int)ceil((double)(abs(const_array[end] - const_array[start]))/(double)BlockSize);
       if(num_lines == 0 ) num_lines = 1;
       TotalBlock += num_lines; 
       start = i+1;
       def_gap[NumGap] = dist;
       NumGap ++;
      }
   }
 end = size - 1;
 num_lines = (int)ceil((double)(abs(const_array[end] - const_array[start]))/(double)BlockSize);
 if(num_lines == 0 ) num_lines = 1;
 TotalBlock += num_lines; 


 if ( NumGap > 0 )
   {
     Gap = new int[NumGap];
     for ( i = 0; i < NumGap; i++)
            Gap[i] = def_gap[i];
   }

// cout <<"NumGap = " << NumGap << endl;
// cout << "Gaps are : " ;
// for ( i = 0; i<NumGap; ++i)
//       cout << Gap[i] << " " ;
// cout << endl;
// cout << "Total Block = " << TotalBlock << endl; 
 
}

float CacheBlock::Estimate(int IsSS, int s_degree)
 // IsSS ---- IsSelfSpatial
 // s_degree --- unroll amount causing GS
{
 int i, total_block, block, decrement, increment;
 
 decrement = increment = 0;
 block = (int)ceil((double)s_degree/(double)BlockSize);

 if(IsSS)  // Is Self Spatial
   {
    for( i=0; i<NumGap; ++i)
        if(Gap[i]<s_degree) decrement ++; 
     return((float)(NumGap+1 - decrement)/(float)BlockSize);
   }
 else
   {
    for( i=0; i<NumGap; ++i)
        if(Gap[i] > s_degree) increment += block; 
        else increment += (int)ceil((double)Gap[i]/(double)BlockSize);
    increment += block; // add last block in the end of Cache Row
    
    return( (float)(TotalBlock + increment));
   }
}

void Rows::PrintOut()
{
 SomeRow *r;

 cout <<"\t # 0f Basic Rows : " << numbasicrows << endl;
 for( RowsIter rowsiter(this);
	r = rowsiter();)
     r->e->PrintOut();
}

int Rows::const_array_size()
{
 SomeRow *r;
 int size = 0;
 
   for( RowsIter rowsiter(this);
          r = rowsiter(); ) 
      size += r->e->SizeofGTSet();    
   
  return size;
}

void Rows::FillArray(int* array, int size)
{
 SomeRow *r;
 int start ;

 start = 0; 
  for( RowsIter rowsiter(this);
	  r = rowsiter(); )
   {
    r->e->FillArray(array, start);
    start += r->e->SizeofGTSet();
   }
}
