/* $Id: SymTranslate.C,v 1.7 1997/03/11 14:35:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/problems/symbolic/SymTrans.h>
#include <libs/ipAnalysis/problems/symbolic/SymValAnnot.h>

ValNumber SymTranslate(CallGraphEdge *edge, ValTable *NV, ValNumber ov,
		       SymTransCallback transFn, void *handle)
{
    ValNumber nv;	//  return value

    SymValAnnot *sinkAnnot = (SymValAnnot *) 
	(edge->Callee())->GetAnnotation(SYMVAL_ANNOT, true);

    nv = sinkAnnot->cached(NV, ov);
    if (nv != VAL_NIL) return nv;

    ValTable *OV = sinkAnnot->valIp()->values;

    ValEntry  *vp = &((*OV)[ov]);
    ValType   type = ve_type(*vp);
    ValNumber *kids = NULL;
    ValNumber temp1, temp2, temp3;
    int i;

    switch(type)
    {
      case VAL_BOT_TYPE:
      case VAL_TOP_TYPE:
	nv = ov;
	break;

      case VAL_CONST:
	nv = val_lookup_const(*NV, ve_expType(*vp), ve_const(*vp));
	break;

      case VAL_TEXT:
	nv = val_lookup_text(*NV, ve_expType(*vp), 
			     val_get_const_text(OV, ov));
	break;

      case VAL_RANGE:
	nv = val_lookup_range(*NV,
			      SymTranslate(edge, NV, ve_lo(*vp),
					   transFn, handle),
			      SymTranslate(edge, NV, ve_hi(*vp),
					   transFn, handle),
			      SymTranslate(edge, NV, ve_align(*vp),
					   transFn, handle),
			      SymTranslate(edge, NV, ve_step(*vp),
					   transFn, handle));
					   
	break;

      case VAL_IVAR:
	temp1 = SymTranslate(edge, NV, ve_bounds(*vp), transFn, handle);

	//  Imprecise range in ivar can't be permitted (might lead
	//  to erroneous identities between values with different ranges)
	//
	if ((ve_lo((*NV)[temp1])    == VAL_BOTTOM) ||
	    (ve_hi((*NV)[temp1])    == VAL_BOTTOM) ||
	    (ve_align((*NV)[temp1]) == VAL_BOTTOM) ||
	    (ve_step((*NV)[temp1])  == VAL_BOTTOM))
	{
	    nv = VAL_BOTTOM;
	}
	else
	{
	    //  Ooh, possible bug.  Can simplification flip things on us?
	    //
	    nv = val_lookup_ivar(*NV, ve_level(*vp), temp1, ve_flipped(*vp));
	}
	break;

      case VAL_VARIANT:
	nv = VAL_BOTTOM;
	break;

      case VAL_OP:
	kids = new ValNumber[ve_arity(*vp)];
	nv = VAL_TOP;

	for (i = 0; i < ve_arity(*vp); i++)
	{
	    kids[i] = SymTranslate(edge, NV, ve_kid(*vp, i),
				   transFn, handle);
	    if (kids[i] == VAL_BOTTOM) 
	    {
		nv = VAL_BOTTOM;
		break; /* from for */
	    }
	}
	if (nv != VAL_BOTTOM)
	{
	    nv = val_lookup_op(*NV, ve_expType(*vp),
			       ve_opType(*vp), ve_arity(*vp), kids);
	}
	break; /* from switch */

      case VAL_INTRINSIC:
	kids = new ValNumber[ve_arity(*vp)];
	nv = VAL_TOP;

	for (i = 0; i < ve_arity(*vp); i++)
	{
	    kids[i] = SymTranslate(edge, NV, ve_kid(*vp, i),
				   transFn, handle);
	    if (kids[i] == VAL_BOTTOM) 
	    {
		nv = VAL_BOTTOM;
		break; /* from for */
	    }
	}
	if (nv != VAL_BOTTOM)
	{
	    nv = val_lookup_intrinsic(*NV, ve_expType(*vp),
				      val_get_const_text(OV, ve_entry(*vp)),
				      ve_arity(*vp), kids);
	}
	break;

      case VAL_PURE_FN:
	kids = new ValNumber[ve_arity(*vp)];
	nv = VAL_TOP;

	for (i = 0; i < ve_arity(*vp); i++)
	{
	    kids[i] = SymTranslate(edge, NV, ve_kid(*vp, i),
				   transFn, handle);
	    if (kids[i] == VAL_BOTTOM) 
	    {
		nv = VAL_BOTTOM;
		break; /* from for */
	    }
	}
	if (nv != VAL_BOTTOM)
	{
	    nv = val_lookup_pure(*NV, ve_expType(*vp),
				 val_get_const_text(OV, ve_entry(*vp)),
				 ve_arity(*vp), kids);
	}
	break;

      case VAL_PHI:
	//
	//  Bottom unless all kids are the same value
	//
	kids = new ValNumber[ve_arity(*vp)];
	nv = SymTranslate(edge, NV, ve_kid(*vp, 0), transFn, handle);
	kids[0] = nv;

	for (i = 1; i < ve_arity(*vp); i++)
	{
	    kids[i] = SymTranslate(edge, NV, ve_kid(*vp, i), 
				   transFn, handle);
	    if (kids[i] != nv) 
	    {
		nv = VAL_BOTTOM;
		break; /* from for */
	    }
	}
	break;

      case VAL_GAMMA:
      {
	  //
	  //  Bottom if any kid is bottom, unless test selects non-bottom kid.
	  //  Bottom if test is bottom, unless all kids are same.
	  //
	  kids = new ValNumber[ve_arity(*vp)];
	  nv = temp1 = VAL_TOP;

	  for (i = 0; i < ve_arity(*vp); i++)
	  {
	      kids[i] = SymTranslate(edge, NV, ve_kid(*vp, i), 
				     transFn, handle);
	      if (kids[i] == VAL_BOTTOM)
	      {
		  nv = VAL_BOTTOM;
		  // don't break out -- may be saved by test value
	      }
	      temp1 = val_merge(NV, temp1, kids[i]);
	  }
	  ValNumber test = SymTranslate(edge, NV, ve_test(*vp), 
					transFn, handle);

	  if (test == VAL_BOTTOM)
	  {
	      if (ve_type((*NV)[temp1]) != VAL_RANGE)
		  nv = temp1;
	      else
		  nv = VAL_BOTTOM;
	      break; /* from switch */
	  }

	  if (ve_type((*NV)[test]) == VAL_CONST)
	  {
	      nv = kids[ve_const((*NV)[test])];
	      break;
	  }
	  // else
	  //
	  if (nv != VAL_BOTTOM)
	  {
	      nv = val_lookup_gamma(*NV, ve_expType(*vp), test,
				    ve_arity(*vp), kids);
	  }
	  break;
      }

      case VAL_LIST:
	kids = new ValNumber[ve_arity(*vp)];
	nv = VAL_TOP;

	for (i = 0; i < ve_arity(*vp); i++)
	{
	    kids[i] = SymTranslate(edge, NV, ve_kid(*vp, i), 
				   transFn, handle);
	    if (kids[i] == VAL_BOTTOM) 
	    {
		nv = VAL_BOTTOM;
		break; /* from for */
	    }
	}
	if (nv != VAL_BOTTOM)
	{
	    nv = val_lookup_list(*NV, ve_arity(*vp), kids);
	}
	break;

      case VAL_TEST:
	nv = VAL_BOTTOM;
	break;

      case VAL_ETA:
	temp1 = SymTranslate(edge, NV, ve_test(*vp), transFn, handle);
	temp2 = SymTranslate(edge, NV, ve_final(*vp), transFn, handle);

	if ((temp1 == VAL_BOTTOM) || (temp2 == VAL_BOTTOM))
	{
	    nv = VAL_BOTTOM;
	    break;
	}
	nv = val_lookup_eta(*NV, temp1, temp2);
	break;

      case VAL_OK_MU:
	temp1 = SymTranslate(edge, NV, ve_init(*vp), transFn, handle);
	temp2 = SymTranslate(edge, NV, ve_iter(*vp), transFn, handle);

	if ((temp1 == VAL_BOTTOM) || (temp2 == VAL_BOTTOM))
	{
	    nv = VAL_BOTTOM;
	    break;
	}
	nv = val_lookup_ok_mu(*NV, temp1, temp2, ve_level(*vp));
	break;

      case VAL_VAR_MU:
	nv = VAL_BOTTOM;
	break;

      case VAL_MU_PH:
	nv = val_lookup_mu_ph(*NV, ve_level(*vp));
	break;

      case VAL_ARRAY:
	temp1 = SymTranslate(edge, NV, ve_state(*vp), transFn, handle);
	if (ve_rhs(*vp) != VAL_NIL)
	    temp2 = SymTranslate(edge, NV, ve_rhs(*vp),   transFn, handle);
	else
	    temp2 = VAL_NIL;
	temp3 = SymTranslate(edge, NV, ve_subs(*vp),  transFn, handle);

	if ((temp1 == VAL_BOTTOM) || 
	    (temp2 == VAL_BOTTOM) || 
	    (temp3 == VAL_BOTTOM))
	{
	    nv = VAL_BOTTOM;
	    break;
	}
	nv = val_lookup_array(*NV, ve_expType(*vp),
			      temp1, temp2, temp3);
	break;

      case VAL_ENTRY:
      {
	  //  This is the heart of the routine, translating a passed value
	  //
	  //  Map from the variable name to formal parameter position, if any.
	  //
	  char *name = val_get_const_text(OV, ve_name(*vp));

	  int parm;
	  if (edge->Callee()->LookupFormal(name))
	  {
	      //  Formal parameter
	      //
	      parm = edge->Callee()->FormalNameToPosition(name);
	      name = "";
	  }
	  else
	  {
	      //  Common block variable
	      //
	      parm = ve_offset(*vp);
	  }
	  nv = transFn(handle, edge, name, parm, ve_length(*vp));
	  break;
      }

      case VAL_RETURN:
      case VAL_DUMMY:
	nv = VAL_BOTTOM;
	break;

      default:
	die_with_message("SymTranslate: disallowed type\n");
	break;
    }
    if (kids) delete kids;
    if (nv <= VAL_NIL) nv = VAL_BOTTOM;
    //
    //  temporarily disable caching for sake of aux. i.v. recognition
    //
    // sinkAnnot->encache(NV, ov, nv);
    return nv;
}
