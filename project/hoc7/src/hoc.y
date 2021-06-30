%{
#include "hoc.h"
#include <stdio.h>
#define	code2(c1,c2)	code(c1); code(c2)
#define	code3(c1,c2,c3)	code(c1); code(c2); code(c3)
extern int indef;
%}
%union {
	Symbol	*sym;	/* symbol table pointer */
	Inst	*inst;	/* machine instruction */
	int	narg;	/* number of arguments */
}
%token	<sym>	NUMBER STRING PRINT VAR BLTIN UNDEF WHILE IF ELSE ARR MAT FOR
%token	<sym>	FUNCTION PROCEDURE RETURN FUNC PROC READ
%token	<narg>	ARG
%type	<inst>	expr stmt asgn prlist stmtlist expr2 sortexpr
%type	<inst>	cond while if begin end for tfor cond2
%type	<sym>	procname
%type	<narg>	arglist
%right	'='
%left	OR
%left	AND
%left	GT GE LT LE EQ NE
%left	'+' '-'
%left	'*' '/'
%left	UNARYMINUS NOT INC DEC
%right	'^'
%%
list:	  /* nothing */
	| list '\n'
	| list defn '\n'
	| list asgn '\n'  { code2(pop, STOP); return 1; }
	| list stmt '\n'  { code(STOP); return 1; } 
	| list expr '\n'  { code2(print, STOP); return 1; }
	| list error '\n' { yyerrok; }
	;
asgn:	  VAR '=' expr { code3(varpush,(Inst)$1,assign); $$=$3; }
	| ARR '[' expr ']' '=' expr { code3(varpush,(Inst)$1,assignArr); $$=$6; }
	| MAT '(' expr ',' expr ')' '=' expr { code3(varpush, (Inst)$1, assignMat); $$=$8; }
	| ARG '=' expr
	    { defnonly("$"); code2(argassign,(Inst)$1); $$=$3;}
	;
stmt:	  expr	{ code(pop); }
	| RETURN { defnonly("return"); code(procret); }
	| RETURN expr
	        { defnonly("return"); $$=$2; code(funcret); }
	| PROCEDURE begin '(' arglist ')'
		{ $$ = $2; code3(call, (Inst)$1, (Inst)$4); }
	| PRINT prlist	{ $$ = $2; }
	| for '(' expr2 ';' cond2 ';' expr2 ')' stmt end {
		($1)[1] = (Inst)$9;	/* body of loop */
		($1)[2] = (Inst)$3;   /* init */
		($1)[3] = (Inst)$5;   /*condition*/
		($1)[4] = (Inst)$7; /* increment or decrement */ 
		($1)[5] = (Inst)$10; }	/* end, if cond fails */
	| while cond stmt end {
		($1)[1] = (Inst)$3;	/* body of loop */
		($1)[2] = (Inst)$4; }	/* end, if cond fails */
	| if cond stmt end {	/* else-less if */
		($1)[1] = (Inst)$3;	/* thenpart */
		($1)[3] = (Inst)$4; }	/* end, if cond fails */
	| if cond stmt end ELSE stmt end {	/* if with else */
		($1)[1] = (Inst)$3;	/* thenpart */
		($1)[2] = (Inst)$6;	/* elsepart */
		($1)[3] = (Inst)$7; }	/* end, if cond fails */
	| '{' stmtlist '}'	{ $$ = $2; }
	;
cond:	  '(' expr ')'	{ code(STOP); $$ = $2; }
	;
cond2:  expr   { code(STOP); $$ = $1; }
	;
for: 	FOR	{ $$ = code3(forcode,STOP,STOP); code3(STOP,STOP,STOP); }
	;
while:	  WHILE	{ $$ = code3(whilecode,STOP,STOP); }
	;
if:	  IF	{ $$ = code(ifcode); code3(STOP,STOP,STOP); }
	;
begin:	  /* nothing */		{ $$ = progp; }
	;
end:	  /* nothing */		{ code(STOP); $$ = progp; }
	;
stmtlist: /* nothing */		{ $$ = progp; }
	| stmtlist '\n'
	| stmtlist stmt
	;
