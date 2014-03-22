/*************************************************************************\

Demo code for a simple sphere-physics world

\**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "GL/glut.h"
#include <gmtl/gmtl.h>
#include "objects.h"
#include <time.h>
#include <iostream>
#include <vector>
#include "Grid.h"
using namespace std;
using namespace gmtl;

// The timestep of the simulation
double deltat = 0.001;
// Number of spheres in the sim, hit '+' for more
int numspheres = 1;
// Define gravity vector
Vec3d gravity(0.0, -9.8, 0.0);  // Notice down in -y
// Define air friction scaling
double air_friction = 0.1;

// The collection of spheres
// If you haven't seen stl vectors, look them up
vector< sphere > spheres;

// The six walls. Spheres and planes are defined in objects.h
plane walls[6]; // The box is made up of 6 planes
double wallRadius; // wall dimension

// Hitting 's' adds energy to the scene with some scaling as defined below. 
double shakemag = 100.0;

// Some mouse control stuff
int mode;
double beginx, beginy;

// Some parameters for allowing mouse rotations of the scene
double dis = 10.0, azim = 0.0, elev = 0.0;
double ddis = 0.0, dazim = 0.0, delev = 0.0;
double rot1 = 0.0, rot2 = 0.0, rot3 = 0.0;

// Whether the state is updated or not
int animate = 1;

Grid* _grid;
bool _drawGrid = false;
bool _useGrid = false;
bool _fixedSphereToggle = false;
int frame = 0;
int curtime = 0;
int timebase = 0;
char s[50];
bool _displayFPS = true;
bool _useEuler = false;
bool _drawScene = true;
// Called at beginning to define scene
void
InitViewerWindow()
{
	GLfloat Ambient[] = { 0.4, 0.4, 0.4, 1.0 };  
	GLfloat Diffuse[] = { 0.3, 0.3, 0.3, 1.0 };  
	GLfloat Specular[] = { 0.2, 0.2, 0.2, 1 };   
	GLfloat SpecularExp[] = { 30 };              
	GLfloat Emission[] = { 0.0, 0.0, 0.0, 0.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, Ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, SpecularExp);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emission);

	glMaterialfv(GL_BACK, GL_AMBIENT, Ambient);
	glMaterialfv(GL_BACK, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_BACK, GL_SPECULAR, Specular);
	glMaterialfv(GL_BACK, GL_SHININESS, SpecularExp);
	glMaterialfv(GL_BACK, GL_EMISSION, Emission);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	glEnable(GL_COLOR_MATERIAL);

	GLfloat light_position[] = { 0.4, 0.3, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_FLAT);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_NORMALIZE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.4,0.4,-0.4,0.4,2.0,15.0);

	glMatrixMode(GL_MODELVIEW);
}

// A draw cycle calls this, then the actual drawing code, then the EndDraw function.
void
BeginDraw()
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);


	GLfloat Ambient[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat Diffuse[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat Specular[] = { 0.2, 0.2, 0.2, 1 };
	GLfloat SpecularExp[] = { 30 };
	GLfloat Emission[] = { 0.0, 0.0, 0.0, 0.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, Ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, SpecularExp);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emission);

	glMaterialfv(GL_BACK, GL_AMBIENT, Ambient);
	glMaterialfv(GL_BACK, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_BACK, GL_SPECULAR, Specular);
	glMaterialfv(GL_BACK, GL_SHININESS, SpecularExp);
	glMaterialfv(GL_BACK, GL_EMISSION, Emission);

	
	GLfloat light_position[] = { 0.4, 0.3, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_FLAT);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_NORMALIZE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.4, 0.4, -0.4, 0.4, 2.0, 15.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -(dis + ddis));
	glRotated(elev + delev, 1.0, 0.0, 0.0);
	glRotated(azim + dazim, 0.0, 1.0, 0.0);
}

void
EndDraw()
{
	glFlush();
	glutSwapBuffers();
}

// Define some keyboard controls
void 
KeyboardCB(unsigned char key, int x, int y) 
{

	switch(key) 
	{
	case 'q': exit(0);
	case 's':
		for ( unsigned int i = 0; i < spheres.size(); i++ )
		{
			Vec3d shake;
			shake[0] = (rand() / (double)RAND_MAX) * 0.002 - 0.001;
			shake[1] = (rand() / (double)RAND_MAX) * 0.002 - 0.001;
			shake[2] = (rand() / (double)RAND_MAX) * 0.002 - 0.001;
			spheres[i].v += shake * shakemag;
		}
		shakemag *= 2.0;
		cout << "Shake: " << shakemag << endl;
		break;
	case '>':
		deltat = deltat * 2.0;
		cout << "Delta t " << deltat << endl;
		break;
	case '<':
		deltat = deltat / 2.0;
		cout << "Delta t " << deltat << endl;
		break;

	case '+':
		for ( int i = 0; i < 5; i++ )
		{
			sphere s;
			s.makeRandomSphere( wallRadius - 0.1); //, wall/4.0 * (rand() / (double)RAND_MAX) );
			spheres.push_back( s );
		}
		
		numspheres +=5;
		cout << "Num spheres: " << spheres.size() << endl;
		break;

	case '-':
		numspheres -=5;		
		spheres.resize( numspheres );
		cout << "Num spheres: " << spheres.size() << endl;
		break;
	case 'p':
		cout << "Num spheres: " << spheres.size() << endl;
		if (_useGrid){
			_grid->PrintGridInfo();
		}
		else cout << "Not using grid. Press 'p' to use grid" << endl;
		break;
	case 'd':
		_drawGrid = !_drawGrid;
		std::cout << "Draw grid: " << std::boolalpha << _drawGrid << std::endl;
		break;
	case 'g':
		_useGrid = !_useGrid;
		std::cout << "Using grid: " << std::boolalpha << _useGrid << std::endl;
		break;
	case 'f':
			_fixedSphereToggle = !_fixedSphereToggle;
			spheres[0].fixed = true;
			std::cout << "Fixed sphere 0: " << std::boolalpha << _fixedSphereToggle << std::endl;
			break;
	case 'h':
		_displayFPS = !_displayFPS;
		break;
	case 'e':
		_useEuler = !_useEuler;
		std::cout << "Euler integration: " << std::boolalpha << _useEuler << std::endl;
		break;
	case 'r':
		_drawScene = !_drawScene;
		std::cout << "Draw scene: " << std::boolalpha << _drawScene << std::endl;
		break;
	default: animate = 1 - animate; // any other key, like spacebar, starts and stops physics update
	}

	glutPostRedisplay();
}

// Allows the user to click drag to rotate the scene
void
MouseCB(int _b, int _s, int _x, int _y)
{
	if (_s == GLUT_UP)
	{
		dis += ddis;
		azim += dazim;
		elev += delev;
		ddis = 0.0;
		dazim = 0.0;
		delev = 0.0;
		return;
	}

	if (_b == GLUT_RIGHT_BUTTON)
	{
		mode = 0;
		beginy = _y;
		return;
	}
	else
	{
		mode = 1;
		beginx = _x;
		beginy = _y;
	}
}

void
MotionCB(int _x, int _y)
{
	if (mode == 0)
	{
		ddis = dis * (_y - beginy)/200.0;
	}
	else
	{
		dazim = (_x - beginx)/5.0;
		delev = (_y - beginy)/5.0;      
	}

	glutPostRedisplay();
}

// The main loop
void
IdleCB() 
{
	// Need timing code here

	// I wanted to let the user do more and more violent shaking. So the shaking decays
	// over time, but also doubles in magnitude when the scene is shook.
	shakemag = shakemag * 0.99; // Making a shake adds in a decaying velocity change - decay it here.
	if ( shakemag < 10.0 )
		shakemag = 10.0;

	// You will want to build a grid here. Rebuild fresh each time as we assume all objects move. 
	// Normally you might just want to update the moving objects
	if (_useGrid){
		_grid->ConstructGrid(spheres);
		vector<sphere*> neighborSpheres;
		//in each cell - get the sphere indices within - loop over the spheres among its neighbors
		for (int z = 0; z < _grid->N_CELLS; z++){
			for (int y = 0; y < _grid->N_CELLS; y++){
				for (int x = 0; x < _grid->N_CELLS; x++){
					int i = 0;
					for (i = 0; i < _grid->_cells[x][y][z].size(); i++){
						
						neighborSpheres.push_back(&spheres[_grid->_cells[x][y][z][i]]);
					}
					for (i = 0; i < neighborSpheres.size(); i++){
						neighborSpheres[i]->computeForcesWithNeighbors(gravity, air_friction, walls, neighborSpheres);
					}
					neighborSpheres.clear();
				}
			}
		}
	}
	else{
		// Do the physics simulation
		for (unsigned int i = 0; i < spheres.size(); i++)
		{
			// This call passes in all the spheres which get checked for collision. Instead, you might
			// want to maintain a separate vector for each sphere that gives its potential colliding pairs or a vector in each grid cell.
			spheres[i].computeForces(gravity, air_friction, walls, spheres);
		}
	}
	
		// Now integrate forces
	for (unsigned int i = 0; i < spheres.size(); i++){
		if (_useEuler) spheres[i].Euler(deltat);
		else spheres[i].EulerCromer(deltat);
	}
	
	glutPostRedisplay(); // Calls the registered display function - DisplayCB
}
void specialKeyCB(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		if (_fixedSphereToggle)	spheres[0].p[1] += 0.05;
		break;
	case GLUT_KEY_DOWN:
		if (_fixedSphereToggle) spheres[0].p[1] -= 0.05f;
		break;
	case GLUT_KEY_LEFT:
		if (_fixedSphereToggle) spheres[0].p[0] -= 0.05;
		break;
	case GLUT_KEY_RIGHT:
		if (_fixedSphereToggle) spheres[0].p[0] += 0.05;
		break;
	case GLUT_KEY_PAGE_DOWN:
		if (_fixedSphereToggle) spheres[0].p[2] += 0.05;
		break;
	case GLUT_KEY_END:
		if (_fixedSphereToggle) spheres[0].p[2] -= 0.05;
		break;
	}
}
void renderBitmapString(
	float x,
	float y,
	void *font,
	char *string) {

	char *c;
	glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

void DrawFPS()
{
	frame++;
	curtime = glutGet(GLUT_ELAPSED_TIME);

	if (curtime - timebase > 1000) {
		sprintf(s, "FPS:%4.2f",	frame*1000.0 / (curtime - timebase));
		timebase = curtime;
		frame = 0;
	}
	//cout << s << endl;
	if (_displayFPS){
		//glColor3f(1.0f, 1.0f, 0.0f);
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
	
	
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-10, 10, -10, 10, 1, 20);
		glTranslatef(-8,8, 0);
		renderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, s);
		glPopMatrix();
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glEnable(GL_LIGHTING);
	}
	
	
}
void
DisplayCB()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawFPS();
	BeginDraw();

	if (_drawScene){
		for (unsigned int i = 0; i < spheres.size(); i++)
			spheres[i].draw();

		for (unsigned int i = 0; i < 6; i++)
			walls[i].draw(wallRadius); // The plane width is not part of the class since planes are infinite

		if (_drawGrid) _grid->DrawGrid();
	}
	EndDraw();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	cout << "Basic Instructions" << endl;
	cout << "'+' Adds 5 balls '-' Removes 5 balls" << endl;
	cout << "> doubles time step < halves time step" << endl;
	cout << "'s' adds a small, decaying velocity kick to balls. Hit rapidly to build up." << endl;
	cout << "Mouse left-drag rotates scene right-drag zooms" << endl;

	// create the window
	glutInitWindowPosition(300, 0);
	glutInitWindowSize( 700, 700 );
	glutCreateWindow("SphereWorld");

	// set OpenGL graphics state -- material props, perspective, etc.
	InitViewerWindow();

	// set the callbacks
	glutDisplayFunc(DisplayCB);
	glutIdleFunc(IdleCB);
	glutMouseFunc(MouseCB);
	glutMotionFunc(MotionCB);  
	glutKeyboardFunc(KeyboardCB);
	glutSpecialFunc(specialKeyCB);
	// Make a sphere, numspheres is a global. Increment for more or hit '+' in running program
	wallRadius = 1.0;
	for ( int i = 0; i < numspheres; i++ )
	{
		sphere s;
		s.makeRandomSphere( wallRadius - 0.1 );
		spheres.push_back( s );
	}
	_grid = new Grid(wallRadius);
	double boxWallSpring = 1000.0;
	// Build the 6 walls of the environment, walls is a global variable
	walls[0] = plane(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, -wallRadius, 0.0), boxWallSpring);
	walls[1] = plane(Vec3d(1.0, 0.0, 0.0), Vec3d(-wallRadius, 0.0, 0.0), boxWallSpring);
	walls[2] = plane(Vec3d(-1.0, 0.0, 0.0), Vec3d(wallRadius, 0.0, 0.0), boxWallSpring);
	walls[3] = plane(Vec3d(0.0, 0.0, 1.0), Vec3d(0.0, 0.0, -wallRadius), boxWallSpring);
	walls[4] = plane(Vec3d(0.0, 0.0, -1.0), Vec3d(0.0, 0.0, wallRadius), boxWallSpring);
	walls[5] = plane(Vec3d(0.0, -1.0, 0.0), Vec3d(0.0, wallRadius, 0.0), boxWallSpring);
	
	glutMainLoop();
}


