#include "utils.h"
#include "tower.h"
#include "dynamic_array.h"



//------ HEADER STUFFS: reading column names of csv file  -----------------
/* What's that?
   - read the first line of the csv file, which is the header line
   - then store the field titles in array dname[]
   - later, use that array dname[] to print out the search result
*/

// global variables & const for use in this module only
#define NUM_FIELDS 6            // number of fields, #defined for simplicity
static char *dname[NUM_FIELDS]; // array of column headers

// get rid of the header line of .csv, but
//   also gets and stores column titles in dname[]
void readTowerHeader(FILE *f) {
	char tmp[FIELD_LEN+1];
	int i;
	for (i=0; i<NUM_FIELDS-1; i++) {
		fscanf(f, " %[^,],", tmp);
		dname[i]= my_strdup(tmp);
	}
	fscanf(f, " %[^\n]", tmp);
	dname[i]= my_strdup(tmp);
}

// free dname[] 
void freeTowerHeader() {
	int i;
	for (i=0; i<NUM_FIELDS; i++) {
		free(dname[i]);
	}
}
//--------------------- END OF HEADER STUFFS -----------------


/* read next line from file f, form and return the coresponding data 
   returns NULL if EOF
*/ 
tower_t *getTower(FILE *f, int faceID) {
	char id[FIELD_LEN], postcode[FIELD_LEN], contact[FIELD_LEN];
	int population;
	point_t loc;
	if (fscanf(f," %[^,], %[^,], %d, %[^,], %lf, %lf",
	         id, postcode, &population, contact, &(loc.x), &(loc.y))!=NUM_FIELDS) {
		return NULL;
	}
	tower_t *t= my_malloc(sizeof(*t));
	t->id= my_strdup(id);	
	t->postcode= my_strdup(postcode);
	t->population= population;
	t->contact= my_strdup(contact);
	t->location= loc;
	t->face= UNDEF;
	t->diametre= UNDEF;
	return t;
}	
		
int readTowerFile(char *towerFName, tower_t ***pT) {
	FILE *f= my_fopen(towerFName, "r");
	readTowerHeader(f);
	tower_t **T= NULL;  // T is surrogate for *pT
	int sizeT, n;
	arrayInit((void***) &T, &n, &sizeT);
	tower_t *t= NULL;
	int i=0;
	while ( (t=getTower(f, i)) ) {   // get tower and set face= i
		arrayAppend((void***) &T, &n, &sizeT, t);
		i++;
	}
	fclose(f);
	*pT= T;
	return n;
}
	
// prints in the required format
//    
void outputTower(FILE *f, tower_t *t) {
	assert(t);
	int i;
	for (i=0; i<7; i++) {
		dname[6] = "Diameter of Cell";
		fprintf(f, "%s: ", dname[i]);
		switch (i) {
			case 0:
				fprintf(f,"%s, ", t->id);
				break;
			case 1:
				fprintf(f,"%s, ", t->postcode);
				break;
			case 2:
				fprintf(f,"%d, ", t->population);
				break;
			case 3:
				fprintf(f,"%s, ", t->contact);
				break;
			case 4:
				fprintf(f,"%.6lf, ", t->location.x);
				break;
			case 5:
				fprintf(f,"%.6lf, ", t->location.y);
				break;
			case 6:
				fprintf(f,"%.6lf\n", t->diametre);
				break;
		}
	}
}	


/* print face ID, then print all towers of that face
   returns total population in the face   [not quite generous!]
*/
int outputTowerByGroup(FILE *f, tower_t **T, int n, int face) {
	int i, pop= 0;
	for (i=0; i<n; i++) {
		tower_t *t= T[i];
		if (t->face != face) continue;
		outputTower(f, t);
		pop += t->population;
	}
	return pop;
}

// returns total pop of face 
int popByGroup(tower_t **T, int n, int face) {
	int i, pop= 0;
	for (i=0; i<n; i++) {
		tower_t *t= T[i];
		if (t->face != face) continue;
		pop += t->population;
	}
	return pop;
}



tower_t *freeTower(tower_t *t){
	assert(t);
	free(t->id);
	free(t->postcode);
	free(t->contact);
	free(t);
	return NULL;
}

/* finish job with Towers, cleaning up */
int endTower(tower_t **T, int nT) {
	int i;
	for (i=0; i<nT; i++) {
		freeTower(T[i]); 
	}
	free(T);
	freeTowerHeader();
	return 0;
} 

/* get the diameter of each tower */


