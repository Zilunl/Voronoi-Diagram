/* geometry: module for manipulating geometry
  - A geometry_t is essentially a (not reinforced) planar graph, 
    storing set V, E, F of vertices, edges and faces
  - An edge is just an un-ordered pair (v1,v2) of 2 vertices 
  - A dcel_t is for storing data of a halfEdge from v1-->v2 (vertices)
    in a circular doubly-linked list of halfEdges of connected e-. Each linked list represents
	a face   	
  - A face is a pointer: chnaged for ass2 face= (halfEdge, tower) 
   
*/

#include "utils.h"
#include "geometry.h"
#include "dynamic_array.h"




geometry_t *newGeo(){
    geometry_t *geo = my_malloc(sizeof(*geo));
	arrayInit((void***) &(geo->V), &(geo->nV), &(geo->sizeV));
	arrayInit((void***) &(geo->E), &(geo->nE), &(geo->sizeE));
	arrayInit((void***) &(geo->F), &(geo->nF), &(geo->sizeF));
    return geo;
}

/*---------------- TOOLS for halfEdges & DCEL  ---------------------*/
/* Dictionnary:
    halfEdge  = represented by a dcel_t, is a (directional) edge of a face
    DECL=  a list of the halfEdges that compound a face
           which is a circlular doubly linked list of  halfEdges
           represented by a "dcel_t *"
*/  

// join the HE "this" into a dcel, in between "prev" and "next"
//    note that prev and next can be NULL
void insertHE(dcel_t *prev, dcel_t *this, dcel_t *next) {
	if (prev) {
		prev->next= this;
		this->prev= prev;
	}
	if (next) {
		next->prev= this;
		this->next= next;
	}
}

// creates then inserts a new HE  inbetween "prev" and "next"
dcel_t *newHE(int startV, int endV, dcel_t *prev, dcel_t *next, dcel_t *mate, int edgeID, int faceID){

    dcel_t *he = my_malloc(sizeof(*he));
    he->startV = startV;
    he->endV = endV;
    he->face = faceID;
    he->edge = edgeID;
	he->mate = mate;
	insertHE(prev, he, next);

    return he;
}

face_t *newFace(dcel_t *he, tower_t *t) {
    face_t *new= my_malloc(sizeof(*new));
    new->he= he; 
    new->tower= t;
    return new;
}

void freeDcel(dcel_t *start) {
	dcel_t *this= start->next;
	dcel_t *he;

	do {
		he= this;
		this= this->next;
	    free(he); 
	} while (this != start);
	he= start;

	free(start);
}

void freeFace(face_t *face) {
    freeDcel(face->he);
    free(face);
}

/*---------------- BASIC FUNCTIONS & DEBUGGING ---------------------*/
// prints face info for debugging
void printFace(FILE *f, geometry_t *g, int faceID) {
    fprintf(f, "Details for face %d:\n", faceID);
    assert (faceID  < g->nF);
    dcel_t *start= g->F[faceID]->he, *this= start;
    do {
        fprintf(f, "edge %d  vertex %d-->%d (%.1lf,%.1lf) --> (%.1lf,%.1lf) face= %d %s mate\n",
                    this->edge, this->startV, this->endV,
                    g->V[this->startV]->x, g->V[this->startV]->y,
                    g->V[this->endV]->x, g->V[this->endV]->y,
                    this->face, this->mate? "YES":"NO");
        this= this->next;
    } while (this != start);
}

					  
/*--------- Reading Original Polygon to build the first face -----------*/

/* NOTES:
  In the context of ass1, we call this function only once to build the
original boundary area as a single face. Note that in the future, new faces 
are formed just by dividing existing faces.
  The whole geometry is hence defined by the array of faces.
  This function is NOT for adding a polygon into an existing geometry!
*/

