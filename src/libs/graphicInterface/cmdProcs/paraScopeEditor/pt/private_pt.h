/* $Id: private_pt.h,v 1.13 1997/03/11 14:32:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*----------------------------------------------------------------

	private_pt.h		Defines for pt routines

*/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/cd.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/el.h>
#include <libs/frontEnd/include/walk.h>

#define  ERROR 		-1
#define  SCALAR 	0
#define  NOT_SET 	-1
#define  SET		1

#define MAXINT ((unsigned) (-1) >> 1)   /* largest positive signed integer */

/*
 * The following are used to characterize various types of loops for the
 * purpose of loop interchange. For a description of each macro, see the
 * comments at the end of this file.
 */

#define NOT_TRI  0
#define TRI_UL   1
#define TRI_LL   2
#define TRI_UR   3
#define TRI_LR   4
#define TRI_ULD  5
#define TRI_LLD  6
#define TRI_URD  7
#define TRI_LRD  8

#define NOT_TRAP  0
#define TRAP_UL   1
#define TRAP_LL   2
#define TRAP_UR   3
#define TRAP_LR   4

#define NOT_PENT  0
#define PENT_UL   1
#define PENT_LL   2
#define PENT_UR   3
#define PENT_LR   4

#define NOT_SKEW  0
#define SKEW_1    1
#define SKEW_2    2

#define NOT_HEX   0
#define HEX_1_MUL 1
#define HEX_1_DIV 2
#define HEX_2_MUL 3
#define HEX_2_DIV 4

struct pt_ref_params
{
    int defs_num;		/* number of defs found		*/
    int defs_size;		/* size of def buffer		*/
    int subls_num;		/* number of subls found	*/
    int subls_size;		/* size of subls buffer		*/
    int stmts_num;		/* number of stmts found	*/
    int stmts_size;		/* size of stmts buffer		*/
    int uses_num;		/* number of uses found		*/
    int uses_size;		/* size of use buffer		*/
    int iv_num;			/* number of ind vars found	*/
    int iv_size;		/* size of iv buffer		*/
    AST_INDEX *defs;		/* pointer to def buffer	*/
    AST_INDEX *subls;		/* pointer to subls buffer	*/
    AST_INDEX *stmts;		/* pointer to stmts buffer	*/
    AST_INDEX *uses;		/* pointer to use buffer	*/
    AST_INDEX *iv;		/* pointer to iv buffer	 	*/
};


Pt_ref_params *pt_refs();

ControlDep *dstr_graph ();
void delete_data_dep ();


#if 0

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Documentation on loop type constants used for Advanced Loop Interchange.
Written by Matt Brown, July, 1990.

Each type of loop handled by advanced loop interchange is diagammed.
Asterisks on the graph indicate sections of the iteration space that
are indexed during execution.  The DO loop headers on the left give a
sample of how this iteration space might be implemented in a program.
The header after the arrow shows the transformed header (after
interchange).

---------------------------
---  RECTANGULAR LOOPS  ---
---------------------------

      J
  c        d
a-+--------+-->
  |********|  DO i=a,b   ->   DO j=c,d
  |********|    DO j=c,d        DO i=a,b
  |********|
I |********|
  |********|
  |********|
  |********|
  |********|
b-+--------+
  |
  V



--------------------------
---  TRIANGULAR LOOPS  ---
--------------------------

      J
  m        n
m-+--------+-->
  | \      |  Lower left, without diagonal  (TRI_LL)
  |\ \     |  This gets transformed to TRI_UR
  |*\ \    |
I |**\ \   |  DO i=m,n      ->   DO j=m,n
  |***\ \  |    DO j=m,i-1         DO i=j+1,n
  |****\ \ |
  |*****\ \|
  |******\ |
n-+--------+
  |
  V

      J
  m        n
m-+--------+-->
  | \******|  Upper right, without diagonal  (TRI_UR)
  |\ \*****|  This gets transformed to TRI_LL
  | \ \****|
