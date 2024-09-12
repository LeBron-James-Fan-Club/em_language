%option nounput noinput noyywrap
%option reentrant
%option prefix="emlexer"

%{
#include <stdio.h>

#include "scan.h"
#include "misc.h"

// todo: make reentrant
int yyival;
%}

DIGIT		[0-9]
IDENTIFIER	[a-zA-Z_][a-zA-Z0-9_]*

%%

[ \t\n]		/* ignore all whitespace */
<<EOF>>		{ return T_EOF; }

"=" 		{ return T_ASSIGN; }
"+="		{ return T_ASPLUS; }
"-="		{ return T_ASMINUS; }
"*="		{ return T_ASSTAR; }
"/="		{ return T_ASSLASH; }
"%="		{ return T_ASMOD; }
"?"		{ return T_QUESTION; }

"||"		{ return T_LOGOR; } /* not supported yet */
"&&"		{ return T_LOGAND; }

"|"		{ return T_OR; }
"^"		{ return T_XOR; }
"&"		{ return T_AMPER; }

"=="		{ return T_EQ; }
"!="		{ return T_NE; }

"<"		{ return T_LT; }
">"		{ return T_GT; }
"<="		{ return T_LT; }
">="		{ return T_GT; }

"<<"		{ return T_LSHIFT; }
">>"		{ return T_RSHIFT; }

"+"		{ return T_PLUS; }
"-"		{ return T_MINUS; }
"*"		{ return T_STAR; }
"/"		{ return T_SLASH; }
"%"		{ return T_MODULO; }

"++"		{ return T_INC; }
"--"		{ return T_DEC; }
"~"		{ return T_INVERT; }
"!"		{ return T_LOGNOT; }

{DIGIT}+	{ yyival = atoi(yytext); return T_INTLIT; } /* todo: hex */

"void"		{ return T_VOID; }
"i8"		{ return T_CHAR; }
"i32"		{ return T_INT; }

"print"		{ return T_PRINT; }
"input"		{ return T_INPUT; }
"peek"		{ return T_PEEK; }
"poke"		{ return T_POKE; }
"exit"		{ return T_EXIT; }

"if"		{ return T_IF; }
"else"		{ return T_ELSE; }
"label"		{ return T_LABEL; }
"goto"		{ return T_GOTO; }
"while"		{ return T_WHILE; }
"for"		{ return T_FOR; }

"return"	{ return T_RETURN; }

"struct"	{ return T_STRUCT; }
"union"		{ return T_UNION; }
"enum"		{ return T_ENUM; }
"typedef"	{ return T_TYPEDEF; }

"extern"	{ return T_EXTERN; }
"break"		{ return T_BREAK; }
"continue"	{ return T_CONTINUE; }

"switch"	{ return T_SWITCH; }
"case"		{ return T_CASE; }

"default"	{ return T_DEFAULT; }
"sizeof"	{ return T_SIZEOF; }
"static"	{ return T_STATIC; }

"\"".+"\""	{ return T_STRLIT; } /* todo */

";"		{ return T_SEMI; }
{IDENTIFIER}	{ return T_IDENT; }

"{"		{ return T_LBRACE; }
"}"		{ return T_RBRACE; }
"("		{ return T_LPAREN; }
")"		{ return T_RPAREN; }
"["		{ return T_LBRACKET; }
"]"		{ return T_RBRACKET; }

","		{ return T_COMMA; }
"."		{ return T_DOT; }
"->"		{ return T_ARROW; }

":"		{ return T_COLON; }

.		{ fprintf(stderr, "Unrecognized character: %s\n", yytext); }

%%

void *em_scanner_new(FILE *in) {
	/* in the reentrant mode, we maintain our
	   own scanner and its associated state */
	int i;
	yyscan_t scanner;

	if ((i = yylex_init(&scanner)) != 0)
		return NULL;

	yyrestart(in, scanner);
	return scanner;
}

struct token em_scanner_next(void *internal) {
	yyscan_t scanner = (yyscan_t) internal;
	enum OPCODES token = yylex(scanner);
	char *tokstr = yyget_text(scanner);
	int intvalue = yyival;
	return (struct token) { token, tokstr, intvalue };
}

void em_scanner_free(void *internal) {
	yyscan_t scanner = (yyscan_t) internal;
	yylex_destroy(scanner);
}
