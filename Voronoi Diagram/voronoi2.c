#include "utils.h"
#include "tower.h"
#include "geometry.h"

#include "math.h"

#define STAGE1 1 
#define STAGE2 2 
#define STAGE3 3 
#define STAGE4 4 

/* I use Anh's base code, all good:), But some problems arise because 
   maybe in some cases the segmentAndLineIntersects function cannot 
   find the intersection that is already a vertex, and it needs to be 
   searched under certain circumstances. I believe the whole logic is correct. */

void processArg(int argc, char **argv, int *stage, char **towerFName,
	char **polygonFName, char **outFName, char **pointsFName); 

void doStage1(char *pointsFName, FILE *outFile);
void doStage2(char *pointsFName, char *polygonFName, FILE *outFile);
void doStage34(int stage, char *towerFName, char *polygonFName, FILE *outFile);

void getDiameter(tower_t **T, int nT, geometry_t* g);
void sortByDiaetre(tower_t **T, int nT);

int main(int argc, char **argv){

    char *towerFName, 
         *polygonFName,
         *outFName,
	     *pointsFName;
	int stage;
	processArg(argc, argv, &stage, &towerFName, &polygonFName,
	            &outFName, &pointsFName);
   
	FILE *outFile= my_fopen(outFName, "w");

	if (stage==STAGE1) 
		doStage1(pointsFName, outFile);
	else if (stage==STAGE2) 
		doStage2(pointsFName, polygonFName, outFile);
	else 
		doStage34(stage, towerFName, polygonFName, outFile);

	fclose(outFile);
	return 0;
}


/*----------------------- Stage 1 --------------------- */ 
/* Stage 1:
   Outputs the bisectors for the given file pointsFName
*/		
void doStage1(char *pointsFName, FILE *outFile) {
	FILE *inFile= my_fopen(pointsFName, "r");
	point_t a, b;
	bisector_t bs;
	while (fscanf(inFile, "%lf %lf %lf %lf", &a.x, &a.y, &b.x, &b.y) == 4) {
		getBisector(&bs, &a, &b);
		printBisector(outFile, &bs);
	}
	fclose(inFile); 
}


/*----------------------- Stage 2 --------------------- */ 
/* Stage 2:
  1. Makes the first face from the file polygonFName
  2. Outputs the intersections of the bisectors of points in pointsFName
                                              with the polygon 
*/		
void doStage2(char *pointsFName, char *polygonFName, FILE *outFile) {

	// 1. reads polygon file and build the single-face geometry g 
	geometry_t  *g = buildGeoFromPolygonFile(polygonFName);

	// 2. Process bisector one by one 
	FILE *inFile= my_fopen(pointsFName, "r");
	point_t a, b;
	bisector_t bs;
	double k;
	while (fscanf(inFile, "%lf %lf %lf %lf", &a.x, &a.y, &b.x, &b.y) == 4) {

		// get bisector 
		getBisector(&bs, &a, &b);
		if (a.y == b.y){
			k=INFINITY;
		} else {
			k=bs.gradient;
		}

		// find interPoint
		int count =1;
		vertex_t mid= bs.mid;
		dcel_t *he= g->F[0]->he, *this= he;
		do {
		vertex_t *aStart= g->V[this->startV];
		vertex_t *aEnd= g->V[this->endV];
		vertex_t interPoint;
		{
			if (segmentAndLineIntersects(aStart, aEnd, k, &mid, &interPoint)==INTERSECT) {
           		if (count == 1){
           			fprintf(outFile, "From Edge %d (%lf, %lf)",
                   	this->edge, interPoint.x, interPoint.y);
					count++;
				} else {
					fprintf(outFile, " to Edge %d (%lf, %lf)\n",
                   	this->edge, interPoint.x, interPoint.y);
				}
			}
       	}this= this->next;
		} while (this != he);
	}
	fclose(inFile);
	
	// cleaning 
	freeGeo(g);
} 

