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
%token T_INCREMENT T_DECREMENT T_ARROW T_LITERAL_NUMERIC T_LITERAL_STRING T_VOID T_I8 T_I32 T_PRINT T_INPUT T_PEEK
%token T_POKE T_EXIT T_IF T_ELSE T_LABEL T_GOTO T_WHILE T_FOR T_RETURN T_STRUCT T_UNION T_ENUM T_TYPEDEF T_EXTERN
%token T_BREAK T_CONTINUE T_SWITCH T_CASE T_DEFAULT T_SIZEOF T_STATIC T_IDENTIFIER

%%

grammar: %empty				{ *root = ast_list_new(@$, AST_ALL); }
       | grammar globalDeclaration	{ *root = ast_list_expand(@$, $1, $2); }
       ;

globalDeclaration: variableDeclaration	{ $$ = ast_expand(@$, $1); }
                 | functionDeclaration	{ $$ = ast_expand(@$, $1); }
                 | typeDeclaration	{ $$ = ast_expand(@$, $1); }
                 ;

variableDeclaration: type T_IDENTIFIER				{ $$ = ast_variable_declaration(@$, $1, $2, NULL); }
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

block: '{' statements '}'	{ $2->span = @$; $$ = $2; }
     ;

statements: %empty			{ $$ = ast_list_new(@$, AST_ALL); }
          | statement statements	{ $$ = ast_list_expand(@$, $1, $2); }
          ;

statement: expression ';'					{ $$ = NULL; }
         | T_IF '(' expression ')' statement			{ $$ = NULL; }
         | T_IF '(' expression ')' statement T_ELSE statement	{ $$ = NULL; }
         | T_WHILE '(' expression ')' statement			{ $$ = NULL; }
         | block						{ $$ = NULL; }
         | type T_IDENTIFIER					{ $$ = NULL; }
         | type T_IDENTIFIER '=' expression			{ $$ = NULL; }
         | T_IDENTIFIER '=' expression ';'			{ $$ = NULL; }
         ;

expression: literal						{ $$ = NULL; }
          | T_IDENTIFIER					{ $$ = NULL; }
          | expression '+' expression				{ $$ = NULL; }
          | '!' expression					{ $$ = NULL; }
          | expression '?' expression ':' expression		{ $$ = NULL; }
          | T_IDENTIFIER '(' ')'				{ $$ = NULL; }
          | T_IDENTIFIER '(' parameterList ')'			{ $$ = NULL; }
          ;

parameterList: expression					{ $$ = NULL; }
             | expression ',' parameterList			{ $$ = NULL; }
             ;

literal: anonymousLiteral					{ $$ = NULL; }
       | anonymousLiteral '[' T_IDENTIFIER ']'			{ $$ = NULL; }
       ;

anonymousLiteral: T_LITERAL_NUMERIC				{ $$ = NULL; }
                | T_LITERAL_STRING				{ $$ = NULL; }
                ;

type: T_I32			{ $$ = NULL; }
    | T_I8			{ $$ = NULL; }
    | T_VOID			{ $$ = NULL; }
    | T_IDENTIFIER		{ $$ = NULL; }
    | type '*'			{ $$ = NULL; }
    | type '[' expression ']'	{ $$ = NULL; }
    ;

%%

#include <stdlib.h>

void yyerror(YYLTYPE *yyllocp, yyscan_t unused, AstNode *root, const char *msg) {
  fprintf(stderr, "[%d:%d]: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
  exit(EXIT_FAILURE);
}
