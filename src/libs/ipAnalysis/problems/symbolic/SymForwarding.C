/* $Id: SymForwarding.C,v 1.7 1997/03/11 14:35:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/moduleAnalysis/valNum/val_pass.h>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/ipAnalysis/problems/symbolic/globals_fwd_ht.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefQuery.h>
#include <libs/ipAnalysis/problems/symbolic/SymValAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymTrans.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#include <libs/ipAnalysis/problems/symbolic/IpVarList.h>

static ValNumber translate(CallGraphNode *node, ValIP *oldSym,
			   SymValAnnot *newSym, ValNumber vn);
static ValNumber translate_global(CallGraphNode *node, ValIP *oldSym,
				  SymValAnnot *newSym, ValNumber vn,
				  char *varName, int varOffset, int varLength);
static ValNumber trans_returned(CallGraphEdge *edge, 
				char *name, int offset, int length,
				SymValAnnot *handle);
static void register_ip_entry(CallGraphNode *node, ValTable &OV,
			      SymValAnnot *newSym, ValNumber ov);
static void register_ip_ref(CallGraphNode *node, ValTable &OV, 
			    SymValAnnot *newSym, char *cblock,
			    int offset, int length);

SymValAnnot *SymExploitMod(CallGraphNode *node)
{
    ValIP *oldSym = node->local_info->val_ip;

    if (!node || !oldSym) return NULL;

    SymValAnnot *newSym = new SymValAnnot(node);

    ValNumber ov, nv;

    /////////////////////////////////////////////////////////////////////
    //
    //  Create forwarding information
    //
    int count;
    count = oldSym->values->count();

    for (ov = 0; ov < count; ov++) 
    {
	//  translate each value number and enter forwarding value in table
	//
	if (newSym->fwd(ov) != VAL_BOGUS)
	{
	    //  Value of nv won't be used, but convenient to set
	    //  it for debugging.
	    //
	    nv = translate(node, oldSym, newSym, ov);
	}
	//  This method of recording REF information will need to change
	//  to be more stable across edits of the callee (and even just
	//  across invocations of the program compiler!)  paco, Oct 1993
	//
	ValType vt = ve_type((*(oldSym->values))[ov]);
	if ((vt == VAL_ENTRY) || (vt == VAL_DUMMY))
	{
	    register_ip_entry(node, *(oldSym->values), newSym, ov);
	}
    }

    char *fname, *aname;
    VarScope vscope;
    int offset, length;

    /////////////////////////////////////////////////////////////////////
    //
    //  Go through all the call sites, get REF annotations, and 
    //  save passed values
    //
    CallGraphEdge *edge;
    CallGraphNode *sink;

    for (CallGraphNodeOutEdgeIterator out(node);
	 edge = out.Current();
	 out++)
    {
	sink = edge->Callee();
	//  Record passed values of global variables...
	//
	SymValAnnot *sinkInfo = (SymValAnnot *) 
	    sink->GetAnnotation(SYMVAL_ANNOT, true);

	for (IpVarListIter scalars(sinkInfo->refScalars());
	     fname = scalars.Current(offset, length);
	     scalars++)
	{
	    if (sink->LookupFormal(fname))
	    {
		int fid = sink->FormalNameToPosition(fname);

		//  Add by position
		//
		ov = oldSym->pass->query_entry(edge->c_id, "", fid);

		if (ov == VAL_NIL) continue;

		nv = translate(node, oldSym, newSym, ov);

		newSym->valIp()->pass->add_entry(edge->c_id, 
						 "", fid, nv);

		//  Add by name
		//
		ParamBinding *abind;
		abind = edge->param_bindings.GetReverseBinding(fname);

		if (abind && abind->actual && (abind->a_class == OtherActual))
		{
		    aname = abind->actual;
		    ov = oldSym->pass->query_entry(edge->c_id, aname, 0);

		    nv = translate(node, oldSym, newSym, ov);

		    newSym->valIp()->pass->add_entry(edge->c_id, aname, 0, nv);

		    //  Would normally check to see that this variable
		    //  is in the list for the caller, but this shouldn't
		    //  be necessary -- should already be added because
		    //  of VAL_ENTRY handling above, if it is formal
		    //  or common, and otherwise we don't care.
		}
	    }
	    else		//  We have a common block var
	    {
		ov = oldSym->pass->query_entry(edge->c_id, fname, offset);

		if (ov == VAL_NIL) 
		{
		    //  Have to build passed value from that for DUMMY_GLOBAL
		    //
		    ov = oldSym->pass->query_entry(edge->c_id, "DUMMY_GLOBAL");
		    nv = translate_global(node, oldSym, newSym,
					  ov, fname, offset, length);
		}
		else
		{
		    nv = translate(node, oldSym, newSym, ov);
		}
		newSym->valIp()->pass->add_entry(edge->c_id, 
						 fname, offset, nv);

		//  Check to be sure that this variable is in the 
		//  list of variables referenced in the caller.
		//
		register_ip_ref(node, *(oldSym->values), 
				newSym, fname, offset, length);
	    }
	}
    }

    /////////////////////////////////////////////////////////////////////
    //
    //	get the MOD annotation for this node and translate return vals
    //
    ScalarModRefAnnot *mod = (ScalarModRefAnnot *)
	node->GetAnnotation(SCALAR_MOD_ANNOT, true);

    for (ScalarModRefAnnotIterator iter(mod);

	 fname = iter.Current(vscope, offset, length);
	 iter++)
    {
	ov = oldSym->pass->query_entry(fname, offset);

	if (ov != VAL_NIL)
	{
	    //  Found return value in old table
	    //
	    nv = translate(node, oldSym, newSym, ov);
	}
	else if (vscope == GlobalScope)
	{
	    //  Have to build return value from that for DUMMY_GLOBAL
	    //
	    ov = oldSym->pass->query_entry("DUMMY_GLOBAL", 0);
	    nv = translate_global(node, oldSym, newSym, ov, 
				  fname, offset, length);
	}
	else // local variable, not a formal even
	{
	    continue;
	}

	//  Add by name
	//
	newSym->valIp()->pass->add_entry(fname, offset, nv);

	//  Add by parameter number
	//
	if (node->LookupFormal(fname))
	{
	    int fid = node->FormalNameToPosition(fname);

	    newSym->valIp()->pass->add_entry("", fid, nv);
	}
    }

    return newSym;
}

/************************************************************************/