/*----------------------- Stage 3 and 4 --------------------- */ 
/* Stage 3/4:
  1. Makes the first face from the file polygonFName
  2. Genarates voronopi cell for each tower from file twoerFName
  3. (stage 4 only) sorts the tower by diametre of their faces
  4. Outputs the towers
*/
void doStage34(int stage, char *towerFName, char *polygonFName, FILE *outFile) {
	// 1. reads polygon file and build the single-face geometry g 
	geometry_t  *g = buildGeoFromPolygonFile(polygonFName);
	
	// 2. reads towers and build dynamic array of (watch)towers
	tower_t **T; int nT=0;
    nT = readTowerFile(towerFName, &T);

	// 3. Processing tower by tower, build its cell   
	point_t a, b;
	bisector_t bs;
	split_t split;
	double k;
	int face;
	for (int i=0; i<nT; i++) {
		// add Voronoi cell for the tower T[i] 
		// T[i]->face= getFace(&(T[i]->location), g);
		// addVoronoi
		// UNRESOLVED: what if the tower is outside of the polygon?	
		T[i]->face = i;
		if (i == 0){
			g->F[i]->tower = T[i];
		} else {
			// Find face, add bisector
			point_t *p = &T[i]->location;
			face = findFace(p, g);
			point_t *p1 = &T[face]->location;

			a = g->F[face]->tower->location;
			b = T[i]->location;
			getBisector(&bs, &a, &b);

			if (a.y == b.y){
				k=INFINITY;
			} else {
				k=bs.gradient;
			}

			// find interPoint
			int count =1;
			vertex_t mid= bs.mid;
			dcel_t *he= g->F[face]->he, *this= he;
			do {
			vertex_t *aStart= g->V[this->startV];
			vertex_t *aEnd= g->V[this->endV];
			vertex_t interPoint;
			{
				if (segmentAndLineIntersects(aStart, aEnd, k, &mid, &interPoint)==INTERSECT) {
           			if (count == 1){
           				split.startPoint = interPoint;
						split.startHE = this;
						count++;
					} else {
						split.endPoint = interPoint;
						split.endHE = this;
						count++;
					}
				}
      		}this= this->next;

			/* decide the direction of bisector */
			if (count==3 && !rightSide(p1, &split.startPoint, &split.endPoint)){
				vertex_t temp1 = split.startPoint;
				dcel_t *temp2 = split.startHE;
				split.startPoint = split.endPoint;
				split.startHE = split.endHE;
				split.endPoint = temp1;
				split.endHE = temp2;
			}
			
		} while (this != he);
		doBiSplit(&split, g, face);
		g->F[i]->tower=T[i];
		}

		/* make more bisector with those edge have mate */
		if (i == 0){
		dcel_t *he1= g->F[i]->he, *this1= he1;
		this1 = this1->next;
		while(this1 != he1){

			if (this1->mate != NULL){
				int face1=this1->mate->face;
				a = T[i]->location;
				b = T[face1]->location;
				getBisector(&bs, &a, &b);\

				if (a.y == b.y){
					k=INFINITY;
				} else {
					k=bs.gradient;
				}

				// find interPoint
				int count =1;
				vertex_t mid= bs.mid;
				dcel_t *he2= g->F[face1]->he, *this2= he2;
				if (g->nV-1 == this1->startV){
					split.startPoint = *g->V[this1->mate->endV];
				} else if (g->nV == this1->endV){
					split.startPoint = *g->V[this1->mate->startV];
				}

				split.startHE = this1->mate;
				
				do {
				vertex_t *aStart= g->V[this2->startV];
				vertex_t *aEnd= g->V[this2->endV];
				vertex_t interPoint;
				{
					if (segmentAndLineIntersects(aStart, aEnd, k, &mid, &interPoint)==INTERSECT) {
           				if (count == 1){
           					split.endPoint = interPoint;
							split.endHE = this2;
							count++;
						} else if (count==2) {
							split.startPoint = interPoint;
							split.startHE = this2;
							count++;
						}else if (count==3){
							split.endPoint = interPoint;
							split.endHE = this2;
							count++;
						}
						
					}
					
      			}this2= this2->next;
				
				// decide the direction of bisector 
				if (!rightSide(&T[face1]->location, &split.startPoint, &split.endPoint)){
					vertex_t temp1 = split.startPoint;
					dcel_t *temp2 = split.startHE;
					split.startPoint = split.endPoint;
					split.startHE = split.endHE;
					split.endPoint = temp1;
					split.endHE = temp2;
				}
			
				} while (this2 != he2);
				doBiSplit(&split, g, face1);

				// merge face and clean those no used
				dcel_t *oldface= g->F[i]->he, *oldhe;
				while (oldface->edge != this1->edge){
					oldface = oldface->next;
				}
				oldhe = oldface->prev;
				dcel_t *newface=g->F[i+1]->he->next->next, *newhe;
				while (newface->edge != this1->edge){
					newface = newface->next;
				}
				newhe = newface->next;

				// linked each eage in the face
				while (newhe->edge != this1->mate->edge){
					oldhe->next = newhe;
					newhe->prev = oldhe;
					oldhe = oldhe->next;
					newhe = newhe->next;
				}
				oldhe->next=this1->next;
				this1->prev=oldhe;

				g->nF--;

			}
			this1 = this1->next;
		}
		}
		
	}

	// 4. compute diametre & sort the tower array
	getDiameter(T, nT, g);
	if (stage==STAGE4) {
		 sortByDiaetre(T, nT);
	}

	// 5. outputs as required 
	for (int i=0; i<nT; i++){
		outputTower(outFile, T[i]);
	}
	
	// 6. clearning
	endTower(T, nT);
	freeGeo(g);
}



