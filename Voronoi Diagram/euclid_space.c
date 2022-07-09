/* working with 2D space, adding 3D later :-) */

#include "utils.h"
#include "euclid_space.h"

// should I be serious with this? 
int isEqual(double a, double b) {
	return a==b;
}

point_t *mid2points(point_t *pa, point_t *pb) {
	point_t mid={(pa->x+pb->x)/2, (pa->y+pb->y)/2};
	point_t *pmid= my_malloc(sizeof(*pmid));
	*pmid= mid;
	return pmid;
}

// return TRUE if point p is on the right of vector (a-->b)
int rightSide(point_t *p, point_t *a, point_t *b) {
	if (isEqual(a->x, b->x)) return (a->y - b->y)*(p->x - a->x)<0;  
	if (isEqual(a->y, b->y)) return (a->x - b->x)*(p->y - a->y)>0;  

	// find the line y= mx + c that connects 2 points
	double m= (b->y - a->y) / (b->x - a->x);
	double c= b->y - m* b->x;

    // checking, hmmmm what's if the point is in the line?
	double delta= p->y - (m * p->x + c);
	return (a->x - b->x)*delta >=0 ;
}

/*-------------- bisector -------------------*/
void getBisector(bisector_t *bs, point_t *a, point_t *b) {
	//fprintf(stderr, "getBisector not yet implemented\n");
    /* fill in */
    point_t mid = *mid2points(a, b);
	double gradient = - (b->x - a->x) / (b->y - a->y);
	bs->first = *a;
	bs->second = *b;
	bs->mid = mid;
	bs->gradient = gradient;
}

void printBisector(FILE *f, bisector_t *bs) {
	// fprintf(stderr, "getBisector not yet implemented\n");
    /* CHANGE IT! */
	if ( bs->first.y == bs->second.y) {
		fprintf(f, "x = %lf\n", bs->mid.x);
	} else { 
		fprintf(f, "y = %lf * (x - %lf) + %lf\n",
			 bs->gradient, bs->mid.x, bs->mid.y );
	}
}	 


vertex_t *newVertex(double x, double y) {
	vertex_t *v= my_malloc(sizeof(*v));
	v->x= x;
	v->y= y;
	return v;
}


/*------------- Intersection-related functions ----------------- */ 
/*>>>>>>>>>>>>>>>>>>>> ACKNOWLEDGMENT -------------------------- 
This code is adapted from the assignment 2 Base code, where it said:

This intersection is based on code by Joseph O'Rourke and is provided for use in 
COMP20003 Assignment 1.

The approach for intersections is:
- Use the bisector to construct a finite segment and test it against the half-edge.
- Use O'Rourke's segseg intersection (https://hydra.smith.edu/~jorourke/books/ftp.html)
    to check if the values overlap.
*/

/*  Anh's GENERAL NOTES:
    The basic function implemented here is:
           segmentIntersects: find the intersect between 2 straight line segments

	 - I believe that the basic function  returns INTERSECT
       if and only if the two *segments*, not the 2 corresponding lines, intersect
	 - While it fits well our purpose (we want to find the intersect of a bisector
	   with a *halfEdge*, not with the straight line that contains
	   the halfEdge as a segment), care should be taken for the bisector:
		  + in order to use function we must supply 2 points in the bisector
		    and make sure that the segment betwen these 2 points is long enough 
		    in both side to go pass the halfEdge
		  + if you are not sure how to decide the segment length of bisector,
		    use Grady's defintion:
		    #define DEFAULTMINLENGTH (200)    
            and (in general) take a point at x coordinates x_mid - DEFAULTMINLENGTH
            and another point at x coordinates x_mid + DEFAULTMINLENGTH
		    in the bisector line to define the bisector segment
*/

#define DEFAULTMINLENGTH (200) 
           /* FIX IT:  chnage to something close to infinity later!  */

/*====== function segmentAndLineIntersects :
    Input:
    - straigt line segment specified by starting point *aStart end ending point *aEnd,
    - straight line specified by the line tangent k and a poind *mid of the line,
      note that the equation of the line is:
           y= k ( x - x_mid ) + y_mid      if k is not INFINITY
           x= x_mid                        if k is INFINITY
      also note that INFINITY is defined in math.h
      and that the equation above is made to be compatible with bisector in ASS2
      You MUST set k= INFINITY if the line is a vertical line	  

   Output:
    - if the 2 input lines intersect at a single point:
         + "*interstectPoint" is set to the intersection point
         + returns the value INTERSECT  (ie. 1)  
    - returns something else otherwise, *intersectPoint might or might be altered
*/ 
intersectType_t segmentAndLineIntersects(
		vertex_t *aStart, vertex_t *aEnd,     // the segment
		double k, vertex_t *mid,              // the line
		vertex_t *intersectPoint) {           // the output intersection point

	// set  starting point and ending points in the line y= k(x- x_mid)+ y_mid
	vertex_t bStart, bEnd;
	double extendedLength= DEFAULTMINLENGTH;
	if (k==INFINITY) {  // the line is a vertical line
		bStart.x= bEnd.x= mid->x;
		bStart.y= mid->y - extendedLength;
		bEnd.y= mid->y + extendedLength;
	} else {
		bStart.x= mid->x - extendedLength;
		bEnd.x= mid->x + extendedLength;
		bStart.y= k * (bStart.x - mid->x) + mid->y;
		bEnd.y= k * (bEnd.x - mid->x) + mid->y;
	}

	// returns the intersect
	return segmentIntersects(aStart, aEnd, &bStart, &bEnd, intersectPoint);
}

