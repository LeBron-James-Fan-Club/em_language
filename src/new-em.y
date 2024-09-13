%define api.pure full
%define api.value.type { AstNode }
%define api.location.type { struct ast_node_span }
%locations
%param { yyscan_t scanner }

%code requires {
  #include "strong_ast.h"

  typedef void *yyscan_t;
}

%code {
  int yylex(YYSTYPE *yylvalp, YYLTYPE *yyllocp, yyscan_t scanner);
  void yyerror(YYLTYPE *yyllocp, yyscan_t unused, AstNode *root, const char *msg);
}

%parse-param { AstNode *root }  /* To pass AST root back to driver */

%token T_ADD_EQ T_SUB_EQ T_MUL_EQ T_DIV_EQ T_REM_EQ T_LOG_OR T_LOG_AND T_EQ T_NE T_LEQ T_GEQ T_SHIFT_LEFT T_SHIFT_RIGHT
%token T_INCREMENT T_DECREMENT T_ARROW T_LITERAL_NUMERIC T_LITERAL_STRING T_VOID T_CHAR T_INT T_PRINT T_INPUT T_PEEK
%token T_POKE T_EXIT T_IF T_ELSE T_LABEL T_GOTO T_WHILE T_FOR T_RETURN T_STRUCT T_UNION T_ENUM T_TYPEDEF T_EXTERN
%token T_BREAK T_CONTINUE T_SWITCH T_CASE T_DEFAULT T_SIZEOF T_STATIC T_IDENTIFIER

%%

input: expr                 { *root = $1; }
expr : T_LITERAL_NUMERIC    { $$ = ast_literal(@$); }
     | '(' expr ')'         { expand_span($2, @$); $$ = $2; }
     | expr '+' expr        { $$ = ast_binary(@$, BINARY_PLUS, $1, $3); }
     ;

%%

#include <stdlib.h>

void yyerror(YYLTYPE *yyllocp, yyscan_t unused, AstNode *root, const char *msg) {
  fprintf(stderr, "[%d:%d]: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
  exit(EXIT_FAILURE);
}
