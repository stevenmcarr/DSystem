/* $Id: label.C,v 1.2 1992/10/03 15:50:35 rn Exp $ */
#include <sr.h>
#include <malloc.h>
#include <label.h>

char *ut_symtab_get_label_str(SymDescriptor symtab)

  {
   static int label = 10000;
   fst_index_t index;
   char *sval;

   sval = (char *)malloc(5*sizeof(char));
   do
     {
      sprintf(sval,"%d",label++);
      index = fst_QueryIndex(symtab,sval);
     } while (fst_index_is_valid(index));
   index = fst_Index(symtab,sval);
   fst_PutFieldByIndex(symtab,index,SYMTAB_STORAGE_CLASS,SC_STMT_LABEL);
   return(sval);
  }

static int chk_labels(AST_INDEX     stmt,
		      int           level,
		      SymDescriptor symtab)

  {
   AST_INDEX label,
             new_label,
             label_ref;
   char      *new_label_str;
   fst_index_t index;

     if (is_comment(stmt))
       return(WALK_CONTINUE);
     if ((label = gen_get_label(stmt)) != AST_NIL)
       {
	new_label_str = ut_symtab_get_label_str(symtab);
	new_label = pt_gen_label_def(new_label_str);
	index = fst_QueryIndex(symtab,new_label_str);
	set_label_sym_index(new_label,index);
	fst_PutFieldByIndex(symtab,index,LBL_STMT,stmt);
	fst_PutField(symtab,gen_get_text(label),NEW_LBL_INDEX,index);
	fst_PutFieldByIndex(symtab,index,REFS,
			    (int)fst_GetField(symtab,gen_get_text(label),
					      REFS));
	pt_tree_replace(label,new_label);
       }
     if (is_do(stmt))
       if ((label_ref = gen_DO_get_lbl_ref(stmt)) != AST_NIL)
	 pt_tree_replace(label_ref,pt_gen_label_ref((char *)
                         fst_GetFieldByIndex(symtab,(fst_index_t)
                             fst_GetField(symtab,gen_get_text(label_ref),
					  NEW_LBL_INDEX),SYMTAB_NAME)));
       else;
     else if (is_goto(stmt))
       {
	label_ref = gen_GOTO_get_lbl_ref(stmt);
	pt_tree_replace(label_ref,pt_gen_label_ref((char *)
                        fst_GetFieldByIndex(symtab,(fst_index_t)
                            fst_GetField(symtab,gen_get_text(label_ref),
					 NEW_LBL_INDEX),SYMTAB_NAME)));
       }
     else if (is_arithmetic_if(stmt))
       {
	label_ref = gen_ARITHMETIC_IF_get_lbl_ref1(stmt);
	pt_tree_replace(label_ref,pt_gen_label_ref((char *)
                        fst_GetFieldByIndex(symtab,(fst_index_t)
                            fst_GetField(symtab,gen_get_text(label_ref),
					 NEW_LBL_INDEX),SYMTAB_NAME)));
	label_ref = gen_ARITHMETIC_IF_get_lbl_ref2(stmt);
	pt_tree_replace(label_ref,pt_gen_label_ref((char *)
                        fst_GetFieldByIndex(symtab,(fst_index_t)
                            fst_GetField(symtab,gen_get_text(label_ref),
					 NEW_LBL_INDEX),SYMTAB_NAME)));
	label_ref = gen_ARITHMETIC_IF_get_lbl_ref3(stmt);
	pt_tree_replace(label_ref,pt_gen_label_ref((char *)
                        fst_GetFieldByIndex(symtab,(fst_index_t)
                            fst_GetField(symtab,gen_get_text(label_ref),
					 NEW_LBL_INDEX),SYMTAB_NAME)));
       }
     else if (is_computed_goto(stmt))
       for (label_ref = list_first(gen_COMPUTED_GOTO_get_lbl_ref_LIST(stmt));
	    label_ref != AST_NIL;
	    label_ref = list_next(label_ref))
	 pt_tree_replace(label_ref,pt_gen_label_ref((char *)
                         fst_GetFieldByIndex(symtab,(fst_index_t)
                             fst_GetField(symtab,gen_get_text(label_ref),
					  NEW_LBL_INDEX),SYMTAB_NAME)));
     return(WALK_CONTINUE);
  }

void ut_update_labels(AST_INDEX     stmt,
		      SymDescriptor symtab)

  {

   /*
     No backward control flow allowed, so walking statements in reverse
     will guarantee that label defs are seen before label refs.
   */

   walk_statements_reverse(stmt,LEVEL1,NOFUNC,chk_labels,(Generic)symtab);
  }
