#include "utils.h"
#include "tower.h"
#include "geometry.h"

int main(int argc, char **argv){
    if(argc !=4 ){
		fprintf(stderr, "Usage: %s towerFName polygonFName outputFName\n",
						argv[0]);
        exit(EXIT_FAILURE);
    }
    char *towerFName= argv[1];
    char *polygonFName= argv[2];
    char *outputFName= argv[3];
   
	// 1. reads file1 and build dynamic array of (watch)towers
	tower_t **T; int nT=0;
    nT = readTowerFile(towerFName, &T);
    
	// 2. reads file2 and build the single-face geometry g 
	geometry_t  *g = buildGeoFromPolygonFile(polygonFName);

	// 3. reads cuts from std in and do the corresponding division in g
	//    
	cut_t v;
	while (scanf(" %d %d", &v.startE, &v.endE) == 2) {
		assert( 0<=v.startE && v.startE<g->nE && 0<=v.endE && v.endE<g->nE);  
		doCut(&v, g);
	} 

	// 4. sets the face that each tower belong to
	//    ie for tower *t, set t->group= corresponding faceID
	
	int i;
	for (i=0; i<nT; i++) {
		T[i]->face= findFace(&(T[i]->location), g);
	}
     
	// 5. outputs as required for assignment 1
	FILE *f= my_fopen(outputFName,"w");
	int face;
	int *pops= my_malloc((g->nF+1) * sizeof(*pops));
	for (face=0; face < g->nF; face++) {
		fprintf(f, "%d\n", face);
 		pops[face] = outputTowerByGroup(f,T, nT, face);
	}
	#ifdef _PRINT_UNDEF
	pops[face]= 0;
	if (popByGroup(T, nT, UNDEF)) {
		fprintf(f, "%d\n", face);
 		pops[face] = outputTowerByGroup(f,T, nT, UNDEF);
	}
	#endif

	for (face=0; face < g->nF; face++) {
		fprintf(f, "Face %d population served: %d\n", face, pops[face]);
	}
	#ifdef _PRINT_UNDEF
	if (pops[face]) fprintf(f, "Face undefined population served: %d\n", pops[face]);
	#endif

	free(pops);
	fclose(f);
	// 6. clearning
	endTower(T, nT);
	g= freeGeo(g);
    return EXIT_SUCCESS;
}

