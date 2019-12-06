/*
 *  CSCI 441, Computer Graphics, Fall 2019
 *
 *  Project: lab10
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with GLSL Geometry shader.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2019
 *
 */

//*************************************************************************************

#include <GL/glew.h>        // include GLEW to get our OpenGL 3.0+ bindings
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>				// for printf functionality
#include <stdlib.h>				// for exit functionality
#include <time.h>					// for time functionality
#include <iostream>
#include <fstream>
#include <sstream>

// note that all of these headers end in *3.hpp
// these class library files will only work with OpenGL 3.0+
#include <CSCI441/modelLoader3.hpp> // to load in OBJ models
#include <CSCI441/objects3.hpp>     // to render our 3D primitives
#include <CSCI441/OpenGLUtils3.hpp> // to print info about OpenGL
#include <CSCI441/ShaderProgram3.hpp>   // our shader helper functions
#include <CSCI441/TextureUtils.hpp>   // our texture helper functions

#include "particle.h"
#include "particleSystem.h"

//*************************************************************************************
//
// Global Parameters
 
int windowWidth, windowHeight;
float sbWidth = 20;

bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

float cameraTheta = 6.0;
float cameraPhi = 1.0;
glm::vec3 camDir;
float camSpeed = 1.0;
glm::vec3 eyePoint(   10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

GLuint skyboxTHs[6];
GLuint skyboxVAO;
GLuint skyboxVBO;
GLuint skyboxTexVBO;

CSCI441::ShaderProgram *geoshaderProgram = NULL;
GLint modelview_uniform_location, projection_uniform_location;
GLint vpos_attrib_location, alpha_attrib_location;

//texture shader attributes
CSCI441::ShaderProgram *textureshaderProgram = NULL;
GLint texture_mvp_uniform_location = -1;
GLint texture_vpos_attrib_location = -1;
GLint texture_coor_attrib_location = -1;

vector<PS::ParticleSystem*> psystems;
vector<vector<string>> systemLines;

const int NUM_POINTS = 20;
struct Vertex { GLfloat x, y, z; };
Vertex points[NUM_POINTS];
GLuint pointsVAO, pointsVBO;

GLuint textureHandle;
 
//******************************************************************************
//
// Helper Functions

vector<string> tokenize(string input){
	vector<string> tokenized_string;

	istringstream stream(input);
	string current_word;
	
	while( stream >> current_word){
		if(!current_word.empty()){
			tokenized_string.push_back(current_word);
		}
	}
	return tokenized_string;
}

bool readInputFile(string filename){
	ifstream config(filename);
	if(!config.is_open()){
		cout << "[ERROR}: Could not open " <<filename<<endl;
		return false;
	}

	string line;
	int lineNum = 0;
	while(getline(config, line)){
		lineNum++;
		vector<string> tokens = tokenize(line);
		if( (tokens[0] == "F" && tokens.size() == 10) || (tokens[0] == "R" && tokens.size() == 9) ){
			systemLines.push_back(tokens);
		}else{
			cout<<"[WARNING]: Line "<<lineNum<< " is invalid, skipping"<<endl;
		}
	}
	return true;
}


// convertSphericalToCartesian() ///////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraAngles is updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeOrientation() {
	float r = sinf(cameraPhi);
	camDir = glm::vec3(1*r*sinf(cameraTheta), -1*cosf(cameraPhi), -1*r*cosf(cameraTheta));

	camDir = normalize(camDir);
	lookAtPoint = eyePoint+camDir;
}

//******************************************************************************
//
// Event Callbacks

// error_callback() ////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: %s\n", description);
}