geometry_t *buildGeoFromPolygonFile(char *polygonFName) {
	geometry_t *g= newGeo();
	FILE *f= my_fopen(polygonFName, "r");
	int startV= 0, endV= UNDEF;
	vertex_t v;
	int faceID= 0;
	// Will build a DCEL list
	//   that forms (the face of) the new polygon. related variable:
	dcel_t *first= NULL;  // "first" is also used as the "head" 
	dcel_t *this= NULL;   // current == the last element of the DCEL chain
	dcel_t *prev=NULL;			

	while (fscanf(f," %lf %lf", &(v.x), &(v.y))==2) {
		// 1. adds the new vertex to V
		vertex_t *p= my_malloc(sizeof(*p));
		*p= v;
		endV= g->nV; 	  // id of next vertex in array g->V
		arrayAppend((void***) &g->V, &g->nV, &g->sizeV, p);
		if (g->nV < 2) continue;    // no new edge, no edge to be added
		
		// 2. builds the halfEdge startV-->endV of the being formed face,
		int edgeID= g->nE;				// id of new edge
		this = newHE(startV, endV, prev, NULL, NULL, edgeID, faceID);
		if (!first) first= this;
		
		// 3. adds new edge & updates for next round
		arrayAppend((void***) &g->E, &g->nE, &g->sizeE, this);
		startV= endV;
		prev= this;
	}
	fclose(f);
	if (g->nV==0) return g;    // returns empty geo!
	assert(g->nV > 2);

	// adding final edge & halfedge
	this = newHE(startV, 0, this, first, NULL, g->nE , faceID);
	arrayAppend((void***) &g->E, &g->nE, &g->sizeE, this);
	
    // adding the new face to the geometry
    face_t *face= newFace(first, NULL);
    arrayAppend((void***) &g->F, &g->nF, &g->sizeF, face);
	
	return g;
}
		
/*---------------- TOOLS FOR PERFORMING A CUT ---------------------*/

// Given a cut c
//   finds & returns the face that the cut belongs to
//   also sets up the starting and ending halfEdges of the cut
// Method: check if the cut is associated two same-face halfEdges
int cutGetFaceAndHE(cut_t *c, geometry_t *g, dcel_t **pstartHE, dcel_t **pendHE) {
	*pstartHE= g->E[c->startE], *pendHE= g->E[c->endE];

	int round= 0;	// loop for trying both pstartHE and its mate
	while (round<2 && *pstartHE &&  (*pstartHE)->face != (*pendHE)->face ) {
		if ((*pendHE)->mate && (*pstartHE)->face== (*pendHE)->mate->face) {	
			*pendHE= (*pendHE)->mate;
			break;
		}
		*pstartHE= (*pstartHE)->mate;
		round++;
	}
	assert (*pstartHE && *pendHE && (*pstartHE)->face == (*pendHE)->face );
	return (*pstartHE)->face;
}

// performs a cut on geometry g
void doCut(cut_t *c, geometry_t *g) {
	// 1. finds the face and the halfEdges that envoled in the cut
	dcel_t *startHE= NULL, *endHE= NULL;
	/*int faceID= */ cutGetFaceAndHE(c, g, &startHE, &endHE);
	// 2. finds the midpoints of the cut's edges 
	vertex_t *startV= mid2points(g->V[startHE->startV], g->V[startHE->endV]);
	vertex_t *endV= mid2points(g->V[endHE->startV], g->V[endHE->endV]);
	split_t split= {startHE, endHE, *startV, *endV};
    doSplit(&split, g);
}
	

/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  START OF SPLIT FUNCTION ---------------------------- */

// this function should be in euclide_space.c and .h
//    it's here to minimize the effort of students to chnage thier code
double distance(vertex_t *a, vertex_t *b) {
    double dx= a->x - b->x;
    double dy= a->y - b->y;
    return sqrt(dx*dx + dy*dy); 
}

// this function should be in euclide_space.c and .h
//    it's here to minimize the effort of students to chnage thier code
// returns 1 if the 2 vertices are too close
int isSameVertex(vertex_t *a, vertex_t *b) {
    return distance(a,b) < EPS;
}


