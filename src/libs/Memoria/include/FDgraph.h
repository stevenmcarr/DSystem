/************************************************************************/
/*									*/
/*	ped_cp/pt/FDgraph.h						*/
/*             	Data structures for managing Fusion & Distribution	*/
/*		author: Kathryn S. McKinley				*/
/*									*/
/*	These data structures were designed to perform Fusion on two	*/
/*	or more loops and distribution on a loop, for uniprocessors and	*/
/*	for parallel code generation in a unified manner.		*/
/*									*/
/*	The algorithms that are implemented are typed fusion from Rice  */ 
/*      TR94-221 for a variety of headers and the greedy and simple	*/
/*	algorithms from Kennedy & McKinley "Maximizing Loop Parallelism */
/*	and Improving Reuse via Loop Fusion and Distribution" in	*/
/*	The Sixth Annual Languages and Compilers for Parallelism 	*/
/*	Workshop, Portland, August 1992.  				*/
/*									*/
/*      The graph consists of nodes and edges that mirror dependences  	*/
/*	and statments from the original program.  A node either repre-	*/
/*      sents a loop to be fused or a statement in a loop nest where 	*/
/*	the header (one or more perfectly nested loop) is being distr- 	*/
/*	ibuted.  In distribution, the graph is refined by forming	*/
/*	strongly connected components (sets of statements that cannot 	*/
/*	be separated).  Each statement still has a node after the 	*/
/*	scc are discovered, but the node with the lowest dfn in a scc	*/
/*	is selected to be a header.  The header nodes are stored in  	*/
/*	FD_List->roots which acts as a worklist.  The other nodes in   	*/
/*	the scc are linked in a list via FD_Node->nextNodeInThisSCC.	*/
/*									*/
/*	The edges connecting FD_Nodes are weighted by the amount of	*/
/*	reuse of memory locations they provide.	  Fusion-preventing	*/
/*	edges are marked.						*/
/*									*/
/*	The data structures are all public, since other applications of	*/  
/*	fusion and distribution may want to partition differently, and	*/
/*	therefore can still use the problem setup and code generation  	*/
/*	routines.  User partitioning is only required to set		*/
/*	FD_Node->partition in a manner consistent with the constraints	*/
/*	in the Fusion or Distribution problem set up.			*/
/*									*/
/*	Important User Routines:				        */
/*		fdBuildFusion						*/
/*		fdBuildDistribution					*/
/*		fdDestroyProblem					*/
/*		fdPReuse						*/
/*		fdPParallel						*/
/*		fdPGreedy						*/
/*		fdPSimple						*/
/*		fdPCodeGen						*/
/*									*/
/************************************************************************/

#ifndef FDgraph_h
#define FDgraph_h

#ifndef	stdio_h
#include	<stdio.h>	        /* NULL				*/
#endif

#ifndef	ast_h
#include	<fort/ast.h>		/* AST_INDEX			*/
#endif	
				
#ifndef	general_h
#include	<general.h>	        /* Boolean			*/
#endif	
			
#ifndef	list_h
#include	<misc/list.h>           /* UtilList			*/
#endif

#define FD_ALL 100

/*  Represents a statement or a loop to be fused.
 */ 

typedef struct  FDListNode {
    AST_INDEX   loop; 	           /* Fusing or distributing DO             */
    AST_INDEX   perfect;           /* Innermost loop in a perfect nest for  */
                                   /* Fusion or Distribution                */
    AST_INDEX   stmt;              /* statement or stmtList inside DO       */
    Boolean     parallel;          /* Node can be performed in parallel     */
    int	        dfn;               /* Depth First Number, initially node #  */
    int	        partition;         /* Partition Number, initially dfn       */
    int	        type;              /* classification for typed fusion, or   */
				   /* level of outer perfect loop to        */
				   /* distribute                            */
    int         depth;             /* the depth of header compatibility     */ 
    struct FDListNode *next;       /* singley links all FDNodes             */
                                   /* singley linked list of nodes in a scc */
    struct FDListNode *nextNodeInThisSCC; 
    struct FDListEdge *succ;       /* Doubley linked lists of edges         */
    struct FDListEdge *pred;
} FDNode; 
 
/*  Dependences between FDNodes
 */
typedef struct FDListEdge {
    struct FDListEdge *nextSucc;
    struct FDListEdge *prevSucc;
    struct FDListEdge *nextPred;
    struct FDListEdge *prevPred;
    FDNode        *src;
    FDNode        *sink;
    int            weight;
    Boolean        fp;          /* Fusion-Preventing?                */
    Boolean        directed;    /* true if the nodes must be ordered */

/*    struct FDListEdge *next;     singley links all FDEdges         */
/*    int            dgEdge;       corresponding dg edge in PED_DG   */ 
/*    int            dgPhony;      constructed dg edge not in PED_DG */
} FDEdge;

/*  Contains a fusion or distribution problems and solutions
 */
typedef struct fdProblemGraph {
    FDNode	 *FDnodes;
    FDEdge       *FDedges;
    Boolean       parallel; /* parallelism and reuse or reuse only   */
    int           size;     /* number of nodes (i.e. loops or stmts) */
    int           types;    /* the number of different types of      */
			    /* loops (by header, variable, etc.)     */
    int           depth;    /* Deepest fusion performed              */
    UtilList     *roots;    /* current worklist                      */
}  FDGraph;

/************************************************************************/
/* Provided functions                                                   */
/************************************************************************/

EXTERN(void, fdMake, (PedInfo ped, Boolean fusion));
/* 
 * Dummy routine that calls the next three routines from the menu.
 * Should either become a better dialog with options or removed.
 */

/* Call in the Fusion routines in this order */
EXTERN(FDGraph *, fdBuildFusion,
                      (PedInfo ped, AST_INDEX stmt, Boolean parallel,
		       int depth) );
/* 
 * Returns an instance of the graph for fusion.  Starts looking at stmt 
 * to find a loop cluster (group of adjacent nests).  If parallel is true
 * it marks the loops appropriately and checks dependences appropriately.
 * If depth == FD_ALL, then it tries to fuse at the deepest level at 
 * which it finds type matches.  If some fusions occur at level 2, then 
 * a subsequent call with depth = 1 may find more fusions.  Depth must be
 * greater than 0!!
 */

EXTERN( void,     fdGreedyFusion,
                      (PedInfo ped,  FDGraph *problem, Boolean wantAll,
		       Boolean *all, Boolean *any) );
/* 
 * Performs  greedy fusion on an instance of a fusion problem.  If wantAll 
 * is true, tries to fuse all the loops into one regardless of reuse 
 * between pairs.
 * Sets "any" to true if at least one pair of canidate loops are fused.  
 * Sets "all" to true if all the canidates for fusion are fused into one
 * at some level, not necessarily the depth of the deepest nest.
 */

EXTERN( void,     fdDoFusion, (PedInfo ped, FDGraph  *problem ) );
/*
 * Changes the dependence graph and the AST/program to reflect fusion in 
 * problem.
 */

EXTERN(void, fdDestroyProblem, (FDGraph *problem) );
/*
 * Frees the data structures.
 */



/* NOT YET
EXTERN(FDGraph *, fdBuildDistribution, 
                      (PedInfo ped, AST_INDEX oloop, Boolean parallel) );
*/

#endif
