#include "types.h"

#include <stdio.h>

#include "ast.h"
#include "defs.h"
#include "misc.h"

bool inttype(enum ASTPRIM type) { return type == P_INT || type == P_CHAR; }

bool ptrtype(enum ASTPRIM type) {
    return type == P_VOIDPTR || type == P_INTPTR || type == P_CHARPTR;
}

ASTnode modify_type(ASTnode tree, enum ASTPRIM rtype, enum ASTOP op) {
    enum ASTPRIM ltype;
    int lsize, rsize;
    ltype = tree->type;
    
    if ((inttype(ltype) && inttype(rtype)) ||
        // For assignment operator
        (ptrtype(ltype) && ptrtype(rtype))) {
        if (ltype == rtype) return tree;
        lsize = PrimSize(ltype);
        rsize = PrimSize(rtype);
        if (lsize > rsize) {
            return NULL;
        } else if (lsize < rsize) {
            return ASTnode_NewUnary(A_WIDEN, ltype, tree, 0);
        }
    }

    if (ptrtype(rtype)) {
        if (op == A_NONE && ptrtype(rtype)) {
            return tree;
        }
    }

    if (op == A_ADD || op == A_SUBTRACT) {
        // I don't know why but the types are swapped
        // Pointer -> left, Value -> right

        if (inttype(ltype) && ptrtype(rtype)) {
            rsize = PrimSize(value_at(rtype));
            if (rsize > 1) {
                return ASTnode_NewUnary(A_SCALE, rtype, tree, rsize);
            } else {
                return tree;
            }
        }
    }

    // Types are incompatible
    return NULL;
}

enum ASTPRIM pointer_to(enum ASTPRIM type) {
    switch (type) {
        case P_VOID:
            return P_VOIDPTR;
        case P_INT:
            return P_INTPTR;
        case P_CHAR:
            return P_CHARPTR;
        default:
            fatala("InternalError: Unknown type %d for pointer", type);
    }
}

enum ASTPRIM value_at(enum ASTPRIM type) {
    switch (type) {
        case P_VOIDPTR:
            return P_VOID;
        case P_INTPTR:
            return P_INT;
        case P_CHARPTR:
            return P_CHAR;
        default:
            fatala("InternalError: Unknown type %d for pointer", type);
    }
}