#include <mh.h>
#include "log.h"

static void print_log_info(model_loop *loop_data,
			   int        loop,
			   FILE       *logfile,
			   char       *buffer)

  {
   model_loop *split_loop;

     fprintf(logfile,"%sloop number %d\n",buffer,loop);
     fprintf(logfile,"%sloop level %d\n",buffer,loop_data[loop].level);
     fprintf(logfile,"%smax unroll %d\n",buffer,loop_data[loop].max);
     fprintf(logfile,"%sedge count %d\n",buffer,loop_data[loop].count);
     fprintf(logfile,"%sunroll amount %d\n",buffer,loop_data[loop].val);
     fprintf(logfile,"%sinitial loop balance %.4f\n",buffer,
	     loop_data[loop].ibalance);
     fprintf(logfile,"%sfinal loop balance %.4f\n",buffer,
	     loop_data[loop].fbalance);
     fprintf(logfile,"%sregisters used %d\n",buffer,loop_data[loop].registers);
     fprintf(logfile,"%sinterlock factor %.4f\n",buffer,loop_data[loop].rho);
     if (loop_data[loop].interchange)
       fprintf(logfile,"%sunroll prevented\n",buffer);
     else
       fprintf(logfile,"%sunroll safe\n",buffer);
     switch(loop_data[loop].type)
       {
	case COMPLEX: fprintf(logfile,"%scomplex loop\n",buffer);
	              break;
	case RECT:    fprintf(logfile,"%srectangular loop\n",buffer);
	              break;
	case TRI_UL:  fprintf(logfile,"%striangular upper left loop\n",buffer);
	              fprintf(logfile,"%scoefficient %d\n",buffer,
			      loop_data[loop].tri_coeff);
	              break;
	case TRI_LL:  fprintf(logfile,"%striangular lower left loop\n",buffer);
	              fprintf(logfile,"%scoefficient %d\n",buffer,
			      loop_data[loop].tri_coeff);
	              break;
	case TRI_UR: fprintf(logfile,"%striangular upper right loop\n",buffer);
	              fprintf(logfile,"%scoefficient %d\n",buffer,
			      loop_data[loop].tri_coeff);
	              break;
	case TRI_LR: fprintf(logfile,"%striangular lower right loop\n",buffer);
	              fprintf(logfile,"%scoefficient %d\n",buffer,
			      loop_data[loop].tri_coeff);
	              break;
	case TRAP:    fprintf(logfile,"%strapezoidal loop\n");
	              switch(loop_data[loop].trap_fn)
		        {
			 case FN_MIN:  fprintf(logfile,"%smin function\n",
					       buffer);
			               break;
			 case FN_MAX:  fprintf(logfile,"%smax function\n",
					       buffer);
			               break;
			 case FN_BOTH: fprintf(logfile,"%sboth functions\n",
					       buffer);
			               break;
			}
       }
  }

static void walk_loops(model_loop *loop_data,
		       int        loop,
		       FILE       *logfile,
		       char       *buffer)

  {
   int next;
   
     print_log_info(loop_data,loop,logfile,buffer);
     if (loop_data[loop].inner_loop != -1)
       walk_loops(loop_data,loop_data[loop].inner_loop,logfile,
		  strcat(buffer,"   "));
     for (next = loop_data[loop].next_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       walk_loops(loop_data,next,logfile,buffer);
  }

void mh_log_data(model_loop *loop_data,
		 FILE       *logfile)

  {
   char buffer[65];

     buffer[0] = '\0';
     walk_loops(loop_data,0,logfile,buffer);
  }        
