/* $Id: moderate.C,v 1.4 1992/10/03 15:48:50 rn Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <sr.h>
#include <Arena.h>
#include <name.h>
#include <moderate.h>
#include <table.h>
#include <fort/gi.h>
#include <malloc.h>

/*

  not usable with changes to make all LIAV gens be allocated

static void do_knapsack(glist,allocate,num_gens,free_regs,ped)

  UtilList *glist;
  Set      allocate;
  int      num_gens,
           *free_regs;
  PedInfo  ped;

  {
   reg_element    *reg_data;
   int            i,j,max_pos = 0;
   int            max_value = 0;
   int            cost;
   UtilNode       *node;
   name_node_type *name_node;

     reg_data = (reg_element *)malloc((*free_regs+1)*sizeof(reg_element));
     reg_data[0].value = 0;
     reg_data[0].in_pack = ut_create_set(num_gens);
     for (i = 1; i <= *free_regs; i++)
       {
	reg_data[i].value = 0;
	reg_data[i].in_pack = ut_create_set(num_gens);
	for (node = UTIL_HEAD(glist), j = 0;
	     node != NULLNODE;
	     node = UTIL_NEXT(node), j++)
	  {
	   name_node = (name_node_type *)UTIL_NODE_ATOM(node);
	   if (gen_get_converted_type(name_node->gen) == TYPE_REAL)
	     cost = name_node->cost;
	   else if (gen_get_converted_type(name_node->gen) == 
		    TYPE_DOUBLE_PRECISION)
	     cost = name_node->cost *
	            ((config_type *)PED_MH_CONFIG(ped))->double_regs;
	   else if (gen_get_converted_type(name_node->gen) == TYPE_COMPLEX)
	     cost = name_node->cost << 1;
	   else 
	     cost = 0;
	   if (cost <= i)
	     if (name_node->benefit + reg_data[i-cost].value >
		 reg_data[i].value && 
		 !ut_member_number(reg_data[i-cost].in_pack,j))
	       {
		reg_data[i].value = name_node->benefit + 
		                    reg_data[i-cost].value;
		ut_copy12(reg_data[i].in_pack,reg_data[i-cost].in_pack);
		ut_add_number(reg_data[i].in_pack,j);
		if (reg_data[i].value > max_value)
		  {
		   max_value = reg_data[i].value;
		   max_pos = i;
		  }
	       }
	  }
       }
   ut_copy12(allocate,reg_data[max_pos].in_pack);
   for (i = 0; i <= *free_regs; i++)
     free((char *)reg_data[i].in_pack);
   free((char *)reg_data);
   (*free_regs) -= max_pos;
  }
*/

static void heapify(heap_type *heap,
		    int       i,
		    int       j,
		    int       n)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int k,i1,i2,
       temp_index;
   name_node_type *temp_name;
   UtilNode       *temp_lnode;

     if (i < ((j+1) >> 1))
       {
	i1 = (i << 1) + 1;
	i2 = (i << 1) + 2;
	if (i2 > n)
	  i2 = i1;
	if (heap[i1].name->ratio > heap[i].name->ratio || 
	    heap[i2].name->ratio > heap[i].name->ratio)
	  {
	   if (heap[i1].name->ratio > heap[i2].name->ratio)
	     k = i1;
	   else
	     k = i2;
	   temp_index = heap[i].index;
	   temp_name = heap[i].name;
	   temp_lnode = heap[i].lnode;
	   heap[i].index = heap[k].index;
	   heap[i].name = heap[k].name;
	   heap[i].lnode = heap[k].lnode;
	   heap[k].index = temp_index;
	   heap[k].name = temp_name;
	   heap[k].lnode = temp_lnode;
	   heapify(heap,k,j,n);
	  }
       }
  }


