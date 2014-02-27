//
//  main.cpp
//  MP3
//
//  Created by Joel Holsteen on 10/22/13.
//  Copyright (c) 2013 Joel Holsteen. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <GLUT/glut.h>
#include <fstream>
#include <cstring>
#include <math.h>
#include "src/SOIL.h"
#define PI (3.1415926)


using std::ifstream;
using namespace std;

//Structs that will hold all my triangle information
typedef struct vector{
    double x,y,z;
} vector;

typedef struct Normal{
    vector theVector;
} Normal;

typedef struct verticeNode{
    int faceCtr;
    int num;
    double x,y,z;
    double s,t;
    Normal normal;
}verticeNode;

typedef struct triangleNode{
    verticeNode * v1, *v2,*v3;
    Normal normal;
}triangleNode;

GLfloat matrix[16];
int nFPS = 60;

//make an array of vertices and triangles of the correct number of each in the file (+1 for the vertices so we have index base 1)
int vSize = 1;
int tSize = 2256;
//My arrays that will hold the information
triangleNode tArray[2256];
verticeNode ** vArray;

//Keypress variables
double zoom=0;
double yRotAngle = 0;
double xRotAngle = 0;
GLboolean upPressed=false;
GLboolean downPressed=false;
GLboolean leftPressed=false;
GLboolean rightPressed=false;
GLboolean qPressed = false;
GLboolean wPressed = false;
GLboolean theTexture = false;
GLboolean theReflection = false;
GLboolean neither = true;

//My array of textures
GLuint textures[3];



void calculateTriangleNormal(triangleNode * theTriangle)
{
    vector U, V;
    
    
    
    //Set Vector U to (Triangle.p2 minus Triangle.p1)
    //Set Vector V to (Triangle.p3 minus Triangle.p1)
    U.x = theTriangle->v2->x - theTriangle->v1->x;
    U.y = theTriangle->v2->y - theTriangle->v1->y;
    U.z = theTriangle->v2->z - theTriangle->v1->z;

    V.x = theTriangle->v3->x - theTriangle->v1->x;
    V.y = theTriangle->v3->y - theTriangle->v1->y;
    V.z = theTriangle->v3->z - theTriangle->v1->z;
    
    
    //Set Normal.x to (multiply U.y by V.z) minus (multiply U.z by V.y)
    //Set Normal.y to (multiply U.z by V.x) minus (multiply U.x by V.z)
    //Set Normal.z to (multiply U.x by V.y) minus (multiply U.y by V.x)
    theTriangle->normal.theVector.x= (U.y * V.z) - (U.z * V.y);
    theTriangle->normal.theVector.y= (U.z * V.x) - (U.x * V.z);
    theTriangle->normal.theVector.z= (U.x * V.y) - (U.y * V.x);
    return;
}


void normalizeTriangles(triangleNode * theTriangle)
{
    //find the magnitude of the normal
    double magnitude = pow(theTriangle->normal.theVector.x,2.0) + pow(theTriangle->normal.theVector.y,2.0) + pow(theTriangle->normal.theVector.z,2.0);
    magnitude = pow(magnitude, 0.5);
    
    //divide by magnitude to normalize to 1
    theTriangle->normal.theVector.x= theTriangle->normal.theVector.x/magnitude;
    theTriangle->normal.theVector.y= theTriangle->normal.theVector.y/magnitude;
    theTriangle->normal.theVector.z= theTriangle->normal.theVector.z/magnitude;
    
    return;


}


void normalizeVertex(verticeNode * theVertex)
{

    //find the magnitude of the normal
    double magnitude = pow(theVertex->normal.theVector.x,2.0) + pow(theVertex->normal.theVector.y,2.0) + pow(theVertex->normal.theVector.z,2.0);
    magnitude = pow(magnitude, 0.5);
    printf("numVertex=%d\n", theVertex->num);

    //divide by the magnitude to normalize to 1
    theVertex->normal.theVector.x= theVertex->normal.theVector.x /(magnitude);
    theVertex->normal.theVector.y= theVertex->normal.theVector.y /(magnitude);
    theVertex->normal.theVector.z= theVertex->normal.theVector.z /(magnitude);
    
    return;
}



