// Estimate.h
// Data Structure Used to Analysis and Estimate Miss Amount

#ifndef Estimate_h
#define Estimate_h

#ifndef la_h
#include la.h
#endif


class CacheBlock
       {
        private:
                int NumGap;
                int TotalBlock;
                int *Gap;
		int Num_Rows;
		class Rows *rows;
        public:
                CacheBlock();
                int Miss( int );  // estimate cache miss according to the unroll amount
		int least_unroll_amount;
		int start;
		int end;
		void TakeRows(class GroupSpatialEntry*);
		void Analysis();
		float Estimate(int, int);
       };        

class SomeRow: 
        public SinglyLinkedListEntry
       {
        public:
                class GroupSpatialEntry *e;
                SomeRow(class GroupSpatialEntry *r)
                {
                 e = r ;
                };
       };

class Rows:
        public SinglyLinkedList
       {
        private:
                 friend class RowsIter;
                 int numbasicrows;
        public:
                Rows(){ numbasicrows = 0; }
                Rows* operator+=(GroupSpatialEntry* e)
                {
                 SomeRow* r = new SomeRow(e);
                 SinglyLinkedList::Append(r);
                 numbasicrows ++;
                 return this;
                };
		int const_array_size();
		void FillArray(int*, int);
		void computeblocks();
                void PrintOut();
       };

class RowsIter:
        public SinglyLinkedListIterator
       {
        public:
                RowsIter( Rows *l):
                SinglyLinkedListIterator(l)
                {
                };

                RowsIter( Rows &l):
                SinglyLinkedListIterator(l)
                {
                };

                SomeRow* operator() ()
                {
                 SomeRow *e =
                    (SomeRow*) SinglyLinkedListIterator::Current();
                 (*this)++;
                 return e;
                };
       };

struct Entry_Info
	{
	 GroupSpatialEntry *e;
	 int leader_const;
	 int Have_Follower;
	 int Have_Ceiling;
	 int Follower;
	 int Dist_fr_Follower;
	 int Dist_to_Ceiling;
	 int Ceiling;
	};

class ComputeBoard
        {
         private:
                Rows *rows;
                int stride;
                int dimension;
                la_matrix coef_mat;  // Coefficient Matrix
                class CacheBlock *block_array;
		GroupSpatialEntry **gse_array;
		struct Entry_Info *entry_info_array;
		int matrix_row;
		int matrix_col;
                int Num_Rows;
		int Num_CacheBlocks;
		int Min_Const;
		int Max_Const;
         public:
                ComputeBoard(){};
                ComputeBoard(int, int );
                void TakeRows( GroupSpatialEntry* );
                int TotalMiss( int );
                void PrintOut();
		void GetReady();
		void GetReady1stD();
		void GetReadyRestD();
		void Process_Info_Array();
		void BuildCoefMatrix();
		int ComputeCoefficient(int, int);
		float Estimate(int, int, int);
        };
#endif
 