I |  \ \***|  DO i=m,n      ->   DO j=m,n
  |   \ \**|    DO j=i+1,n         DO i=m,j-1
  |    \ \*|
  |     \ \|
  |      \ |
n-+--------+
  |
  V

      J
  m        n
m-+--------+-->
  |*\      |  Lower left, with diagonal  (TRI_LLD)
  |\*\     |  This gets transformed to TRI_URD
  |*\*\    |
I |**\*\   |  DO i=m,n    ->   DO j=m,n
  |***\*\  |    DO j=m,i         DO i=j,n
  |****\*\ |
  |*****\*\|
  |******\*|
n-+--------+
  |
  V

      J
  m        n
m-+--------+-->
  |*\******|  Upper right, with diagonal  (TRI_URD)
  |\*\*****|  This gets transformed to TRI_LLD
  | \*\****|
I |  \*\***|  DO i=m,n    ->   DO j=m,n
  |   \*\**|    DO j=i,n         DO i=m,j
  |    \*\*|
  |     \*\|
  |      \*|
n-+--------+
  |
  V

      J
  m        n
m-+--------+-->
  |******/ |  Upper left, without diagonal  (TRI_UL)
  |*****/ /|  This gets transformed to TRI_UL
  |****/ / |
I |***/ /  |  DO i=m,n          ->   DO j=m,n
  |**/ /   |    DO j=m,n-i+m-1         DO i=m,n-j+m-1
  |*/ /    |
  |/ /     |
  | /      |
n-+--------+
  |
  V

      J
  m        n
m-+--------+-->
  |      / |  Lower right, without diagonal  (TRI_LR)
  |     / /|  This gets transformed to TRI_LR
  |    / /*|
  |   / /**|  DO i=m,n          ->   DO j=m,n
I |  / /***|    DO j=n-i+m+1,n         DO i=n-j+m+1,n
  | / /****|
  |/ /*****|
  | /******|
n-+--------+
  |
  V

      J
  m        n
m-+--------+-->
  |******/*|  Upper left, with diagonal  (TRI_ULD)
  |*****/*/|  This gets transformed to TRI_ULD
  |****/*/ |
I |***/*/  |  DO i=m,n        ->   DO j=m,n
  |**/*/   |    DO j=m,n-i+m         DO i=m,n-j+m
  |*/*/    |
  |/*/     |
  |*/      |
n-+--------+
  |
  V

      J
  m        n
m-+--------+-->
  |      /*|  Lower right, with diagonal  (TRI_LRD)
  |     /*/|  This gets transformed to TRI_LRD
  |    /*/*|
I |   /*/**|  DO i=m,n        ->   DO j=m,n
  |  /*/***|    DO j=n-i+m,n         DO i=n-j+m,n
  | /*/****|
  |/*/*****|
  |*/******|
n-+--------+
  |
  V



---------------------------
---  TRAPEZOIDAL LOOPS  ---
---------------------------

    _c_      J
  a/   \               d
a-+-----+--------------+-->
  |      \*************|      Upper right  (TRAP_UR)
  |       \************|
  |        \***********|      DO i=a,b      ->   DO j=a+c,d
I |         \**********|        DO j=i+c,d         DO i=a,min(b,j-c)
  |          \*********|
  |           \********|
  |            \*******|
b-+-------------+------+
  |
  V

            J 
  a                    d
