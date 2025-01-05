#ifndef ASM_H
#define ASM_H

#include "comp.h"
#include "context.h"
#include "defs.h"
#include "sym.h"

// forward declarations
bool ptrtype(enum ASTPRIM type);
enum ASTPRIM value_at(enum ASTPRIM type);
enum ASTPRIM pointer_to(enum ASTPRIM type);
int type_size(enum ASTPRIM type, SymTableEntry cType);
// end


void MIPS_Pre(Compiler);
void MIPS_Post(Compiler);

void MIPS_PreFunc(Compiler this, SymTable st, Context ctx);
void MIPS_PostFunc(Compiler this, Context ctx);

int MIPS_Load(Compiler, int value);
int MIPS_Add(Compiler, int r1, int r2);
int MIPS_Mul(Compiler, int r1, int r2);
int MIPS_Sub(Compiler, int r1, int r2);
int MIPS_Div(Compiler, int r1, int r2);
int MIPS_Mod(Compiler, int r1, int r2);

void MIPS_PrintInt(Compiler, int r);
void MIPS_PrintChar(Compiler, int r);
void MIPS_PrintStr(Compiler this, int r);

int MIPS_LoadGlobStr(Compiler this, SymTableEntry sym);
int MIPS_LoadGlob(Compiler this, SymTableEntry sym, enum ASTOP op);

void MIPS_GlobSym(Compiler this, SymTableEntry sym);
int MIPS_StoreGlob(Compiler, int r1, SymTableEntry sym);
int MIPS_StoreRef(Compiler this, int r1, int r2, enum ASTPRIM type);

int MIPS_LoadLocal(Compiler this, SymTableEntry sym, enum ASTOP op);
int MIPS_StoreLocal(Compiler this, int r1, SymTableEntry sym);

int MIPS_Align( enum ASTPRIM type, int offset, int dir);

int MIPS_EqualSet(Compiler, int r1, int r2);
int MIPS_EqualJump(Compiler, int r1, int r2, char *l);

int MIPS_NotEqualSet(Compiler, int r1, int r2);
int MIPS_NotEqualJump(Compiler, int r1, int r2, char *l);

int MIPS_LessThanSet(Compiler, int r1, int r2);
int MIPS_LessThanJump(Compiler, int r1, int r2, char *l);

int MIPS_GreaterThanSet(Compiler, int r1, int r2);
int MIPS_GreaterThanJump(Compiler, int r1, int r2, char *l);

int MIPS_LessThanEqualSet(Compiler, int r1, int r2);
int MIPS_LessThanEqualJump(Compiler, int r1, int r2, char *l);

int MIPS_GreaterThanEqualSet(Compiler, int r1, int r2);
int MIPS_GreaterThanEqualJump(Compiler, int r1, int r2, char *l);

int MIPS_InputInt(Compiler);
int MIPS_InputChar(Compiler this);
void MIPS_InputString(Compiler this, char *name, int size);

void MIPS_Label(Compiler, char *l);
void MIPS_Jump(Compiler, char *l);

void MIPS_GotoLabel(Compiler, SymTableEntry sym);
void MIPS_GotoJump(Compiler, SymTableEntry sym);
void MIPS_ReturnLabel(Compiler this, Context ctx);
void MIPS_ReturnJump(Compiler this, Context ctx);

void MIPS_Return(Compiler, int r, Context ctx);
int MIPS_Call(Compiler, SymTableEntry sym);
void MIPS_ArgCopy(Compiler this, int r, int argPos);

void MIPS_RegPush(Compiler this, int r);
void MIPS_RegPop(Compiler this, int r);

void MIPS_StoreParam(Compiler this, int r1);

void MIPS_Move(Compiler this, int r1, int r2);
int MIPS_Address(Compiler, SymTableEntry sym);
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

int MIPS_ToBool(Compiler this, enum ASTOP parentOp, int r, char *label);

int MIPS_Boolean(Compiler this, int r, enum ASTOP op, char *label);
void MIPS_LoadBoolean(Compiler this, int r, int val);

void MIPS_Poke(Compiler this, int r1, int r2);
int MIPS_Peek(Compiler this, int r1, int r2);
void MIPS_Exit(Compiler this, int r);
void MIPS_Switch(Compiler this, int r, int caseCount, char *topLabel,
                 char **caseLabel, int *caseVal, char *defaultLabel,
                 struct label nodeLabel);

int Compiler_GenLabel(Compiler this);

void Compiler_FreeAllReg(Compiler, int keepReg);
void Compiler_GenData(Compiler, SymTable st);
int allocReg(Compiler this);

void Compiler_SpillAllRegs(Compiler this);

int PrimSize(enum ASTPRIM type);

#endif