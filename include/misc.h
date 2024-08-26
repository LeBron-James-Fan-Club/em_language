#ifndef ERRORS_H
#define ERRORS_H

#include <stdnoreturn.h>

#include "scan.h"

#define noreturn _Noreturn

noreturn void lfatal(Scanner s, char *msg);
noreturn void lfatala(Scanner s, char *msg, ...);

noreturn void fatal(char *msg);
noreturn void fatala(char *msg, ...);

void debug(char *msg, ...);

#endif