expr:	  NUMBER { $$ = code2(constpush, (Inst)$1); }
	| VAR	 { $$ = code3(varpush, (Inst)$1, eval); }
	| ARR '[' expr ']' {$$ = code3(varpush, (Inst)$1, eval); }
	| MAT '(' expr ',' expr ')' {$$ = code3(varpush, (Inst)$1, eval); }
	| ARG	 { defnonly("$"); $$ = code2(arg, (Inst)$1); }
	| asgn
	| FUNCTION begin '(' arglist ')'
		{ $$ = $2; code3(call,(Inst)$1,(Inst)$4); }
	| READ '(' VAR ')' { $$ = code2(varread, (Inst)$3); }
	| BLTIN '(' expr ')' { $$=$3; code2(bltin, (Inst)$1->u.ptr); }
	| BLTIN '(' sortexpr ')' { $$=$3; code(bltinArr);  }
	| '(' expr ')'	{ $$ = $2; }
	| expr '+' expr	{ code(add); }
	| expr '-' expr	{ code(sub); }
	| expr '*' expr	{ code(mul); }
	| expr '/' expr	{ code(div); }
	| expr '^' expr	{ code (power); }
	| '-' expr   %prec UNARYMINUS   { $$=$2; code(negate); }
	| expr GT expr	{ code(gt); }
	| expr GE expr	{ code(ge); }
	| expr LT expr	{ code(lt); }
	| expr LE expr	{ code(le); }
	| expr EQ expr	{ code(eq); }
	| expr NE expr	{ code(ne); }
	| expr AND expr	{ code(and); }
	| expr OR expr	{ code(or); }
	| NOT expr	{ $$ = $2; code(not); }
	| DEC expr  { $$ = $2; code(dec); }
	| INC expr  { $$ = $2; code(inc); }
	| expr DEC  { $$ = $1; code(dec); }
	| expr INC  { $$ = $1; code(inc); }
	;
expr2: /* nothing */
	| expr			{ code(STOP); $$ = $1; }
	;
expr3: /* nothing */
	| expr		
	;
sortexpr: expr ',' expr 
	;
prlist:	  expr			{ code(prexpr); }
	| STRING		{ $$ = code2(prstr, (Inst)$1); }
	| prlist ',' expr	{ code(prexpr); }
	| prlist ',' STRING	{ code2(prstr, (Inst)$3); }
	;
defn:	  FUNC procname { $2->type=FUNCTION; indef=1; }
	    '(' ')' stmt { code(procret); define($2); indef=0; }
	| PROC procname { $2->type=PROCEDURE; indef=1; }
	    '(' ')' stmt { code(procret); define($2); indef=0; }
	;
procname: VAR
	| FUNCTION
	| PROCEDURE
	;
arglist:  /* nothing */ 	{ $$ = 0; }
	| expr			{ $$ = 1; }
	| arglist ',' expr	{ $$ = $1 + 1; }
	;
%%
	/* end of grammar */
#include <stdio.h>
#include <ctype.h>
char	*progname;
int	lineno = 1;
#include <signal.h>
#include <setjmp.h>
jmp_buf	begin;
int	indef;
char	*infile;	/* input file name */
FILE	*fin;		/* input file pointer */
char	**gargv;	/* global argument list */
int	gargc;

int c;	/* global for use by warning() */

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

