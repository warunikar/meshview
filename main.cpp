#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
//#include "../Dependencies/glew/glew.h"
#include <GL/glut.h>
#include "glui.h"
#include "process.h"

//Usual variables
//char *fileNameF = "";
char *fileNameF = "";
char fileName[80] = "sphere.smf";
char opFileName[80] = "output.smf";
char text[20] = {"man.smf"};
char optext[20] = {"output.smf"};
int testvar;
//char *fileName = "C:\\Users\\wranawee\\CMPT 764 Geom\\Opengl\\interface\\Debug\\";

//Window attributes
int myWindow;
int windowX = 50;
int windowY = 50;
int windowWidth = 500;
int windowHeight = 500;
float color[] = { 1.0, 1.0, 1.0 };

GLUI *myGluiWindow;

//Winged-edge data structure variables
WingedEdge *shape = NULL;

//Live variables
//IDs for controls
enum
{
	DISPLAY_LISTBOX = 0,
	TRANSLATION_XY,
	TRANSLATION_Z,
	ROTATION,
	SCALE_SPINNER,
	QUIT_BUTTON,
	OPEN_BUTTON,
	SAVE_BUTTON,
	FILE_INPUT,
	FILE_OUTPUT
};
int segments = 8;
int wireframe = 0;
float translate_xy[2] = {0, 0};
float translate_z = 0;
float scale = 1;
int listbox_id = 1; /*1-flat shaded, 2-smooth, 3-wireframe, 4-shaded with edges*/
float rotation_matrix[16] = {1.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, 0.0,
							0.0, 0.0, 1.0, 0.0,
							0.0, 0.0, 0.0, 1.0};


