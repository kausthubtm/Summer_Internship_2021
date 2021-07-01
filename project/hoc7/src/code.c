#include "hoc.h"
#include "y.tab.h"
#include <stdio.h>

#define	NSTACK	256

static Datum stack[NSTACK];	/* the stack */
static Datum *stackp;		/* next free spot on stack */

#define	NPROG	2000
Inst	prog[NPROG];	/* the machine */
Inst	*progp;		/* next free spot for code generation */
Inst	*pc;		/* program counter during execution */
Inst	*progbase = prog; /* start of current subprogram */
int	returning;	/* 1 if return stmt seen */

typedef struct Frame {	/* proc/func call stack frame */
	Symbol	*sp;	/* symbol table entry */
	Inst	*retpc;	/* where to resume after return */
	Datum	*argn;	/* n-th argument on stack */
	int	nargs;	/* number of arguments */
} Frame;
#define	NFRAME	100
Frame	frame[NFRAME];
Frame	*fp;		/* frame pointer */
Symbol* temp;

initcode() {
	progp = progbase;
	stackp = stack;
	fp = frame;
	returning = 0;
}

push(d)
	Datum d;
{
	if (stackp >= &stack[NSTACK])
		execerror("stack too deep", (char *)0);
	*stackp++ = d;
	// printf("pushing\n");
}

Datum pop()
{
	if (stackp == stack)
		execerror("stack underflow", (char *)0);
	// printf("popping\n");
	return *--stackp;
}

constpush()
{
	Datum d;
	d.val = ((Symbol *)*pc++)->u.val;
	// printf("number pushed into stack: %lf\n",d.val);
	push(d);
}

varpush()
{
	Datum d;
	d.sym = (Symbol *)(*pc++);
	// printf("variable pushed into stack: %s\n",d.sym->name);
	push(d);
}

whilecode()
{
	Datum d;
	Inst *savepc = pc;

	execute(savepc+2);	/* condition */
	d = pop();
	while (d.val) {
		// printf("d val before loop : %lf\n", d.val);
		execute(*((Inst **)(savepc)));	/* body */
		if (returning)
			break;

		// printf("d val after loop : %lf\n", d.val);
	}
	if (!returning)
		pc = *((Inst **)(savepc+1)); /* next stmt */
}

forcode()
{
	Datum d;
	Inst *savepc = pc;

	// printf("hello from for loop\n");

	execute(*((Inst **)(savepc+1))); /* init */
	// printf("init instruction\n");

	execute(*((Inst **)(savepc+2)));	/* condition */
	// printf("condition instruction\n");

	d = pop();
	while (d.val) {
		// printf("d val before loop : %lf\n", d.val);
		execute(*((Inst **)(savepc)));	/* body */
		if (returning)
			break;
		execute(*((Inst **)(savepc+3)));
		// printf("inicrement instruction\n");
		execute(*((Inst **)(savepc+2)));	/* condition */
		// printf("condition instruction\n");
		d = pop();
		// printf("d val after loop : %lf\n", d.val);
	}
	if (!returning)
		pc = *((Inst **)(savepc+4)); /* next stmt */
}

uforcode()
{
	Datum d;
	Inst *savepc = pc;

	// printf("hello from for loop\n");

	execute(*((Inst **)(savepc+1))); /* init */
	// printf("init instruction\n");

	execute(*((Inst **)(savepc+2)));	/* condition */
	// printf("condition instruction\n");

	d = pop();
	Symbol *s;
	s = lookup("UNROLL");
	int unroll = (int) s->u.val;
	printf(" times to unroll : %d\n",unroll);

	while (d.val) {
		// printf("d val before loop : %lf\n", d.val);

		for(int i=0; i<unroll; i++){
			execute(*((Inst **)(savepc)));	/* body */
			if (returning)
				break;
			execute(*((Inst **)(savepc+3)));
		}
		// printf("inicrement instruction\n");
		execute(*((Inst **)(savepc+2)));	/* condition */
		// printf("condition instruction\n");
		d = pop();
		// printf("d val after loop : %lf\n", d.val);
	}
	if (!returning)
		pc = *((Inst **)(savepc+4)); /* next stmt */
}

ifcode() 
{
	Datum d;
	Inst *savepc = pc;	/* then part */

	execute(savepc+3);	/* condition */
	d = pop();
	if (d.val)
		execute(*((Inst **)(savepc)));	
	else if (*((Inst **)(savepc+1))) /* else part? */
		execute(*((Inst **)(savepc+1)));
	if (!returning)
		pc = *((Inst **)(savepc+2)); /* next stmt */
}

define(sp)	/* put func/proc in symbol table */
	Symbol *sp;
{
	sp->u.defn = (Inst)progbase;	/* start of code */
	progbase = progp;	/* next code starts here */
}

