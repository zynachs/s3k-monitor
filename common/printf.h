#pragma once
#include <stdarg.h>

int printf(const char *, ...);
int snprintf(char *, unsigned long int, const char *, ...);
int vsnprintf(char *, unsigned long int, const char *, va_list);
