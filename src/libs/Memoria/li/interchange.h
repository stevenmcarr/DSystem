#ifndef interchange_h
#define interchange_h

#include <Arena.h>

typedef struct {
  int           index;
  SymDescriptor symtab;
 } index_info_type;

typedef struct {
  SymDescriptor symtab;
  PedInfo       ped;
  model_loop    *loop_data;
  UtilList      *loop_list;
 } int_info_type;

typedef struct {
  PedInfo    ped;
  heap_type  *heap;
  model_loop *loop_data;
  int        num_loops;
 } upd_info_type;

typedef struct {
  int level;
  PedInfo ped;
 } level_info_type;

#define INDEX  "mh: index"
#define PRE_VAL  1
#define POST_VAL 2

typedef enum {MISS, IHIT, AHIT,REGISTER} RefType;

#endif
