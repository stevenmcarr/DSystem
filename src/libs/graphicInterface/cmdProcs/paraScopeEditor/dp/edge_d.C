/* $Id: edge_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*											*/
/*	ped_cp/PEditorCP/dp/edge_d.c			*/
/*											*/
/*	Filter for dependences edges in PED		*/
/*											*/
/*											*/
/************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>

Dialog *
  edge_dialog_create (EdgeDia *eh)
{
  Dialog         *di;		       /* the dialog instance   	*/
  
  eh->text_type  = ssave("");
  eh->text_name  = ssave("");
  eh->text_dims  = ssave("");
  eh->text_block = ssave("");

  di = dialog_create(
         "dependence filter facility",
	  edge_handler,
          (dialog_helper_callback) 0,
     	  (Generic) eh,
	  dialog_desc_group(
	     DIALOG_VERT_CENTER,
	     9,
	     dialog_desc_group(
		 DIALOG_HORIZ_CENTER,
		 2,
		 item_button(TEXT_SRC,  "Source reference", DEF_FONT_ID, false),
	         eh->text_src = item_title(UNUSED, "......................", DEF_FONT_ID)),
	    dialog_desc_group(
	      	DIALOG_HORIZ_CENTER,
	    	2,
	      	item_button(TEXT_SINK,  "Sink reference  ", DEF_FONT_ID, false),
		eh->text_sink = item_title(UNUSED, "......................", DEF_FONT_ID)),
	    item_text(TEXT_TYPE, "Dependence type ", DEF_FONT_ID, &eh->text_type, 32),
	    item_text(TEXT_NAME, "Variable name   ", DEF_FONT_ID, &eh->text_name, 32),
	    item_text(TEXT_DIMS, "Dimension of var", DEF_FONT_ID, &eh->text_dims, 32),
	    item_text(TEXT_BLOCK,"Common blk name ", DEF_FONT_ID, &eh->text_block, 32),
            dialog_desc_group(
		DIALOG_HORIZ_CENTER,
		3,
	        item_button (SHOW, 	"show     ", DEF_FONT_ID, false),
	        item_button (HIDE, 	"hide     ", DEF_FONT_ID, false),
	        item_button (DELETE, 	"delete   ", DEF_FONT_ID, false)),
            dialog_desc_group (
		DIALOG_HORIZ_CENTER, 
		4,
		item_button (SHOWALL, 	"show all ", DEF_FONT_ID, false),
		item_button (CLEAR, 	"clear    ", DEF_FONT_ID, false),
	        item_button (PUSH, 	"push     ", DEF_FONT_ID, false),
	        item_button (POP, 	"pop      ", DEF_FONT_ID, false)),
	    dialog_desc_group (
	        DIALOG_HORIZ_CENTER,
		4,
	        item_button (SORT_BY_TYPE,  "sort type", DEF_FONT_ID, false),
	        item_button (SORT_BY_SRC,   "sort src ", DEF_FONT_ID, false),
	        item_button (SORT_BY_SINK,  "sort sink", DEF_FONT_ID, false),
	        item_button (SORT_BY_BLOCK, "sort blk ", DEF_FONT_ID, false))
	    )
	);
    item_title_justify_left(eh->text_src);
    (void) item_title_change(eh->text_src, "");
    item_title_justify_left(eh->text_sink);
    (void) item_title_change(eh->text_sink, "");
    eh->src_ast  = AST_NIL;
    eh->sink_ast = AST_NIL;
    return di;
}

void
edge_dialog_run (EdgeDia *eh)
{
    	dialog_modeless_show(eh->di);
}

