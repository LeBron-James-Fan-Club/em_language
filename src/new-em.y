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
%token T_POKE T_EXIT T_IF T_ELSE T_LABEL T_GOTO T_WHILE T_FOR T_RETURN T_STRUCT T_UNION T_ENUM T_TYPEDEF T_EXTERN T_CONST
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
%left T_ARROW '.' T_INCREMENT T_DECREMENT
/*%right '*'*/
/*%right '&'*/
%nonassoc '[' ']'
%nonassoc '(' ')'

// fix?
%nonassoc PREC_IF
%nonassoc T_ELSE

%initial-action             { yylloc = (struct ast_node_span) { 1, 1, 1, 1, 0, 0 }; }

%%

grammar: %empty				{ *root = $$ = ast_list_new(@$, AST_ALL); }
       | grammar globalDeclaration	{ *root = ast_list_expand(@$, $1, $2); }
       ;

globalDeclaration: variableDeclaration	{ $$ = ast_expand(@$, $1); }
                 | functionDeclaration	{ $$ = ast_expand(@$, $1); }
                 | typeDeclaration	{ $$ = ast_expand(@$, $1); }
                 ;

// TODO: Add pointers and array decls
//variableDeclaration: typeSpecifier declarator ';'			{ $$ = ast_variable_declaration(@$, $1, $2, NULL); }
//                   | typeSpecifier declarator '=' expression ';'	{ $$ = ast_variable_declaration(@$, $1, $2, $4); }
//                   ;

functionDeclaration: typeSpecifier declarator '(' parameterDeclarationList ')' ';'	{ $$ = ast_function_declaration(@$, $1, $2, $4, NULL); }
                   | typeSpecifier declarator '(' parameterDeclarationList ')' block	{ $$ = ast_function_declaration(@$, $1, $2, $4, $6); }
                   ;

typeDeclaration: T_STRUCT T_IDENTIFIER '{' structMembers '}' ';'	{ $$ = ast_struct_declaration(@$, $2, $4); }
               | T_ENUM   T_IDENTIFIER '{' enumMembers   '}' ';'	{ $$ = ast_enum_declaration(@$, $2, $4); }
               | T_TYPEDEF typeSpecifier declarator ';'			{ $$ = ast_type_name_pair(@$, $2, $3, NULL); }
               ;

parameterDeclarationList: %empty		{ $$ = ast_list_new(@$, AST_ALL); }
                        | parameterDeclarations	{ $$ = ast_expand(@$, $1); }
                        ;