static void do_greedy(UtilList *glist,
		      heap_type *heap,
		      Set      allocate,
		      Set      opt_allocate,
		      int      num_gens,
		      int      *free_regs)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


  {
   name_node_type *temp_name;
   UtilNode       *node,
                  *temp_lnode;
   int            i,temp_index;

     for (node = UTIL_HEAD(glist), i = 0;
	  node != NULLNODE;
	  node = UTIL_NEXT(node), i++)
       {
	heap[i].index = i;
	heap[i].name = (name_node_type *)UTIL_NODE_ATOM(node);
	heap[i].lnode = node;
	if (heap[i].name->opt_will_allocate)
	  ut_add_number(opt_allocate,i);
       }
     for (i = (num_gens-1) >> 1; i >= 0; i--)
       heapify(heap,i,num_gens-1,num_gens-1);
     for (i = num_gens-1;i > 0 && *free_regs > 0; i--)
       {
	if (heap[0].name->cost <= *free_regs)
	  {
	   ut_add_number(allocate,heap[0].index);
	   (*free_regs) -= heap[0].name->cost;
	  }
	temp_index = heap[i].index;
	temp_name = heap[i].name;
	temp_lnode = heap[i].lnode;
	heap[i].index = heap[0].index;
	heap[i].name = heap[0].name;
	heap[i].lnode = heap[0].lnode;
	heap[0].index = temp_index;
	heap[0].name = temp_name;
	heap[0].lnode = temp_lnode;
	heapify(heap,0,i-1,i-1);
       }
     if (i == 0 && *free_regs > 0)
       {
	if(heap[0].name->cost <= *free_regs)
	  {
	   ut_add_number(allocate,heap[i].index);
	   (*free_regs) -= heap[0].name->cost;
	  }
       }
  }

static void deallocate(name_node_type *nptr)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   UtilNode         *node;
   scalar_info_type *sptr;

     if (nptr == NULL) return;
     for (node = util_pop(nptr->nlist);
	  node != NULLNODE;
	  node = util_pop(nptr->nlist))
       {
	sptr = get_scalar_info_ptr((AST_INDEX)UTIL_NODE_ATOM(node));
	sptr->is_generator = false;
	sptr->generator = -1;
	util_free_node(node);
       }
     util_list_free(nptr->nlist);
  }

static void do_partial_allocation(UtilNode       *lnode,
				  int            *free_regs)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   name_node_type    *nptr;
   UtilNode         *node,
                    *next_node;
   scalar_info_type *sptr;
   int              num_regs = 0;

     nptr = (name_node_type *)UTIL_NODE_ATOM(lnode);
     for (node = UTIL_HEAD(nptr->nlist);
	  node != NULLNODE;
	  node = next_node)
       {
	next_node = UTIL_NEXT(node);
	sptr = get_scalar_info_ptr((AST_INDEX)UTIL_NODE_ATOM(node));
	if (!sptr->is_generator)
	  if (sptr->gen_distance >= *free_regs || sptr->gen_type == LCPAV)
	    {
	     sptr->generator = -1;
	     util_pluck(node);
	     util_free_node(node);
	    }
	  else
	    if (sptr->gen_distance >= num_regs)
	      num_regs = sptr->gen_distance + 1;
       }
     if (num_regs == 0)
       {
	deallocate(nptr);
	util_pluck(lnode);
	util_free_node(lnode);
       }
     else
       {
	for (node = UTIL_HEAD(nptr->nlist);
	     node != NULLNODE;
	     node = UTIL_NEXT(node))
	  get_scalar_info_ptr((AST_INDEX)UTIL_NODE_ATOM(node))->num_regs = 
	                                                            num_regs;
	*free_regs -= num_regs;
       }
  }

static void do_partition(AST_INDEX name,
			 UtilList  *nlist,
			 DG_Edge   *dg,
			 PedInfo   ped)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 

  {
   scalar_info_type *sptr;
   int              refl;
   EDGE_INDEX       edge;

     sptr = get_scalar_info_ptr(name);
     sptr->list_index = util_node_alloc(name,"name"); 
     util_append(nlist,sptr->list_index);
     sptr->visited = true;
     refl = get_info(ped,name,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if ((dg[edge].type == dg_true || dg[edge].type == dg_input) &&
	   dg[edge].level == LOOP_INDEPENDENT)
	 {
	  sptr = get_scalar_info_ptr(dg[edge].sink);
	  if(!sptr->visited)
	    do_partition(dg[edge].sink,nlist,dg,ped);
	 }
     for (edge = dg_first_sink_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))
       if ((dg[edge].type == dg_true || dg[edge].type == dg_input) &&
	   dg[edge].level == LOOP_INDEPENDENT)
	 {
	  sptr = get_scalar_info_ptr(dg[edge].src);
	  if(!sptr->visited)
	    do_partition(dg[edge].src,nlist,dg,ped);
	 }
  }

