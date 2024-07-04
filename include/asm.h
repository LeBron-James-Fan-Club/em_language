#ifndef ASM_H
#define ASM_H

#include "comp.h"

void MIPS_Pre(Compiler);
void MIPS_Post(Compiler);

void MIPS_PreFunc(Compiler, SymTable st, int id);
void MIPS_PostFunc(Compiler);

int MIPS_Load(Compiler, int value);
int MIPS_Add(Compiler, int r1, int r2);
int MIPS_Mul(Compiler, int r1, int r2);
int MIPS_Sub(Compiler, int r1, int r2);
int MIPS_Div(Compiler, int r1, int r2);
int MIPS_Mod(Compiler, int r1, int r2);
void MIPS_PrintInt(Compiler, int r);

int MIPS_LoadGlob(Compiler, SymTable st, int id);
void MIPS_GlobSym(Compiler, char *sym);
int MIPS_StoreGlob(Compiler, int r1, SymTable st, int id);


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

int label(Compiler);

void Compiler_FreeAllReg(Compiler);
void Compiler_GenData(Compiler, SymTable st);

#endif