// key_callback() //////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's keypress callback.
//	Responds to key presses and key releases
//
////////////////////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((key == GLFW_KEY_ESCAPE || key == 'Q') && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if( action == GLFW_PRESS ) {
		switch( key ) {
			
			case GLFW_KEY_W:
				eyePoint+=camSpeed*camDir;
				recomputeOrientation();
				break;
			case GLFW_KEY_S:
				eyePoint-=camSpeed*camDir;
				recomputeOrientation();
				break;
		}
	}

	if( action == GLFW_REPEAT ){
		switch( key ) {
			case GLFW_KEY_W:
				eyePoint+=camSpeed*camDir;
				recomputeOrientation();
				break;
			case GLFW_KEY_S:
				eyePoint-=camSpeed*camDir;
				recomputeOrientation();
				break;
		}
	}
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//		We will register this function as GLFW's mouse button callback.
//	Responds to mouse button presses and mouse button releases.  Keeps track if
//	the control key was pressed when a left mouse click occurs to allow
//	zooming of our arcball camera.
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
		leftMouseDown = true;
	} else {
		leftMouseDown = false;
		mousePosition.x = -9999.0f;
		mousePosition.y = -9999.0f;
	}
  controlDown = mods & GLFW_MOD_CONTROL;
}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's cursor movement callback.
//	Responds to mouse movement.  When active motion is used with the left
//	mouse button an arcball camera model is followed.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	// make sure movement is in bounds of the window
	// glfw captures mouse movement on entire screen
	if( xpos > 0 && xpos < windowWidth ) {
		if( ypos > 0 && ypos < windowHeight ) {
			// active motion
			if( leftMouseDown ) {
				float dx = xpos-mousePosition.x;
				float dy = ypos-mousePosition.y;
				cameraTheta+=0.005*dx;
				float newPhi = cameraPhi-0.005*dy;
				if(newPhi>=0 && newPhi<=M_PI){
					cameraPhi = newPhi;
				}
				recomputeOrientation();
			}
			mousePosition.x = xpos;
			mousePosition.y = ypos;
			
		}
	}
}

// scroll_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's scroll wheel callback.
//	Responds to movement of the scroll where.  Allows zooming of the arcball
//	camera.
//
////////////////////////////////////////////////////////////////////////////////
static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {

}

//******************************************************************************
//
// Setup Functions

// setupGLFW() /////////////////////////////////////////////////////////////////
//
//		Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
////////////////////////////////////////////////////////////////////////////////
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback(error_callback);

	// initialize GLFW
	if (!glfwInit()) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit(EXIT_FAILURE);
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );						// request forward compatible OpenGL context
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );	// request OpenGL Core Profile context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );		// request OpenGL 3.x context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );		// request OpenGL 3.3 context

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow(640, 480, "Lab10: Geometry Shaders", NULL, NULL);
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(	window );	// make the created window the current window
	glfwSwapInterval( 1 );				    // update our screen after at least 1 screen refresh

	glfwSetKeyCallback( 			  window, key_callback				  );	// set our keyboard callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function
	glfwSetCursorPosCallback(	  window, cursor_callback  			);	// set our cursor position callback function
	glfwSetScrollCallback(			window, scroll_callback			  );	// set our scroll wheel callback function

	return window;										// return the window that was created
}

// setupOpenGL() ///////////////////////////////////////////////////////////////
//
//      Used to setup everything OpenGL related.
//
////////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
	glEnable( GL_DEPTH_TEST );					// enable depth testing
	glDepthFunc( GL_LESS );							// use less than depth test

	glEnable(GL_BLEND);									// enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// use one minus blending equation

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black

	glEnable( GL_TEXTURE_2D );					// enable 2D texturing

	glPointSize( 4.0 );									// make our points bigger
}

// setupGLEW() /////////////////////////////////////////////////////////////////
//
//      Used to initialize GLEW
//
////////////////////////////////////////////////////////////////////////////////
void setupGLEW() {
	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();

	/* check for an error */
	if( glewResult != GLEW_OK ) {
		printf( "[ERROR]: Error initalizing GLEW\n");
		/* Problem: glewInit failed, something is seriously wrong. */
  	fprintf( stderr, "[ERROR]: %s\n", glewGetErrorString(glewResult) );
		exit(EXIT_FAILURE);
	} else {
		 fprintf( stdout, "[INFO]: GLEW initialized\n" );
		 fprintf( stdout, "[INFO]: Status: Using GLEW %s\n", glewGetString(GLEW_VERSION) );
	}

	if( !glewIsSupported( "GL_VERSION_2_0" ) ) {
		printf( "[ERROR]: OpenGL not version 2.0+.  GLSL not supported\n" );
		exit(EXIT_FAILURE);
	}
}

