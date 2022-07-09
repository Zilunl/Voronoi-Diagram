/* --------------------- ACKNOWLEDGMENT -------------------------- 
This code is adapted from the assignment 2 Base code, where it said:

This intersection is based on code by Joseph O'Rourke and is provided for use in 
COMP20003 Assignment 1.

*/


#include "intersect.h"


// Grady's skeleton for intersects, "translated" to this base datya structure
//    find intersection between:
//         - the straight line segment defined by halEdge he, and
//         - the line defined by bisector b
// Note: "minLength" was skipped, always use DEFAULTMINLENGTH     
/*intersectType_t intersects(dcel_t *he, bisector_t *b, 
    geometry_t *g, vertex_t *intersectPoint){

	fprintf(stderr, "Function intersects not yet implemented\n");
	return 0;
	
	// 1. set the segment
	vertex_t *aStart= g->V[he->startV];
	vertex_t *aEnd= g->V[he->endV];

    // 2. set 2 points in the bisector to make the second segment
	vertex_t bStart={0.0, 0.0}, bEnd= {0.0, 0.0};
     You need to set bStart to one point in the bisector
                       bEnd   to another point in the bisector
                  using DEFAULTMINLENGTH
                see notes in euclid_space.c for more information
             
        setting the above values will be equivalent to Grady's part:
             double bSx = ... ;
             double bSy = ... ;
             double bEx = ... ;
             double bEy = ... ;
   
}*/