//Function to read the file and parse it. Paramater is name of the file
void readFile(void){
    
    //Declare the input stream
    ifstream inputfile;
    
    //a char for if it is a triangle or a vertex and doubles for coordinates
    char indicator;
    double x,y,z;
    int v1, v2, v3;
    string line;
    
    //the Counters for the arrays (vertexCtr is set at one so we start base 1)
    int vertexCtr = 1;
    int triangleCtr = 0;
    
    verticeNode * badOne = (verticeNode *) malloc(sizeof(verticeNode));
    vArray = (verticeNode **) malloc(sizeof(verticeNode *));
    vArray[0] = badOne;
    
    
    
    //check to see if the file opened correctly
    inputfile.open("/Users/Grindlemire/Code/xcode/MP3/data.txt");
    
    if(!inputfile)
    {
        printf("could not open the file!!\n");
    }
    
    //While we are not at the end of the file parse the line
    while(!inputfile.eof())
    {
        
        inputfile>>indicator;
        
        //If it is a vertex store it into vertex array, create normal array and texture coord
        if(indicator=='v'){
            
            
            //read input
            inputfile>>x;
            inputfile>>y;
            inputfile>>z;
            
            //make a node
            verticeNode * temp = (verticeNode *) malloc(sizeof(verticeNode));
            temp->x = x;
            temp->y = y;
            temp->z = z;
            temp->num = vertexCtr;
            
            double theta = atan2(temp->z, temp->x);
            temp->s = (theta+PI)/(2*PI);
            temp->t = temp->y/3.15;
            
            
            //if the size of the array is not big enough make it bigger
            if(vertexCtr >= vSize)
            {
                vSize = vSize * 2;
                vArray = (verticeNode **) realloc(vArray, vSize * sizeof(verticeNode*));
            }
            
            //add node to array and increment counter
            vArray[vertexCtr] = temp;
            vertexCtr++;            
            
        }
        
        //else if it is a triangle store it into triangle array, calculate the normal for the face and update the normal array for the vertex.
        else if(indicator =='f'){
            inputfile>>v1;
            inputfile>>v2;
            inputfile>>v3;
            tArray[triangleCtr].v1 = vArray[v1];
            tArray[triangleCtr].v2 = vArray[v2];
            tArray[triangleCtr].v3 = vArray[v3];
            
            //calculate the normal of the triangle
            calculateTriangleNormal(&tArray[triangleCtr]);
            
            normalizeTriangles(&tArray[triangleCtr]);
            
            //update the normals on the vertice's used
            vArray[v1]->normal.theVector.x = vArray[v1]->normal.theVector.x + tArray[triangleCtr].normal.theVector.x;
            vArray[v1]->normal.theVector.y = vArray[v1]->normal.theVector.y + tArray[triangleCtr].normal.theVector.y;
            vArray[v1]->normal.theVector.z = vArray[v1]->normal.theVector.z + tArray[triangleCtr].normal.theVector.z;
            
            vArray[v2]->normal.theVector.x = vArray[v2]->normal.theVector.x + tArray[triangleCtr].normal.theVector.x;
            vArray[v2]->normal.theVector.y = vArray[v2]->normal.theVector.y + tArray[triangleCtr].normal.theVector.y;
            vArray[v2]->normal.theVector.z = vArray[v2]->normal.theVector.z + tArray[triangleCtr].normal.theVector.z;
            
            vArray[v3]->normal.theVector.x = vArray[v3]->normal.theVector.x + tArray[triangleCtr].normal.theVector.x;
            vArray[v3]->normal.theVector.y = vArray[v3]->normal.theVector.y + tArray[triangleCtr].normal.theVector.y;
            vArray[v3]->normal.theVector.z = vArray[v3]->normal.theVector.z + tArray[triangleCtr].normal.theVector.z;
            
            
            
            triangleCtr++;
            
        }
        //if its a comment just ignore it
        else{
            //ignore the line
            inputfile.ignore(200,'\n');
            
        }
        
        
    }
    //Normalize the vertex array normals so they are unit length
    int j=1;

    while(j<=1202)
    {
        printf("%d\n", j);
        normalizeVertex(vArray[j]);
        j++;
    }
    
    
}


