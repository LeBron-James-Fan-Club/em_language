#pragma once

typedef struct ast_node *AstNode;
typedef AstNode OptionalAstNode;

struct ast_node_span {
    int first_line;
    int first_column;
    int last_line;
    int last_column;

    int start_byte;
    int end_byte;
};

enum ast_literal_type {
    LITERAL_NUMERIC,
    LITERAL_STRING,
};

#define AST_NODE(type_name, struct_name, structure) type_name,
#define AST_MARKER(x) x,
enum ast_node_t {
#  include "strong_ast_nodes.h"
};
#undef AST_MARKER
#undef AST_NODE

#define AST_NODE(enum_name, struct_name, structure) struct ast_##struct_name structure;
#define AST_MARKER(x)
#  include "strong_ast_nodes.h"
#undef AST_MARKER
#undef AST_NODE

struct ast_node {
    enum ast_node_t type;
    struct ast_node_span span;

    union {
#define AST_NODE(enum_name, struct_name, structure) struct ast_##struct_name as_##struct_name;
#define AST_MARKER(x)
#  include "strong_ast_nodes.h"
#undef AST_MARKER
#undef AST_NODE
    };
};

AstNode ast_parse(char *source);

AstNode ast_list_new(struct ast_node_span span, enum ast_node_t children_type);

AstNode ast_expand(struct ast_node_span span, AstNode node);

AstNode ast_list_add(AstNode list, AstNode child);

AstNode ast_list_expand(struct ast_node_span span, AstNode list, AstNode child);

AstNode ast_variable_declaration(struct ast_node_span span, AstNode type, AstNode name, OptionalAstNode initializer);

AstNode ast_function_declaration(struct ast_node_span span, AstNode type, AstNode name, AstNode parameter_list, OptionalAstNode body);

AstNode ast_struct_declaration(struct ast_node_span span, AstNode name, AstNode members);

AstNode ast_type_name_pair(struct ast_node_span span, AstNode type, AstNode name);

AstNode ast_if_statement(struct ast_node_span span, AstNode condition, AstNode truthy, AstNode falsy);

AstNode ast_while_statement(struct ast_node_span span, AstNode condition, AstNode body);

AstNode ast_assignment(struct ast_node_span span, AstNode name, AstNode expression);

AstNode ast_literal_expression(struct ast_node_span span, AstNode literal, OptionalAstNode storage);

AstNode ast_identifier(struct ast_node_span span);

AstNode ast_unary_operator(struct ast_node_span span, char operator, AstNode inner);

AstNode ast_binary_operator(struct ast_node_span span, char operator, AstNode left, AstNode right);

AstNode ast_ternary_operator(struct ast_node_span span, AstNode condition, AstNode truthy, AstNode falsy);

AstNode ast_invocation(struct ast_node_span span, AstNode function, AstNode parameterList);

AstNode ast_literal(struct ast_node_span span, enum ast_literal_type type);

void ast_free(AstNode node);
