#ifndef _UTILS_H_
#define _UTILS_H_

#include "common_defs.h"

void *my_malloc(size_t n);
void *my_realloc(void *old, size_t n);
FILE *my_fopen(const char *fname, const char *mode);
char *my_strdup(char *);

#endif
