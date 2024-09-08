#ifndef DECL_H
#define DECL_H

#include "ast.h"
#include "context.h"
#include "defs.h"
#include "flags.h"
#include "gen.h"
#include "scan.h"
#include "stmt.h"
#include "sym.h"

#define TABLE_INCREMENT 10

enum ASTPRIM parse_type(Compiler c, Scanner s, SymTable st, Token tok,
                        Context ctx, SymTableEntry *ctype,
                        enum STORECLASS *class);
enum ASTPRIM declare_list(Compiler c, Scanner s, SymTable st, Token tok,
                                 Context ctx, SymTableEntry *cType,
                                 enum STORECLASS class, enum OPCODES end1,
                                 enum OPCODES end2);
void global_declare(Compiler c, Scanner s, SymTable st, Token tok, Context ctx);

#endif