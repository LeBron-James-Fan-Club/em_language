AST_MARKER(AST_ALL)

AST_NODE(AST_LIST, list, {
    enum ast_node_t children_type;
    AstNode *children;
    int num_children;
})

AST_NODE(AST_VARIABLE_DECLARATION, variable_declaration, {
    AstNode type;
    AstNode name;
    OptionalAstNode initializer;
})

AST_NODE(AST_FUNCTION_DECLARATION, function_declaration, {
    AstNode type;
    AstNode name;
    AstNode parameter_list;
    OptionalAstNode body;
})

AST_NODE(AST_STRUCT_DECLARATION, struct_declaration, {
    AstNode name;
    AstNode members;
})

AST_NODE(AST_TYPE_NAME_PAIR, type_name_pair, {
    AstNode type;
    AstNode name;
})

AST_NODE(AST_IF_STATEMENT, if_statement, {
    AstNode condition;
    AstNode truthy;
    OptionalAstNode falsy;
})

AST_NODE(AST_WHILE_STATEMENT, while_statement, {
    AstNode condition;
    AstNode body;
})

AST_NODE(AST_ASSIGNMENT, assignment, {
    AstNode name;
    AstNode expression;
})

AST_MARKER(AST_EXPRESSION_START)

AST_NODE(AST_LITERAL_EXPRESSION, literal_expression, {
    AstNode literal;
    OptionalAstNode storage;
})

AST_NODE(AST_IDENTIFIER, identifier, {})

AST_NODE(AST_UNARY_OPERATOR, unary_operator, {
    char operator;
    AstNode inner;
})

AST_NODE(AST_BINARY_OPERATOR, binary_operator, {
    char operator;
    AstNode left;
    AstNode right;
})

AST_NODE(AST_TERNARY_OPERATOR, ternary_operator, {
    AstNode condition;
    AstNode truthy;
    AstNode falsy;
})

AST_NODE(AST_INVOCATION, invocation, {
    AstNode function;
    AstNode parameter_list;
})

AST_MARKER(AST_EXPRESSION_END)

AST_NODE(AST_LITERAL, literal, {
    enum ast_literal_type type;
})
