AST_MARKER(AST_ALL)

AST_NODE(AST_LIST, list, {
    enum ast_node_t children_type;
    AstNode *children;
    int num_children;
})

AST_NODE(AST_CUSTOM_LABEL, custom_label, {
    AstNode label;
    AstNode child;
})

AST_NODE(AST_CASE, case, {
    AstNode value;
    AstNode body;
})

AST_NODE(AST_VARIABLE_DECLARATION_LIST, variable_declaration_list, {
    AstNode type;
    AstNode declaration_list; // contains ast_variable_declaration list
}) 

AST_NODE(AST_VARIABLE_DECLARATION, variable_declaration, {
    OptionalAstNode pointer;
    OptionalAstNode array;
    AstNode initializer;
})

AST_NODE(AST_FUNCTION_DECLARATION, function_declaration, {
    AstNode type;
    AstNode name;
    AstNode parameter_list;
    OptionalAstNode body;
})

// TODO: Combine them later on cause they are basically same structure
AST_NODE(AST_STRUCT_DECLARATION, struct_declaration, {
    AstNode name;
    AstNode members;
})

AST_NODE(AST_UNION_DECLARATION, union_declaration, {
    AstNode name;
    AstNode members;
})

AST_NODE(AST_ENUM_DECLARATION, enum_declaration, {
    AstNode name;
    AstNode values;
})

AST_NODE(AST_TYPE_NAME_PAIR, type_name_pair, {
    AstNode type;
    AstNode name;
    OptionalAstNode initializer;
})


AST_NODE(AST_BLOCK, block, {
    AstNode statements;
})

AST_NODE(AST_IF_STATEMENT, if_statement, {
    AstNode condition;
    AstNode truthy;
    OptionalAstNode falsy;
})

AST_NODE(AST_SWITCH_STATEMENT, switch_statement, {
    AstNode expression;
    AstNode child;
})

AST_NODE(AST_WHILE_STATEMENT, while_statement, {
    AstNode condition;
    AstNode body;
})

AST_NODE(AST_FOR_STATEMENT, for_statement, {
    AstNode pre;
    AstNode condition;
    AstNode post;
    AstNode body;
})

// Not used
AST_NODE(AST_ASSIGNMENT, assignment, {
    AstNode target;
    AstNode expression;
})

AST_NODE(AST_GOTO_STATEMENT, goto_statement, {
    AstNode label;
})

AST_MARKER(AST_EXPRESSION_START)

AST_NODE(AST_LITERAL_EXPRESSION, literal_expression, {
    AstNode literal;
    OptionalAstNode storage;
})

AST_NODE(AST_IDENTIFIER, identifier, {})

AST_NODE(AST_UNARY_OPERATOR, unary_operator, {
    enum ast_unary_operation operation;
    AstNode inner;
})

AST_NODE(AST_BINARY_OPERATOR, binary_operator, {
    enum ast_binary_operation operation;
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
