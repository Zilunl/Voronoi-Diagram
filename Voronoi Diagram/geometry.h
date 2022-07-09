/* geometry: module for manipulating geometry
  - A geometry_t is essentially a (not reinforced) planar graph, 
    storing set V, E, F of vertices, edges and faces
  - A dcel_t is for storing data of a half edge	
  - A face is determined by one of its halfEdde
   
*/

/* Note: why do I put the struct defs here instead of in .c file?

  1. Ideally, I should put them on .c files to prevent access to
     the struct components from other modules/programs. In this 
     case, all members of a struct are "private" objects of the module

  2. But, sometimes I really need access
     to some members. Private members prevents that. And, I 
     need to write, say, a function just to get a single value.

  3. Ideally, I need a tool to declare some members as "public"
     to allow access to them. But that facility is not available in C

  4. So, I put the description here, knowing that it's dangerous
     since all members become "public". But that reduces some
	 coding.

  When possible, it's better to put the struct desription in .c files
*/ 
 

#ifndef _DCEL_H_
#define _DCEL_H_

#include "euclid_space.h"
#include "tower.h"

// types & structs:
typedef point_t vertex_t;
typedef struct geometry geometry_t;
typedef struct dcel dcel_t;
typedef struct cut cut_t;



/* dcel_t: for a half-edge,
           but a chain of dcel_t represents a polygon with a its face
*/ 
struct dcel {
    int startV;    // index of the vertex at the start of the half-edge.
    int endV;      // index of the vertex at the end of the half-edge
    dcel_t *next;  // pointer to the next half-edge in the face.
    dcel_t *prev;  // pointer to the previous half-edge in the face
    dcel_t *mate;  // pointer to the other half-edge (opposit dorection)
    int face;      // index of the incident face, can be UNDEF
    int edge;      // index of the original undirectional edge 
};

/* Each edge should have: 
     pointer to either (non-NULL) half-edge in the edge
	Note that the halfEdge specify the start and end vertices 
*/
typedef dcel_t* edge_t;

/* Each face should have:
    pointer to any half-edge in the face.
*/
typedef struct face face_t;
struct face {
    dcel_t *he;
    tower_t *tower;
};

/* We can probably start from F[0] to process all sub-divisions
   We probably CANNOT use E[0] for that purpose, as it might have been deleted
*/
struct geometry {  // actually is a graph, with additional "faces"
    vertex_t **V;   // array of vertices
    int nV;
    int sizeV;
    dcel_t **E;     // array of edges
    int nE;
    int sizeE;
    face_t **F;     // array of faces
    int nF;
    int sizeF;
};

struct cut {
	int startE;
	int endE;
};


/*---- new stuffs for Assignment 2, adapted from Grady base code --------*/

typedef struct split split_t;
struct split {
    dcel_t *startHE;
    dcel_t *endHE;
    vertex_t startPoint;
    vertex_t endPoint;
    // int verticesSpecified;   // =?, perhaps we don't need
};

dcel_t *doSplit(split_t *s, geometry_t *g);



// public functions (note: private functions not declared here)
geometry_t *buildGeoFromPolygonFile(char *polygonFName);
void doCut(cut_t *cut, geometry_t *g); 
geometry_t *freeGeo(geometry_t *g);
int findFace(point_t *p, geometry_t *g);
void printFace(FILE *f, geometry_t *g, int faceID);
double distance(vertex_t *a, vertex_t *b);

dcel_t *doBiSplit(split_t *s, geometry_t *g, int face);
#endif
