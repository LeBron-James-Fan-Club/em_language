#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include "strong_ast.h"

static AstNode ast_basic_node(enum ast_node_t type, struct ast_node_span span) {
    AstNode node = malloc(sizeof(*node));

    if (node == NULL) {
        perror("could not allocate node");
        exit(EXIT_FAILURE);
    }

    node->type = type;
    node->span = span;
    return node;
}

static void ensure(AstNode node, enum ast_node_t type) {
    if (type == AST_ALL) {
        assert(1);
    } else if (type == AST_EXPRESSION_START) {
        assert(AST_EXPRESSION_START < node->type && node->type < AST_EXPRESSION_END);
    } else {
        assert(node->type == type);
    }
}

AstNode ast_list_new(struct ast_node_span span, enum ast_node_t children_type) {
    AstNode node = ast_basic_node(AST_LIST, span);
    node->as_list.children_type = children_type;
    node->as_list.children = NULL;
    node->as_list.num_children = 0;
    return node;
}

AstNode ast_expand(struct ast_node_span span, AstNode node) {
    // todo: assert the old span is actually a subset of the new span
    node->span = span;
    return node;
}

AstNode ast_list_add(AstNode list, AstNode child) {
    ensure(list, AST_LIST);
    ensure(child, list->as_list.children_type);

    int num_children = list->as_list.num_children;
    AstNode *new_list = reallocarray(list->as_list.children, num_children + 1, sizeof(AstNode));

    if (new_list == NULL) {
        perror("could not allocate enough space");
        exit(EXIT_FAILURE);
    }

    list->as_list.children = new_list;
    list->as_list.num_children = num_children + 1;
    return list;
}

AstNode ast_identifier(struct ast_node_span span, enum ast_literal_type literal_type) {
    AstNode node = ast_basic_node(AST_IDENTIFIER, span);
    node->as_literal.literal_type = literal_type;
    return node;
}

AstNode ast_literal(struct ast_node_span span) {
    return ast_basic_node(AST_LITERAL, span);
}

AstNode ast_unary(struct ast_node_span span, enum ast_unary_operator operator, AstNode inner) {
    AstNode node = ast_basic_node(AST_UNARY, span);
    node->as_unary.operator = operator;
    node->as_unary.inner = inner;
    return node;
}

AstNode ast_binary(struct ast_node_span span, enum ast_binary_operator operator, AstNode left,
                   AstNode right) {
    AstNode node = ast_basic_node(AST_BINARY, span);
    node->as_binary.operator = operator;
    node->as_binary.left = left;
    node->as_binary.right = right;
    return node;
}
