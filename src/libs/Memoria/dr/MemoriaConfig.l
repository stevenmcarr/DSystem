%{
/* $Id: MemoriaConfig.l,v 1.1 1997/10/30 15:10:35 carr Exp $ */

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
%}

%%

max[ ]regs[ \t]*[0-9]+	 sscanf(&yytext[9],"%d",&config->max_regs);

int[ ]regs[ \t]*[0-9]+	 sscanf(&yytext[9],"%d",&config->int_regs);

pipe[ ]length[ \t]*[0-9]+	 sscanf(&yytext[12],"%d",&config->pipe_length);

beta_m[ \t]*[0-9]*"."[0-9]+      sscanf(&yytext[7],"%f",&config->beta_m);

line[ ]size[ \t]*[0-9]+	 sscanf(&yytext[10],"%d",&config->line);

prefetch[ ]latency[ \t]*[0-9]+	 sscanf(&yytext[17],"%d",&config->prefetch_latency);

prefetch[ ]buffer[ \t]*[0-9]+	 sscanf(&yytext[16],"%d",&config->prefetch_buffer);

double[ ]fetches[ \t]*[0-9]+	 sscanf(&yytext[15],"%d",&config->double_fetches);

double[ ]regs[ \t]*[0-9]+	 sscanf(&yytext[12],"%d",&config->double_regs);

divide[ ]cycles[ \t]*[0-9]+	 sscanf(&yytext[14],"%d",&config->div_cycles);

multiply[ ]cycles[ \t]*[0-9]+	 sscanf(&yytext[16],"%d",&config->mul_cycles);

add[ ]cycles[ \t]*[0-9]+	 sscanf(&yytext[11],"%d",&config->add_cycles);

hit[ ]cycles[ \t]*[0-9]+	 sscanf(&yytext[11],"%d",&config->hit_cycles);

miss[ ]cycles[ \t]*[0-9]+	 sscanf(&yytext[12],"%d",&config->miss_cycles);

instruction[ ]size[ \t]*[0-9]+	 sscanf(&yytext[17],"%d",&config->instruction_size);

Chow[ \t]+[yn]			 {
				  char temp_str[1];

				  sscanf(&yytext[6],"%s",temp_str);
				  config->chow_alloc = (temp_str[0] == 'y');
				 }

opt_li[ \t]+[yn]	 	{
				  char temp_str[1];

				  sscanf(&yytext[7],"%s",temp_str);
				  config->opt_li_alloc = (temp_str[0] == 'y');
				 }

fma[ \t]+[yn]	 	        {
				  char temp_str[1];

				  sscanf(&yytext[5],"%s",temp_str);
				  config->mult_accum = (temp_str[0] == 'y');
				 }

sdiv[ \t]+[yn]	 	        {
				  char temp_str[1];

				  sscanf(&yytext[6],"%s",temp_str);
				  config->soft_div= (temp_str[0] == 'y');
				 }

logging[ \t]+[yn]	 	{
				  char temp_str[1];

                                  sscanf(&yytext[9],"%s",temp_str);
				  config->logging = (temp_str[0] == 'y');
				 }

aggressive[ \t]+[yn]	 	{
				  char temp_str[1];

				  sscanf(&yytext[11],"%s",temp_str);
				  config->aggressive = (temp_str[0] == 'y');
				 }
write[ ]back[ \t]+[yn]	 	{
				  char temp_str[1];

				  sscanf(&yytext[12],"%s",temp_str);
				  config->write_back = (temp_str[0] == 'y');
				 }
write[ ]allocate[ \t]+[yn] 	{
				  char temp_str[1];

				  sscanf(&yytext[16],"%s",temp_str);
				  config->write_allocate=(temp_str[0] == 'y');
				 }
[\n]		|
.				;

%%

void mh_get_config(config,filename)

  config_type *config;
  char        *filename;

  {
   FILE *temp,*fd;
   char fn[80];
   int  ret;
	
              /* default settings */

     config->max_regs = 8;
     config->int_regs = 8;
     config->pipe_length = 0;
     config->beta_m = 1.0;
     config->prefetch_latency = 0.0;
     config->prefetch_buffer = 0.0;
     config->double_fetches = 2;
     config->double_regs = 2;
     config->chow_alloc = 0;
     config->opt_li_alloc = 0;
     config->line = 64;
     config->mult_accum = 0;
     config->soft_div = 0;
     config->aggressive = 0;
     config->logging = 0;
     config->logfile = NULL;
     config->div_cycles = 10;
     config->mul_cycles = 1;
     config->add_cycles = 1;
     config->min_flop_cycles = 1;
     config->hit_cycles = 1;
     config->miss_cycles = 1;
     config->write_back= 1;
     config->write_allocate= 0;
     config->instruction_size = 100;

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
     temp = yyin;
     yyin = fd;
     ret = mh_yylex(config);
     yyin = temp;
     config->min_flop_cycles = config->mul_cycles < config->add_cycles ?
                                 (config->mul_cycles < config->div_cycles ?
                                  config->mul_cycles :
                                  config->div_cycles):
                                 (config->add_cycles < config->div_cycles ?
                                  config->add_cycles :
                                  config->div_cycles); 
     fclose(fd);
  }
	