static ValNumber translate(CallGraphNode *node, ValIP *oldSym,
			   SymValAnnot *newSym, ValNumber vn)
{
    ValTable *OV, *NV;
    ValEntry *vp;
    ValType type;
    ValNumber nv, input, *kids, initVal;
    ScalarModRefAnnot *mod;

    if (vn == VAL_NIL) return VAL_NIL;

    if (newSym->fwd(vn) == VAL_BOGUS)
    {
	//  Don't translate DUMMY_GLOBAL values with this routine
	//
	//  return VAL_BOTTOM;	//  dangerous hack!  (should return VAL_NIL?)
	//  die_with_message("translate: attempt made with VAL_BOGUS\n");
	return VAL_BOTTOM;	//  could be VAL_TOP? elidable?
    }
    if (newSym->fwd(vn) != VAL_NIL)
    {
	return newSym->fwd(vn);
    }
    //  Protect from infinite loops
    //
    newSym->fwd(vn) = VAL_BOTTOM;

    OV = oldSym->values;
    vp = &((*OV)[vn]);
    NV = newSym->valIp()->values;
    type = ve_type(*vp);
    nv = VAL_BOTTOM;
    kids = NULL;

    switch(type)
    {
      case VAL_OP:
      case VAL_INTRINSIC:
      case VAL_PURE_FN:
      case VAL_PHI:
      case VAL_GAMMA:
      case VAL_LIST:
      {
	  kids = new ValNumber[ve_arity(*vp)];

	  for (int i = 0; i < ve_arity(*vp); i++)
	  {
	      kids[i] = translate(node, oldSym, newSym, ve_kid(*vp, i));
	  }
	  break;
      }
      default:
	break;
    }

    switch(type)
    {
      case VAL_BOT_TYPE:
      case VAL_TOP_TYPE:
	nv = vn;
	break;

      case VAL_CONST:
	nv = val_lookup_const(*NV, ve_expType(*vp), 
			      ve_const(*vp));
	break;

      case VAL_TEXT:
	nv = val_lookup_text(*NV, ve_expType(*vp),
			     val_get_const_text(OV, vn));
	break;

      case VAL_RANGE:
	nv = val_lookup_range(*NV, 
			      translate(node, oldSym, newSym, ve_lo(*vp)),
			      translate(node, oldSym, newSym, ve_hi(*vp)),
			      translate(node, oldSym, newSym, ve_align(*vp)), 
			      translate(node, oldSym, newSym, ve_step(*vp)));
	break;

      case VAL_IVAR:
	nv = val_lookup_ivar(*NV, ve_level(*vp),
			     translate(node, oldSym, newSym, ve_bounds(*vp)),
			     ve_flipped(*vp));
	break;

      case VAL_VARIANT:
	nv = val_lookup_variant(*NV, ve_expType(*vp),
				ve_occur(*vp), ve_level(*vp));
	break;

      case VAL_OP:
	nv = val_lookup_op(*NV, ve_expType(*vp),
			   ve_opType(*vp), ve_arity(*vp), kids);
	break;

      case VAL_INTRINSIC:
	nv = val_lookup_intrinsic(*NV, ve_expType(*vp),
				  val_get_const_text(OV, ve_entry(*vp)),
				  ve_arity(*vp), kids);
	break;

      case VAL_PURE_FN:
	nv = val_lookup_pure(*NV, ve_expType(*vp),
			     val_get_const_text(OV, ve_entry(*vp)),
			     ve_arity(*vp), kids);
	break;

      case VAL_PHI:
	nv = val_lookup_phi(*NV, ve_expType(*vp), ve_stmt(*vp),
			    ve_arity(*vp), kids);
	break;

      case VAL_GAMMA:
	nv = val_lookup_gamma(*NV, ve_expType(*vp),
			      translate(node, oldSym, newSym, ve_test(*vp)),
			      ve_arity(*vp), kids);
	break;

      case VAL_LIST:
	nv = val_lookup_list(*NV, ve_arity(*vp), kids);
	break;

      case VAL_TEST:
	nv = val_lookup_test(*NV,
			     translate(node, oldSym, newSym, ve_itest(*vp)),
			     ve_testType(*vp), ve_occur(*vp));
	break;

      case VAL_ETA:
	nv = val_lookup_eta(*NV,
			    translate(node, oldSym, newSym, ve_test(*vp)),
			    translate(node, oldSym, newSym, ve_final(*vp)));
	break;

      case VAL_OK_MU:
	nv = val_lookup_ok_mu(*NV,
			      translate(node, oldSym, newSym, ve_init(*vp)),
			      translate(node, oldSym, newSym, ve_iter(*vp)),
			      ve_level(*vp));
	break;

      case VAL_VAR_MU:
	//
	//  Try to collapse cycles
	//
	initVal = translate(node, oldSym, newSym, ve_init(*vp));

	nv = val_lookup_var_mu(*NV, ve_occur(*vp), initVal, ve_level(*vp));
	//
	//  Add this early to thwart infinite recursion
	//
	newSym->fwd(vn) = nv;

	input = translate(node, oldSym, newSym, ve_varIter(*vp));

	if (input == nv) nv = initVal;
	else
	    ve_varIter((*NV)[nv]) = input;
	break;

      case VAL_MU_PH:
	nv = val_lookup_mu_ph(*NV, ve_level(*vp));
	break;

      case VAL_ARRAY:
	nv = val_lookup_array(*NV, ve_expType(*vp),
			      translate(node, oldSym, newSym, ve_state(*vp)),
			      translate(node, oldSym, newSym, ve_rhs(*vp)),
			      translate(node, oldSym, newSym, ve_subs(*vp)));
	break;

      case VAL_ENTRY:
	nv = val_lookup_entry(*NV, ve_expType(*vp),
			      val_get_const_text(OV, ve_entry(*vp)),
			      val_get_const_text(OV, ve_name(*vp)),
			      ve_offset(*vp), ve_length(*vp));
	break;

      case VAL_RETURN:
      {
	  CallGraphEdge *edge = 
	      (node->GetCallGraph())->LookupEdge(node->procName,
						 ve_call(*vp));
	  mod =(ScalarModRefAnnot *)edge->GetAnnotation(SCALAR_MOD_ANNOT, true);

	  input = translate(node, oldSym, newSym, ve_input(*vp));

	  char *aname = val_get_const_text(OV, ve_name(*vp));

	  if (mod->IsMember(aname, ve_offset(*vp), ve_length(*vp)))
	  {
	      //  Get any old formal that is bound to this actual...
	      //  Assume that return value computation correctly handles
	      //  aliasing problems for us.
	      //
	      ParamBindingsSetIterator foo(
			      edge->param_bindings.GetForwardBindings(aname));

	      if (foo.Current())
	      {
		  char *fname = foo.Current()->formal;

		  nv = trans_returned(edge, fname, 0, ve_length(*vp), newSym);
	      }
	      else // global?
	      {
		  nv = trans_returned(edge, aname,
				      ve_offset(*vp), ve_length(*vp), newSym);
	      }
	      if (nv == VAL_NIL)
	      {
		  nv = val_lookup_return(*NV, ve_expType(*vp), ve_level(*vp),
					 ve_call(*vp),
					 val_get_const_text(OV, ve_name(*vp)),
					 ve_offset(*vp), ve_length(*vp),
					 input);
	      }
	  }
	  else
	  {
	      nv = input;
	  }
	  break;
      }

      case VAL_DUMMY:
	mod = (ScalarModRefAnnot *) node->GetAnnotation(SCALAR_MOD_ANNOT, true);

	if (mod->IsMember(val_get_const_text(OV, ve_name(*vp)),
			  ve_offset(*vp), ve_length(*vp)))
	{
	    nv = translate_global(node, oldSym, newSym, ve_state(*vp), 
				  val_get_const_text(OV, ve_name(*vp)), 
				  ve_offset(*vp), ve_length(*vp));
	}
	else
	{
	    nv = val_lookup_entry(*NV, ve_expType(*vp),
				  node->procName,
				  val_get_const_text(OV, ve_name(*vp)),
				  ve_offset(*vp), ve_length(*vp));
	}
	break;
      default:
	die_with_message("translate: disallowed type\n");
	break;
    }
    if (kids) delete kids;
    if (nv <= VAL_NIL) nv = VAL_BOTTOM;
    newSym->fwd(vn) = nv;
    return nv;
}

