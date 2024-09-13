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

%token IDENTIFIER STRUCT TYPEDEF IF ELSE WHILE INT_LITERAL STRING_LITERAL I8 I32 VOID
%left '?' ':'
%left '+'
%left '!'

%%

input: expr                 { *root = $1; }
expr : INT_LITERAL          { $$ = ast_literal(@$); }
     | '(' expr ')'         { expand_span($2, @$); $$ = $2; }
     | expr '+' expr        { $$ = ast_binary(@$, BINARY_PLUS, $1, $3); }
     ;

%%

#include <stdlib.h>

void yyerror(YYLTYPE *yyllocp, yyscan_t unused, AstNode *root, const char *msg) {
  fprintf(stderr, "[%d:%d]: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
  exit(EXIT_FAILURE);
}