// setupShaders() //////////////////////////////////////////////////////////////
//
//      Create our shaders.  Send GLSL code to GPU to be compiled.  Also get
//  handles to our uniform and attribute locations.
//
////////////////////////////////////////////////////////////////////////////////
void setupShaders() {
	// LOOKHERE #1
	geoshaderProgram = new CSCI441::ShaderProgram( "shaders/billboardQuadShader.v.glsl",
												"shaders/billboardQuadShader.g.glsl",
												"shaders/billboardQuadShader.f.glsl" );
	modelview_uniform_location  = geoshaderProgram->getUniformLocation( "mvMatrix" );
	projection_uniform_location = geoshaderProgram->getUniformLocation( "projMatrix" );
	vpos_attrib_location		= geoshaderProgram->getAttributeLocation( "vPos" );
	alpha_attrib_location       = geoshaderProgram->getAttributeLocation( "alpha" );

	textureshaderProgram = new CSCI441::ShaderProgram( "shaders/textureShader.v.glsl", 
												"shaders/textureShader.f.glsl");
	texture_mvp_uniform_location = textureshaderProgram->getUniformLocation( "mvpMatrix");
	texture_vpos_attrib_location = textureshaderProgram->getAttributeLocation( "vPosition");
	texture_coor_attrib_location = textureshaderProgram->getAttributeLocation( "texCoordIn");

}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {
	// LOOKHERE #2
	//-------------------------Particles------------------------------//
	for(vector<string> line: systemLines){
		if(line.size()==10){
			psystems.push_back( new PS::ParticleSystem(vpos_attrib_location, alpha_attrib_location, textureHandle, PS::ParticleSystem::Type::fountain, glm::vec3(stof(line[1]), stof(line[2]), stof(line[3])), stof(line[4]), 0.0f, 0.0f, stoi(line[5]), stoi(line[6]), stof(line[7]), stof(line[8]), stoi(line[9])) );
		}else if(line.size()==9){
			psystems.push_back( new PS::ParticleSystem(vpos_attrib_location, alpha_attrib_location, textureHandle, PS::ParticleSystem::Type::rain, glm::vec3(stof(line[1]), stof(line[2]), stof(line[3])), 0.0, stof(line[4]), stof(line[5]), 30, 60, stof(line[6]), stof(line[7]), stoi(line[8])) );
		}
	}
	
	

	

	//--------------------------Skybox------------------------//
	struct vert {
		float x;
		float y;
		float z;
	};

	struct texcoor {
		float x;
		float y;
	};

	//points on the box
	vert boxPoints[8]={
		{sbWidth, sbWidth, sbWidth},
		{sbWidth, -sbWidth, sbWidth},
		{-sbWidth, -sbWidth, sbWidth},
		{-sbWidth, sbWidth, sbWidth},
		{sbWidth, sbWidth, -sbWidth},
		{sbWidth, -sbWidth, -sbWidth},
		{-sbWidth, -sbWidth, -sbWidth},
		{-sbWidth, sbWidth, -sbWidth}
	};

	//an order of points represented by indexes of the points in boxPoints
	int boxOrder[36]={
		0,1,2,0,2,3, 
		4,5,1,4,1,0,
		5,6,7,5,7,4,
		3,2,6,3,6,7,
		7,4,0,7,0,3, 
		5,6,2,5,2,1
	};

	//texture coordinates, each line corresponds with each line of points above and the overal order of the textures
	texcoor texPoints[36]={ 
		{1,1},{1,0},{0,0},{1,1},{0,0},{0,1},
		{1,1},{1,0},{0,0},{1,1},{0,0},{0,1},
		{0,0},{1,0},{1,1},{0,0},{1,1},{0,1},
		{1,1},{1,0},{0,0},{1,1},{0,0},{0,1},
		{0,1},{1,1},{1,0},{0,1},{1,0},{0,0},
		{1,0},{0,0},{0,1},{1,0},{0,1},{1,1}
	};

	vert boxOrderedPoints[36];

	for(int i = 0; i<36; i++){
		boxOrderedPoints[i]=boxPoints[boxOrder[i]];
	}

	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(boxOrderedPoints), boxOrderedPoints, GL_STATIC_DRAW);

	glEnableVertexAttribArray(texture_vpos_attrib_location);
	glVertexAttribPointer(texture_vpos_attrib_location, 3, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)0);


	glGenBuffers(1, &skyboxTexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxTexVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(texPoints), texPoints, GL_STATIC_DRAW);

	glEnableVertexAttribArray(texture_coor_attrib_location);
	glVertexAttribPointer(texture_coor_attrib_location, 2, GL_FLOAT, GL_FALSE, sizeof(texcoor), (void*)0);
}
 