static ValNumber translate_global(CallGraphNode *node, ValIP *oldSym,
				  SymValAnnot *newSym, ValNumber vn,
				  char *varName, int varOffset, 
				  int varLength)
{
    ValTable *OV = oldSym->values;
    ValTable *NV = newSym->valIp()->values;
    ValEntry  *vp = &((*OV)[vn]);
    ValType   type = ve_type(*vp);
    ValNumber nv, input;
    ValNumber *kids;
    ValNumber nameId = val_lookup_text(*NV, TYPE_CHARACTER, varName);
    ScalarModRefAnnot *mod;
    int i;

    ValNumber fwd = newSym->fwd(vn);

    if ((fwd == VAL_NIL) || (fwd == VAL_TOP) || (fwd == VAL_BOTTOM))
	return VAL_BOTTOM;

    if (fwd != VAL_BOGUS)
    {
	//  Temporary fix?
	return VAL_BOTTOM;	// could be VAL_TOP? elidable?
	//  die_with_message("translate_global: attempt made on non-global\n");
    }

    nv = newSym->getGlobal(vn, nameId, varOffset);

    if (nv != VAL_NIL)
	return nv;

    //  Protect from infinite loops
    //
    newSym->addGlobal(vn, nameId, varOffset, VAL_BOTTOM);

    switch(type)
    {
      case VAL_ENTRY:
	nv = val_lookup_entry(*NV, TYPE_INTEGER, node->procName,
			      varName, varOffset, varLength);
	break;

      case VAL_RETURN:
      {
	  CallGraphEdge *edge = 
	      (node->GetCallGraph())->LookupEdge(node->procName,
						 ve_call(*vp));
	  mod =(ScalarModRefAnnot *)edge->GetAnnotation(SCALAR_MOD_ANNOT, true);

	  input = translate_global(node, oldSym, newSym, ve_input(*vp),
				   varName, varOffset, varLength);

	  if (mod->IsMember(varName, varOffset, varLength))
	  {
	      nv = val_lookup_return(*NV, TYPE_INTEGER, ve_level(*vp), 
				     ve_call(*vp), 
				     varName, varOffset, varLength, input);
	  }
	  else
	  {
	      nv = input;
	  }
	  break;
      }

      case VAL_GAMMA:
	kids = new ValNumber[ve_arity(*vp)];

	for (i = 0; i < ve_arity(*vp); i++)
	{
	    kids[i] = translate_global(node, oldSym, newSym, ve_kid(*vp, i),
				       varName, varOffset, varLength);
	}
	nv = val_lookup_gamma(*NV, TYPE_INTEGER,
			      translate(node, oldSym, newSym, ve_test(*vp)),
			      ve_arity(*vp), kids);
	delete kids;
	break;

      case VAL_PHI:
	kids = new ValNumber[ve_arity(*vp)];

	for (i = 0; i < ve_arity(*vp); i++)
	{
	    kids[i] = translate_global(node, oldSym, newSym, ve_kid(*vp, i),
				       varName, varOffset, varLength);
	}
	nv = val_lookup_phi(*NV, TYPE_INTEGER, ve_stmt(*vp),
			    ve_arity(*vp), kids);
	delete kids;
	break;

      case VAL_ETA:
	nv = val_lookup_eta(*NV,
			    translate(node, oldSym, newSym, ve_test(*vp)),
			    translate_global(node, oldSym, newSym, 
					     ve_final(*vp),
					     varName, varOffset, varLength));
	break;

      case VAL_VAR_MU:
      {
	  //
	  //  Try to reduce cycles...
	  //
	  ValNumber initVal = translate_global(node, oldSym, newSym, 
					       ve_init(*vp), varName, 
					       varOffset, varLength);

	  nv = val_lookup_var_mu(*NV, ve_occur(*vp), initVal, ve_level(*vp));
	  //
	  //  Add this early to thwart infinite recursion...
	  //
	  newSym->addGlobal(vn, nameId, varOffset, nv);

	  input = translate_global(node, oldSym, newSym, ve_varIter(*vp),
				   varName, varOffset, varLength);

	  if (input == nv) nv = initVal;
	  else
	      ve_varIter((*NV)[nv]) = input;
	  break;
      }

      default:
	die_with_message("translate_global: disallowed type\n");
    }
    //
    //  Add new value nv to forwarding table and return
    //
    if (nv <= VAL_NIL) nv = VAL_BOTTOM;
    newSym->addGlobal(vn, nameId, varOffset, nv);
    return nv;
}

