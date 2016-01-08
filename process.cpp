#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
//#include "glui.h"
#include "process.h"

GLuint vertexCount = 0;
GLuint faceCount = 0;

////reads from an smf file to a "winged-edge" data structure
WingedEdge* readShapeFile(char *name){
	FILE *shapeFile;
	GLuint countv = 0;
	GLuint countf = 0;
	GLfloat x, y, z;
	GLuint vert1, vert2, vert3;
	int i=0, j=0;

	Vertex **vertexList;
	Vertex *currentPoint, *prevPoint;
	Face *currentFace, *prevFace;
	Triangle **faceList;
	Edge **edgeList;

	WingedEdge *wEdge = new WingedEdge;

	shapeFile = fopen(name, "r");
	if(shapeFile){
		char ar[128]; //temp array to store file contents line by line
		printf("Loading the shape from SMF file %s...\n", name);
		while (fscanf(shapeFile, "%s", ar) != EOF) {
			switch(ar[0]){
				case '#':
					//read the number of verts and faces and allocate buffer sizes
					fscanf(shapeFile, "%d %d", &vertexCount, &faceCount);
					printf("Vertex count: %d\n", vertexCount);
					printf("Face count: %d\n", faceCount);
					break;
				case 'v':
					if(countv == 0)
						vertexList = new Vertex*[vertexCount];
					//vertex: get x,y,z coordinates
					fscanf(shapeFile, "%f %f %f", &x, &y, &z);
					//printf("vertex x coordinate: %f %f %f\n" ,x, y, z);
					vertexList[countv] = new Vertex;
					vertexList[countv]->x = x;
					vertexList[countv]->y = y;
					vertexList[countv]->z = z;
					//printf("vertex x coordinate: %f %f %f\n" ,vertexList[countv]->x, vertexList[countv]->y, vertexList[countv]->z);
					countv++;
					break;
				case 'f':
					if(countf == 0)
						faceList = new Triangle*[faceCount];
					//face: get 3 vertices that form the face
					fscanf(shapeFile, "%d %d %d", &vert1, &vert2, &vert3);
					faceList[countf] = new Triangle;
					faceList[countf]->vertices[0] = vert1 - 1;
					faceList[countf]->vertices[1] = vert2 - 1;
					faceList[countf]->vertices[2] = vert3 - 1;
					//printf("face vertex 1: %d %d %d\n", faceList[countf]->vertices[0], faceList[countf]->vertices[1], faceList[countf]->vertices[2]);
					countf++;
					break;
				default:
					fgets(ar, sizeof(ar), shapeFile);
					break;
			}
		}
	}
	else
		printf("Error in readShapeFle: failed to read the SMF file!\n");

	//create a vertex list and point the start to WingedEdge
	prevPoint = wEdge->startVertex = vertexList[0];
	//try this
	//prevPoint = vertexList[0];
	//wEdge->startVertex = prevPoint;

	for(i=1; i<vertexCount; i++){
		currentPoint = vertexList[i];
		prevPoint->next = currentPoint;
		prevPoint = currentPoint;
	}

	//printf("%f, %f, %f, %f \n", wEdge->startVertex->x, vertexList[0]->x, wEdge->startVertex->next->x, vertexList[1]->x);

	//create a starting face for Winged Edge
	wEdge->startFace = currentFace = new Face;
	
	/***populate the edge table**/
	edgeList = new Edge*[3 * faceCount];
	for(i=0; i<faceCount; i++){
		for(j=0; j<3; j++){
			edgeList[3*i + j] = new Edge;
			edgeList[3*i + j]->start = vertexList[faceList[i]->vertices[j%3]];
			edgeList[3*i + j]->end = vertexList[faceList[i]->vertices[(j+1)%3]];
			edgeList[3*i + j]->leftFace = currentFace;
		}

		currentFace->edge = edgeList[3*i];
		//printf("%f, %f\n", wEdge->startFace->edge->start->x, currentFace->edge->start->x);
		if (i+1 < faceCount){
			currentFace->next = new Face;
			prevFace = currentFace;
			currentFace = currentFace->next;
		}

		//after filling the edge table, find the leftPrevious and leftNext edges
		for(j=0; j<3; j++){
			edgeList[3*i + j]->leftNext = edgeList[3*i+(j+1)%3];
			edgeList[3*i + j]->leftPrevious = edgeList[3*i+(j+1)%3];
		}
	}
	//printf("%f\n", wEdge->startFace->next->next->next->next->next->next->next->edge->start->x);
	//printf("%f, %f\n", wEdge->startFace->next->edge->start->x, vertexList[faceList[1]->vertices[0]]->x);
	//printf("face vertex 1: %d %d %d\n", faceList[0]->vertices[0], faceList[0]->vertices[1], faceList[0]->vertices[2]);
	//printf("vertex x coordinate: %f %f %f\n" ,vertexList[0]->x, vertexList[0]->y, vertexList[0]->z);

	//Discover symmetric edges (same start, end vertices, reversed)
	for(i=0; i < 3*faceCount-1; i++){
		if(edgeList[i]->symmetric!=NULL)
			continue;
		for(j=i+1; j < 3*faceCount; j++){
			if(edgeList[j]->symmetric!=NULL)
				continue;
			if((edgeList[i]->start == edgeList[j]->end) && (edgeList[j]->start == edgeList[i]->end)){
				edgeList[i]->symmetric = edgeList[j];
				edgeList[j]->symmetric = edgeList[j];
				//printf("%f, %f\n", edgeList[i]->start->x, edgeList[j]->end->x);
			}
		}
	}
	return wEdge;
}

