/* $Id: ftxform.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 			      ftxform.c
 * 									
 *			AST transform utility
 * 									
 *   FUNCTION:
 *	create and modify a transformation object which can then
 *	be applied to a FortTree to transform it
 *
 *   USAGE:
 *
 *	ftx_state s = ftx_create(init_graph_routine);
 *	ftx_register_fn(s, xformfn, fn_name, <dependences ...>);
 *	...
 *	ftx_request_fn(s, xformfn, true/false);
 *	...
 *	ftx_transform(s);
 *	ftx_destroy(s);
 *
 *   AUTHOR:
 *	Robert Hood
 *	    with design consulting from:
 *		Alan Carle
 *		Ben Chase
 *		John Mellor-Crummey
 *
 *   MODIFICATION HISTORY:
 *      June 1992                             John Mellor-Crummey
 *       -- generalize argument passing to use an annotations hashtable 
 *          associated with the graph
 *       -- add support for FOLLOW ordering constraints   
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libs/frontEnd/ast/ftxform.h>

#include <libs/support/tables/cNameValueTable.h>
#include <libs/support/msgHandlers/log_msg.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

struct vertex;

typedef struct edge_t {
  struct vertex* sink;
  struct edge_t* next;
} edge;

typedef enum marktype_enum {
  unmarked,
  marked,
  visiting,
  visited
} marktype;

typedef FUNCTION_POINTER(int, Vertex_functPtr, (ftx_state));

struct vertex {
  Vertex_functPtr  fn;
  char*            name;
  Boolean          requested;
  Boolean          mustdo;
  edge*            must_precede;
  edge*            must_follow;
  edge*            precede;
  marktype         mark;
};

/*------------------------------------------------------------------------*/
/*                     operation graph abstraction                        */
/*------------------------------------------------------------------------*/

struct operation_graph_t {
  Boolean          modified;
  cNameValueTable  map;
};

typedef struct operation_graph_t* operation_graph;

struct ftx_state_t {
  operation_graph graph;
  cNameValueTable annotations;
};

STATIC(operation_graph, operation_graph_create, (void));
STATIC(void, operation_graph_destroy, (operation_graph g));
STATIC(Boolean, operation_graph_modified, (operation_graph g));
STATIC(void, operation_graph_set_modified, (operation_graph g));
STATIC(void, operation_graph_mark_musts, (operation_graph g));
STATIC(void, operation_graph_print, (operation_graph g));
STATIC(struct vertex*, operation_graph_lookup, (operation_graph g, Vertex_functPtr f));
STATIC(void, operation_graph_visit, (operation_graph g, ftx_state s));

STATIC(void, destroy, (edge *el));
STATIC(void, destroyVertex, (PFI f, struct vertex *v));

STATIC(void, resetVertex, (PFI f, struct vertex *v));
STATIC(void, markMust, (PFI f, struct vertex *v));
STATIC(void, mark_list, (edge *list));
STATIC(void, mark, (struct vertex *v));
STATIC(void, visit, (Vertex_functPtr f, struct vertex *v, va_list arg_list));
STATIC(void, visit1, (Vertex_functPtr f, struct vertex *v, ftx_state s));
STATIC(void, register_precede, (ftx_state s, Vertex_functPtr f1, Vertex_functPtr f2));
STATIC(void, register_must_order, 
         (ftx_state s, Vertex_functPtr f1, Vertex_functPtr f2, Boolean precede));
STATIC(Boolean, member, (struct vertex *v, edge *el));
STATIC(void, insert, (struct vertex *v, edge **p_el));
STATIC(void, print_list, (edge *el, char *lname));
STATIC(void, printVertex, (PFI f, struct vertex *v));
STATIC(char*, markString, (marktype mark));
 

static operation_graph operation_graph_create(void)
{
  operation_graph g = (operation_graph)get_mem(sizeof(struct operation_graph_t), 
			                       "ftx_create_graph():operation_graph_t");
  
  assert(g != (operation_graph) 0);
  
  g->modified = true;
  g->map = NameValueTableAlloc(16, NameValueTableIntCompare, NameValueTableIntHash);

  return g;
}


static void operation_graph_destroy(operation_graph g)
{
  /* for each vertex v: delete v and its dependence edges */
  NameValueTableForAllV(g->map, (NameValueTableForAllCallbackV)destroyVertex);
  NameValueTableFree(g->map);
  free_mem((void*)g);

  return;
}


static Boolean operation_graph_modified(operation_graph g)
{
  return g->modified;
}


static void operation_graph_set_modified(operation_graph g)
{
  g->modified = true;

  return;
}


