#ifndef scalar_h
#define scalar_h

typedef struct {
  AST_INDEX   node;
  int         def,
              regs;
  float       profit;
  Boolean     partial;
 } array_table_type;

typedef struct {
  int              array_refs,
                   scalar_regs,
                   surrounding_do,
                   def_num,
                   level,
                   stmt_num;
  Boolean          contains_cf,
                   contains_goto,
                   jumps_ok;
  array_table_type *array_table;
  SymDescriptor    symtab;
  PedInfo          ped;
  arena_type       *ar;
}  prelim_info_type;

typedef struct {
  int      inner_do,
           do_num;
  PedInfo  ped;
  Boolean  abort;
  SymDescriptor symtab;
  arena_type *ar;
 } do_info_type;

typedef struct {
  config_type *config;
  int         expr_regs;
 } reg_info_type;

typedef struct {
  int        mem,
             flops;
  PedInfo    ped;
 } bal_info_type;


#define put_label(n,v) \
   ast_put_scratch(n,v)

#define get_label(n) \
   (int)ast_get_scratch(n)

#define is_binary_op(n) \
  (is_binary_plus(n) || is_binary_minus(n) || is_binary_times(n) || \
   is_binary_divide(n) || is_binary_exponent(n))

#endif
