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

    new_list[num_children] = child;
    list->as_list.children = new_list;
    list->as_list.num_children = num_children + 1;
    return list;
}

AstNode ast_list_expand(struct ast_node_span span, AstNode list, AstNode child) {
    ast_list_add(list, child);
    ast_expand(span, list);
    return list;
}

AstNode ast_variable_declaration(struct ast_node_span span, AstNode type, AstNode name, OptionalAstNode initializer) {
    AstNode node = ast_basic_node(AST_VARIABLE_DECLARATION, span);
    node->as_variable_declaration.type = type;
    node->as_variable_declaration.name = name;
    node->as_variable_declaration.initializer = initializer;
    return node;
}

AstNode ast_function_declaration(struct ast_node_span span, AstNode type, AstNode name, AstNode parameter_list, OptionalAstNode body) {
    AstNode node = ast_basic_node(AST_FUNCTION_DECLARATION, span);
    node->as_function_declaration.type = type;
    node->as_function_declaration.name = name;
    node->as_function_declaration.parameter_list = parameter_list;
    node->as_function_declaration.body = body;
    return node;
}

AstNode ast_block(struct ast_node_span span, AstNode statements) {
    AstNode node = ast_basic_node(AST_BLOCK, span);
    node->as_block.statements = statements;
    return node;
}

AstNode ast_struct_declaration(struct ast_node_span span, AstNode name, AstNode members) {
    AstNode node = ast_basic_node(AST_STRUCT_DECLARATION, span);
    node->as_struct_declaration.name = name;
    node->as_struct_declaration.members = members;
    return node;
}

AstNode ast_type_name_pair(struct ast_node_span span, AstNode type, AstNode name) {
    AstNode node = ast_basic_node(AST_TYPE_NAME_PAIR, span);
    node->as_type_name_pair.type = type;
    node->as_type_name_pair.name = name;
    return node;
}

AstNode ast_if_statement(struct ast_node_span span, AstNode condition, AstNode truthy, AstNode falsy) {
    AstNode node = ast_basic_node(AST_IF_STATEMENT, span);
    node->as_if_statement.condition = condition;
    node->as_if_statement.truthy = truthy;
    node->as_if_statement.falsy = falsy;
    return node;
}

AstNode ast_while_statement(struct ast_node_span span, AstNode condition, AstNode body) {
    AstNode node = ast_basic_node(AST_WHILE_STATEMENT, span);
    node->as_while_statement.condition = condition;
    node->as_while_statement.body = body;
    return node;
}

AstNode ast_assignment(struct ast_node_span span, AstNode target, AstNode expression) {
    AstNode node = ast_basic_node(AST_ASSIGNMENT, span);
    node->as_assignment.target = target;
    node->as_assignment.expression = expression;
    return node;
}

AstNode ast_literal_expression(struct ast_node_span span, AstNode literal, OptionalAstNode storage) {
    AstNode node = ast_basic_node(AST_LITERAL_EXPRESSION, span);
    node->as_literal_expression.literal = literal;
    node->as_literal_expression.storage = storage;
    return node;
}

AstNode ast_identifier(struct ast_node_span span) {
    return ast_basic_node(AST_IDENTIFIER, span);
}

AstNode ast_unary_operator(struct ast_node_span span, char operator, AstNode inner) {
    AstNode node = ast_basic_node(AST_UNARY_OPERATOR, span);
    node->as_unary_operator.operator = operator;
    node->as_unary_operator.inner = inner;
    return node;
}

AstNode ast_binary_operator(struct ast_node_span span, char operator, AstNode left, AstNode right) {
    AstNode node = ast_basic_node(AST_BINARY_OPERATOR, span);
    node->as_binary_operator.operator = operator;
    node->as_binary_operator.left = left;
    node->as_binary_operator.right = right;
    return node;
}

AstNode ast_ternary_operator(struct ast_node_span span, AstNode condition, AstNode truthy, AstNode falsy) {
    AstNode node = ast_basic_node(AST_TERNARY_OPERATOR, span);
    node->as_ternary_operator.condition = condition;
    node->as_ternary_operator.truthy = truthy;
    node->as_ternary_operator.falsy = falsy;
    return node;
}

AstNode ast_invocation(struct ast_node_span span, AstNode function, AstNode parameter_list) {
    AstNode node = ast_basic_node(AST_INVOCATION, span);
    node->as_invocation.function = function;
    node->as_invocation.parameter_list = parameter_list;
    return node;
}

AstNode ast_literal(struct ast_node_span span, enum ast_literal_type type) {
    AstNode node = ast_basic_node(AST_LITERAL, span);
    node->as_literal.type = type;
    return node;
}

void ast_free(AstNode node) {}
