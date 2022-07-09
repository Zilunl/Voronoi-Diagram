#ifndef _TOWER_H_
#define _TOWER_H_

#include "euclid_space.h"


#define FIELD_LEN 128
#define LINE_LEN 512

typedef struct tower tower_t;
struct tower {
    char *id;
    char *postcode;
    int population;
    char *contact;
	point_t location;
	int face;             /* faceID of the Voronoi cell; */
	double diametre;      /* Cell/Face diametre */
};

/* read file into dynamic array (*pT)[], which having capacity *sizeT */
int readTowerFile(char *towerFName, tower_t ***pT);

/* print face ID, then print all towers of that face
   returns total population in the face   [not quite generous!]
*/
int outputTowerByGroup(FILE *f, tower_t **T, int n, int face); 

// returns total pop of face 
int popByGroup(tower_t **T, int n, int face);


/* finish job with Towers, cleaning up */
int endTower(tower_t **pT, int nT);
void outputTower(FILE *f, tower_t *t);
	
#endif