static void operation_graph_mark_musts(operation_graph g)
{
     /* for each vertex v: v->mark = unmarked; v->mustdo = false; */
  NameValueTableForAllV(g->map, (NameValueTableForAllCallbackV)resetVertex);

     /* for each vertex v: if v->requested then DFS from v & mark as mustdo */
  NameValueTableForAllV(g->map, (NameValueTableForAllCallbackV)markMust);

  g->modified = false;

  return;
}


static void operation_graph_print(operation_graph g)
{
     /* for each vertex v: print the vertex */
  NameValueTableForAllV(g->map, (NameValueTableForAllCallbackV)printVertex);

  return;
}


static struct vertex* operation_graph_lookup(operation_graph g, Vertex_functPtr f)
{
  struct vertex		*v, *dummy;
  
  if (NOT(NameValueTableQueryPair(g->map, (Generic)f, (Generic*)&v))) {
    /* allocate a new vertex and fill it up */
    v = (struct vertex*)get_mem(sizeof(struct vertex), "ftx_register_fn()");
    assert(v != (struct vertex*)0);
    
    v->fn            = f;
    v->requested     = false;
    v->mustdo        = false;
    v->must_precede  = (edge*)0;
    v->must_follow   = (edge*)0;
    v->precede       = (edge*)0;
    NameValueTableAddPair(g->map, (Generic)f, (Generic)v, (Generic*)&dummy);
  }

  return v;
}

static void operation_graph_visit(operation_graph g, ftx_state s)
{ 
     /* walk graph in topological order & perform xform if mustdo */ 
  NameValueTableForAllV(g->map, (NameValueTableForAllCallbackV)visit, s);
  
  return;
}

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

typedef FUNCTION_POINTER(void, FtxInitRoutine_functPtr, (ftx_state));

ftx_state ftx_create(FtxInitRoutine_functPtr ftx_init_routine)
{
  ftx_state s;

  s = (ftx_state)get_mem(sizeof(struct ftx_state_t), "ftx_create():ftx_state_t");
  assert (s != (ftx_state)0);

  s->graph = operation_graph_create();
  s->annotations  = NameValueTableAlloc(16, (NameCompareCallback)strcmp, 
                                            (NameHashFunctCallback)hash_string);

  ftx_init_routine(s);

  return s;
}


void ftx_destroy(ftx_state s)
{
  operation_graph_destroy(s->graph);
  NameValueTableFree(s->annotations);
  free_mem((void*)s);

  return;
}

static void destroy(edge *el)
{
  edge *current;
  while (el != (edge *)0) {
    current = el;
    el = el->next;
    free_mem((void*)current);
  }

  return;
}

static void destroyVertex(PFI f, struct vertex *v)
{
  struct vertex	*vf;
  edge *e, *ef;

  destroy(v->must_precede);
  destroy(v->precede);

  destroy(v->must_follow);
  
  free_mem((void*)v);

  return;
}


void ftx_register_fn(ftx_state s, XFORM_FUNCTION xformfn, char* name, ...)
{
  struct vertex		*v;
  va_list		 deps;

  XFORM_FUNCTION	 f;
  Boolean		 precede = true,
                         follow  = false,
			 must    = true;

  va_start(deps, name);
    {
       v       = operation_graph_lookup(s->graph, (Vertex_functPtr)xformfn);
       v->name = name; /* add edges to graph */
       f = va_arg(deps, XFORM_FUNCTION);
       while (f != FTX_END) {
         if (f == FTX_PRECEDE) {
           precede = true;
           follow = false;
           must = false;
         }
         else if (f == FTX_FOLLOW) {
           precede = false;    follow = true;      must = false;
         }
         else if (f == FTX_MUST_NOT) {
           precede = true;     follow = true;      must = false;
         }
         else if (f == FTX_MUST_PRECEDE) {
           precede = true;     follow = false;     must = true;
         }
         else if (f == FTX_MUST_FOLLOW) {
           precede = false;    follow = true;      must = true;
         }
         else {
           if (precede)  register_precede(s, (Vertex_functPtr)xformfn, (Vertex_functPtr)f);
           if (follow)   register_precede(s, (Vertex_functPtr)f, (Vertex_functPtr)xformfn);
           if (must)     register_must_order(s, (Vertex_functPtr)xformfn, 
                                             (Vertex_functPtr)f, precede);
         }
         f = va_arg(deps, XFORM_FUNCTION);
       }
    }
  va_end(deps);

  return;
}


void ftx_request_fn(ftx_state s, Vertex_functPtr xformfn, Boolean doit)
{
  struct vertex	*v;

  v            = operation_graph_lookup(s->graph, xformfn);
  v->requested = doit;

  operation_graph_set_modified(s->graph);
}


Boolean ftx_will_do_fn(ftx_state s, Vertex_functPtr xformfn)
{
  struct vertex	*v;

  v = operation_graph_lookup(s->graph, xformfn);

  if (operation_graph_modified(s->graph))
    operation_graph_mark_musts(s->graph);

  return v->mustdo;
}



