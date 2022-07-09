/* working with 2D space, 
   these funtions are for general use, not just for ass1
*/

#ifndef _EUCLID_SPACE_H_
#define _EUCLID_SPACE_H_

#include <stdio.h>
#include <math.h>

#define RIGHT_SIDE (+1)
#define EPS 1e-9        /* for isEqual comparison */


typedef struct vertex vertex_t;
typedef vertex_t point_t;       /* they are synonyms, for convieneinece */
typedef struct bisector bisector_t;
typedef enum intersectType intersectType_t;


/*--------------- functions for ass1 ----------------------*/
// creates and returns the middle (of the line segment) of 2 points
point_t *mid2points(point_t *pa, point_t *pb);

// return +1 if point p is on the right of vector (a-->b),  0 otherwise
int rightSide(point_t *p, point_t *a, point_t *b);

/*-------------- bisector & intersection -----------------*/ 
void getBisector(bisector_t *bs, point_t *a, point_t *b);
void printBisector(FILE *f, bisector_t *bs);
vertex_t *newVertex(double x, double y);

/* >>>>>>>>>>>>>>> ACKNOWLEDGMENT -------------------------- 
This part is adapted from the assignment 2 Base code, where it said:

This intersection is based on code by Joseph O'Rourke and is provided for use in 
COMP20003 Assignment 1.
*/

enum intersectType {
    DOESNT_INTERSECT = 0,  // Doesn't intersect
    INTERSECT = 1,         // Intersects
    SAME_LINE_OVERLAP = 2, // Lines are the same
    ENDS_OVERLAP = 3       // Intersects at exactly one point (endpoint)
};


intersectType_t segmentIntersects(
	vertex_t *aStart, struct vertex *aEnd,    // segment 1
	vertex_t *bStart, struct vertex *bEnd,    // segment 2
	vertex_t *intersectPoint);                // output intersection


intersectType_t segmentAndLineIntersects(
		vertex_t *aStart, vertex_t *aEnd,     // the segment
		double k, vertex_t *mid,              // the line
		vertex_t *intersectPoint);            // the output intersection point

/* <<<<<<<<<<<<<<<<<< end of ACKNOWLEDGMENT --------------------------*/ 





/*--------------- struct definition ---------------------*/
// point_t for a point in 2D space
struct vertex {
	double x, y;
};

// a general bisector y= k(x-x_mid) + y_mid, k can be INFINITY
struct bisector {
	// delete next line and fill in your
	point_t first;
	point_t second;
	point_t mid;
	double gradient;
};




 

#endif
