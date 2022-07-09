/* Module for working with dynamic array of pointers
   Limitation: array element can only be a single pointer
*/

#ifndef _DYNAMIC_ARRAY_H_
#define _DYNAMIC_ARRAY_H_

#include "utils.h"

#define INIT_SIZE 4

void arrayInit(void ***_A, int *_N, int *_SIZE);
void arrayAppend(void ***_A, int * _N, int *_SIZE, void *_DATA);


#endif