static void find_LI_generator(UtilNode *lnode,
			      DG_Edge  *dg,
			      PedInfo  ped)

  {
   UtilNode         *node;
   AST_INDEX        astnode;
   scalar_info_type *sptr;
   Boolean          is_generator;
   name_node_type   *name_node;
   int              vector;
   EDGE_INDEX       edge;

     name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
     name_node->opt_will_allocate = true;
     for (node = UTIL_HEAD(name_node->nlist);
	  node != NULLNODE;
	  node = UTIL_NEXT(node))
       {
	astnode = (AST_INDEX)UTIL_NODE_ATOM(node);
	sptr = get_scalar_info_ptr(astnode);
	sptr->gen_distance = 0;
	if (sptr->generator != -1)
	  {
	   vector = get_info(ped,astnode,type_levelv);
	   is_generator = true;
	   for (edge = dg_first_sink_ref( PED_DG(ped),vector);
		edge != END_OF_LIST && is_generator;
		edge = dg_next_sink_ref( PED_DG(ped),edge))
	     if (dg[edge].level == LOOP_INDEPENDENT)
	       if (dg[edge].type == dg_true || dg[edge].type == dg_input)
	         is_generator = false;
	   if (!is_generator)
	     sptr->is_generator = false;
	   else
	     {
	      sptr->num_regs = 1;
	      sptr->is_generator = true;
	      sptr->generator = -1;
	      name_node->gen = astnode;
	     }

	  }
	else 
	  {
	   sptr->num_regs = 1;
	   sptr->is_generator = true;
	   name_node->gen = astnode;
	  }
       }
  }


static void partition_names(AST_INDEX  node,
			    PedInfo    ped,
			    UtilList   *glist,
			    DG_Edge    *dg,
			    int        *free_regs,
			    arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   name_node_type   *name_node;
   UtilNode         *lnode;

     name_node = (name_node_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
						 sizeof(name_node_type));
     lnode = util_node_alloc((Generic)name_node,"generator");
     util_append(glist,lnode);
     name_node->nlist = util_list_alloc((Generic)NULL,"name-list");
     do_partition(node,name_node->nlist,dg,ped);
     find_LI_generator(lnode,dg,ped);
     if (gen_get_converted_type(name_node->gen) == TYPE_REAL)
       (*free_regs)--;
     else if (gen_get_converted_type(name_node->gen) == 
	      TYPE_DOUBLE_PRECISION)
       (*free_regs) -= ((config_type *)PED_MH_CONFIG(ped))->double_regs;
     else if (gen_get_converted_type(name_node->gen) == TYPE_COMPLEX)
       (*free_regs) -= 2;
  }
 

static void do_opt_allocation(UtilList *glist,
			      UtilNode *lnode,
			      int      *free_regs,
			      PedInfo  ped,
			      arena_type *ar)

  {
   scalar_info_type *sptr;
   UtilNode         *node,*next_node;
   AST_INDEX        astnode;
   DG_Edge          *dg;
   Boolean          still_gen = false;

     for (node = UTIL_HEAD(((name_node_type *)UTIL_NODE_ATOM(lnode))->nlist);
	  node != NULLNODE;
	  node = UTIL_NEXT(node))
       get_scalar_info_ptr((AST_INDEX)UTIL_NODE_ATOM(node))->visited = false;
     dg = dg_get_edge_structure( PED_DG(ped));
     for (node = UTIL_HEAD(((name_node_type *)UTIL_NODE_ATOM(lnode))->nlist);
	  node != NULLNODE;
	  node = UTIL_NEXT(node))
       {
	astnode = (AST_INDEX)UTIL_NODE_ATOM(node);
	sptr = get_scalar_info_ptr(astnode);
	if (!sptr->visited &&sptr->gen_type == LIAV)
	  partition_names(astnode,ped,glist,dg,free_regs,ar);
       }
     for (node = UTIL_HEAD(((name_node_type *)UTIL_NODE_ATOM(lnode))->nlist);
	  node != NULLNODE;
	  node = next_node)
       {
	next_node = UTIL_NEXT(node);
	sptr = get_scalar_info_ptr((AST_INDEX)UTIL_NODE_ATOM(node));
	if (sptr->visited)
	  {
	   util_pluck(node);
	   util_free_node(node);
	  }
	else if (sptr->gen_type != BOGUS)
	  if (sptr->gen_type != LCPAV)
	    still_gen = true;
	  else
	    {
	     sptr->generator = -1;
	     util_pluck(node);
	     util_free_node(node);
	    }
       }
     if (still_gen)
       sr_find_generator(lnode,dg,ped);
     else
       {
        deallocate((name_node_type *)UTIL_NODE_ATOM(lnode));
	UTIL_NODE_ATOM(lnode) = (Generic)NULL;
       }
  }

