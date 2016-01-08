#include <GL/glut.h>
#include "glui.h"
#include <stdio.h>
#include <memory.h>

WingedEdge* readShapeFile(char *name);
void writeShapeFile(WingedEdge* we, char* filename);
void normalize(WingedEdge *wEdge);
Vertex* product(Vertex* u, Vertex* v);
void vertexNormalize(WingedEdge *wEdge);

void myShapeWire(WingedEdge *wEdge);
void myShapeSolid(WingedEdge *wEdge);
void myShapeSolidSmooth(WingedEdge *wEdge);
void myShapeShadedEdge(WingedEdge *wEdge);

// Vertices composing a face
class Triangle{
	public:
		Triangle(){memset(this,0,sizeof(Triangle));} 
		GLuint vertices[3];
};

//vertex table
class Vertex{
	public:
		Vertex(){memset(this,0,sizeof(Vertex));}
		GLfloat x, y, z;
		Vertex *next;
		GLint count;
		Vertex *vnormal;
};

//edge table
class Edge{
	public:
		Edge(){memset(this,0,sizeof(Edge));}
		Vertex *start, *end;
		Edge *leftPrevious, *leftNext, *symmetric; //symmteric edge defines the CW order (rightPrevious, rightNext)
		class Face *leftFace, *rightFace;
};

//face table
class Face{
	public:
		Face(){memset(this,0,sizeof(Face));}
		Edge *edge;
		Face *next;
		Vertex *normals;
};

//keeps track of all the edge and face starting points
class WingedEdge{
	public:
		WingedEdge(){memset(this,0,sizeof(WingedEdge));}
		Face *startFace;
		Vertex *startVertex;
};