call() 		/* call a function */
{
	Symbol *sp = (Symbol *)pc[0]; /* symbol table entry */
				      /* for function */
	if (fp++ >= &frame[NFRAME-1])
		execerror(sp->name, "call nested too deeply");
	fp->sp = sp;
	fp->nargs = (int)pc[1];
	fp->retpc = pc + 2;
	fp->argn = stackp - 1;	/* last argument */
	execute(sp->u.defn);
	returning = 0;
}

ret() 		/* common return from func or proc */
{
	int i;
	for (i = 0; i < fp->nargs; i++)
		pop();	/* pop arguments */
	pc = (Inst *)fp->retpc;
	--fp;
	returning = 1;
}

funcret() 	/* return from a function */
{
	Datum d;
	if (fp->sp->type == PROCEDURE)
		execerror(fp->sp->name, "(proc) returns value");
	d = pop();	/* preserve function return value */
	ret();
	push(d);
}

procret() 	/* return from a procedure */
{
	if (fp->sp->type == FUNCTION)
		execerror(fp->sp->name,
			"(func) returns no value");
	ret();
}

double *getarg() 	/* return pointer to argument */
{
	int nargs = (int) *pc++;
	if (nargs > fp->nargs)
	    execerror(fp->sp->name, "not enough arguments");
	return &fp->argn[nargs - fp->nargs].val;
}

arg() 	/* push argument onto stack */
{
	Datum d;
	d.val = *getarg();
	push(d);
}

argassign() 	/* store top of stack in argument */
{
	Datum d;
	d = pop();
	push(d);	/* leave value on stack */
	*getarg() = d.val;
}

bltin() 
{
	Datum d1;
	d1 = pop();
	d1.val = (*(double (*)())*pc++)(d1.val);
	push(d1);
	return;
}

bltinArr()
{
	Datum d1, d2;
	d1 = pop();
	d2 = pop();
	// printf("hello %lf\n", d1.val);

	sorting(d1,d2);



	return ;
}

eval()		/* evaluate variable on stack */
{
	Datum d;
	d = pop();
	if (d.sym->type != VAR && d.sym->type != UNDEF && d.sym->type != ARR && d.sym->type != MAT)
		execerror("attempt to evaluate non-variable", d.sym->name);
	if (d.sym->type == UNDEF)
		execerror("undefined variable", d.sym->name);
	if(d.sym->type == VAR) {
		temp = d.sym; 
		d.val = d.sym->u.val;
		push(d);
		return;
	}
	if(d.sym->type == ARR) {
		Datum index;
		index = pop();
		double index_value = index.val;
		// printf("index : %d \n", (int)index_value);
		// printf("evaluating a array: %s \n", d.sym->name);
		temp = d.sym; 
		d.val = d.sym->u.arr[(int)index.val];
		//array.val = index;
		push(d); 
		return ;
	}
	if(d.sym->type == MAT) {
		Datum iIndex, jIndex;
		jIndex = pop();
		iIndex = pop();
		double iIndex_value = iIndex.val;
		double jIndex_value = jIndex.val;
		d.val = d.sym->u.mat[(int)iIndex_value][(int)jIndex_value];
		push(d);
		return ;
	}
}

add()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val += d2.val;
	push(d1);
}

sub()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val -= d2.val;
	push(d1);
}

mul()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val *= d2.val;
	push(d1);
}

div()
{
	Datum d1, d2;
	d2 = pop();
	if (d2.val == 0.0)
		execerror("division by zero", (char *)0);
	d1 = pop();
	d1.val /= d2.val;
	push(d1);
}

negate()
{
	Datum d;
	d = pop();
	d.val = -d.val;
	push(d);
}

inc() {
	Datum d;
	d = pop();
	d.val = d.val + 1;
	temp->u.val = d.val;
	// printf("incrementing %s \n", temp->name);
	push(d);
}

dec() {
	Datum d;
	d = pop();
	d.val = d.val - 1;
	temp->u.val = d.val;
	push(d);
}

gt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val > d2.val);
	push(d1);
}

lt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	// printf("checking condition : %lf \n", d1.val);
	d1.val = (double)(d1.val < d2.val);
	push(d1);
}

ge()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val >= d2.val);
	push(d1);
}

le()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val <= d2.val);
	push(d1);
}

eq()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val == d2.val);
	push(d1);
}

ne()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val != d2.val);
	push(d1);
}

and()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val != 0.0 && d2.val != 0.0);
	push(d1);
}

or()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val != 0.0 || d2.val != 0.0);
	push(d1);
}

not()
{
	Datum d;
	d = pop();
	d.val = (double)(d.val == 0.0);
	push(d);
}

power()
{
	Datum d1, d2;
	extern double Pow();
	d2 = pop();
	d1 = pop();
	d1.val = Pow(d1.val, d2.val);
	push(d1);
}