static ValNumber trans_passed(void *handle, CallGraphEdge *edge, 
			      char *name, int offset, int length)
{
    ValNumber nv, ov;

    SymValAnnot *newSym = (SymValAnnot *) handle;

    nv = newSym->valIp()->pass->query_entry(edge->c_id, name, offset);

    if (nv != VAL_NIL) return nv;

    //  else we haven't built a translated value yet...
    //
    ValIP *oldSym = edge->Caller()->local_info->val_ip;

    ov = oldSym->pass->query_entry(edge->c_id, name, offset);

    if (ov != VAL_NIL) return translate(edge->Caller(), oldSym, newSym, ov);

    //  else this must be a global variable that wasn't directly referenced
    //
    ov = oldSym->pass->query_entry(edge->c_id, "DUMMY_GLOBAL");

    nv = translate_global(edge->Caller(), oldSym, newSym, ov,
			  name, offset, length);

    return nv;
}

static ValNumber trans_returned(CallGraphEdge *edge, 
				char *name, int offset, int length,
				SymValAnnot *handle)
{
    SymValAnnot *sinkAnnot = (SymValAnnot *) 
	(edge->Callee())->GetAnnotation(SYMVAL_ANNOT, true);

    ValNumber ov = sinkAnnot->valIp()->pass->query_entry(name, offset);

    if (ov == VAL_NIL)
    {
	return VAL_NIL;
    }
    else
    {
	return SymTranslate(edge, handle->valIp()->values, ov,
			    trans_passed, handle);
    }
}

