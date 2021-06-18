typedef struct Symbol {	/* symbol table entry */
	char	*name;
	short	type;
	union {
		double	val;		/* VAR */
		double	(*ptr)();	/* BLTIN */
		int	(*defn)();	/* FUNCTION, PROCEDURE */
		char	*str;		/* STRING */
		double  arr[100];          /* ARRAY */ 
		double mat[100][100]   /* MATRIX */
	} u;
	struct Symbol	*next;	/* to link to another */
} Symbol;
Symbol	*install(), *lookup();

typedef union Datum {	/* interpreter stack type */
	double	val;
	Symbol	*sym;
} Datum;
extern	Datum pop();
extern	eval(), add(), sub(), mul(), div(), negate(), power(), sorting();

typedef int (*Inst)();
#define	STOP	(Inst) 0

extern	Inst *progp, *progbase, prog[], *code();
extern	assign(), assignArr(), assignMat(), bltin(), bltinArr(), varpush(), constpush(), print(), varread();
extern	prexpr(), prstr();
extern	gt(), lt(), eq(), ge(), le(), ne(), and(), or(), not();
extern	ifcode(), whilecode(), forcode(), call(), arg(), argassign();
extern	funcret(), procret();