void myDisplay(void){
	static float rotationX = 0.0, rotationY = 0.0;
	glPushMatrix();

	glClearColor( .9f, .9f, .9f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glTranslatef( 0.0, 0.0, -1.0 );

	glTranslatef (translate_xy[0], translate_xy[1], -translate_z); //translate
	glMultMatrixf (rotation_matrix); //rotate
	glScalef (scale, scale, scale); //scale

	switch(listbox_id){
		case 1: //flat shaded
			myShapeSolid(shape);
			break;
		case 2: //smooth shaded
			myShapeSolidSmooth(shape);
			break;
		case 3: //wireframe
			myShapeWire(shape);
			break;
		case 4: //shaded with edges
			//myShapeSolid(shape);
			myShapeShadedEdge(shape);
			break;
	}
	glutSwapBuffers();

}
void myReshape(int x, int y ){
	float xy_aspect;

	xy_aspect = (float)x / (float)y;
	GLUI_Master.auto_set_viewport();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glFrustum(-xy_aspect*.08, xy_aspect*.08, -.08, .08, .1, 15.0 );

	glutPostRedisplay();

}
void myIdle(){
	if ( glutGetWindow() != myWindow )
		glutSetWindow(myWindow);

	glutPostRedisplay();
}
void myGluiCallback (int callID){
	
	switch(callID){
		case DISPLAY_LISTBOX:
			printf("Changing mesh display to %d..\n", listbox_id);
			break;
		case TRANSLATION_XY:
			printf("XY translation..\n");
			break;
		case TRANSLATION_Z:
			printf("Z translation..\n");
			break;
		case ROTATION:
			printf("Rotating the mesh..\n");
			break;
		case SCALE_SPINNER:
			printf("Scaling..\n");
			break;
		case QUIT_BUTTON:
			printf("Exiting..\n");
			exit(1);
		case OPEN_BUTTON:
			printf("Opening the mesh..\n");
			shape = readShapeFile(fileName); //load the shape into a winged-edge data structure
			normalize(shape); //face
			vertexNormalize(shape);
			glutPostRedisplay();
			break;
			break;
		case SAVE_BUTTON:
			printf("Saving the mesh..\n");
			strcpy(opFileName, fileNameF);
			strcat(opFileName, optext);
			writeShapeFile(shape, opFileName);
			break;
		case FILE_INPUT:
			//printf("File name %s..\n", text);
			strcpy(fileName, fileNameF);
			strcat(fileName, text);
			puts(fileName);
	}
}
void lighting(){
	/*set up OpenGL lighting*/
	GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
	GLfloat light0_position[] = {1.0f, 1.0f, 1.0f, 0.0f};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glEnable(GL_DEPTH_TEST);
}
void buildInterface(){
	myGluiWindow = GLUI_Master.create_glui ("Options", 0, windowX+500, windowY);

	/*file open options, textbox and buttons*/
	myGluiWindow->add_edittext("Input file: ", GLUI_EDITTEXT_TEXT, text, FILE_INPUT, myGluiCallback);
	myGluiWindow->add_button ("Open", OPEN_BUTTON, myGluiCallback);

	/*adding new panel*/
	GLUI_Panel *myPanel1 = myGluiWindow->add_panel("Object Properties");
	/*list box for mesh displaying*/
	GLUI_Listbox *myListBox= myGluiWindow->add_listbox_to_panel (myPanel1, "Display Mesh", &listbox_id, DISPLAY_LISTBOX, myGluiCallback);
	myListBox->add_item (1, "Flat Shaded");
	myListBox->add_item (2, "Smooth Shaded");
	myListBox->add_item (3, "Wireframe");
	myListBox->add_item (4, "Shaded with edges");

	/*adding one more panel*/
	GLUI_Panel *myPanel2 = myGluiWindow->add_panel("Transformation");
	GLUI_Panel *mySubPanel1 = myGluiWindow->add_panel_to_panel(myPanel2, "");

	/*translate X, Y*/
	GLUI_Translation *myTranslation = myGluiWindow->add_translation_to_panel(mySubPanel1, "Translation XY", GLUI_TRANSLATION_XY, translate_xy, TRANSLATION_XY, myGluiCallback);
	myTranslation->set_speed(0.005);

	/*adding a new column (side by side display)*/
	myGluiWindow->add_column_to_panel(mySubPanel1, false);
	
	/*translate Z*/
	GLUI_Translation *myTranslationZ = myGluiWindow->add_translation_to_panel(mySubPanel1, "Translation Z", GLUI_TRANSLATION_Z, &translate_z, TRANSLATION_Z, myGluiCallback);
	myTranslationZ->set_speed(0.005);

	/*adding sub panel*/
	GLUI_Panel *mySubPanel2 = myGluiWindow->add_panel_to_panel(myPanel2, "");

	/*set up rotation*/
	myGluiWindow->add_rotation_to_panel(mySubPanel2, "Rotation", rotation_matrix, ROTATION, myGluiCallback);
	
	/*adding a new column (side by side display)*/
	myGluiWindow->add_column_to_panel(mySubPanel2, false);
	
	/*set up zoom (i.e. scale)*/
	GLUI_Spinner *mySpinner = myGluiWindow->add_spinner_to_panel (mySubPanel2, "Scale", GLUI_SPINNER_FLOAT, &scale, SCALE_SPINNER, myGluiCallback);
	mySpinner->set_float_limits (-4.0, 4.0);

	/*save mesh*/
	myGluiWindow->add_edittext("Output file: ", GLUI_EDITTEXT_TEXT, optext, FILE_OUTPUT, myGluiCallback);
	myGluiWindow->add_button ("Save", SAVE_BUTTON, myGluiCallback);

	/*add quit button*/
	myGluiWindow->add_button ("Quit", QUIT_BUTTON, myGluiCallback);

	GLUI_Master.set_glutIdleFunc(myIdle);
	GLUI_Master.set_glutReshapeFunc(myReshape);
	myGluiWindow->set_main_gfx_window(myWindow);
}

int main(int argc, char* argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(windowWidth,windowWidth);
	glutInitWindowPosition(windowX,windowY);

	shape = readShapeFile(fileName); //load the shape into a winged-edge data structure
	//readShapeFile(fileName);
	scaled(shape);
	normalize(shape);
	vertexNormalize(shape);

	myWindow = glutCreateWindow("CMPT 764");
	glutDisplayFunc(myDisplay);
	glutReshapeFunc(myReshape);
	//glShadeModel(GL_FLAT);
	//glShadeModel(GL_SMOOTH);
	lighting();

	buildInterface();
	glutMainLoop();
	return 0;
}
