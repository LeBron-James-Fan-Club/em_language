AST_MARKER(AST_ALL)

AST_NODE(AST_LIST, list, {
    enum ast_node_t children_type;
    AstNode *children;
    int num_children;
})

AST_MARKER(AST_EXPRESSION_START)

AST_NODE(AST_IDENTIFIER, identifier, {})

AST_NODE(AST_LITERAL, literal, {
    enum ast_literal_type literal_type;
})

AST_NODE(AST_UNARY, unary, {
    enum ast_unary_operator operator;
    AstNode inner;
})

AST_NODE(AST_BINARY, binary, {
    enum ast_binary_operator operator;
    AstNode left;
    AstNode right;
})

AST_MARKER(AST_EXPRESSION_END)
