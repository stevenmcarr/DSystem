/* $Id: fit.C,v 1.1 1997/06/25 14:41:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
  fit.c -- routines for fitting a function to the communication performance
  data. Uses the chi-squared function fitting routine from Numerical
  Recipes (C version).

  >> The software in this file is public domain. You may copy, modify and 
  >> use it as you wish, provided you cite the author for the original source.
  >> Remember that anything free comes with no guarantees. 
  >> Use it at your own risk. 

  Author: Vas, July 1990.

  Modification History:
  21 Feb 92 NM  - abstract out fit routine and move to nrutil.c

 ****************************************************************************/

#include <math.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/fort.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>

#include <libs/support/numerical/NumericalRecipeUtils.h>

#define MAX_POINTS  100
#define SPREAD      0.6  /* estimated std deviation of measured data */

static float sqrarg;
#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)


static
void compute_pkt_costs(float a, float b, CommData *oldcptr, float *stup, float *pkt);

/*-------------------------------------------------------------------------
   chi_squared_fit -- fit a linear function to the data using the chi2 fit.
 --------------------------------------------------------------------------*/
void
chi_squared_fit(char *databuf, int npoints, CommInfo *commptr)
{
  float         *x, *y, *sig;
  float         a, b, chi2, q, siga, sigb;
  int           k, i, xlimit, xpoint = 0;
  float         ypoint = 0.0;
  static char   point[20];
  int           rc, len, mwt;
  CommData      *cptr, *oldcptr;
  int           first_time = 1;
  int           numpackets = 0;

  /*
   * NOTE:  the communication data is actually a discontinuous function,
   * with the discontinuities occuring at the PACKET_SIZE boundaries
   * (due to the packetization cost each time another pkt is sent).
   * Each of the coninuous line segments between the discontinuities are
   * linear functions. This routine uses the chi2 fit to find
   * the a, b in the equation y = a + bx of each of the line segments.
   * The average over all a and b for each of the line segments is taken
   * to be the final value of a and b.
   * Remember that y = a + bx is the equation of each linear line
   * segment, and it is NOT the equation of the entire performance graph.
   * To get the correct y coord for a given x coord on the performance
   * graph, the msg_startup cost must be added to a and the packetization
   * cost involved in sending x/PACKET_SIZE pkts must be also be added to
   * a.
   */

  /* allocate space for vectors x, y and sig */
  x   = nrutil_alloc_vector(1, MAX_POINTS);
  y   = nrutil_alloc_vector(1, MAX_POINTS);
  sig = nrutil_alloc_vector(1, MAX_POINTS);
  
  /* get data for a single packet into x[] and y[] */
  k = 0;
  rc = 1;
  xlimit = 0;
  len = strlen(databuf);
  oldcptr = NULL;

  while (k <= len) {
    i = 1;
    xlimit += PACKET_SIZE;
    while (xpoint <= xlimit) {
      rc = sscanf(databuf+k, "%s %*s", point);
      if (rc <= 0) break;
      sscanf(point, "%d,%f", &xpoint, &ypoint);
      x[i] = (float) xpoint;
      y[i] = ypoint * 0.001; /* conversion from msecs -> secs */
      sig[i++] = SPREAD;
      if (xpoint <= xlimit) {
	k += strlen(point) + 1;
      }
    }

    numpackets ++;

    /* use chi-squared fit to find the function that fits this data */
    mwt = 1; /* 0 => ignore std deviations sig[], 1 => take them into account */
    nrutil_fit(x, y, i-1, sig, mwt, &a, &b, &siga, &sigb, &chi2, &q);

    /* store the data */
    cptr = (CommData *) get_mem(sizeof(CommData), "perf estimator");
    cptr->xrange[0] = xlimit-PACKET_SIZE+1;
    cptr->xrange[1] = xlimit;
    cptr->a         = a;
    cptr->b         = b;
    cptr->next      = NULL;

    compute_pkt_costs(a, b, oldcptr, &(commptr->StartCost), &(commptr->pktization));

    if (first_time == 1) {
      first_time = 0;
      commptr->commdata = cptr;
    }
    else {
      oldcptr->next = cptr;
    }
    
    debug("msg size (x range) = %d : %d\n", cptr->xrange[0], cptr->xrange[1]);
    debug("a = %e   siga = %.4f\n", cptr->a, siga);
    debug("b = %e   sigb = %.4f\n", cptr->b, sigb);
    debug("chi2 = %e\n\n", chi2);

    if (rc <= 0 || (k+strlen(point)+1) > (size_t)len)
      break;
    else
      oldcptr = cptr;
    
  }

  /* estimate the StartCost and pktization cost for larger msgs */
  if (numpackets > 0) {
    commptr->pktization = commptr->pktization / (numpackets-1) ;

    // hack to rectify large oscillations in the data for small
    //   message sizes in iSR type communication 

    if (commptr->StartCost < 0.0)
      commptr->StartCost = 0.0;
  }

  /* free allocated space */
  nrutil_free_vector(sig, 1, MAX_POINTS);
  nrutil_free_vector(y, 1, MAX_POINTS);
  nrutil_free_vector(x, 1, MAX_POINTS);

}

/*-------------------------------------------------------------------------
  compute_pkt_costs -- compute the msg startup and packetization costs.
  -------------------------------------------------------------------------*/
static
void compute_pkt_costs(float a, float b, CommData *oldcptr, float *stup, float *pkt)
{
  float  diff;

  if (oldcptr == NULL) {
    *stup = a;
    *pkt  = 0.0;
  }
  else {
    diff = a - oldcptr->a ;
    *pkt += diff;
  }
}