/*>>>>>>>>>>>>> START OF basic intersect function implementation  -----------------*/
/*            the rest can be treated as a black box                               */

int collinear(double sx, double sy, double ex, double ey, double x, double y);
int areaSign(double sx, double sy, double ex, double ey, double x, double y);
int between(double sx, double sy, double ex, double ey, double x, double y);
enum intersectType parallelIntersects(double heSx, double heSy, double heEx, double heEy,
   double bSx, double bSy, double bEx, double bEy, double *x, double *y);


/* segmentIntersects: find the intersect between 2 straight line segments
   Input:
	  - the straight line segment connecting vertices *aStart and *aEnd, and
      - the straight line segment connecting vertices *bStart and *bEnd
 
   Output:
    - if the 2 input segments intersect at a single point:
         + "*interstectPoint" is set to the intersection point
         + returns the value INTERSECT  (ie. 1)  
    - otherwise: (if they don't intersect, or their lines are parallel or overlap)
		+ returns a value which is not INTERSECT 
        + (intersectPoint might or might not set)
      note: in ths assignment:
           + we just want to know if the segments intersect, and where
           + we don't pay attentio on the details when no intersection

*/

intersectType_t segmentIntersects(
	vertex_t *aStart, struct vertex *aEnd,    // the first segment
	vertex_t *bStart, struct vertex *bEnd,    // the second segmnet
	vertex_t *intersectPoint){                // the output intersection

    /* Two points of the first line (with is a halfEdge in our assignment */
    double heSx = aStart->x;
    double heSy = aStart->y;
    double heEx = aEnd->x;
    double heEy = aEnd->y;
    
    /* Two points of the second line (the line is a bisector in our assignment) */
    double bSx= bStart->x;
    double bSy= bStart->y;
    double bEx= bEnd->x;
    double bEy= bEnd->y;
    
    // fprintf(stderr,"Finding intersect b/w (%lf, %lf) -> (%lf, %lf) and (%lf, %lf) -> (%lf, %lf)\n", heSx, heSy, heEx, heEy, bSx, bSy, bEx, bEy); 
    
    /* Parametric equation parameters */
    double t1;
    double t2;
    /* Numerators for X and Y coordinate of intersection. */
    double numeratorX;
    double numeratorY;
    /* Denominators of intersection coordinates. */
    double denominator;
    
    /*
    See http://www.cs.jhu.edu/~misha/Spring20/15.pdf
    for explanation and intuition of the algorithm here.
    x_1 = heSx, y_1 = heSy    |    p_1 = heS
    x_2 = heEx, y_2 = heEy    |    q_1 = heE
    x_3 = bSx , y_3 = bSy     |    p_2 =  bS
    x_4 = bEx , y_4 = bEy     |    q_2 =  bE
    ----------------------------------------
    So the parameters t1 and t2 are given by:
    | t1 |   | heEx - heSx  bSx - bEx | -1  | bSx - heSx |
    |    | = |                        |     |            |
    | t2 |   | heEy - heSy  bSy - bEy |     | bSy - heSy |
    
    Hence:
    | t1 |       1     | bSy - bEy        bEx - bSx |  | bSx - heSx |
    |    | = --------- |                            |  |            |
    | t2 |    ad - bc  | heSy - heEy    heEx - heSx |  | bSy - heSy |
    
        where 
        a = heEx - heSx
        b = bSx  -  bEx
        c = heEy - heSy
        d = bSy  -  bEy
    */
    
    /* Here we calculate ad - bc */
    denominator = heSx * (bEy  -  bSy) +
                  heEx * (bSy  -  bEy) +
                  bEx  * (heEy - heSy) +
                  bSx  * (heSy - heEy);
    
    if(denominator == 0){
        /* In this case the two are parallel */
        return parallelIntersects(heSx, heSy, heEx, heEy, bSx, bSy, bEx, bEy,
				&(intersectPoint->x), &(intersectPoint->y));
    }
    
    /*
    Here we calculate the top row.
    | bSy - bEy        bEx - bSx |  | bSx - heSx |
    |                            |  |            |
    |                            |  | bSy - heSy |
    */
    numeratorX = heSx * (bEy  -  bSy) +
                 bSx  * (heSy -  bEy) +
                 bEx  * (bSy  - heSy);
    
    /*
    Here we calculate the bottom row.
    |                            |  | bSx - heSx |
    |                            |  |            |
    | heSy - heEy    heEx - heSx |  | bSy - heSy |
    */
    numeratorY = -(heSx * (bSy  -  heEy) +
                   heEx * (heSy -  bSy) +
                   bSx  * (heEy  - heSy));
    
    /* Use parameters to convert to the intersection point */
    t1 = numeratorX/denominator;
    t2 = numeratorY/denominator;
    intersectPoint->x = heSx + t1 * (heEx - heSx);
    intersectPoint->y = heSy + t1 * (heEy - heSy);
    
    /* Make final decision - if point is on segments, parameter values will be
    between 0, the start of the line segment, and 1, the end of the line segment.
    */
    if (0.0 < t1 && t1 < 1.0 && 0.0 < t2 && t2 < 1.0){
        return INTERSECT;
    } else if(t1 < 0.0 || 1.0 < t1 || t2 < 0.0 || 1.0 < t2){
        /* s or t outside of line segment. */
        return DOESNT_INTERSECT;
    } else {
        /* 
        ((numeratorX == 0) || (numeratorY == 0) || 
         (numeratorX == denominator) || (numeratorY == denominator))
        */
        return ENDS_OVERLAP;
    }
}

