%define api.pure full
%define api.value.type { AstNode }
%define api.location.type { struct ast_node_span }
%locations
%param { yyscan_t scanner }

%code requires {
  #include "strong_ast.h"

  # define YYLLOC_DEFAULT(Cur, Rhs, N)                      \
  do                                                        \
    if (N)                                                  \
      {                                                     \
        (Cur).first_line   = YYRHSLOC(Rhs, 1).first_line;   \
        (Cur).first_column = YYRHSLOC(Rhs, 1).first_column; \
        (Cur).last_line    = YYRHSLOC(Rhs, N).last_line;    \
        (Cur).last_column  = YYRHSLOC(Rhs, N).last_column;  \
        (Cur).start_byte   = YYRHSLOC(Rhs, 1).start_byte;   \
        (Cur).end_byte     = YYRHSLOC(Rhs, N).end_byte;     \
      }                                                     \
    else                                                    \
      {                                                     \
        (Cur).first_line   = (Cur).last_line   =            \
          YYRHSLOC(Rhs, 0).last_line;                       \
        (Cur).first_column = (Cur).last_column =            \
          YYRHSLOC(Rhs, 0).last_column;                     \
        (Cur).start_byte = (Cur).end_byte =                 \
          YYRHSLOC(Rhs, 0).end_byte;                        \
      }                                                     \
  while (0)

  typedef void *yyscan_t;
}

%code {
  #define NULL ((void *) 0)

  int yylex(YYSTYPE *yylvalp, YYLTYPE *yyllocp, yyscan_t scanner);
  void yyerror(YYLTYPE *yyllocp, yyscan_t unused, AstNode *root, const char *msg);
}

%parse-param { AstNode *root }  /* To pass AST root back to driver */

%token T_ADD_EQ T_SUB_EQ T_MUL_EQ T_DIV_EQ T_REM_EQ T_LOG_OR T_LOG_AND T_EQ T_NE T_LEQ T_GEQ T_SHIFT_LEFT T_SHIFT_RIGHT
%token T_INCREMENT T_DECREMENT T_ARROW T_LITERAL_NUMERIC T_LITERAL_STRING T_VOID T_I8 T_I32 T_PRINT T_INPUT T_PEEK
%token T_POKE T_EXIT T_IF T_ELSE T_LABEL T_GOTO T_WHILE T_FOR T_RETURN T_STRUCT T_UNION T_ENUM T_TYPEDEF T_EXTERN
%token T_BREAK T_CONTINUE T_SWITCH T_CASE T_DEFAULT T_SIZEOF T_STATIC T_IDENTIFIER

%right T_ADD_EQ T_SUB_EQ T_MUL_EQ T_DIV_EQ T_REM_EQ '='
%right '?' ':'
%left T_LOG_OR
%left T_LOG_AND
%left '|'
%left '^'
%left '&'
%left  T_SHIFT_LEFT T_SHIFT_RIGHT
%left '+' '-'
%left '*' '/'
%left T_NE T_EQ
%left T_GEQ T_LEQ
%left '<' '>'
%nonassoc '(' ')'

%initial-action             { yylloc = (struct ast_node_span) { 1, 1, 1, 1, 0, 0 }; }

%%

grammar: %empty				{ *root = $$ = ast_list_new(@$, AST_ALL); }
       | grammar globalDeclaration	{ *root = ast_list_expand(@$, $1, $2); }
       ;

globalDeclaration: variableDeclaration	{ $$ = ast_expand(@$, $1); }
                 | functionDeclaration	{ $$ = ast_expand(@$, $1); }
                 | typeDeclaration	{ $$ = ast_expand(@$, $1); }
                 ;

variableDeclaration: type T_IDENTIFIER ';'			{ $$ = ast_variable_declaration(@$, $1, $2, NULL); }
                   | type T_IDENTIFIER '=' expression ';'	{ $$ = ast_variable_declaration(@$, $1, $2, $4); }
                   ;

functionDeclaration: type T_IDENTIFIER '(' parameterDeclarationList ')' ';'	{ $$ = ast_function_declaration(@$, $1, $2, $4, NULL); }
                   | type T_IDENTIFIER '(' parameterDeclarationList ')' block	{ $$ = ast_function_declaration(@$, $1, $2, $4, $6); }
                   ;