parameterDeclarations: parameterDeclaration				{ $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
                     | parameterDeclarations ',' parameterDeclaration	{ $$ = ast_list_expand(@$, $1, $3); }
                     ;

// before it was T_IDENTIFIER, i presumed that since T_IDENTIFIER didnt return as ASTnode it would
// cause an error, hence why im using identifier
parameterDeclaration: typeSpecifier identifier	{ $$ = ast_type_name_pair(@$, $1, $2); }
                    ;

blockItemList: blockItem			{ $$ = ast_list_add(@$, ast_list_new(@$, AST_ALL), $1); }
            | blockItemList blockItem	{ $$ = ast_list_expand(@$, $1, $2); }
            ;

blockItem: declaration  { $$ = ast_expand(@$, $1); }
         | statement	{ $$ = ast_expand(@$, $1); }
         ;

statement: labelledStatement					    { $$ = ast_expand(@$, $1); }
         | compoundStatement						  { $$ = ast_expand(@$, $1); }
         | expressionStatement            { $$ = ast_expand(@$, $1); }
         | iterationStatement             { $$ = ast_expand(@$, $1); }
         | jumpStatement                  { $$ = ast_expand(@$, $1); }
         ;

// TODO: remove the label keyword
labelledStatement: identifier ':' statement { $$ = ast_label_statement(@$, $1, $3); }
                 | T_CASE expression ':' statement { $$ = ast_case_statement(@$, $2, $4); }

compoundStatement: '{' '}'                { $$ = ast_block(@$, NULL); }
                 | '{' blockItemList '}'	{ $$ = ast_block(@$, $2); }
                 ;

expressionStatement: ';' { $$ = NULL; } 
                   | expression ';'	{ $$ = ast_expand(@$, $1); }
                   ;

// * need to figure out how to work on the , for the if statements
selectionStatement: T_IF '(' expression ')' statement T_ELSE statement	{ $$ = ast_if_statement(@$, $3, $5, $7); }
                  | T_IF '(' expression ')' statement		{ $$ = ast_if_statement(@$, $3, $5, NULL); }
                  | SWITCH '(' expression ')' statement		{ $$ = ast_switch_statement(@$, $3, $5); }
                  ;

iterationStatement: T_WHILE '(' expression ')' statement			{ $$ = ast_while_statement(@$, $3, $5); }
                  | T_FOR '(' expressionStatement expressionStatement expression ')' statement	{ $$ = ast_for_statement(@$, $3, $4, $5, $7); }

jumpStatement: T_GOTO identifier ';' { $$ = ast_goto_statement(@$, $2); }
             | T_CONTINUE ';' { $$ = ast_continue_statement(@$); }
             | T_BREAK ';' { $$ = ast_break_statement(@$); }
             | T_RETURN ';' { $$ = ast_return_statement(@$, NULL); }
             | T_RETURN expression ';' { $$ = ast_return_statement(@$, $2); }
             ;
// Implement cast to array later on
postfixExpression: primary
       | postfixExpression '[' expression ']'		  { $$ = ast_binary_operator(@$, ARRAY_ACCESS, $1, $3); }
       | postfixExpression '(' parameterList ')'	{ $$ = ast_invocation(@$, $1, $3); }
       | postfixExpression '.' T_IDENTIFIER			  { $$ = ast_binary_operator(@$, MEMBER_ACCESS, $1, $3); }
       | postfixExpression T_ARROW T_IDENTIFIER		{ $$ = ast_binary_operator(@$, POINTER_MEMBER_ACCESS, $1, $3); }
       | postfixExpression T_INCREMENT			      { $$ = ast_unary_operator(@$, POST_INCREMENT, $1); }
       | postfixExpression T_DECREMENT			      { $$ = ast_unary_operator(@$, POST_DECREMENT, $1); }
       ;

unaryExpression: postfixExpression
        | T_INCREMENT unaryExpression	                  { $$ = ast_unary_operator(@$, PRE_INCREMENT, $2); }
        | T_DECREMENT unaryExpression	                  { $$ = ast_unary_operator(@$, PRE_DECREMENT, $2); }
        | unaryOperator unaryExpression                 { $$ = ast_unary_operator(@$, $1, $2); }
        | T_SIZEOF '(' pointerTypeSpecifier ')'	            { $$ = ast_unary_operator(@$, SIZEOF, $3); }
        | T_PEEK '(' expression ')'			                { $$ = ast_unary_operator(@$, PEEK, $3); }
        ;

pointerTypeSpecifier: pointer typeSpecifier { $$ = } // ! not implemented yet
  
unaryOperator: '&' { $$ = $1 }
             | '*' { $$ = $1 }
             | '+' { $$ = $1 }
             | '-' { $$ = $1 }
             | '~' { $$ = $1 }
             | '!' { $$ = $1 }
             ;

identifier: T_IDENTIFIER { $$ = ast_identifier(@$); }
          ;

primary: T_IDENTIFIER { $$ = ast_identifier(@$); }
        | literal { $$ = ast_literal_expression(@$, $1, NULL); }
        | '(' expression ')' { $$ = ast_expand(@$, $1); }
        ;

// TODO: Support pointer casting later on
castExpression: unaryExpression
              | '(' pointerTypeSpecifier ')' castExpression { $$ = ast_cast_expression(@$, $2, $4); } // ! not implemented yet
              ;

multiplicativeExpression: castExpression
                        | multiplicativeExpression '*' castExpression	{ $$ = ast_binary_operator(@$, '*', $1, $3); }
                        | multiplicativeExpression '/' castExpression	{ $$ = ast_binary_operator(@$, '/', $1, $3); }
                        | multiplicativeExpression '%' castExpression	{ $$ = ast_binary_operator(@$, '%', $1, $3); }
                        ;

additiveExpression: multiplicativeExpression
                  | additiveExpression '+' multiplicativeExpression	{ $$ = ast_binary_operator(@$, '+', $1, $3); }
                  | additiveExpression '-' multiplicativeExpression	{ $$ = ast_binary_operator(@$, '-', $1, $3); }
                  ;

shiftExpression: additiveExpression
               | shiftExpression T_SHIFT_LEFT additiveExpression	{ $$ = ast_binary_operator(@$, SHIFT_LEFT, $1, $3); }
               | shiftExpression T_SHIFT_RIGHT additiveExpression	{ $$ = ast_binary_operator(@$, SHIFT_RIGHT, $1, $3); }
               ;

relationalExpression: shiftExpression
                    | relationalExpression '<' shiftExpression	{ $$ = ast_binary_operator(@$, '<', $1, $3); }
                    | relationalExpression '>' shiftExpression	{ $$ = ast_binary_operator(@$, '>', $1, $3); }
                    | relationalExpression T_LEQ shiftExpression	{ $$ = ast_binary_operator(@$, LEQ, $1, $3); }
                    | relationalExpression T_GEQ shiftExpression	{ $$ = ast_binary_operator(@$, GEQ, $1, $3); }
                    ;

equalityExpression: relationalExpression
                  | equalityExpression T_EQ relationalExpression	{ $$ = ast_binary_operator(@$, EQ, $1, $3); }
                  | equalityExpression T_NE relationalExpression	{ $$ = ast_binary_operator(@$, NE, $1, $3); }
                  ;

andExpression: equalityExpression
             | andExpression '&' equalityExpression	{ $$ = ast_binary_operator(@$, '&', $1, $3); }
             ;

xorExpression: andExpression
             | xorExpression '^' andExpression	{ $$ = ast_binary_operator(@$, '^', $1, $3); }
             ;

orExpression: xorExpression
            | orExpression '|' xorExpression	{ $$ = ast_binary_operator(@$, '|', $1, $3); }
            ;

logicalAndExpression: orExpression
                    | logicalAndExpression T_LOG_AND orExpression	{ $$ = ast_binary_operator(@$, LOGICAL_AND, $1, $3); }
                    ;

logicalOrExpression: logicalAndExpression
                   | logicalOrExpression T_LOG_OR logicalAndExpression	{ $$ = ast_binary_operator(@$, LOGICAL_OR, $1, $3); }
                   ;

conditionalExpression: logicalOrExpression
                     | logicalOrExpression '?' expression ':' conditionalExpression	{ $$ = ast_ternary_operator(@$, $1, $3, $5); }
                     ;

assignmentExpression: conditionalExpression
                    | unaryExpression assignmentOperator assignmentExpression	{ $$ = ast_binary_operator(@$, $2, $1, $3); }

expression: assignmentExpression { $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
          | expression ',' assignmentExpression	{ $$ = ast_list_expand(@$, $1, $3); }
          ;

assignmentOperator: '=' // ! check if it returns itself
                  | T_ADD_EQ
                  | T_SUB_EQ
                  | T_MUL_EQ
                  | T_DIV_EQ
                  | T_REM_EQ
                  ;

parameterList: %empty						{ $$ = ast_list_new(@$, AST_ALL); }
             | parameters					{ $$ = ast_expand(@$, $1); }
             ;

parameters: assignmentExpression						{ $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
          | parameters ',' assignmentExpression				{ $$ = ast_list_expand(@$, $1, $3); }
          ;

string: T_LITERAL_STRING { $$ = ast_list_add(ast_list_new(@$, ast_literal), $1);}
      | string T_LITERAL_STRING { $$ = ast_list_expand(@$, $1, $2); }

literal: T_LITERAL_NUMERIC					{ $$ = ast_literal(@$, LITERAL_NUMERIC); }
       | string					{ $$ = $1 }
       | string '[' T_IDENTIFIER ']'  { $$ = ast_custom_label(@$, $1) }
       ;

initializer: '{' initializerList '}' { $$ = ast_list_add(ast_list_new(@$, AST_ALL), $2); }
           | '{' initializerList ',' '}' { $$ = ast_list_add(ast_list_new(@$, AST_ALL), $2); }
           | expression { $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
           ;
  
initializerList: initializer                    { $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
              | initializerList ',' initializer { $$ = ast_list_expand(@$, $1, $3); }
              ;

initDeclarator: declarator
              | declarator '=' initializer { $$ = ast_variable_declaration(@$, $1, NULL, $3); } 

// TODO: Add designations later on e.g. [0] = 1 and .bob = 1

//! Fix this its incorrect
declarator: pointer directDeclarator { $$ = ast_variable_declaration(@$, ) }
          | directDeclarator { $$ = $1; }

pointer: '*' { $$ = ast_list_add(ast_list_new(@$, AST_ALL), ast_unary_operator(@$, DEREFERENCE, $1)); }
       | pointerDecl '*' { $$ = ast_list_expand(@$, $1, ast_unary_operator(@$, DEREFERENCE, $2)); }
       ;

// ! i dont think this works
directDeclarator: T_IDENTIFIER { $$ = ast_identifier(@$); } 
                |  directDeclarator '[' expression ']' { $$ = ast_list_add(ast_list_new(@$, AST_ALL), $1); }
                // TODO: Fold the expression and check if its a int -> if not then error
                |  directDeclarator '[' expression ']' { $$ = ast_list_expand(@$, $1, $3); } // Should be a list of ast i think 
                ;

initDeclaratiorList : initDeclarator { $$ = ast_list_add(@$, ast_list_new(@$, AST_ALL), $1); }
                | initDeclaratiorList ',' initDeclarator { $$ = ast_list_expand(@$, $1, ast_identifier(@$)); }
                ;

typeSpecifier: T_I32 { $$ = NULL; } // * for now
             | T_I8
             | T_VOID
             // Will need to change this i think?
             | T_IDENTIFIER
             | structSpecifier
             | unionSpecifier
             | enumSpecifier
             ;

structSpecifier: T_STRUCT '{' structOrUnionMembers '}'	{ $$ = ast_struct_declaration(@$, NULL, $3); }
               | T_STRUCT '{' structOrUnionMembers ',' '}'	{ $$ = ast_struct_declaration(@$, NULL, $3); }
               | T_STRUCT  T_IDENTIFIER '{' structOrUnionMembers '}'	{ $$ = ast_struct_declaration(@$, $2, $4); }
               | T_STRUCT  T_IDENTIFIER	{ $$ = ast_struct_declaration(@$, $2, NULL); }
               ;

unionSpecifier: T_UNION '{' structOrUnionMembers '}'	{ $$ = ast_union_declaration(@$, NULL, $3); }
              | T_UNION '{' structOrUnionMembers ',' '}'	{ $$ = ast_union_declaration(@$, NULL, $3); }
              | T_UNION  T_IDENTIFIER '{' structOrUnionMembers '}'	{ $$ = ast_union_declaration(@$, $2, $4); }
              | T_UNION  T_IDENTIFIER	{ $$ = ast_union_declaration(@$, $2, NULL); }
              ;

structOrUnionMembers: %empty				              { $$ = ast_list_new(@$, AST_ALL); }
             | structOrUnionMembers structOrUnionMember	{ $$ = ast_list_expand(@$, $1, $2); }
             ;

structOrUnionMember: typeSpecifier T_IDENTIFIER ';'	{ $$ = ast_type_name_pair(@$, $1, $2, NULL); }
            ;

enumSpecifier: T_ENUM '{' enumMembers '}'	{ $$ = ast_enum_declaration(@$, NULL, $3); }
            | 
            | T_ENUM  T_IDENTIFIER '{' enumMembers '}'	{ $$ = ast_enum_declaration(@$, $2, $4); }
            | T_ENUM  T_IDENTIFIER	{ $$ = ast_enum_declaration(@$, $2, NULL); }
            ; 

enumMembers: %empty				            { $$ = ast_list_new(@$, AST_ALL); }
           | enumMembers enumMember		{ $$ = ast_list_expand(@$, $1, $2); }
           ;

enumMember: T_IDENTIFIER '=' T_LITERAL_NUMERIC ';'	{ $$ = ast_type_name_pair(@$, $1, ); }
          | T_IDENTIFIER ';'				{ $$ = ast_enum_member(@$, $1, NULL); }
          ;

// TODO: Need to add a check if there is a lone i32 or i8
declaration:  declarationSpecifier ';' { $$ = $1 }
            | declarationSpecifier declarationList ';' { $$ = ast_variable_declaration_list(@$, $1, $2) }
            ;

declarationSpecifier: typeSpecifier
                    | storageClassSpecifier declarationSpecifier
                    ;

storageClassSpecifier: T_TYPEDEF
                     | T_EXTERN
                     | T_STATIC
                     | T_CONST
                     ;

%%

#include <stdio.h>
#include <stdlib.h>

void yyerror(YYLTYPE *yyllocp, yyscan_t unused, AstNode *root, const char *msg) {
  fprintf(stderr, "[%d:%d]: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
  exit(EXIT_FAILURE);
}