/*--------------- sub-functions for intterect  ----------------------*/


/* Returns 1 if the point (x, y) is in the line from s(x, y) to e(x, y), 0 otherwise. */

int collinear(double sx, double sy, double ex, double ey, double x, double y){
    /* Work out area of parallelogram - if it's 0, points are in the same line. */
    if (areaSign(sx, sy, ex, ey, x, y) == 0){
        return 1;
    } else {
        return 0;
    }
}

/* Returns -1, 0 or 1, based on the area enclosed by the three points. 0 corresponds
    to no area enclosed.
*/
int areaSign(double sx, double sy, double ex, double ey, double x, double y){
    double areaSq;
    /* |AB x AC|^2, squared area */
    /* See https://mathworld.wolfram.com/CrossProduct.html */
    areaSq = (ex - sx) * (y  - sy) -
             (x  - sx) * (ey - sy);
    
    if(areaSq > 0.0){
        return 1;
    } else if(areaSq == 0.0){
        return 0;
    } else {
        return -1;
    }
}


/* Returns 1 if point (x, y) is between (sx, sy) and (se, se) */

int between(double sx, double sy, double ex, double ey, double x, double y){
    if(sx != ex){
        /* If not vertical, check whether between x. */
        if((sx <= x && x <= ex) || (sx >= x && x >= ex)){
            return 1;
        } else {
            return 0;
        }
    } else {
        /* Vertical, so can't check _between_ x-values. Check y-axis. */
        if((sy <= y && y <= ey) || (sy >= y && y >= ey)){
            return 1;
        } else {
            return 0;
        }
    }
}


/* Solves the intersection when the two lines are parallel or overlapped 
*/
enum intersectType parallelIntersects(double heSx, double heSy, double heEx, double heEy,
    double bSx, double bSy, double bEx, double bEy, double *x, double *y){
    if(!collinear(heSx, heSy, heEx, heEy, bSx, bSy)){
        /* Parallel, no intersection so don't set (x, y) */
        return DOESNT_INTERSECT;
    }
    /* bS between heS and heE */
    if(between(heSx, heSy, heEx, heEy, bSx, bSy)){
        *x = bSx; 
        *y = bSy;
        return SAME_LINE_OVERLAP;
    }
    /* bE between heS and heE */
    if(between(heSx, heSy, heEx, heEy, bEx, bEy)){
        *x = bEx;
        *y = bEy;
        return SAME_LINE_OVERLAP;
    }
    /* heS between bS and bE */
    if(between(bSx, bSy, bEx, bEy, heSx, heSy)){
        *x = heSx;
        *y = heSy;
        return SAME_LINE_OVERLAP;
    }
    /* heE between bS and bE */
    if(between(bSx, bSy, bEx, bEy, heSx, heSy)){
        *x = heEx; 
        *y = heEy;
        return SAME_LINE_OVERLAP;
    }
    
    return DOESNT_INTERSECT;
}

/*<<<<<<<<<<<<<<<<<<< end of ACKNOWLEDGMENT --------------------------  */

