%{
	/* Lex specification for parsing the host_list  file */
	#include <string.h>
	#include <stdio.h>
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

IP [DDD]
BL " "
EQ "="


%%

[ |\t|\n]+
 
['#'].*  {}


{D}+\.{D}+\.{D}+\.{D}+ {printf("%s",yytext);}

.*    {}
%%

main(int argc,char *argv[])
{

	int i , S;
	char *p;
	FILE *fp ;
	
        fp = fopen("host_list" , "r");

	yyin = fp;


	yylex();
	fclose(fp);

/*I have to construct the lex pattern for 255.255.255.255 and also 10.1.30.2 i.e. the host names in IPV4 form*/
	
}

int yywrap()
{
	return 1;
}
