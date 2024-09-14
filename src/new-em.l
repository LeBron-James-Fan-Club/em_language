%option noinput nounput noyywrap 8bit nodefault
%option yylineno
%option reentrant bison-bridge bison-locations

%{
  #include <stdlib.h>
  #include <string.h>
  #include "new-em.tab.h"

  #define YY_USER_ACTION                                             \
    yylloc->first_line = yylloc->last_line;                          \
    yylloc->first_column = yylloc->last_column;                      \
    yylloc->start_byte = yylloc->end_byte;                           \
    yylloc->end_byte += yyleng;                                      \
    if (yylloc->last_line == yylineno)                               \
      yylloc->last_column += yyleng;                                 \
    else {                                                           \
      yylloc->last_line = yylineno;                                  \
      yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n'); \
    }
%}

HP		(0[xX])
H		[a-fA-F0-9]
NZ		[1-9]
D		[0-9]
O		[0-7]
ES		(\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
IDENTIFIER	[a-zA-Z_][a-zA-Z0-9_]*

%%

[ \t\n]+			/* ignore all whitespace */

"+="				{ return T_ADD_EQ; }
"-="				{ return T_SUB_EQ; }
"*="				{ return T_MUL_EQ; }
"/="				{ return T_DIV_EQ; }
"%="				{ return T_REM_EQ; }
"||"				{ return T_LOG_OR; }
"&&"				{ return T_LOG_AND; }
"=="				{ return T_EQ; }
"!="				{ return T_NE; }
"<="				{ return T_LEQ; }
">="				{ return T_GEQ; }
"<<"				{ return T_SHIFT_LEFT; }
">>"				{ return T_SHIFT_RIGHT; }
"++"				{ return T_INCREMENT; }
"--"				{ return T_DECREMENT; }
"->"				{ return T_ARROW; }

{HP}{H}+			{ return T_LITERAL_NUMERIC; }
{NZ}{D}*			{ return T_LITERAL_NUMERIC; }
"0"{O}*				{ return T_LITERAL_NUMERIC; }
"'"([^'\\\n]|{ES})+"'"		{ return T_LITERAL_NUMERIC; }
\"([^"\\\n]|{ES})*\"		{ return T_LITERAL_STRING; }

"void"				{ return T_VOID; }
"i8"				{ return T_I8; }
"i32"				{ return T_I32; }
"print"				{ return T_PRINT; }
"input"				{ return T_INPUT; }
"peek"				{ return T_PEEK; }
"poke"				{ return T_POKE; }
"exit"				{ return T_EXIT; }
"if"				{ return T_IF; }
"else"				{ return T_ELSE; }
"label"				{ return T_LABEL; }
"goto"				{ return T_GOTO; }
"while"				{ return T_WHILE; }
"for"				{ return T_FOR; }
"return"			{ return T_RETURN; }
"struct"			{ return T_STRUCT; }
"union"				{ return T_UNION; }
"enum"				{ return T_ENUM; }
"typedef"			{ return T_TYPEDEF; }
"extern"			{ return T_EXTERN; }
"break"				{ return T_BREAK; }
"continue"			{ return T_CONTINUE; }
"switch"			{ return T_SWITCH; }
"case"				{ return T_CASE; }
"default"			{ return T_DEFAULT; }
"sizeof"			{ return T_SIZEOF; }
"static"			{ return T_STATIC; }
{IDENTIFIER}			{ return T_IDENTIFIER; }

.				{ return *yytext; }

%%

AstNode ast_parse(char *source) {
  // this is not the nicest place to put this, but should
  // be foolproof with the Makefile and ordering

  yyscan_t scanner;
  yylex_init(&scanner);

#ifdef DEBUG
  yyset_debug(1, scanner);
  yydebug = 1;
#endif

  yy_switch_to_buffer(yy_scan_string(source, scanner), scanner);
  yyset_lineno(0, scanner);

  AstNode root = NULL;
  yyparse(scanner, &root);

  yylex_destroy(scanner);
  return root;
}