sorting(Datum d1, Datum d2)
{
	// printf("hello %lf\n", d1.val);
	// printf("hello %lf\n", d2.val);
	int n = (int) d1.val;
	
	if (temp->type != ARR && temp->type != UNDEF)
		execerror("sort function to a non-array", d1.sym->name);
	
	double* array = temp->u.arr; 

	// printf("%lf \n", arr[1]);
	// for(int i=0; i<4; i++) {
	//  	printf("%lf ", array[i]);
	// }
	// printf("\n");


	for(int i=0; i<n; i++) {
    	int minIndx = i;
    	for(int j=i+1; j < n; j++) {
        	if(array[j] < array[minIndx]) {
            	minIndx = j;
        	}
    	}
    	double temp = array[minIndx];
    	array[minIndx] = array[i];
    	array[i] = temp;
	}
	// printf("sorted array : ");
	// for(int i=0; i<4; i++) {
	// 	printf("%lf ", array[i]);
	// }
	// printf("done \n");
	
	// push(d2);
	return;
}

assign()
{
	Datum d1, d2;
	d1 = pop();
	d2 = pop();
	if (d1.sym->type != VAR && d1.sym->type != UNDEF)
		execerror("assignment to non-variable", d1.sym->name);
	d1.sym->u.val = d2.val;
	d1.sym->type = VAR;
	// printf("assigning a variable : %s \n", d1.sym->name);
	// printf("d2 value : %d \n", d2.val);
	// printf("variable name : %s value : %d \n", d1.sym->name, d1.sym->u.val);
	push(d2);
	return ;
}

assignArr() {
	Datum d1, d2, d3;
	d1 = pop(); // printf("popped variable : %s \n", d1.sym->name);
	d2 = pop(); // printf("popped number : %lf \n", d2.val);
	d3 = pop(); // printf("popped number : %lf \n", d3.val);
	double index = d3.val;
	if (d1.sym->type != ARR && d1.sym->type != UNDEF)
		execerror("assignment to non-variable", d1.sym->name);
	d1.sym->u.arr[(int)index] = d2.val;
	// printf("value : %lf \n", d1.sym->u.arr[(int)index]);
	d1.sym->type = ARR;
	// printf("d2 value : %d \n", d2.val);
	// printf("variable name : %s value : %d \n", d1.sym->name, d1.sym->u.arr[index]);
	push(d2);
	return ;
}

assignMat() {
	Datum d1, d2, d3, d4;
	d1 = pop(); // printf("popped variable : %s \n", d1.sym->name);
	d2 = pop(); // printf("popped number : %lf \n", d2.val);
	d3 = pop(); // printf("popped number : %lf \n", d3.val);
	d4 = pop(); // printf("popped number : %lf \n", d4.val);
	double iIndex = d4.val;
	double jIndex = d3.val;
	if (d1.sym->type != MAT && d1.sym->type != UNDEF)
		execerror("assignment to non-variable", d1.sym->name);
	d1.sym->u.mat[(int)iIndex][(int)jIndex] = d2.val;
	// printf("value : %lf \n", d1.sym->u.arr[(int)index]);
	d1.sym->type = MAT;
	// printf("d2 value : %d \n", d2.val);
	// printf("variable name : %s value : %d \n", d1.sym->name, d1.sym->u.arr[index]);
	push(d2);
	return ;
}

print()	/* pop top value from stack, print it */
{
	if (stackp == stack) {
		return ;
	}
	Datum d;
	d = pop();
	// printf("hello from print");
	printf("\t%.8g\n", d.val);
	return ;
}

prexpr()	/* print numeric value */
{
	Datum d;
	d = pop();
	printf("%.8g ", d.val);
}

prstr()		/* print string value */ 
{
	printf("%s", (char *) *pc++);
}

varread()	/* read into variable */
{
	Datum d;
	extern FILE *fin;
	Symbol *var = (Symbol *) *pc++;
  Again:
	switch (fscanf(fin, "%lf", &var->u.val)) {
	case EOF:
		if (moreinput())
			goto Again;
		d.val = var->u.val = 0.0;
		break;
	case 0:
		execerror("non-number read into", var->name);
		break;
	default:
		d.val = 1.0;
		break;
	}
	var->type = VAR;
	push(d);
}

Inst *code(f)	/* install one instruction or operand */
	Inst f;
{
	Inst *oprogp = progp;
	if (progp >= &prog[NPROG])
		execerror("program too big", (char *)0);
	*progp++ = f;
	// printf("Inst\n");
	return oprogp;
}

execute(p)
	Inst *p;
{
	for (pc = p; *pc != STOP && !returning; ){
		// printf("execute\n");
		(*((++pc)[-1]))();
	}
		
}