EXTERN(ValNumber, cfgval_translate_passed, (void *cfg, int site, char *name, 
					    int offset, int length));

static ValNumber trans_pass_cfgval(void *cfg, CallGraphEdge *edge,
				   char *name, int offset, int length)
{
    return cfgval_translate_passed(cfg, edge->c_id, name, offset, length);
}

EXTERN(ValNumber, trans_ret_cfgval, (void *v_cg, ValTable *V, 
				     char *proc, int site,
				     char *name, int offset, int length, 
				     void *cfg));

ValNumber trans_ret_cfgval(void *v_cg, ValTable *V, char *proc, int site,
			   char *name, int offset, int length, void *cfg)
{
    CallGraph *cg = (CallGraph *) v_cg;
    CallGraphEdge *edge = cg->LookupEdge(proc, site);

    SymValAnnot *sinkAnnot = (SymValAnnot *) 
	(edge->Callee())->GetAnnotation(SYMVAL_ANNOT, true);

    if (!strcmp(name, ""))
    {
	name = (edge->Callee())->FormalPositionToName(offset);
	offset = 0;
    }
    ValNumber ov = sinkAnnot->valIp()->pass->query_entry(name, offset);

    if (ov == VAL_NIL) return VAL_NIL;

    return SymTranslate(edge, V, ov, trans_pass_cfgval, cfg);
}

