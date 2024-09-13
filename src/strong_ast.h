#pragma once

typedef struct ast_node *AstNode;

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

enum ast_unary_operator {
    UNARY_NOT,
};

enum ast_binary_operator {
    BINARY_PLUS,
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

AstNode ast_list_new(struct ast_node_span span, enum ast_node_t children_type);

AstNode ast_expand(struct ast_node_span span, AstNode node);

AstNode ast_list_add(AstNode list, AstNode child);

AstNode ast_identifier(struct ast_node_span span, enum ast_literal_type literal_type);

AstNode ast_literal(struct ast_node_span span);

AstNode ast_unary(struct ast_node_span span, enum ast_unary_operator operator, AstNode inner);

AstNode ast_binary(struct ast_node_span span, enum ast_binary_operator operator, AstNode left, AstNode right);
