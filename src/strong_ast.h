#pragma once

struct ast_node_span {
    int first_line;
    int first_column;
    int last_line;
    int last_column;

    int start_byte;
    int end_byte;
};

enum ast_node_t {
    AST_LIST,

    AST_EXPRESSION_START,   // marker, must not be used
    AST_IDENTIFIER,
    AST_LITERAL,

    AST_UNARY,
    AST_BINARY,

    AST_EXPRESSION_END,     // marker, must not be used
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

typedef struct ast_node *AstNode;

void expand_span(AstNode node, struct ast_node_span span);

AstNode ast_list_new(struct ast_node_span span, enum ast_node_t children_type);

void ast_list_add(AstNode list, AstNode child);

AstNode ast_identifier(struct ast_node_span span, enum ast_literal_type literal_type);

AstNode ast_literal(struct ast_node_span span);

AstNode ast_unary(struct ast_node_span span, enum ast_unary_operator operator, AstNode inner);

AstNode ast_binary(struct ast_node_span span, enum ast_binary_operator operator, AstNode left, AstNode right);