a-+-------------+------+-->
  |            /*******|      Lower right  (TRAP_LR)
  |           /********|
  |          /*********|      DO i=a,b        ->  DO j=a+c,d
I |         /**********|        DO j=a+b+c-i,d      DO i=max(a,a+b+c-j),b
  |        /***********|
  |       /************|      Let u=a+b+c
  |      /*************|
b-+-----+--------------+      DO i=a,b      ->   DO j=u-b,d
  |\_c_/                        DO j=u-i,d         DO i=max(a,u-j),b
  V

    _c_   _d_       J
  a/   \ /   \
a-+-----+-----+----------->
  |     |******\              Lower left  (TRAP_LL)
  |     |*******\         
  |     |********\            DO i=a,b        -> DO j=a+c,b+c+d
I |     |*********\             DO j=a+c,c+d+i     DO i=max(a,j-c-d),b
  |     |**********\      
  |     |***********\         Let u=a+c   v=c+d
  |     |************\  
b-+-----+-------------+       DO i=a,b      ->   DO j=u,b+v
  |                             DO j=u,i+v         DO i=max(a,j-v),b
  V

    _c_   _d_       J
  a/   \ /   \
a-+-----+-----+-------+--->
  |     |*****|******/        Upper left  (TRAP_UL)
  |     |*****|*****/     
  |     |*****|****/          DO i=a,b             ->  DO j=a+c,b+c+d
I |     |*****|***/             DO j=a+c,a+b+c+d-i       DO i=a,min(b,a+b+c+d-j)
  |     |*****|**/        
  |     |*****|*/             Let u=a+c   v=a+b+c+d
  |     |*****|/        
b-+-----+-----+               DO i=a,b      ->   DO j=u,v-a
  |                             DO j=u,v-i         DO i=a,min(b,v-j)
  V



--------------------------
---  PENTAGONAL LOOPS  ---
--------------------------

    _f_      J
  a/   \   c           e
a-+-----+--+-----------+-->
  |      \ |***********|      Upper right  (PENT_UR)
  |       \|***********|
  |        |***********|      DO i=a,b            -> DO j=a+c,d
I |        |\**********|        DO j=max(c,i+f),e      DO i=a,min(b,j-c)
  |        | \*********|
  |        |  \********|
  |        |   \*******|
b-+--------+----+------+
  |
  V

              J
  a        e           d
a-+--------+----+------+-->
  |        |   /*******|      Lower right  (PENT_LR)
  |        |  /********|
  |        | /*********|      DO i=a,b                ->  DO j=e,d
I |        |/**********|        DO j=max(e,a+b+c-i),d       DO i=max(a,a+b+c-j),b
  |        |***********|
  |       /|***********|      Let u=a+b+c
  |      / |***********|
b-+-----+--+-----------+      DO i=a,b             ->   DO j=e,d
  |\_c_/                        DO j=max(e,u-i),d         DO i=max(a,u-j),b
  V

           J
  a     c          e
a-+-----+-----+----+------>
  |     |*****|\   |          Lower left  (PENT_LL)
  |     |*****|*\  |      
  |     |*****|**\ |          DO i=a,b           -> DO j=c,e
I |     |*****|***\|            DO j=c,min(e,f+i)     DO i=max(a,j-f),b
  |     |*****|****|      
  |     |*****|****|\        
  |     |*****|****| \   
b-+-----+-----+----+--+     
  |\_____f___/              
  V

             J
  a                e
a-+-----+-----+----+--+--->
  |     |*****|****| /        Upper left  (PENT_UL)
  |     |*****|****|/      
  |     |*****|****|          DO i=a,b                    ->  DO j=a+c,e
I |     |*****|***/|            DO j=a+c,min(e,a+b+c+d-i)       DO i=a,min(b,a+b+c+d-j)
  |     |*****|**/ |      
  |     |*****|*/  |          Let u=a+c   v=a+b+c+d
  |     |*****|/   |    
b-+-----+-----+----+          DO i=a,b             ->   DO j=u,e
  |\_c_/ \_d_/                  DO j=u,min(e,v-i)         DO i=a,min(b,v-j)
  V



----------------------
---  SKEWED LOOPS  ---
----------------------

Note: m=amount of skew (m>0), "/"=integer division

                J
  a     c  d