yylex()		/* hoc7 */
{


	while ((c=getc(fin)) == ' ' || c == '\t')                                            /* remove whitespaces */
		;


	if (c == EOF)									                                    /* EOF */
		return 0;


	if (c == '.' || isdigit(c)) {	                                                    /* number */
		double d;
		ungetc(c, fin);
		fscanf(fin, "%lf", &d);
		yylval.sym = install("", NUMBER, d);
		return NUMBER;
	}



	if (isalpha(c)) {                                                                   /* variable */
		Symbol *s;
		char sbuf[100], *p = sbuf;
		do {
			if (p >= sbuf + sizeof(sbuf) - 1) {
				*p = '\0';
				execerror("name too long", sbuf);
			}
			*p++ = c;
		} while ((c=getc(fin)) != EOF && isalnum(c));

		if(c == '['){
			ungetc(c, fin);
			*p = '\0';
			if ((s=lookup(sbuf)) == 0)
				s = install(sbuf, UNDEF, 0.0);
			yylval.sym = s;
			return s->type == UNDEF ? ARR : s->type;
		}
		if(c == '('){
			ungetc(c, fin);
			*p = '\0';
			if ((s=lookup(sbuf)) == 0)
				s = install(sbuf, UNDEF, 0.0);
			yylval.sym = s;
			return s->type == UNDEF ? MAT : s->type;
		}
		else {
			ungetc(c, fin);
			*p = '\0';
			if ((s=lookup(sbuf)) == 0)
				s = install(sbuf, UNDEF, 0.0);
			yylval.sym = s;
			return s->type == UNDEF ? VAR : s->type;
		}
	}

	if(c == '[' || c == ']') return c;


	if (c == '$') {	                                                                    /* argument? */
		int n = 0;
		while (isdigit(c=getc(fin)))
			n = 10 * n + c - '0';
		ungetc(c, fin);
		if (n == 0)
			execerror("strange $...", (char *)0);
		yylval.narg = n;
		return ARG;
	}


	if (c == '"') {	                                                                   /* quoted string */
		char sbuf[100], *p, *emalloc();
		for (p = sbuf; (c=getc(fin)) != '"'; p++) {
			if (c == '\n' || c == EOF)
				execerror("missing quote", "");
			if (p >= sbuf + sizeof(sbuf) - 1) {
				*p = '\0';
				execerror("string too long", sbuf);
			}
			*p = backslash(c);
		}
		*p = 0;
		yylval.sym = (Symbol *)emalloc(strlen(sbuf)+1);
		strcpy(yylval.sym, sbuf);
		return STRING;
	}
	


	switch (c) {                                                                     /* comparison operators */
	case '>':	return follow('=', GE, GT);
	case '<':	return follow('=', LE, LT);
	case '=':	return follow('=', EQ, '=');
	case '!':	return follow('=', NE, NOT);
	case '|':	return follow('|', OR, '|');
	case '&':	return follow('&', AND, '&');
	case '-':   return follow('-', DEC, '-');
	case '+':   return follow('+', INC, '+');
	case '\n':	lineno++; return '\n';
	default:	return c;
	}

}

backslash(c)	                                                        /* get next char with \'s interpreted */
	int c;
{
	char *index();	/* `strchr()' in some systems */
	static char transtab[] = "b\bf\fn\nr\rt\t";
	if (c != '\\')
		return c;
	c = getc(fin);
	if (islower(c) && index(transtab, c))
		return index(transtab, c)[1];
	return c;
}

follow(expect, ifyes, ifno)	                                              /* look ahead for >=, etc. */
{
	int c = getc(fin);

	if (c == expect)
		return ifyes;
	ungetc(c, fin);
	return ifno;
}

defnonly(s)	                                                            /* warn if illegal definition */
	char *s;
{
	if (!indef)
		execerror(s, "used outside definition");
}

yyerror(s)	                                                           /* report compile-time error */
	char *s;
{
	warning(s, (char *)0);
}

execerror(s, t)	                                                       /* recover from run-time error */
	char *s, *t;
{
	warning(s, t);
	fseek(fin, 0L, 2);		                                           /* flush rest of file */
	longjmp(begin, 0);
}

fpecatch()	                                                           /* catch floating point exceptions */
{
	execerror("floating point exception", (char *) 0);
}

main(argc, argv)	/* hoc7 */
	char *argv[];
{
	int i, fpecatch();

	progname = argv[0];
	if (argc == 1) {	/* fake an argument list */
		static char *stdinonly[] = { "-" };

		gargv = stdinonly;
		gargc = 1;
	} else {
		gargv = argv+1;
		gargc = argc-1;
	}
	init();
	while (moreinput()) 
		run();
	return 0;
}

moreinput()
{
	if (gargc-- <= 0)
		return 0;
	if (fin && fin != stdin)
		fclose(fin);
	infile = *gargv++;
	lineno = 1;
	if (strcmp(infile, "-") == 0) {
		fin = stdin;
		infile = 0;
	} else if ((fin=fopen(infile, "r")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", progname, infile);
		return moreinput();
	}
	return 1;
}

run()	/* execute until EOF */
{
	setjmp(begin);
	signal(SIGFPE, fpecatch);
	for (initcode(); yyparse(); initcode()){
		/* printf("hi\n"); */
		execute(progbase);
	}
		
}

warning(s, t)	/* print warning message */
	char *s, *t;
{
	fprintf(stderr, "%s: %s", progname, s);
	if (t)
		fprintf(stderr, " %s", t);
	if (infile)
		fprintf(stderr, " in %s", infile);
	fprintf(stderr, " near line %d\n", lineno);
	while (c != '\n' && c != EOF)
		c = getc(fin);	/* flush rest of input line */
	if (c == '\n')
		lineno++;
}