void setupTextures() {
	textureHandle = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/fireworks.png" );
	skyboxTHs[0] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/Tanto/posz.jpg");
	skyboxTHs[1] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/Tanto/posx.jpg");
	skyboxTHs[2] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/Tanto/negz.jpg");
	skyboxTHs[3] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/Tanto/negx.jpg");
	skyboxTHs[4] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/Tanto/posy.jpg");
	skyboxTHs[5] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/Tanto/negy.jpg");
}

//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMtx, glm::mat4 projMtx ) {
	// LOOKHERE #3

	//------------------Skybox----------------------//
	
	// stores our model matrix
	glm::mat4 modelMtx = glm::mat4(1.0f);



	// precompute our MVP CPU side so it only needs to be computed once
	glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
	// send MVP to GPU

	//use texture shader program
	textureshaderProgram->useProgram();
	glUniformMatrix4fv(texture_mvp_uniform_location, 1, GL_FALSE, &mvpMtx[0][0]);
	glBindVertexArray(skyboxVAO);
	glEnable(GL_TEXTURE_2D);
	// for each set of six points use the correct texture
	for(int i =0; i<36;i+=6){
		glBindTexture(GL_TEXTURE_2D, skyboxTHs[i/6]);
		glDrawArrays(GL_TRIANGLES, i, 6);
	}
	glDisable(GL_TEXTURE_2D);

	//-------------------------Particles (snowflaake)---------------------//
	// stores our model matrix
	modelMtx = glm::mat4(1.0,0.0,0.0,0.0,
					0.0,1.0,0.0,0.0,
					0.0,0.0,1.0,0.0,
					0.0,0.0,0.0,1.0);

	// use our shader program
	geoshaderProgram->useProgram();

	// precompute our MVP CPU side so it only needs to be computed once
	glm::mat4 mvMtx = viewMtx * modelMtx;;
	// send MVP to GPU
	glUniformMatrix4fv(modelview_uniform_location, 1, GL_FALSE, &mvMtx[0][0]);
	glUniformMatrix4fv(projection_uniform_location, 1, GL_FALSE, &projMtx[0][0]);

	for(PS::ParticleSystem* ps:psystems){
		ps->update(modelMtx, eyePoint, lookAtPoint);
		ps->draw();
	}
	

  
	

}

///*****************************************************************************
//
// Our main function

// main() ///////////////////////////////////////////////////////////////
//
//		Really you should know what this is by now.
//
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {

	srand( time(0) );									// seed our RNG

	if(!readInputFile(argv[1])){
		exit(EXIT_FAILURE);
	}

  // GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
	setupOpenGL();										// initialize all of the OpenGL specific information
	setupGLEW();											// initialize all of the GLEW specific information

  CSCI441::OpenGLUtils::printOpenGLInfo();

	setupShaders(); 									// load our shader program into memory
	setupTextures();									// load all our textures into memory
	setupBuffers();										// load all our VAOs and VBOs into memory
	

  // needed to connect our 3D Object Library to our shader
  CSCI441::setVertexAttributeLocations( vpos_attrib_location );

	recomputeOrientation();		// set up our camera position

  //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
    glDrawBuffer( GL_BACK );				// work with our back frame buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, windowWidth, windowHeight );

		// set the projection matrix based on the window size
		// use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projectionMatrix = glm::perspective( 45.0f, windowWidth / (float) windowHeight, 0.001f, 100.0f );

		// set up our look at matrix to position our camera
		glm::mat4 viewMatrix = glm::lookAt( eyePoint,lookAtPoint, upVector );

		// draw everything to the window
		// pass our view and projection matrices as well as deltaTime between frames
		renderScene( viewMatrix, projectionMatrix );

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen
	}

  glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;
}
