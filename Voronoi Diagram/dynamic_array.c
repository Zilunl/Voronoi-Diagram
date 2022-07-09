#include "utils.h"
#include "dynamic_array.h"


void arrayInit(void ***_A, int *_N, int *_SIZE) {
    *_A= NULL;
    *_N= 0;
    *_SIZE = INIT_SIZE;
    *_A= my_malloc( (*_SIZE) * sizeof( **(_A) ) );
}

void arrayAppend(void ***_A, int * _N, int *_SIZE, void *_DATA) { 
	if ( *(_N)==*(_SIZE) ) {
		*_SIZE = (*_SIZE) * 2;						
		*_A= my_realloc( *_A, (*_SIZE) * sizeof( *(_A) ) );	
	}												
	(*_A)[(*_N)++]= _DATA;								
}