/*ARGSUSED*/
/*static*/ Boolean
edge_handler(Dialog *di, Generic EH, Generic item_id)
        /* di: the dialog instance */
    	/* EH: the selection variable */
    	/* item_id: the id of the item */
{
	EdgeDia		*eh;
	EL_Instance 	*el;
	PedInfo  	ped;
	Query		*q;
	Boolean		ok, rc;
	
	eh = (EdgeDia *)EH;
	el = (EL_Instance *)eh->EL;
	ped = (PedInfo)eh->ped;
	
	switch (item_id)
	{
		case DIALOG_CANCEL_ID:
			dialog_modeless_hide (di);
			break;

		case TEXT_SRC:
			if(PED_SELECTION(ped) == NO_SELECTION)
			   break;
			else if(is_identifier(PED_SELECTION(ped))){
			   eh->src_ast = PED_SELECTION(ped);
			   (void) item_title_change(eh->text_src, el_gen_get_text(PED_SELECTION(ped)));
			}
			else if(is_subscript(PED_SELECTION(ped))) {
			   eh->src_ast = gen_SUBSCRIPT_get_name(PED_SELECTION(ped));
			   (void) item_title_change(eh->text_src, el_gen_get_text (eh->src_ast)); 
			}
			break;
			
		case TEXT_SINK:
			if(PED_SELECTION(ped) == NO_SELECTION)
			   break;
			else if (is_identifier(PED_SELECTION(ped))){
			   eh->sink_ast = PED_SELECTION(ped);
			   (void) item_title_change(eh->text_sink, el_gen_get_text(PED_SELECTION(ped)));
			}
			else if (is_subscript(PED_SELECTION(ped))) {
			   eh->sink_ast = gen_SUBSCRIPT_get_name(PED_SELECTION(ped));
			   (void) item_title_change(eh->text_sink, el_gen_get_text (eh->src_ast)); 
			}
			break;
						
		case CLEAR:
			edge_dialog_edge_clear (eh);
			break;

		case HIDE:
			el_hide( PED_EL(eh->ped), PED_DG(eh->ped), eh->text_type, eh->text_name,
				eh->src_ast, eh->sink_ast,
				eh->text_dims, eh->text_block);
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "");		/* sort on active */
			eh->update((Generic)eh->ped);
			break;

		case SHOW:
			el_show( PED_EL(eh->ped), PED_DG(eh->ped), eh->text_type, eh->text_name, 
				eh->src_ast, eh->sink_ast,
				eh->text_dims, eh->text_block);
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "");		/* sort on active */
			eh->update((Generic)eh->ped);
			break;

		case DELETE:
			rc = BOOL(yes_no("Confirm removal of all dependences\nthat match this query:", &ok, false));
			if (ok == false)
			   break;
			el_remove( PED_EL(eh->ped), PED_DG(eh->ped), eh->text_type, eh->text_name, 
				eh->src_ast, eh->sink_ast,
				eh->text_dims, eh->text_block);
			/* call force update */
			forcePedUpdate(eh->ped,PED_SELECTED_LOOP(ped),PED_SELECTION(ped));
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "");		/* sort on active */
			eh->update((Generic)eh->ped);
			break;

		case SHOWALL:
			el_showall(PED_EL(eh->ped));
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "");		/* sort on active */
			eh->update((Generic)eh->ped);
			break;

		case POP:
		        /* pop dep list from stack */
			if(PED_STACK_DEPTH(ped) > 0) {
			   eh->EL = (EL_Instance *)eh->pop((Generic)eh->ped);
	                   el = (EL_Instance *)eh->EL;
			   /* then display its associated query -vas */
#undef	OLD_VERSION
#ifdef	OLD_VERSION
			   q = &(el->query);
			   convert(q->type);
			   edge_dialog_edge_clear (eh);
			   sfree(q->type);
			   sfree(q->name);
			   sfree(q->dims);
			   sfree(q->block);
#else	/* OLD_VERSION */
			   el_query_convert(el);
			   edge_dialog_edge_clear(eh);
			   el_query_free(el);
#endif	/* OLD_VERSION */
			   el_show( PED_EL(eh->ped), PED_DG(eh->ped), eh->text_type, eh->text_name,
				   eh->src_ast, eh->sink_ast,
				   eh->text_dims, eh->text_block);
			   el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "src"); /* sort on active */
			   eh->update((Generic)eh->ped);
			}
			break;

		case PUSH:
			if(PED_STACK_DEPTH(ped) <  MAX_QUERY_STACK_DEPTH) {
		           /* save the current query -vas */
#ifdef	OLD_VERSION
			   q = &(el->query);
		           q->type = ssave(eh->text_type);
		           q->name = ssave(eh->text_name);
			   q->src  = eh->src_ast;
			   q->sink = eh->sink_ast;
			   q->dims = ssave(eh->text_dims);
			   q->block = ssave(eh->text_block);
#else	/* OLD_VERSION */
			   el_query_init(el,
					 ssave(eh->text_type),
					 ssave(eh->text_name),
					 eh->src_ast,
					 eh->sink_ast,
					 ssave(eh->text_dims),
					 ssave(eh->text_block) );
#endif	/* OLD_VERSION */
			   /* then push its dep list on stack */
			   eh->EL = (EL_Instance*)eh->push((Generic)eh->ped);			   
			}
			break;

		case SORT_BY_TYPE:
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "type");
			eh->update((Generic)eh->ped);
			break;

		case SORT_BY_SRC:
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "src");
			eh->update((Generic)eh->ped);
			break;

		case SORT_BY_SINK:
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "sink");
			eh->update((Generic)eh->ped);
			break;

		case SORT_BY_BLOCK:
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "block");
			eh->update((Generic)eh->ped);
			break;

		case SORT_BY_DIMS:
			el_sort( PED_EL(eh->ped), PED_DG(eh->ped), "dim");
			eh->update((Generic)eh->ped);
			break;

		default:
			break;
    	}
    	return DIALOG_NOMINAL;
}