static void fix_allocation(heap_type *heap,
			   int       size,
			   Set       allocate,
			   Set       opt_allocate,
			   int       *free_regs,
			   UtilList  *glist,
			   PedInfo   ped,
			   arena_type *ar)

  {
   int index;

     index = 1;
     do 
       {
	if (ut_member_number(allocate,heap[index].index))
	  {
	   ut_delete_number(allocate,heap[index].index);
	   (*free_regs) += heap[index].name->cost;
	   if (ut_member_number(opt_allocate,heap[index].index))
	     do_opt_allocation(glist,heap[index].lnode,free_regs,ped,ar);
	  }
	index++;
       } while(index < size && *free_regs < 0);
  }
   
       
static void do_allocation(UtilList *glist,
			  Set      allocate,
			  Set      opt_allocate,
			  int      *free_regs,
			  heap_type *heap,
			  int      size,
			  PedInfo  ped,
			  arena_type *ar)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   UtilNode *node,
            *next_node;
   int      i;

     if (((config_type *)PED_MH_CONFIG(ped))->opt_li_alloc)
       {
	for (node = UTIL_HEAD(glist), i = 0;
	     node != NULLNODE && i < size;
	     node = next_node, i++)
	  {
	   next_node = UTIL_NEXT(node);
	   if (ut_member_number(opt_allocate,i) && 
	       !ut_member_number(allocate,i))
	     do_opt_allocation(glist,node,free_regs,ped,ar);
	  }
	if (*free_regs < 0)
	  fix_allocation(heap,size,allocate,opt_allocate,free_regs,glist,ped,
			 ar);
       }
     for (node = UTIL_HEAD(glist), i = 0;
	  node != NULLNODE && i < size;
	  node = next_node, i++)
       {
	next_node = UTIL_NEXT(node);
	if (!ut_member_number(allocate,i))
	  if (*free_regs > 0 && UTIL_NODE_ATOM(node) != (Generic)NULL)
	    do_partial_allocation(node,free_regs);
	  else
	    {
	     deallocate((name_node_type *)UTIL_NODE_ATOM(node));
	     util_pluck(node);
	     util_free_node(node);
	    }
       }
  }

static void complete_temp_names(UtilList *glist,
				Boolean  *redo,
				array_table_type *array_table)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   UtilNode         *node,
                    *lnode;
   scalar_info_type *sptr,
                    *gptr;
   name_node_type   *name_node;

     for (lnode = UTIL_HEAD(glist);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
	gptr = get_scalar_info_ptr(name_node->gen);
	array_table[gptr->table_index].regs = gptr->num_regs;
	for (node = UTIL_HEAD(name_node->nlist);
	     node != NULLNODE;
	     node = UTIL_NEXT(node))
	  {
	   sptr = get_scalar_info_ptr(UTIL_NODE_ATOM(node));
	   sptr->def = gptr->def;
	   if (sptr->generator != -1 && sptr->generator != gptr->table_index)
	     {
	      *redo = true;
	      sptr->generator = gptr->table_index;
	     }
	   sptr->num_regs = gptr->num_regs;
	  }
       }
  }

