/* $Id: strong.C,v 1.1 1997/06/25 15:10:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	dep/utilities/strong.c						*/
/*									*/
/*	strong.c -- finds strongly connected regions in the dependence	*/
/*		    graph for distribution and scalar expansion.	*/
/*									*/
/*	Includes the following routines:				*/
/*									*/
/*		E X P O R T E D						*/
/*		pt_allocnode						*/
/*		pt_strong_regions					*/
/*		pt_getindex						*/
/*		pt_create_adj_list					*/
/*		pt_destroy_adj_list					*/
/*		pt_add_edge						*/
/*		pt_print_regions					*/
/*		pt_split_loop		unused				*/
/*									*/
/*		N O T    U S E D    E X T E R N A L L Y 		*/
/*		pt_alloclink 						*/
/*		pt_search						*/
/*		pt_push							*/
/*		pt_pop							*/
/*		pt_make_regions						*/
/*									*/
/*		THE FOLLOWING FUNCTION IS NOT USED			*/
/*		pt_check_if_parallel					*/
/*									*/
/*									*/
/************************************************************************/


/*
	Find stronly connected regions of a directed graph to be used
	for loop distribution.

	Input:  pointer to adjacency list of nodes

	Output:	same adjancency list header with a field indicating
		to which region it belongs.
*/

#include <stdio.h>
#include <libs/support/misc/general.h>
#include <libs/moduleAnalysis/dependence/utilities/strong.h>

#include <libs/support/memMgmt/mem.h>
#define START_NODES 512

 
static int pt_stackptr;
static int pt_count;

/*	----------------------------------------------------------	*/
/*		F O R W A R D     D E C L A R A T I O N S		*/
/*	----------------------------------------------------------	*/


STATIC( int, pt_alloclink, ( Adj_List * adj_list ));

STATIC( void, pt_search, ( Adj_List * adj_list, int node ));

STATIC( void, pt_push, ( int node, Adj_Node * node_array ));

STATIC( int, pt_pop, ( int label, Adj_Node * node_array ));

STATIC( void, pt_make_regions, ( Adj_List * adj_list ));


STATIC( Boolean, pt_check_if_parallel,
		( Adj_List * adj_list, AST_INDEX node ) );



/* ====================================================================	*/
/*		E X T E R N A L    F U N C T I O N S			*/
/* ====================================================================	*/


int 
pt_allocnode (Adj_List *adj_list, AST_INDEX statement, AST_INDEX do_node)
{
     int	new_size;
     int        new_index;
     Adj_Node   *new_node;

     new_index = (adj_list -> last_used_node) + 1;
     if (new_index == 0) 
     {
     	 adj_list -> node_array = (Adj_Node *) get_mem(START_NODES * sizeof(Adj_Node), "adj_list node array");
	 adj_list -> nodes_allocated = START_NODES;
     }
     else if (new_index == adj_list -> nodes_allocated)
     {
         new_size = (adj_list -> nodes_allocated) * 2;
     	 adj_list -> node_array = (Adj_Node *) reget_mem((void *)(adj_list -> node_array),
				new_size * sizeof(Adj_Node),
				"adj_list node array");
         adj_list -> nodes_allocated = new_size;
     }
     adj_list -> last_used_node = new_index;
     new_node = &(adj_list -> node_array[new_index]);
     /* Initialise the node                                   */
     new_node -> statement = statement;
     new_node -> marked = false;
     new_node -> first_in_list = NIL;
     new_node -> stacklink = NIL;
     new_node -> instack = false;
     new_node -> do_node = do_node;
     new_node -> loop = NIL;
     return(new_index);
}
  
void
pt_strong_regions(Adj_List *adj_list)
{
     int i;
       
     pt_count    = 0;
     pt_stackptr = NIL;
     for (i = 0;  i <= adj_list -> last_used_node; i++)
     {
        if(adj_list -> node_array[i].marked == false)
           pt_search(adj_list,i);
     }
     pt_make_regions(adj_list);     
}
 
/* pt_getindex returns an index into the node_array
 */
int 
pt_getindex(Adj_List *adj_list, AST_INDEX stmt)
{
    Adj_Node  *node_array;
    int i;
    
    node_array = adj_list -> node_array;
    for (i= 0 ; i <= adj_list -> last_used_node ; i++)
       if (node_array[i].statement == stmt)
          return(i);
    return(NIL);	  
    
}

Adj_List *pt_create_adj_list(void)
{      
   Adj_List  *adj_list;
   
   adj_list = (Adj_List *) get_mem(sizeof(Adj_List), "adj_list");
   adj_list -> last_used_node = -1;
   adj_list -> last_used_link = -1;
   adj_list -> max_region     = -1;
   adj_list -> nodes_allocated = 0;
   adj_list -> links_allocated = 0;
   adj_list -> node_array = NULL;
   adj_list -> link_array = NULL;
   adj_list -> region_array = NULL;
   adj_list -> loop_array   = NULL;
   return(adj_list);
    
}

