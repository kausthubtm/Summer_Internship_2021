%{
#include "hoc.h"
#include "y.tab.h"
extern int lineno;
%}
%%
[ \t]	{ ; }	/* skip blanks and tabs */

[0-9]+\.?|[0-9]*\.[0-9]+ {
	double d;
	sscanf(yytext, "%lf", &d);
	yylval.sym = install("", NUMBER, d); 
	return NUMBER; 
}

[a-zA-Z][a-zA-Z0-9]* {
	Symbol *s;
	if ((s=lookup(yytext)) == 0)
		s = install(yytext, UNDEF, 0.0);
	yylval.sym = s;
	return s->type == UNDEF ? VAR : s->type; 
}

[a-zA-Z][a-zA-Z0-9]*\[ {
	Symbol *s;
	if ((s=lookup(yytext)) == 0)
		s = install(yytext, ARRAY, 0,0);
	yylval.sym = s;
	return s->type == UNDEF ? ARRAY : s->type; 
}

\n	{ lineno++; return '\n'; }   /* everything else */

.	{ return yytext[0]; }

%%

int yywrap () {return -1;}