// do the split "s",
//    Input: the split s, where
//        - the s->startHE or s->newHE determine the old face
//          also determinne the starting HE and ending HE of the split
//        - s->startPoint, s->endPoint are coordinates of end points of the split
//    Output:
//        - the old face is divided into old face and new face
//        - the new face has ID of g->nF-1
//        - return the halfEdge of the new face that divides the old and new face
//    Notes:
//        To deal with the split from an end of the edges, the algorithm:
//         - 2 vertices are considered as the same if their fistance is less than EPS
//           (EPS = 10^-9 as defined in euclid_space.h)
//        - changes the startHE and endHE of the split if there are some incinsistency
//        - not creates zero-length edges
//
dcel_t *doSplit(split_t *s, geometry_t *g) {
    // 1. preparation: testing for inconsistency & exchange
    if (isSameVertex( &(s->startPoint), g->V[s->startHE->startV]))
        s->startHE= s->startHE->prev;
    if (isSameVertex( &(s->endPoint), g->V[s->endHE->endV]))
        s->endHE= s->endHE->next;

    dcel_t *startHE= s->startHE;
    dcel_t *endHE= s->endHE;

    int faceID= s->startHE->face;

    int he1Created= isSameVertex( &(s->startPoint), g->V[startHE->endV])? 0 : 1;
    int he2Created= isSameVertex( &(s->endPoint), g->V[endHE->startV])? 0 : 1;

    //  2. add 2 vertices
    int newfaceID= g->nF;   // will build a new face with this ID
    vertex_t *startV, *endV;
    int startVID, endVID;
    if (he1Created) {
        startVID= g->nV;
        startV= newVertex(s->startPoint.x, s->startPoint.y);
        arrayAppend((void***) &g->V, &g->nV, &g->sizeV, startV);
    } else {
        startVID=  startHE->endV;
        startV= g->V[startVID];
    }
    if (he2Created) {
        endVID= g->nV;
        endV= newVertex(s->endPoint.x, s->endPoint.y);
        arrayAppend((void***) &g->V, &g->nV, &g->sizeV, endV);
    } else {
        endVID= endHE->startV;
        endV= g->V[endVID];
    }

    // 3. creats unconnected he0 and its mate he0mate
    int e0id= g->nE;
    dcel_t *he0= newHE(startVID, endVID, NULL, NULL, NULL, e0id, faceID);
    dcel_t *he0Mate= newHE(endVID, startVID, NULL, NULL, he0, e0id, newfaceID);
    he0->mate = he0Mate;

    // 4. adds associated edge & face to the geometry
    arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he0);
    arrayAppend((void***) &g->F, &g->nF, &g->sizeF, newFace(he0Mate,NULL));
    g->F[faceID]->he= he0;

    // 5. makes new HE he1, he2 for the new face, also makes corresponding edges
    dcel_t *he1= NULL, *he2= NULL;
    int e1ID= UNDEF, e2ID= UNDEF;
    if (he1Created) {
        he1= newHE(startVID, startHE->endV, NULL, NULL, NULL, g->nE,newfaceID);
        e1ID= g->nE;
        arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he1);
    } else {
        he1= startHE->next;
        e1ID= he1->edge;
        he1Created= 0;
    }

    if (he2Created) {
        he2= newHE(endHE->startV, endVID, NULL, NULL, NULL, g->nE,newfaceID);
        e2ID= g->nE;
        arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he2);
    } else {
        he2= endHE->prev;
        e2ID= he2->edge;
        he2Created= 0;
    }

    // 6. joins he1 and he2 to the new face
    if (startHE->endV == endHE->startV            // if 2 cut's edges are adjacent
            || he1->endV== endHE->startV           // even when have zerolength
            || startHE->endV== he2->startV ) {
        insertHE(he0Mate, he1, he2);
        insertHE(he1, he2, he0Mate);
    } else {                             //  if not adjacent
        dcel_t *next= he1->next? he1->next : startHE->next;
        insertHE(he0Mate, he1, next);
        dcel_t *prev= he2->prev? he2->prev : endHE->prev;
        insertHE(prev, he2, he0Mate);
    }
    // set face for he of new face 
    dcel_t *tmp= he0->mate;    
    do {
        tmp->face= newfaceID;
        tmp= tmp->next;
    } while (tmp != he0->mate);

    // 7. repairs the original halfEdges of the original face
    startHE->endV= startVID;
    endHE->startV= endVID;
    insertHE(startHE, he0, endHE);

    // 8. repairs and adds mates of the just-built halfEdges if needed
    dcel_t *mate;
    if ( (mate= startHE->mate) && he1Created ) {
        he1->mate= newHE(he1->endV, he1->startV, mate->prev, mate, he1, e1ID, mate->face);
        mate->startV= he1->startV;
    }
    if ( (mate= endHE->mate) && he2Created) {
        he2->mate= newHE(he2->endV, he2->startV, mate, mate->next, he2, e2ID, mate->face);
        mate->endV= he2->endV;
    }