static int build_cost_info(UtilList *glist,
			   int      *size,
			   int      *regs,
			   PedInfo  ped)

  {
   UtilNode         *node,*lnode;
   int              benefit;
   scalar_info_type *sptr,*gptr;
   name_node_type   *name_node;
   int              total_benefit = 0;

     for (lnode = UTIL_HEAD(glist);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	benefit = 0;
	name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
	for (node = UTIL_HEAD(name_node->nlist);
	     node != NULLNODE;
	     node = UTIL_NEXT(node))
	  {
	   sptr = get_scalar_info_ptr(UTIL_NODE_ATOM(node));
	   if (sptr->generator != -1)
	     {
	      total_benefit++;
	      if (sptr->gen_type == LIAV || sptr->gen_type == LCAV)
	        benefit += 2;
	      else
	        benefit++;
	     }
	   else if (sptr->no_store)
	     benefit += 2;
	  }
	if (name_node->opt_will_allocate)
	  name_node->benefit = benefit * 100;
	else
	  name_node->benefit = benefit;
	gptr = get_scalar_info_ptr(name_node->gen);
	if (gptr->recurrence || gptr->scalar)
	  {
	   gptr->num_regs = 1;
	   total_benefit++;
	  }
	if (!gptr->no_store)
	  if (gen_get_converted_type(name_node->gen) == TYPE_REAL)
	    {
	     name_node->cost = gptr->num_regs;
	     name_node->ratio =(((float) name_node->benefit) /
				((float) name_node->cost) * 100.0);
	    }
	  else if (gen_get_converted_type(name_node->gen) == 
		   TYPE_DOUBLE_PRECISION)
	    {
	     name_node->cost = (gptr->num_regs * 
	                 ((config_type *)PED_MH_CONFIG(ped))->double_regs);
	     name_node->ratio =(((float) name_node->benefit) /
				((float) name_node->cost) * 100.0);
	    }
	  else if (gen_get_converted_type(name_node->gen) == TYPE_COMPLEX)
	    {
	     name_node->cost = (gptr->num_regs << 1);
	     name_node->ratio =(((float) name_node->benefit) /
				((float) name_node->cost) * 100.0);
	    }
	  else
	    {
	     name_node->cost = 0;
	     name_node->ratio = 1000.0;
	    }
	else
	  {
	   name_node->cost = 0;
	   name_node->ratio = 1000.0;
	   total_benefit++;
	  }
	(*regs) += name_node->cost;
	(*size)++;
       }
     return(total_benefit);
  }


void sr_moderate_pressure(PedInfo  ped,
			  UtilList *glist,
			  int      free_regs,
			  Boolean  *redo,
			  array_table_type *array_table,
			  FILE     *logfile,
			  arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int      size = 0,
            regs = 0,
            loads;
   Set      allocate,
            opt_allocate;
   heap_type *heap;

     loads = build_cost_info(glist,&size,&regs,ped);
     if (logfile != NULL)
       {
	fprintf(logfile,"possible references removed = %d\n",loads);
	fprintf(logfile,"register pressure = %d\n",regs);
	fprintf(logfile,"free registers = %d\n",free_regs);
       }
     if (regs > free_regs)
       {
	if (logfile != NULL)
	  fprintf(logfile,"need to spill registers\n");
	allocate = ut_create_set(ar,LOOP_ARENA,size);
	opt_allocate = ut_create_set(ar,LOOP_ARENA,size);
	heap = (heap_type *)ar->arena_alloc_mem(LOOP_ARENA,
						size*sizeof(heap_type));
	do_greedy(glist,heap,allocate,opt_allocate,size,&free_regs);
	regs = free_regs;
        do_allocation(glist,allocate,opt_allocate,&free_regs,heap,size,ped,ar);
	if (logfile != NULL)
	  {
	   loads = build_cost_info(glist,&size,&regs,ped);
	   fprintf(logfile,"references removed = %d\n",loads);
	   fprintf(logfile,"registers used = %d\n",regs-free_regs);
	  }
	/* free((char *)heap);
	   free((char *)allocate);
	   free((char *)opt_allocate); */
       }
   *redo = false;
   complete_temp_names(glist,redo,array_table);
  }