//write to SMF file
void writeShapeFile(WingedEdge* wEdge, char* fileName){
	FILE* outputFile;
	Face* face;
	Vertex* vertex;
	int i =0;
	GLint counter;
	
	outputFile = fopen(fileName, "w");
	if (!outputFile) {
		fprintf(stderr, "File open failed: writeShapeFile()\n");
		return;
	}
	fprintf(outputFile, "# %d %d\n", vertexCount, faceCount);
	counter = 1.0;
	vertex = wEdge->startVertex;
	for(i=0; i<vertexCount; i++){
		fprintf(outputFile, "v %f %f %f\n", vertex->x, vertex->y, vertex->z);
		vertex->count = counter;
		counter += 1;
		vertex = vertex->next;
	}
	face = wEdge->startFace;
	for(i=0; i<faceCount; i++){
		fprintf(outputFile, "f %d %d %d\n",
			(int)(face->edge->start->count),
			(int)(face->edge->leftPrevious->start->count),
			(int)(face->edge->leftPrevious->leftPrevious->start->count));
		face = face->next;
	}
	fclose(outputFile);
}

/*draw shape as a wireframe*/
void myShapeWire(WingedEdge *wEdge){
	Face *currentFace;
	Edge *currentEdge, *firstEdge;

	currentFace = wEdge->startFace;
	while(currentFace){
		firstEdge = currentFace->edge;
		currentEdge = firstEdge;
		glBegin(GL_LINE_LOOP);
		glNormal3f(currentFace->normals->x, currentFace->normals->y, currentFace->normals->z);
		while(1){
			glVertex3f(currentEdge->start->x, currentEdge->start->y, currentEdge->start->z);
			currentEdge = currentEdge->leftNext;
			if(currentEdge == firstEdge)
				break;
		}
		glEnd();
		currentFace = currentFace->next;
	}
}

/*draw shape as a solid - flat shaded*/
void myShapeSolid(WingedEdge *wEdge){
	Face *currentFace;
	Edge *currentEdge, *firstEdge;
	int i=0; int count=0; 

	currentFace = wEdge->startFace;
	while(currentFace){
		//printf("ph %d\n",count++);
		firstEdge = currentFace->edge;
		currentEdge = firstEdge;
		glBegin(GL_TRIANGLES);
		glNormal3f(currentFace->normals->x, currentFace->normals->y, currentFace->normals->z);
		//while(1){
		for(i=0; i<3; i++){
			glVertex3f(currentEdge->start->x, currentEdge->start->y, currentEdge->start->z);
			currentEdge = currentEdge->leftNext;
			//if(currentEdge == firstEdge)
				//break;
		}
		glEnd();
		currentFace = currentFace->next;
	}
}

/*draw shape as a solid - smooth shaded*/
void myShapeSolidSmooth(WingedEdge *wEdge){
	Face *currentFace;
	Edge *currentEdge, *firstEdge;
	int i =0;

	currentFace = wEdge->startFace;
	//glShadeModel(GL_SMOOTH);
	while(currentFace){
		firstEdge = currentFace->edge;
		currentEdge = firstEdge;
		glBegin(GL_TRIANGLES);
		//glNormal3f(currentFace->normals->x, currentFace->normals->y, currentFace->normals->z);
		for(i=0; i<3; i++){
			glNormal3f(currentEdge->start->vnormal->x, currentEdge->start->vnormal->y, currentEdge->start->vnormal->z);
			glVertex3f(currentEdge->start->x, currentEdge->start->y, currentEdge->start->z);
			currentEdge = currentEdge->leftNext;
		}
		glEnd();
		currentFace = currentFace->next;
	}
}