a-+-----+--+--------------->
  |     ****                  Type 1  (SKEW_1)
  |     |  ****
I |     |     ****            DO i=a,b                 -> DO j=c,d+m(b-a)
  |     |        ****           DO j=c+m(i-a),d+m(i-a)      DO i=max(a,(j-d+m-1)/m+a),
  |     |           ****                                         min(b,(j-c)/m+a)
  |     |              ****   Let u=c-ma   v=d-ma
b-+-----+--------------+---
  |      \__m*(b-a)___/       DO i=a,b         ->  DO j=u+ma,v+mb
  V                             DO j=u+mi,v+mi       DO i=max(a,(j-v+m-1)/m),min(b,(j-u)/m)


                J
  a                 c  d
a-+-----------------+--+--->
  |                 ****      Type 2  (SKEW_2)
  |              ****  |
I |           ****     |      DO i=a,b                 -> DO j=c-m(b-a),d+m(b-a)
  |        ****        |        DO j=c-m(i-a),d+m(i-a)      DO i=max(a,(j-d+m-1)/m+a),
  |     ****           |                                         min(b,(j-c)/m+a)
  |  ****              |      Let u=c-ma   v=d-ma
b-+-----+--------------+   
  |      \__m*(b-a)___/       DO i=a,b         ->  DO j=u+ma,v+mb
  V                             DO j=u+mi,v+mi       DO i=max(a,(j-v+m-1)/m),min(b,(j-u)/m)



-------------------------
---  HEXAGONAL LOOPS  ---
-------------------------

Note: m=n=amount of skew (m,n>0), "/"=integer division

                J
  a    d c f           e
a-+----+-+-+-----------+--->
  |    | ***           |      Type 1, multiply version  (HEX_1_MUL)
  |    | |*****        |
I |    | |   *****     |      DO i=a,b
  |    | |      *****  |        DO j=max(c,d+m(i-a)),min(e,f+m(i-a))
  |    | |         *****
  |    | |            **      Let:  u=d-ma   v=f-ma
b-+----+-+------------++---
  |     \___m*(b-a)__/        DO i=a,b
  V                             DO j=max(c,u+mi),min(e,v+mi)

                                        interchanges to:

                              DO j=c,e
                                DO i=max(a,(j-v+m-1)/m),min(b,(j-u)/m)


                              Type 1, division version  (HEX_1_DIV)

                              DO i=a,b
                                DO j=max(c,d+(i-a)/n),min(e,f+(i-a)/n)

                              Let:  u=dn-a   v=fn-a

                              DO i=a,b
                                DO j=max(c,(i+u)/n),min(e,(i+v)/n)

                                        interchanges to:

                              DO j=c,e
                                DO i=max(a,jn-v),min(b,jn-u+n-1)


                J
  a    c           d e f
a-+----+-----------+-+-+--->
  |    |           *** |      Type 2, multiply version  (HEX_2_MUL)
  |    |        *****| |
I |    |     *****   | |      DO i=a,b
  |    |  *****      | |        DO j=max(c,d-m(i-a)),min(e,f-m(i-a))
  |    *****         | |
  |    **            | |      Let:  u=d+am   v=f+am
b-+----++------------+-+   
  |      \__m*(b-a)___/       DO i=a,b
  V                             DO j=max(c,u-im),min(e,v-im)

                                        interchanges to:

                              DO j=c,e
                                DO i=max(a,(u-j+m-1)/m),min(b,(v-j)/m)


                              Type 2, division version  (HEX_2_DIV)

                              DO i=a,b
                                DO j=max(c,d-(i-a+n-1)/n),min(e,f-(i-a)/n)

                              Let:  u=a+dn+n-1   v=a+fn

                              DO i=a,b
                                DO j=max(c,(u-i)/n),min(e,(v-i)/n)

                                        interchanges to:

                              DO j=c,e
                                DO i=max(a,u-jn-n+1),min(b,v-jn)


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