/*----------------------- processing args --------------------- */ 
void processArg(int argc, char **argv, int *stage, char **towerFName,
	char **polygonFName, char **outFName, char **pointsFName) {
	int valid= argc >1 && atoi(argv[1])>=1 && atoi(argv[1])<=4;
	if (valid) {
		*stage= atoi(argv[1]);
		valid = (*stage==STAGE1 && argc == 4) || 
		        (*stage>=STAGE2 && *stage<=STAGE4 && argc==5);
	}
	if (!valid){
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "\t%s %d point_pairs_file output_file\n"
	                "\t%s %d point_pairs_file polygon_file output_file\n"
	                "\t%s %d csv_data_file polygon_file output_file\n"
	                "\t%s %d csv_data_file polygon_file output_file\n",
	                argv[0], STAGE1, argv[0], STAGE2, argv[0], STAGE3, argv[0],STAGE4);
		exit(EXIT_FAILURE);
	}
	*towerFName= *pointsFName= *polygonFName= NULL;
	if (*stage==STAGE1) {
		*pointsFName= argv[2];
		*outFName= argv[3];
	} else {
		*polygonFName= argv[3];
		*outFName= argv[4];
		if (*stage==2) {
			*pointsFName= argv[2];
		} else {
			*towerFName= argv[2];
		}
	} 
}

void getDiameter(tower_t **T, int nT, geometry_t* g) {

	for (int i=0; i<nT; i++){
		double diameter = 0.0;
		point_t *p = &T[i]->location;
		int face = findFace(p, g);
		dcel_t *he= g->F[face]->he, *this= he;
		do {
			vertex_t *aStart= g->V[this->startV];
			dcel_t *this1 = this->next;
			{while (this1 != he){
				vertex_t *bStart= g->V[this1->startV];
				/*double length = sqrt((aStart->x-bStart->x)*(aStart->x-bStart->x)
				 + (aStart->y-bStart->y)*(aStart->y-bStart->y));*/
				 double length= distance(aStart, bStart);
				if (length>diameter){
					diameter = length;
				}
				this1 = this1->next;
			}

			}this= this->next;
		} while (this != he);
		T[i]->diametre = diameter;
		//printf("%d %s %lf\n",i,T[i]->id, diameter);
	}
		

}

/*void sortByDiaetre(tower_t **T, int nT){
	for (int i=0; i<nT-1; i++){
		for (int k=1; k<nT; k++){
			if (T[k]->diametre<T[i]->diametre){
				tower_t *temp=T[k];
				T[k]=T[i];
				T[i]=temp;
			}
		}
	}
}*/

void sortByDiaetre(tower_t **T, int nT){
	int i,j;
	tower_t *temp;
	for (i=1; i<nT; i++){
			temp=T[i];
			j=i-1;
			while(j>=0 && (T[j]->diametre) > (temp->diametre)){
				T[j+1]=T[j];
				j--;
			}
			T[j+1]=temp;
	}
}