//Draw the parsed vertices and triangles
void drawTeapot(void)
{
       
    //draw all the faces of the teapot and their normals
    glPushMatrix();
    glRotated(yRotAngle, 0, 1, 0);
    glRotated(xRotAngle, 1,0, 0);
   
    
    //Draw every Triangle's normal and vertex and also map a texture coordinate to each one using s and t
    int i;
    for(i=0; i< 2256; i++)
    {
        
        glNormal3f(tArray[i].normal.theVector.x, tArray[i].normal.theVector.y, tArray[i].normal.theVector.z);
       
        glBegin(GL_TRIANGLES);
        glNormal3f(tArray[i].v1->normal.theVector.x, tArray[i].v1->normal.theVector.y, tArray[i].v1->normal.theVector.z);
            glTexCoord2f(tArray[i].v1->s, tArray[i].v1->t);
        glVertex3f(tArray[i].v1->x, tArray[i].v1->y, tArray[i].v1->z);
        glNormal3f(tArray[i].v2->normal.theVector.x, tArray[i].v2->normal.theVector.y, tArray[i].v2->normal.theVector.z);
            glTexCoord2f(tArray[i].v2->s, tArray[i].v2->t);
        glVertex3f(tArray[i].v2->x, tArray[i].v2->y, tArray[i].v2->z);
        glNormal3f(tArray[i].v3->normal.theVector.x, tArray[i].v3->normal.theVector.y, tArray[i].v3->normal.theVector.z);
            glTexCoord2f(tArray[i].v3->s, tArray[i].v3->t);
        glVertex3f(tArray[i].v3->x, tArray[i].v3->y, tArray[i].v3->z);
        
        glEnd();

    }
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);

    
    
}







//Initialization function
void init(void)
{
    
    
    
    //pass our arrays into the parser
    readFile();
    
    //clear the color
    glClearColor(0.9,0.9,0.9,1.0);
    
    
    

}
void display(void)
{
    //This is simply to adjust lighting to show a nontextured teapot vs textured. NOTE: I ALWAYS HAVE DEPTH TESTING
    if(!neither)
    {
        GLfloat light_ambient[] = { 10.0, 10.0, 10.0, 10.0 };
        GLfloat light_diffuse[] = { 2.0, 2.0, 2.0, 2.0 };
        GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
        
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        
        glEnable( GL_LIGHTING );
        glEnable( GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);

    }
    else{
       
        GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
        GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
        
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        
        
        glEnable( GL_LIGHTING );
        glEnable( GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);

    }
    
        
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glMatrixMode( GL_MODELVIEW);
    
    
    glLineWidth(1.0);
    glLoadIdentity();
    gluLookAt(5.f, 3.f, 3.f, 1.0, 1.0, 1.0, 0.f, 1.f, 0.f);
     
    
    
   
        
    
    //Mode 1, simply make a background of wood paneling and match the theme with a wood carved teapot
    if(theTexture)
    {
         glDisable(GL_TEXTURE_2D);
        
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &textures[2]);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        
        textures[2] = SOIL_load_OGL_texture("/Users/Grindlemire/Code/xcode/MP3/MP3/wood.bmp",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
        
        static int length = 10;
        static int zValue = 10;
        
        
        glBegin(GL_QUADS);
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,-length);
        glTexCoord2f(0,0);
        glVertex3f(-length,-length,-zValue);
        
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,length);
        glTexCoord2f(0,1);
        glVertex3f(-length,length,-zValue);
        
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,length);
        glTexCoord2f(1,1);
        glVertex3f(length,length,-zValue);
        
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,-length);
        glTexCoord2f(1,0);
        glVertex3f(length,-length,-zValue);
        glEnd();
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);
       
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &textures[0]);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
    
        textures[0] = SOIL_load_OGL_texture("/Users/Grindlemire/Code/xcode/MP3/MP3/wood.bmp",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    }
    
    
    //Mode 2, the best mode a Reflective Teapot in the correct environment (its also beautiful)
    if(theReflection)
    {
        glDisable(GL_TEXTURE_2D);
        
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &textures[2]);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        
        textures[2] = SOIL_load_OGL_texture("/Users/Grindlemire/Code/xcode/forestsunset.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
        
        static int length = 10;
        static int zValue = 10;
        
        
        glBegin(GL_QUADS);
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,-length);
        glTexCoord2f(0,0);
        glVertex3f(-length,-length,-zValue);
        
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,length);
        glTexCoord2f(0,1);
        glVertex3f(-length,length,-zValue);
        
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,length);
        glTexCoord2f(1,1);
        glVertex3f(length,length,-zValue);
        
        glNormal3f(0,0,-1);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,-length);
        glTexCoord2f(1,0);
        glVertex3f(length,-length,-zValue);
        glEnd();
        glPopMatrix();

        
        glDisable(GL_TEXTURE_2D);
       
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &textures[1]);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        
        textures[1] = SOIL_load_OGL_texture("/Users/Grindlemire/Code/xcode/MP3/MP3/sunset.bmp",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_GEN_R);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
        
    drawTeapot();

    
    
    glFlush();
    glutSwapBuffers();
    
    
    //Commands for smooth movement
    if(upPressed){
        xRotAngle += 5;       
    }
    if(downPressed){
        xRotAngle -= 5;
    }
    if(rightPressed){
        yRotAngle += 5;
    }
    if(leftPressed){
        yRotAngle -= 5;
    }
    if(qPressed){
        zoom += .0005;
        
    }
    if(wPressed){
        printf("hi");
        zoom -=.0005;
    }

    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, zoom);
    glMultMatrixf(matrix);
   

}

