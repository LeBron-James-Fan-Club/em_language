%option noinput nounput noyywrap 8bit nodefault
%option yylineno
%option reentrant bison-bridge bison-locations

%{
  #include <stdlib.h>
  #include <string.h>
  #include "new-em.tab.h"
%}

%%

[ \n\t]+          ;

[[:digit:]]+      { return INT_LITERAL; }

.                 { return *yytext; }
