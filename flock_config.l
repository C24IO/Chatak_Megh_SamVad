%{
	/* Lex specification for Condor config file  */
	#include <string.h>
	#include <stdio.h>
	#include "/usr/include/spawn.h"
	#include <stdlib.h>
	
	FILE *new_config;
	char to_macs[100],from_macs[100];
	
%}

L        [a-zA-Z_]
D        [0-9]
Oct      [0-7]
Hex      [0-9a-fA-F]
RelOp    (>)|(<)|(<=)|(>=)|(=)|(==)|(!=)
ArOp     [+\-\*\/%]
BoolOp   (&)|(\|)|(\|\|)|(&&)|(~)|(!)|(^)|(<<)|(>>)
Brackets [(){}\[\]]
Splchar  [@$ \t\n\*]
NEC	 [^(\*\/)]
Exp	(\*[^\/])|(\/)
HASH      "#"
FT 	"FLOCK_TO"
FF	"FLOCK_FROM"


BL " "
EQ "="


%%

[ |\t|\n]+
 
['#'].*  {}


^{FT}.* {fprintf(new_config,"FLOCK_TO =  %s\n",to_macs);}
^{FF}.* {fprintf(new_config,"FLOCK_FROM =  %s \n",from_macs);}
.*    {fprintf(new_config,"%s\n",yytext);}
%%

main(int argc,char *argv[])
{

	int i , S;
	char *p;
	FILE *fp ;
	
/*^{FT}/(BL)|(EQ).* {fprintf(new_config,"FLOCK_TO  %s\n",to_macs);}
^{FF}/(BL)|(EQ).* {fprintf(new_config,"FLOCK_FROM   %s \n",from_macs);}

the problem is that the above statements do not work and if 
i use what i have used now in the lexer then the old values of the flock 
from and flock to get deleted
*/
	fp = fopen("condor_config" , "r");
	new_config = fopen("N_condor_config","w");
	yyin = fp;
	
/*i have to use the strcat here as this thing here is generating sementation fault. */	
	if(argc > 1)
	{
		if(!strcmp(argv[1],"FLOCK_TO"))
		{
			strcpy(to_macs,argv[2]);
/*			printf("%s %s",argv[1],argv[2]);*/
		}
		else if(!strcmp(argv[1],"FLOCK_FROM"))
		{
			strcpy(from_macs,argv[2]);
/*			printf("%s %s",argv[1],argv[2]);*/
		}
		/*if(!strcmp(argv[3],"FLOCK_TO"))
		{
			strcpy(to_macs,argv[4]);
		}
		else if(!strcmp(argv[3],"FLOCK_FROM"))
		{
			strcpy(from_macs,argv[4]);
		}*/
	}
		
		
	
	yylex();
	fclose(fp);
	fclose(new_config);


	
}

int yywrap()
{
	return 1;
}
