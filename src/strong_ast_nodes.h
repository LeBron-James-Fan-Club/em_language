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

AST_MARKER(AST_EXPRESSION_START)

AST_MARKER(AST_EXPRESSION_END)