typeDeclaration: T_STRUCT T_IDENTIFIER '{' structMembers '}' ';'	{ $$ = ast_struct_declaration(@$, $2, $4); }
               | T_TYPEDEF type T_IDENTIFIER ';'			{ $$ = ast_type_name_pair(@$, $2, $3); }
               ;

structMembers: %empty				{ $$ = ast_list_new(@$, AST_ALL); }
             | structMembers structMember	{ $$ = ast_list_expand(@$, $1, $2); }
             ;

structMember: type T_IDENTIFIER ';'	{ $$ = ast_type_name_pair(@$, $1, $2); }
            ;

parameterDeclarationList: %empty		{ $$ = ast_list_new(@$, AST_ALL); }
                        | parameterDeclarations	{ $$ = ast_expand(@$, $1); }
                        ;

parameterDeclarations: parameterDeclaration				{ $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
                     | parameterDeclarations ',' parameterDeclaration	{ $$ = ast_list_expand(@$, $1, $3); }
                     ;

parameterDeclaration: type T_IDENTIFIER	{ $$ = ast_type_name_pair(@$, $1, $2); }
                    ;

block: '{' statements '}'	{ $$ = ast_block(@$, $2); }
     ;

statements: %empty			{ $$ = ast_list_new(@$, AST_ALL); }
          | statements statement	{ $$ = ast_list_expand(@$, $1, $2); }
          ;

statement: expression ';'					{ $$ = ast_expand(@$, $1); }
         | T_IF '(' expression ')' statement			{ $$ = ast_if_statement(@$, $3, $5, NULL); }
         | T_IF '(' expression ')' statement T_ELSE statement	{ $$ = ast_if_statement(@$, $3, $5, $7); }
         | T_WHILE '(' expression ')' statement			{ $$ = ast_while_statement(@$, $3, $5); }
         | block						{ $$ = ast_expand(@$, $1); }
         | type T_IDENTIFIER ';'				{ $$ = ast_variable_declaration(@$, $1, $2, NULL); }
         | type T_IDENTIFIER '=' expression ';'			{ $$ = ast_variable_declaration(@$, $1, $2, $4); }
         | T_IDENTIFIER '=' expression ';'			{ $$ = ast_assignment(@$, $1, $3); }
         ;

expression: '(' expression ')'					{ $$ = ast_expand(@$, $1); }
          | literal						{ $$ = ast_literal_expression(@$, $1, NULL); }
          | literal '[' expression ']'				{ $$ = ast_literal_expression(@$, $1, $3); }
          | T_IDENTIFIER					{ $$ = ast_identifier(@$); }
          | expression '+' expression				{ $$ = ast_binary_operator(@$, '+', $1, $3); }
          | '!' expression					{ $$ = ast_unary_operator(@$, '!', $2); }
          | expression '?' expression ':' expression		{ $$ = ast_ternary_operator(@$, $1, $3, $5); }
          | expression '(' parameterList ')'			{ $$ = ast_invocation(@$, $1, $3); }
          ;

parameterList: %empty						{ $$ = ast_list_new(@$, AST_ALL); }
             | parameters					{ $$ = ast_expand(@$, $1); }
             ;

parameters: expression						{ $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
          | parameters ',' expression				{ $$ = ast_list_expand(@$, $1, $3); }
          ;

literal: T_LITERAL_NUMERIC					{ $$ = ast_literal(@$, LITERAL_NUMERIC); }
       | T_LITERAL_STRING					{ $$ = ast_literal(@$, LITERAL_STRING); }
       ;

type: T_I32			{ $$ = NULL; }
    | T_I8			{ $$ = NULL; }
    | T_VOID			{ $$ = NULL; }
    | T_IDENTIFIER		{ $$ = NULL; }
    | type '*'			{ $$ = NULL; }
    | type '[' expression ']'	{ $$ = NULL; }
    ;

%%

#include <stdio.h>
#include <stdlib.h>

void yyerror(YYLTYPE *yyllocp, yyscan_t unused, AstNode *root, const char *msg) {
  fprintf(stderr, "[%d:%d]: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
  exit(EXIT_FAILURE);
}