void edge_dialog_update (EdgeDia *eh)
{ 
  edge_dialog_edge_clear (eh);
}

void edge_dialog_hide (EdgeDia *eh)
{
  dialog_modeless_hide (eh->di);
}

void 
edge_dialog_edge_clear (EdgeDia *eh)
{
    sfree(eh->text_type);
    sfree(eh->text_name);
    sfree(eh->text_dims);
    sfree(eh->text_block);

    (void) item_title_change(eh->text_src, "");
    (void) item_title_change(eh->text_sink, "");
    eh->src_ast    = AST_NIL;
    eh->sink_ast   = AST_NIL;
    eh->text_type  = ssave("");
    eh->text_name  = ssave("");
    eh->text_dims  = ssave("");
    eh->text_block = ssave("");
    
    dialog_item_modified (eh->di, TEXT_TYPE);
    dialog_item_modified (eh->di, TEXT_NAME);
    dialog_item_modified (eh->di, TEXT_DIMS);
    dialog_item_modified (eh->di, TEXT_BLOCK);

}
void 
edge_dialog_edge_set (EdgeDia *eh, char *text, char *name, AST_INDEX src, 
                      AST_INDEX sink, char *dims, char *block)
{
    sfree(eh->text_type);
    sfree(eh->text_name);
    sfree(eh->text_dims);
    sfree(eh->text_block);

    (void) item_title_change(eh->text_src, el_gen_get_text (src));
    (void) item_title_change(eh->text_sink, el_gen_get_text (sink));
    eh->text_type  = ssave(text);
    eh->text_name  = ssave(name);
    eh->text_dims  = ssave(dims);
    eh->text_block = ssave(block);
    eh->src_ast    = src;
    eh->sink_ast   = sink;
    
    dialog_item_modified (eh->di, TEXT_TYPE);
    dialog_item_modified (eh->di, TEXT_NAME);
    dialog_item_modified (eh->di, TEXT_DIMS);
    dialog_item_modified (eh->di, TEXT_BLOCK);

}
void
edge_dialog_destroy (EdgeDia *eh)
{
    sfree(eh->text_type);
    sfree(eh->text_dims);
    sfree(eh->text_block);
    sfree(eh->text_name);
    dialog_destroy(eh->di);
}

void 
convert(char *str)
{
   char	*buf;
   int  i, num, strLength;
   
   /* convert this string of digits into a string of dependence type
      characters. -vas */
      
   strLength	= strlen(str);
   buf	= (char *)get_mem( 2*strLength, "convert" );
   
   for(i = 0; i < strLength; i++) {
      if(isdigit(str[i])) {
         num = atoi(&str[i]);
	 switch(num) {
	     case dg_true: 	strcat(buf, "t ");
	     			break;
	     case dg_anti: 	strcat(buf, "a ");
	     			break;
	     case dg_output: 	strcat(buf, "o ");
	     			break;
	     case dg_input: 	strcat(buf, "n ");
	     			break;
	     case dg_inductive: strcat(buf, "i ");
	     			break;
	     case dg_exit: 	strcat(buf, "x ");
	     			break;
	     case dg_io: 	strcat(buf, "r ");
	     			break;
	     case dg_call: 	strcat(buf, "p ");
	     			break;
	     default:		break;
	 }
      }
   }
   sfree(str);
   str	= ssave(buf);
   
}
