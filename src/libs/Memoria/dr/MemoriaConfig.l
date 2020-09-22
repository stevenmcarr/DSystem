%{
/* $Id: MemoriaConfig.l,v 1.5 2001/09/14 16:58:59 carr Exp $ */

/*****************************************************************/
/*                                                               */
/*   File:   mh_config.l                                         */
/*                                                               */
/*   Description: Lex definition to read in the memory           */
/*                hierarchy configuration file                   */
/*                                                               */
/*****************************************************************/

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh_config.h>

static config_type *config;
static char ctemp;

%}

%%

max[ ]regs[ \t]*[0-9]+	 sscanf(&Memoria_text[9],"%d",&config->max_regs);

int[ ]regs[ \t]*[0-9]+	 sscanf(&Memoria_text[9],"%d",&config->int_regs);

pipe[ ]length[ \t]*[0-9]+	 sscanf(&Memoria_text[12],"%d",&config->pipe_length);

beta_m[ \t]*[0-9]*"."[0-9]+      sscanf(&Memoria_text[7],"%f",&config->beta_m);

line[ ]size[ \t]*[0-9]+	 sscanf(&Memoria_text[10],"%d",&config->line);

prefetch[ ]latency[ \t]*[0-9]+	 sscanf(&Memoria_text[17],"%d",&config->prefetch_latency);

prefetch[ ]buffer[ \t]*[0-9]+	 sscanf(&Memoria_text[16],"%d",&config->prefetch_buffer);

double[ ]fetches[ \t]*[0-9]+	 sscanf(&Memoria_text[15],"%d",&config->double_fetches);

double[ ]regs[ \t]*[0-9]+	 sscanf(&Memoria_text[12],"%d",&config->double_regs);

divide[ ]cycles[ \t]*[0-9]+	 sscanf(&Memoria_text[14],"%d",&config->div_cycles);

multiply[ ]cycles[ \t]*[0-9]+	 sscanf(&Memoria_text[16],"%d",&config->mul_cycles);

add[ ]cycles[ \t]*[0-9]+	 sscanf(&Memoria_text[11],"%d",&config->add_cycles);

fp[ ]units[ \t]*[0-9]+	         sscanf(&Memoria_text[9],"%d",&config->FPUnits);

integer[ ]units[ \t]*[0-9]+	 sscanf(&Memoria_text[14],"%d",&config->IntegerUnits);

hit[ ]cycles[ \t]*[0-9]+	 sscanf(&Memoria_text[11],"%d",&config->hit_cycles);

miss[ ]cycles[ \t]*[0-9]+	 sscanf(&Memoria_text[12],"%d",&config->miss_cycles);

instruction[ ]size[ \t]*[0-9]+	 sscanf(&Memoria_text[17],"%d",&config->instruction_size);

Chow[ \t]+[yn]			 {
				  sscanf(&Memoria_text[5],"%s",&ctemp);
				  config->chow_alloc = (ctemp == 'y');
				 }

opt_li[ \t]+[yn]	 	{
				  sscanf(&Memoria_text[7],"%s",&ctemp);
				  config->opt_li_alloc = (ctemp == 'y');
				 }

fma[ \t]+[yn]	 	        {
				  sscanf(&Memoria_text[4],"%c",&ctemp);
				  config->mult_accum = (ctemp == 'y');
				 }

sdiv[ \t]+[yn]	 	        {
				  sscanf(&Memoria_text[5],"%c",&ctemp);
				  config->soft_div= (ctemp == 'y');
				 }

logging[ \t]+[yn]	 	{
                                  sscanf(&Memoria_text[8],"%c",&ctemp);
				  config->logging = (ctemp == 'y');
				 }

aggressive[ \t]+[yn]	 	{
				  sscanf(&Memoria_text[11],"%c",&ctemp);
				  config->aggressive = (ctemp == 'y');
				 }
write[ ]back[ \t]+[yn]	 	{
				  sscanf(&Memoria_text[11],"%c",&ctemp);
				  config->write_back = (ctemp == 'y');
				 }
write[ ]allocate[ \t]+[yn] 	{
				  sscanf(&Memoria_text[15],"%c",&ctemp);
				  config->write_allocate=(ctemp == 'y');
				 }

auto[ ]increment[ \t]+[yn] 	{
				  sscanf(&Memoria_text[15],"%c",&ctemp);
				  config->AutoIncrement=(ctemp == 'y');
				 }

integer[ ]pipe[ ]length[ \t]+[0-9]+  {
                                 sscanf(&Memoria_text[20],"%d",
					&config->IntegerPipeLength);
				}

float[ ]pipe[ ]length[ \t]+[0-9]+    {
                                 sscanf(&Memoria_text[18],"%d",
					&config->FloatPipeLength);
				}

non[ ]blocking[ ]cache[ \t]+[yn] {
				  sscanf(&Memoria_text[19],"%c",&ctemp);
				  config->NonBlockingCache=(ctemp == 'y');
				 }

[\n]		|
.				;

%%

int Memoria_wrap()
{
  return 1;
}

void mh_get_config(NewConfig,filename)

  config_type *NewConfig;
  char        *filename;

  {
   FILE *temp,*fd;
   char fn[80];
   int  ret;
	
              /* default settings */

     NewConfig->max_regs = 8;
     NewConfig->int_regs = 8;
     NewConfig->pipe_length = 0;
     NewConfig->beta_m = 1.0;
     NewConfig->prefetch_latency = 0.0;
     NewConfig->prefetch_buffer = 0.0;
     NewConfig->double_fetches = 2;
     NewConfig->double_regs = 2;
     NewConfig->chow_alloc = 0;
     NewConfig->opt_li_alloc = 0;
     NewConfig->line = 64;
     NewConfig->mult_accum = 0;
     NewConfig->soft_div = 0;
     NewConfig->aggressive = 0;
     NewConfig->logging = 0;
     NewConfig->logfile = NULL;
     NewConfig->div_cycles = 10;
     NewConfig->mul_cycles = 1;
     NewConfig->add_cycles = 1;
     NewConfig->FPUnits = 1;
     NewConfig->IntegerUnits = 1;
     NewConfig->min_flop_cycles = 1;
     NewConfig->hit_cycles = 1;
     NewConfig->miss_cycles = 1;
     NewConfig->write_back= 1;
     NewConfig->write_allocate= 0;
     NewConfig->AutoIncrement= 0;
     NewConfig->IntegerPipeLength= 0;
     NewConfig->FloatPipeLength= 0;
     NewConfig->NonBlockingCache= 0;
     NewConfig->instruction_size = 100;

           /* get configuration file */
     if (filename != NULL)
       if ((fd = fopen(filename,"r")) == NULL)
         return;
       else;
     else
       {
        sprintf(fn,"%s/.Memoriarc",getenv("HOME"));
        if ((fd = fopen(fn,"r")) == NULL)
          {
           sprintf(fn,"%s/.mh_config",getenv("DSYSTEM"));
           if ((fd = fopen(fn,"r")) == NULL)
             return;
          }
       }

         /* read in configuration */
     temp = Memoria_in;
     Memoria_in = fd;
     config = NewConfig;
     ret = Memoria_lex();
     Memoria_in = temp;
     NewConfig->min_flop_cycles = NewConfig->mul_cycles < NewConfig->add_cycles ?
                                 (NewConfig->mul_cycles < NewConfig->div_cycles ?
                                  NewConfig->mul_cycles :
                                  NewConfig->div_cycles):
                                 (NewConfig->add_cycles < NewConfig->div_cycles ?
                                  NewConfig->add_cycles :
                                  NewConfig->div_cycles); 
     fclose(fd);
  }
	