/*draw shape as shaded with edges displayed*/
void myShapeShadedEdge(WingedEdge *wEdge){
	Face *currentFace;
	Edge *currentEdge, *firstEdge;
	int i=0;
	//printf("test shape edge overlay\n");

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPolygonOffset(1.0, 1.0);
	//glColor3ub(255,255,255);
	//glColor3f( 1.0, 1.0, 1.0);

	currentFace = wEdge->startFace;
	while(currentFace){
		firstEdge = currentFace->edge;
		currentEdge = firstEdge;
		glBegin(GL_TRIANGLES);
		//glNormal3f(currentFace->normals->x, currentFace->normals->y, currentFace->normals->z);
		for(i=0; i<3; i++){
			glNormal3f(currentEdge->start->vnormal->x, currentEdge->start->vnormal->y, currentEdge->start->vnormal->z);
			glVertex3f(currentEdge->start->x, currentEdge->start->y, currentEdge->start->z);
			currentEdge = currentEdge->leftNext;
		}
		glEnd();
		currentFace = currentFace->next;
	}
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glColor3ub(255,0,0);
	glColor3f(0,0,0);
	glLineWidth(1.0f);

	currentFace = wEdge->startFace;
	//glShadeModel(GL_SMOOTH);
	while(currentFace){
		firstEdge = currentFace->edge;
		currentEdge = firstEdge;
		glBegin(GL_TRIANGLES);
		//glNormal3f(currentFace->normals->x, currentFace->normals->y, currentFace->normals->z);
		for(i=0; i<3; i++){
			glNormal3f(currentEdge->start->vnormal->x, currentEdge->start->vnormal->y, currentEdge->start->vnormal->z);
			glVertex3f(currentEdge->start->x, currentEdge->start->y, currentEdge->start->z);
			currentEdge = currentEdge->leftNext;
		}
		glEnd();
		currentFace = currentFace->next;
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

GLfloat dotProd(Vertex* u, Vertex* v){
	return u->x * v->x + u->y * v->y + u->z * v->z;
}

//normalize to unit length
GLvoid unitNormalize(Vertex* v) {
	GLfloat l;

	l = (GLfloat)sqrt(dotProd(v,v));
	//nvx = vx / l
	v->x /= l;
	v->y /= l;
	v->z /= l;
}

//vector product for normalization
Vertex* product(Vertex* u, Vertex* v) {
	Vertex* result;

	result = new Vertex ;

	//vx = v1y * v2z - v1z * v2y
	result->x = u->y * v->z - u->z * v->y;
	//vy = v1z * v2x - v1x * v2z
	result->y = u->z * v->x - u->x * v->z;
	//vz = v1x * v2y - v1y * v2x
	result->z = u->x * v->y - u->y * v->x;

	return result;
}
// face normals
void normalize(WingedEdge *wEdge){
	Face* face;
	Edge* edge;
	Vertex* edge1;
	Vertex* edge2;

	face = wEdge->startFace;
	 
	while (face) {
		edge1 = new Vertex ;
		edge2 = new Vertex ;

		//v1 = t2 - t1
		edge1->x = face->edge->end->x - face->edge->start->x;
		edge1->y = face->edge->end->y - face->edge->start->y;
		edge1->z = face->edge->end->z - face->edge->start->z;

		//v2 = t3 - t1
		edge2->x = face->edge->leftPrevious->end->x - face->edge->leftPrevious->start->x;
		edge2->y = face->edge->leftPrevious->end->y - face->edge->leftPrevious->start->y;
		edge2->z = face->edge->leftPrevious->end->z - face->edge->leftPrevious->start->z;

		//v=v1*v2
		face->normals = product(edge1, edge2);
		unitNormalize(face->normals);
		 
		free(edge1); free(edge2);

		face = face->next;
	}
}

void vertexNormalize(WingedEdge *wEdge){
	//calculate sum
	int i=0;
	Vertex *vertex = wEdge->startVertex;

	//for all vertices
	while(vertex){
		i++;
		Face *face = wEdge->startFace;
		Vertex *sum = new Vertex;
		sum->x = 0; sum->y = 0; sum->z = 0; 
		//for all faces
		while(face){
			if( face->edge->start == vertex || face->edge->leftPrevious->start == vertex || face->edge->leftPrevious->leftPrevious->start == vertex){
				//printf("this is my face %d\n",i);
				sum->x += face->normals->x;
				sum->y += face->normals->y;
				sum->z += face->normals->z;
			}
			face = face->next;
		}
		unitNormalize(sum);
		vertex->vnormal = sum;
		vertex = vertex->next;
	}
}