void
pt_destroy_adj_list(Adj_List *adj_list)
{
    if (adj_list->node_array)
        free_mem((void *)adj_list -> node_array);

    if (adj_list->link_array)
        free_mem((void *)adj_list -> link_array);

    if (adj_list->region_array)
        free_mem((void *)adj_list -> region_array);

    if (adj_list->loop_array)
        free_mem((void *)adj_list -> loop_array);

    free_mem((void *)adj_list);
}

void 
pt_add_edge(Adj_List *adj_list, AST_INDEX from_stmt, AST_INDEX to_stmt, 
            Boolean carried)
{
   int from_index , to_index;
   int link_index;
   Adj_Node  *node;
   Adj_Link  *link;
   
/*   printf("pt_add_edge: from %d to %d carried %d\n", 
			from_stmt, to_stmt, carried); */

   from_index = pt_getindex(adj_list , from_stmt);
   to_index = pt_getindex(adj_list , to_stmt);
   if ((from_index == NIL) || (to_index == NIL))   return;
   /*
    *  Check if the edge already exists;
    */    
    link_index = adj_list -> node_array[from_index].first_in_list;
    while(link_index != NIL)     
    {
   	if (adj_list -> link_array[link_index].node_index == to_index)
	{
	    if (adj_list -> link_array[link_index].carried == false)
	       adj_list -> link_array[link_index].carried = carried;
	    return;
	}
        link_index = adj_list -> link_array[link_index].next_in_list;	
    }
    link_index = pt_alloclink(adj_list);
    node = &(adj_list -> node_array[from_index]);
    link = &(adj_list -> link_array[link_index]);
    link -> next_in_list = node -> first_in_list;
    node -> first_in_list = link_index;
    link -> carried = carried;
    link -> node_index = to_index;
}

#ifdef NOTUSED

char*
pt_print_regions(Generic AL, Generic handle)
{
    int i;
    int j;
    static char  s[20];
    static char  msg[2000];   /* temporary HACK */
    Adj_List *adj_list;
    
    adj_list = (Adj_List *) AL;
    msg[0] = '\0';

    for(i=0; i<= adj_list->max_region; i++)
    {
       sprintf(s, "\nRegion %d", i);
       strcat(msg, s);
       if (adj_list->region_array[i].parallel)
          strcat(msg, (" (parallel):                     \n"));
       else
          strcat(msg, (" (serial):                       \n"));
       for (j = adj_list->region_array[i].first_stmt ; j != NIL ; 
            j = adj_list->node_array[j].rlink) {
          strcat(msg, pt_get_stmt_text(handle, 
				adj_list->node_array[j].statement));
	  strcat(msg, "\n");
	}
    }
    if(strlen(msg)==0) strcpy(msg, "Cannot be distributed.");
    return(msg);
}

#endif

/* ====================================================================	*/
/*			L O C A L    F U N C T I O N S 			*/
/* ====================================================================	*/


static int 
pt_alloclink (Adj_List *adj_list)
{
     int new_index;
     int new_size;
     
     new_index = (adj_list -> last_used_link ) + 1;
     if (new_index == 0) 
     {
     	 adj_list -> link_array = (Adj_Link *) get_mem (START_NODES * sizeof(Adj_Link), "adj_list link array");
	 adj_list -> links_allocated = START_NODES;
     }
     else if (new_index == adj_list -> links_allocated)
     {
         new_size = (adj_list -> links_allocated) * 2;
     	 adj_list -> link_array = (Adj_Link *) reget_mem((void *)(adj_list -> link_array),
					new_size * sizeof(Adj_Link),
					"adj_list link array");
         adj_list -> links_allocated = new_size;
     }
     adj_list -> last_used_link = new_index;
     return(new_index);
}

static void
pt_search(Adj_List *adj_list, int node)
{
     
    int    next;
    int    top;
    int    i;
    
    Adj_Node *node_array;
    Adj_Link *link_array;
    
    link_array = adj_list -> link_array;
    node_array = adj_list -> node_array;
    node_array[node].marked = true;
    node_array[node].dfnumber = pt_count;
    node_array[node].lowlink = pt_count++;
    pt_push(node, node_array);
    for (i = node_array[node].first_in_list; i != NIL; 
	 i = link_array[i].next_in_list)
    {
        next = link_array[i].node_index;
	if (node_array[next].marked == false)
	{
    	   pt_search(adj_list,next);
	   if (node_array[next].lowlink < node_array[node].lowlink)
	      node_array[node].lowlink = node_array[next].lowlink;
	    
	}
	else
	{
	    if ((node_array[next].dfnumber < node_array[node].dfnumber)
	       && (node_array[next].instack == true))
	    {
	    	if (node_array[node].lowlink > node_array[next].dfnumber)
		   node_array[node].lowlink = node_array[next].dfnumber;
	    }
	     
	}
    }
    if (node_array[node].lowlink == node_array[node].dfnumber)
    {
    /*   printf("Start of a strongly connected region\n"); */
    	(adj_list -> max_region)++;
    	do
	{
	   top  = pt_pop(adj_list -> max_region , node_array);
	    
	}
        while(top != node);     
    	
    }
}

