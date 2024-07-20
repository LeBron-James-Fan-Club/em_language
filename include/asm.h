#ifndef ASM_H
#define ASM_H

#include "comp.h"
#include "defs.h"
#include "context.h"
#include "sym.h"

void MIPS_Pre(Compiler);
void MIPS_Post(Compiler);

void MIPS_PreFunc(Compiler this, SymTable st, Context ctx);
void MIPS_PostFunc(Compiler this, SymTable st, Context ctx);

int MIPS_Load(Compiler, int value);
int MIPS_Add(Compiler, int r1, int r2);
int MIPS_Mul(Compiler, int r1, int r2);
int MIPS_Sub(Compiler, int r1, int r2);
int MIPS_Div(Compiler, int r1, int r2);
int MIPS_Mod(Compiler, int r1, int r2);

void MIPS_PrintInt(Compiler, int r);
void MIPS_PrintChar(Compiler, int r);
void MIPS_PrintStr(Compiler this, int r);

int MIPS_LoadGlobStr(Compiler this, SymTable st, int id);
int MIPS_LoadGlob(Compiler this, SymTable st, int id, enum ASTOP op);

void MIPS_GlobSym(Compiler this, SymTable st, int id);
int MIPS_StoreGlob(Compiler, int r1, SymTable st, int id);
int MIPS_StoreRef(Compiler this, int r1, int r2, enum ASTPRIM type);

int MIPS_LoadLocal(Compiler this, SymTable st, int id, enum ASTOP op);
int MIPS_StoreLocal(Compiler this, int r1, SymTable st, int id);

int MIPS_Widen(Compiler this, int r1, enum ASTPRIM newType);

int MIPS_EqualSet(Compiler, int r1, int r2);
int MIPS_EqualJump(Compiler, int r1, int r2, int l);

int MIPS_NotEqualSet(Compiler, int r1, int r2);
int MIPS_NotEqualJump(Compiler, int r1, int r2, int l);

int MIPS_LessThanSet(Compiler, int r1, int r2);
int MIPS_LessThanJump(Compiler, int r1, int r2, int l);

int MIPS_GreaterThanSet(Compiler, int r1, int r2);
int MIPS_GreaterThanJump(Compiler, int r1, int r2, int l);

int MIPS_LessThanEqualSet(Compiler, int r1, int r2);
int MIPS_LessThanEqualJump(Compiler, int r1, int r2, int l);

int MIPS_GreaterThanEqualSet(Compiler, int r1, int r2);
int MIPS_GreaterThanEqualJump(Compiler, int r1, int r2, int l);

int MIPS_InputInt(Compiler);

void MIPS_Label(Compiler, int l);
void MIPS_Jump(Compiler, int l);

void MIPS_GotoLabel(Compiler, SymTable st, int id);
void MIPS_GotoJump(Compiler, SymTable st, int id);
void MIPS_ReturnLabel(Compiler this,SymTable st, Context ctx);
void MIPS_ReturnJump(Compiler this, SymTable st, Context ctx);

void MIPS_Return(Compiler, SymTable st, int r, Context ctx);
int MIPS_Call(Compiler, SymTable st, int id, int numArgs);
void MIPS_ArgCopy(Compiler this, int r, int argPos, int maxArg);
void MIPS_SetupArgFrame(Compiler this, int maxArgs);

int MIPS_Address(Compiler, SymTable st, int id);
int MIPS_Deref(Compiler, int r, enum ASTPRIM type);
int MIPS_ShiftLeftConstant(Compiler, int r, int c);

int MIPS_ShiftLeft(Compiler, int r1, int r2);
int MIPS_ShiftRight(Compiler, int r1, int r2);

int MIPS_Negate(Compiler, int r);
int MIPS_BitNOT(Compiler, int r);
int MIPS_LogNOT(Compiler, int r);

int MIPS_BitAND(Compiler, int r1, int r2);
int MIPS_BitOR(Compiler, int r1, int r2);
int MIPS_BitXOR(Compiler, int r1, int r2);

int MIPS_ToBool(Compiler this, enum ASTOP parentOp, int r, int label);

void MIPS_Poke(Compiler this, int r1, int r2);
int MIPS_Peek(Compiler this, int r1, int r2);

int label(Compiler);

void Compiler_FreeAllReg(Compiler);
void Compiler_GenData(Compiler, SymTable st);

int PrimSize(enum ASTPRIM type);

#endif