EXTERN(ValNumber, 
       sym_trans_saved,
       (CallGraphEdge *edge, ValNumber ov, SymValAnnot *handle));

ValNumber sym_trans_saved(CallGraphEdge *edge, ValNumber ov,
                          SymValAnnot *handle)
{
    return SymTranslate(edge, handle->valIp()->values, ov,
                        trans_passed, handle);
}

EXTERN(ValNumber,
       sym_trans_saved_cfgval,
       (void *v_cg, ValTable *V, char *proc, 
	int site, ValNumber ov, void *cfg));

ValNumber sym_trans_saved_cfgval(void *v_cg, ValTable *V, char *proc, int site,
                                 ValNumber ov, void *cfg)
{
    CallGraph *cg = (CallGraph *) v_cg;
    CallGraphEdge *edge = cg->LookupEdge(proc, site);

    return SymTranslate(edge, V, ov, trans_pass_cfgval, cfg);
}

//  A variable has a VAL_ENTRY for the calling routine, which implies
//  it seemed in the local analysis to be directly or indirectly 
//  referenced.  If so, add it to our own list of referenced scalars.
//
//  Handling of alternate entry points is pretty broken for now.
//  Hope we don't see any; this would probably combine all their 
//  effects under the main entry point.
//
static void register_ip_entry(CallGraphNode *node, ValTable &OV,
			      SymValAnnot *newSym, ValNumber ov)
{
    ValEntry *vp;

    vp = &(OV[ov]);

    char *vName = val_get_const_text(&OV, ve_name(*vp));
    int offset = ve_offset(*vp);
    int length = ve_length(*vp);

    if (ve_type(*vp) == VAL_DUMMY)
    {
	register_ip_ref(node, OV, newSym, vName, offset, length);
	return;
    }

    char *pName = val_get_const_text(&OV, ve_entry(*vp));

    if (!IPQuery_IsScalarRefNode((Generic)node->GetCallGraph(),
				 pName, vName, offset, length))
    {
	return;
    }

    newSym->refScalars()->Append(vName, offset, length);
}

//  Here we check to see if there is a VAL_ENTRY for the variable.
//  If so, then it should already be on the list for this routine.
//  Otherwise, we need to add it.
//
static void register_ip_ref(CallGraphNode *node, ValTable &OV, 
			    SymValAnnot *newSym, char *cblock,
			    int offset, int length)
{
    ValNumber ov;

    //  This should be a superfluous double-check that the variable
    //  is referenced.
    //
    if (!IPQuery_IsScalarRefNode((Generic)node->GetCallGraph(),
				 node->procName, cblock, offset, length))
    {
	return;
    }

    //  The type and length fields are unhashed for VAL_ENTRY...
    //  insert so that we only add things to the list once
    //
    ov = val_lookup_entry(OV, TYPE_UNKNOWN, node->procName, 
			  cblock, offset, length, /* insert = */ false);

    if (ov != VAL_NIL) return;

    (void) val_lookup_entry(OV, TYPE_UNKNOWN, node->procName, 
			    cblock, offset, length, /* insert = */ true);

    newSym->refScalars()->Append(cblock, offset, length);
}