static void 
pt_push(int node, Adj_Node *node_array)
{
    node_array[node].instack = true;
    if (pt_stackptr == NIL)    
    {
       node_array[node].stacklink = NIL;	
    }
    else{    
    	node_array[node].stacklink = pt_stackptr;
    }
    pt_stackptr = node;
}
 
static int 
pt_pop(int label, Adj_Node *node_array)
{
/*  int  stmt; */
    int  copy;
     
    if (pt_stackptr == NIL) printf ("Empty stack\n");
    node_array[pt_stackptr].region = label;       
/*  stmt = node_array[pt_stackptr].statement; */
    copy = pt_stackptr;
    pt_stackptr = node_array[pt_stackptr].stacklink;
    node_array[copy].stacklink = NIL;
/*  printf("member:%d\n",stmt); */
    node_array[copy].instack = false;
    return(copy);
}

static void 
pt_make_regions(Adj_List *adj_list)
{
    int no_of_regions;
    Region_Type *region_list;
    Adj_Node    *node_array;
    int 	region;
    int		oldfirst;
    int         i, link;
    int         other_end;
   
    no_of_regions = adj_list -> max_region + 1;
    region_list = (Region_Type * ) get_mem(no_of_regions * sizeof(Region_Type), "Region list");
    /*
     *  Initialise region nodes;
     */     
    for (i= 0 ;i < no_of_regions ; i++)
    {
    	region_list[i].parallel = true;
	region_list[i].first_stmt = NIL;
	region_list[i].visited  = false;
    }
    
    node_array = adj_list -> node_array;
    for(i=0; i<= adj_list -> last_used_node ; i++)
    {     
       region = node_array[i].region;
       
       for(link = node_array[i].first_in_list ; 
           region_list[region].parallel == true && link != NIL;
           link = adj_list->link_array[link].next_in_list)
       {
       	   
	  other_end = adj_list -> link_array[link].node_index;
          if (adj_list -> link_array[link].carried && 
              adj_list -> node_array[other_end].region == region )
              region_list[region].parallel = false;
       }
       oldfirst = region_list[region].first_stmt;
       region_list[region].first_stmt = i;
       node_array[i].rlink = oldfirst;
    }
    adj_list -> region_array = region_list;
}

static Boolean 
pt_check_if_parallel(Adj_List *adj_list, AST_INDEX node)
{
   int i;
   int region;
   
   i = pt_getindex(adj_list,node);
   region = adj_list -> node_array[i].region;
   if (adj_list -> region_array[region].parallel == true )
       return(true);
   else return(false);
}

int 
pt_split_loop(Adj_List *adj_list, AST_INDEX node)
{
     int max_region;
     Region_Type *region_array;
     int boundary , i, j;
     Boolean upper;
     Boolean upper_par = true;
     Boolean lower_par = true;
    
    /*
     *  Scan each strongly connected region to check for conflicts
     */     
     max_region = adj_list -> max_region;
     region_array = adj_list -> region_array;
     boundary = pt_getindex(adj_list,node);
     for(i=0; i<=max_region;i++)
     {
        
        j = region_array[i].first_stmt;
        if (j < boundary)
	   upper = true;
	else upper = false;
	if (region_array[i].parallel != true)
	   if (upper)
	      upper_par = false;
	   else
	      lower_par = false;
	      
        while (j != NIL)
	{
	    if ((j < boundary) != (upper == true))
	       return(0);
	    j = adj_list -> node_array[j].rlink;
	}
     }
     if (upper_par)
       if (lower_par)
	 return(SPLIT_BOTH);
       else
	 return(SPLIT_UPPER_ONLY);
     else if(lower_par)
       return(SPLIT_LOWER_ONLY);
     else
       return(SPLIT_NONE);
     
} /* end_pt_split_loop */

#ifdef NOTUSED
 /*
  *  Dummy read routine to test strongly connected region algorithm 
  */  
  main()
  {
      int from , to;
      int node , routine;
      int rc;
      int index;
      int max;
      int carried;
      Adj_List  *graph;
      Adj_Node  *newnode;
      Adj_Link  *newlink;
      
      graph = pt_create_adj_list();

      printf("Print an arbitrary set of nodes.last one is -1 \n");
      do
      {
         scanf("%d",&index);
	 if (index == -1) break;
         pt_allocnode(graph,index);
	  
      }
      while(1);
      printf("input the list of edges terminated by -1  -1\n");
      do
      {
      	  printf("edge from and edge to and then 1 if its carried else 0\n");
	  scanf("%d %d %d",&from , &to, &carried);
	  if (from == -1) break;
	  add_edge(graph,from,to,carried);
      }
      while(1);
      pt_strong_regions(graph);
      pt_print_regions(graph);
      while(1)
      {
      	  printf("Give node num and routine 0-check 1-split\n");
	  scanf("%d %d",&node,&routine);
	  if (node == -1) break;
   	  if (routine == 0)	  
	     rc = check_if_parallel(graph,node);
	  else rc = split_loop(graph,node);
	  printf ("Return code is %d\n",rc);
	  
      }
      destroy_adj_list(graph);
  }
#endif