//Registers when I press down on a key, how I get smooth movement partly
void keyboardInput(int key, int x, int y)
{
    if (key==GLUT_KEY_UP)
        upPressed = true;
    if (key==GLUT_KEY_DOWN)
        downPressed = true;
    if(key==GLUT_KEY_RIGHT)
        rightPressed=true;
    if(key==GLUT_KEY_LEFT)
        leftPressed=true;
    if(key=='w')
        wPressed = true;
    if(key=='q')
        qPressed = true;
    
}
//Registers when I let up on a key, the second half of how I get smooth movement
void keyboardoutInput(int key, int x, int y)
{
    if (key==GLUT_KEY_UP)
        upPressed = false;
    if (key==GLUT_KEY_DOWN)
        downPressed = false;
    if(key==GLUT_KEY_RIGHT)
        rightPressed=false;
    if(key==GLUT_KEY_LEFT)
        leftPressed=false;
    if(key=='w')
        wPressed = false;
    if(key=='q')
        qPressed = false;
    
}




void keyboard(unsigned char key, int x, int y)
{
	// put your keyboard control here
	if (key == 27)
	{
		// ESC hit, so quit
		printf("demonstration finished.\n");
		exit(0);
	}
    if(key=='w')
        wPressed = true;
    if(key=='q')
        qPressed = true;
    if(key=='t')
    {
        theTexture = true;
        theReflection = false;
        neither = false;
        
    }
    if(key=='r')
    {
        theReflection = true;
        theTexture = false;
        neither = false;
    }
    if(key == 'y')
    {
        theReflection = false;
        theTexture = false;
        neither = true;
    }
}

void keyboardups( unsigned char key, int x, int y)
{
    if(key=='w')
        wPressed = false;
    if(key=='q')
        qPressed = false;
}

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
   
	glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
    
	glMatrixMode (GL_MODELVIEW);
}

void timer(int v)
{
    glutPostRedisplay(); // trigger display function by sending redraw into message queue
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again
}

int main(int argc, const char * argv[])
{
    glutInit(&argc, (char**)argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    init ();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardups);
    glutTimerFunc(100,timer,nFPS);
    glutSpecialFunc(keyboardInput); //my high quality movement input
    glutSpecialUpFunc(keyboardoutInput); //my high quality movement input
    
    
    glutMainLoop();
    return 0;
    

    
    
}