int ftx_transform(ftx_state s)
{
  /* always reset the mark and mustdo fields --  JMC 3/93 */
  operation_graph_mark_musts(s->graph);
  operation_graph_visit(s->graph, s);

  return 0;
}


static void resetVertex(PFI f, struct vertex *v)
{
  v->mark   = unmarked;
  v->mustdo = false;
}


static void markMust(PFI f, struct vertex *v)
{
  if (v->requested) mark(v);
}


static void mark_list(edge *list)
{
  while (list != (edge *) 0) {
    mark(list->sink);
    list = list->next;
  }
}

/*
 *	mark mustdo fields of everything linked to v via MUST edges
 */
static void mark(struct vertex *v)
{
  if (v->mark == unmarked) { 
    v->mustdo = true;
    v->mark = marked;
    mark_list(v->must_precede);
    mark_list(v->must_follow);
  }
}


static void visit(Vertex_functPtr f, struct vertex *v, va_list arg_list)
{
  ftx_state s = va_arg(arg_list, ftx_state);
  visit1(f, v,  s);
  va_end(arg_list);
}


static void visit1(Vertex_functPtr f, struct vertex *v, ftx_state s)
{
  edge *e;
  
  if (v->mustdo && v->mark != visited) {
    
    if (v->mark == visiting) {
      fprintf(stderr, "ftxform.c:visit(): circular dependence graph\n");
      exit(-1);
    }
    
    v->mark = visiting;
    
    e = v->precede;
    while (e != (edge *)0) {
      visit1(e->sink->fn, e->sink, s);
      e = e->next;
    }
    
    /* make the call to the transformation function */
    v->fn(s); 
    v->mark = visited;
  }
}


static void register_precede(ftx_state s, Vertex_functPtr f1, Vertex_functPtr f2)
{
  struct vertex	*v1, *v2;

  v1 = operation_graph_lookup(s->graph, f1);
  v2 = operation_graph_lookup(s->graph, f2);

  insert(v2, &v1->precede);
}


static void register_must_order(ftx_state s, Vertex_functPtr f1, 
                                Vertex_functPtr f2, Boolean precede)
{
  struct vertex	*v1, *v2;

  v1 = operation_graph_lookup(s->graph, f1);
  v2 = operation_graph_lookup(s->graph, f2);

  if (precede) insert(v2, &v1->must_precede);
  else insert(v2, &v1->must_follow);
}


static Boolean member(struct vertex *v, edge *el)
{
  edge *e;

  e = el;
  while (e != (edge *) 0) {
    if (e->sink == v)
      return true;

    e = e->next;
  }

  return false;
}


static void insert(struct vertex *v, edge **p_el)
{
  edge *e;

  if (member(v, *p_el))
    return;

  e = (edge *) get_mem(sizeof(edge), "ftxform:insert()");
  assert (e != (edge *)0);

  e->sink = v;
  e->next = *p_el;
  *p_el     = e;
}

static void print_list(edge *el, char *lname)
{
  printf("\t%s: ", lname);
  while (el != (edge *)0) {
    printf(" \"%s\"", el->sink->name);
    el = el->next;
  }
  printf("\n");
}

static void printVertex(PFI f, struct vertex* v)
{
  printf("\n");
  printf("vertex %s 0x%x: fn=0x%x requested=%s mustdo=%s mark=%s\n",
	 v->name, v, f, v->requested ? "true" : "false",
	 v->mustdo ? "true" : "false", markString(v->mark));

  print_list(v->precede, "PRECEDE");
  print_list(v->must_precede, "MUST_PRECEDE");

  print_list(v->must_follow, "MUST_FOLLOW");
}


static char* markString(marktype mark)
{
  switch (mark) {
  case unmarked:
    return "unmarked";
    break;

  case marked:
    return "marked";
    break;

  case visiting:
    return "visiting";
    break;

  case visited:
    return "visited";
    break;

  default:
    return "unknown mark";
    break;
  }
}


void ftx_put_annotation(ftx_state s, char* aname, void* value)
{
  Generic dummy;

  NameValueTableAddPair(s->annotations, (Generic)aname, (Generic)value, (Generic*)&dummy);

  return;
}


Boolean ftx_get_annotation(ftx_state s, char* aname, void* value)
{
  return NameValueTableQueryPair(s->annotations, (Generic)aname, (Generic*)value);
}


void ftx_delete_annotation(ftx_state s, char* aname)
{
  int dummy1, dummy2;

  NameValueTableDeletePair(s->annotations, (Generic)aname, 
                           (Generic*)&dummy1, (Generic*)&dummy2);

  return;
}
