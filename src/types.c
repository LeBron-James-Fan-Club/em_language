#include "types.h"

#include <stdio.h>

#include "asm.h"
#include "ast.h"
#include "defs.h"
#include "misc.h"

bool inttype(enum ASTPRIM type) { return (type & 0xf) == 0; }

bool ptrtype(enum ASTPRIM type) { return (type & 0xf) != 0; }

int type_size(enum ASTPRIM type, SymTableEntry cType) {
    if (type == P_STRUCT || type == P_UNION) return cType->size;
    return PrimSize(type);
}

ASTnode modify_type(ASTnode tree, enum ASTPRIM rtype, enum ASTOP op) {
    enum ASTPRIM ltype;
    int lsize, rsize;
    ltype = tree->type;

    if ((inttype(ltype) && inttype(rtype)) ||
        // For assignment operator
        (ptrtype(ltype) && ptrtype(rtype))) {
        if (ltype == rtype) return tree;
        // TODO: support structs later on

        lsize = type_size(ltype, NULL);
        rsize = type_size(rtype, NULL);
        if (lsize > rsize) {
            return NULL;
        } else if (lsize < rsize) {
            return ASTnode_NewUnary(A_WIDEN, ltype, tree, NULL, 0);
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
                return ASTnode_NewUnary(A_SCALE, rtype, tree, NULL, rsize);
            } else {
                return tree;
            }
        }
    }

    // Types are incompatible
    return NULL;
}

enum ASTPRIM pointer_to(enum ASTPRIM type) {
    if ((type & 0xf) == 0xf) {
        fatala("InternalError: Unknown type %d for pointer", type);
    }
    return type + 1;
}

enum ASTPRIM value_at(enum ASTPRIM type) {
    if ((type & 0xf) == 0) {
        fatala("InternalError: Unknown type %d for value_at", type);
    }
    return type - 1;
}