//int i;
//or (i=0; i<g->nF; i++) printFace(stdout, g, i);

    return he0Mate;
}

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< END OF SPLIT FUNCTION ---------------------------- */
dcel_t *doBiSplit(split_t *s, geometry_t *g, int face) {
	// 1. preparation: testing for inconsistency & exchange
    if (isSameVertex( &(s->startPoint), g->V[s->startHE->startV]))
        s->startHE= s->startHE->prev;
    if (isSameVertex( &(s->endPoint), g->V[s->endHE->endV]))
        s->endHE= s->endHE->next;

    dcel_t *startHE= s->startHE;
    dcel_t *endHE= s->endHE;

    int faceID= s->startHE->face;

    int he1Created= isSameVertex( &(s->startPoint), g->V[startHE->endV])? 0 : 1;
    int he2Created= isSameVertex( &(s->endPoint), g->V[endHE->startV])? 0 : 1;

    //  2. add 2 vertices
    int newfaceID= g->nF;   // will build a new face with this ID
    vertex_t *startV, *endV;
    int startVID, endVID;
    if (he1Created) {
        startVID= g->nV;
        startV= newVertex(s->startPoint.x, s->startPoint.y);
        arrayAppend((void***) &g->V, &g->nV, &g->sizeV, startV);
    } else {
        startVID=  startHE->endV;
        startV= g->V[startVID];
    }
    if (he2Created) {
        endVID= g->nV;
        endV= newVertex(s->endPoint.x, s->endPoint.y);
        arrayAppend((void***) &g->V, &g->nV, &g->sizeV, endV);
    } else {
        endVID= endHE->startV;
        endV= g->V[endVID];
    }

	// 3. creats unconnected he0 and its mate he0mate  
	int e0id= g->nE;
	dcel_t *he0= newHE(startVID, endVID, NULL, NULL, NULL, e0id, faceID);
	dcel_t *he0Mate= newHE(endVID, startVID, NULL, NULL, he0, e0id, newfaceID);
	he0->mate = he0Mate;

	// 4. adds associated edge & face to the geometry 
	arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he0);
	arrayAppend((void***) &g->F, &g->nF, &g->sizeF, newFace(he0Mate,NULL));
	g->F[faceID]->he= he0;
	
	// 5. makes new HE h1, h2 for the new face, also makes corresponding edges
	dcel_t *he1= NULL, *he2= NULL;
    int e1ID= UNDEF, e2ID= UNDEF;
    if (he1Created) {
        he1= newHE(startVID, startHE->endV, NULL, NULL, NULL, g->nE,newfaceID);
        e1ID= g->nE;
        arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he1);
    } else {
        he1= startHE->next;
        e1ID= he1->edge;
        he1Created= 0;
    }

    if (he2Created) {
        he2= newHE(endHE->startV, endVID, NULL, NULL, NULL, g->nE,newfaceID);
        e2ID= g->nE;
        arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he2);
    } else {
        he2= endHE->prev;
        e2ID= he2->edge;
        he2Created= 0;
    }

	// 6. joins he1 and he2 to the new face 
	if (startHE->endV == endHE->startV
			|| he1->endV== endHE->startV           // even when have zerolength
            || startHE->endV== he2->startV) { // if 2 cut's edges are adjacent
		insertHE(he0Mate, he1, he2); 
		insertHE(he1, he2, he0Mate);
	} else {                             //  if not adjacent
        dcel_t *next;       // the next node of h1
        if (he1Created) 
            next= startHE->next;
        else 
            next = he1->next;   // h1 was a existing HE
        // join he1 in order:   he0Mate --> he1 --> next
        insertHE(he0Mate, he1, next);

        dcel_t *prev;       // the prev node of h2
        if (he2Created) 
            prev= endHE->prev;
        else 
            prev = he2->prev;   // h2 was a existing HE, we won't change its prev
        // join he1 in order:   prev --> he2 --> h0Mate
        insertHE(prev, he2, he0Mate);
		/*insertHE(he0Mate, he1, startHE->next); 
		insertHE(endHE->prev, he2, he0Mate);*/
		
	}
	dcel_t *tmp= he0->mate;    
    do {
        tmp->face= newfaceID;
        tmp= tmp->next;
    } while (tmp != he0->mate);
		 
	// 7. repairs the original halfEdges of the original face
	startHE->endV= startVID;
	endHE->startV= endVID;
	insertHE(startHE, he0, endHE); 
	// 8. repairs and adds mates of the just-built halfEdges if needed
	dcel_t *mate;
    if ( (mate= startHE->mate) && he1Created ) {
        he1->mate= newHE(he1->endV, he1->startV, mate->prev, mate, he1, e1ID, mate->face);
        mate->startV= he1->startV;
    }
    if ( (mate= endHE->mate) && he2Created) {
        he2->mate= newHE(he2->endV, he2->startV, mate, mate->next, he2, e2ID, mate->face);
        mate->endV= he2->endV;
    }
	return he0Mate;
    
}
/*dcel_t *doBiSplit(split_t *s, geometry_t *g, int face) {
	vertex_t *startV= newVertex(s->startPoint.x, s->startPoint.y);
	vertex_t *endV= newVertex(s->endPoint.x, s->endPoint.y);
	dcel_t *startHE= s->startHE;
	dcel_t *endHE= s->endHE;

	int faceID= s->startHE->face;
 
    //  1 & 2. add 2 vertices 
	int newfaceID= g->nF;   // will build a new face with this ID
	int startVID= g->nV, endVID= g->nV+1;
	arrayAppend((void***) &g->V, &g->nV, &g->sizeV, startV);
	arrayAppend((void***) &g->V, &g->nV, &g->sizeV, endV);

	// 3. creats unconnected he0 and its mate he0mate  
	int e0id= g->nE;
	dcel_t *he0= newHE(startVID, endVID, NULL, NULL, NULL, e0id, faceID);
	dcel_t *he0Mate= newHE(endVID, startVID, NULL, NULL, he0, e0id, newfaceID);
	he0->mate = he0Mate;

	// 4. adds associated edge & face to the geometry 
	arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he0);
	arrayAppend((void***) &g->F, &g->nF, &g->sizeF, newFace(he0Mate,NULL));
	g->F[faceID]->he= he0;
	
	// 5. makes new HE h1, h2 for the new face, also makes corresponding edges
	dcel_t *he1= newHE(startVID, startHE->endV, NULL, NULL, NULL, g->nE,newfaceID);
	int e1ID= g->nV;
	arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he1);
	dcel_t *he2= newHE(endHE->startV, endVID, NULL, NULL, NULL, g->nE,newfaceID);
	int e2ID= g->nV;
	arrayAppend((void***) &g->E, &g->nE, &g->sizeE, he2);

	// 6. joins he1 and he2 to the new face 
	if (startHE->endV == endHE->startV) { // if 2 cut's edges are adjacent
		insertHE(he0Mate, he1, he2); 
		insertHE(he1, he2, he0Mate);
	} else {                             //  if not adjacent
		insertHE(he0Mate, he1, startHE->next); 
		insertHE(endHE->prev, he2, he0Mate);
		dcel_t *tmp;     // reset face for the taken away halfedges
		for (tmp= he1->next; tmp != he2; tmp= tmp->next) {
			tmp->face= newfaceID;
		}
		
	}
		 
	// 7. repairs the original halfEdges of the original face
	startHE->endV= startVID;
	endHE->startV= endVID;
	insertHE(startHE, he0, endHE); 
	// 8. repairs and adds mates of the just-built halfEdges if needed
	dcel_t *mate;
	if ( (mate= startHE->mate) ) { 
	    newHE(he1->endV, he1->startV, mate->prev, mate, he1, e1ID, mate->face);
		mate->startV= he1->endV;
	}
	if ( (mate= endHE->mate) ) { 
	    newHE(he2->endV, he2->startV, mate->prev, mate, he2, e2ID, mate->face);
		mate->endV= he2->endV;
	}
	return he0Mate;
}*/


// returns TRUE if point "p" belongs to the face having "start" as a halfedge
int pointInFace(point_t *p, dcel_t *start, geometry_t *g) {
	dcel_t *this= start;
	point_t *a, *b;
	do {  // "p" belongs if it is on the right side of all face's HEs
		a= g->V[ this->startV ];
		b= g->V[ this->endV ];
		if (!rightSide(p, a, b)) return FALSE;
		this= this->next;
	} while (this != start);
	return TRUE;
}
	
// returns the faceID of the face that contains point (x,y) inside
int findFace(point_t *p, geometry_t *g) {
	int i;
	for (i=0; i<g->nF; i++) {
		if (pointInFace(p, g->F[i]->he, g) ) return i;
	}
	return UNDEF; 
}

/*---------------- CLEAN-UP  FUNCTIONS ---------------------*/
					  
geometry_t *freeGeo(geometry_t *g) {
	// free the dcel lista
	int i;
	for (i=0; i<g->nV; i++) free(g->V[i]);
	free(g->V);
	free(g->E);	
	for (i=0; i<g->nF; i++) freeFace(g->F[i]);
	free(g->F);
	free(g);
	return NULL;
}
