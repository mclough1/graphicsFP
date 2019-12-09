/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: lab11
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with VAOs & VBOs using a
 *	MD5 model.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2017
 *
 *
 */

//******************************************************************************

#include <GL/glew.h>
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SOIL/SOIL.h>		// for image loading

#include <stdio.h>				// for printf functionality
#include <iostream>
#include <stdlib.h>				// for exit functionality
#include <time.h>				// for time functionality

#include <vector>					// for vector
#include <sstream>
#include <ctime>

#include <CSCI441/modelLoader3.hpp> // to load in OBJ models
#include <CSCI441/objects3.hpp>
#include <CSCI441/ShaderProgram3.hpp>
#include <CSCI441/TextureUtils.hpp>

using namespace std;

//******************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;
bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles(-4.71f, 1.65f, 15.0f);
glm::vec3 originalCameraAngles(-4.71f, 1.65f, 15.0f);
glm::vec3 eyePoint(   0.0f, 0.0f, 0.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

bool freeCamOn = false;

glm::vec2 freeCamAngles( -1.57f, 2.01f);
glm::vec3 freeCamPos(   0.0f, 0.0f, 0.0f );
glm::vec3 freeCamDir( 0.0f,  0.0f,  0.0f );
glm::vec3 freeCamLookAt( 0.0f,  0.0f,  0.0f );

string mansionStr = "models/Luigis_Mansion.obj";
string skyboxStr = "models/Skybox.obj";
string greenMarioStr = "models/greenmario_stand.obj";

const char* mansionModelfile = mansionStr.c_str();
const char* skyboxModelfile = skyboxStr.c_str();
const char* greenMarioModelFile = greenMarioStr.c_str();

CSCI441::ModelLoader* mansionModel = NULL;
CSCI441::ModelLoader* skyboxModel = NULL;
CSCI441::ModelLoader* greenMarioModel = NULL;

const char* greenmarioModelfile = NULL;
vector<CSCI441::ModelLoader*> greenmarioModelFrames;
string greenmarioFilenameTemplate = "models/greenmario_";
int WALKING_FRAME_COUNT = 41;
int currentFrame = 0;

//two shaders, one texture shader for the platform and skybox, and a custom one for everything else
CSCI441::ShaderProgram* textureShaderProgram = NULL;
CSCI441::ShaderProgram* customShaderProgram = NULL;
glm::vec3 lightPos = glm::vec3(-10.0f, 10.0f, -10.0f);
struct ShaderUniformLocations {
	GLint modelMtx;
	GLint viewProjectionMtx;
	GLint tex;
	GLint color;
	GLint camera;
	GLint light;
} textureShaderUniforms, customShaderUniforms;
struct ShaderAttributeLocations {
	GLint vPos;
	GLint vTextureCoord;
	GLint vNorm;
} textureShaderAttributes, customShaderAttributes;

//enemy values, number of enemies and thier size
const GLfloat GROUND_SIZE = 18;
const GLfloat ENEMY_RADIUS = 1.0;
const GLint NUM_ENEMIES = 5;

//colors
glm::vec4 white(1,1,1,1);
glm::vec4 orange(1,0.5,0,1);
glm::vec4 black(0,0,0,1);

// resetPosition constant values (startPos, endBox)
float startX = 28.4f;
float startZ = -188.0f;
float DEFAULT_Y_VAL = 6.25;
float finishUpperLeftX = 131.5f;		// corner of finish box next to left of staircase, facing the mansion
float finishUpperLeftZ = -112.65f;
float finishLowerRightX = 144.0f;		// corner of finish box in front of right of staircase
float finishLowerRightZ = -107.8f;
bool paused = false;
time_t startTime;

//player values, from directional/positional to game state vaalues
glm::vec3 playerPos = glm::vec3(startX, DEFAULT_Y_VAL, startZ);
glm::vec3 playerDir = glm::vec3(0.0f, 0.0f, 0.0f);
bool moveUp, moveDown, moveRight, moveLeft;
bool playerAlive = true;
bool playerWon = false;
float playerSpeed = 0.5;
bool moving = false;

//******************************************************************************
//
// Helper Functions

// updateCamera() ///////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraAngles is updated.
//
////////////////////////////////////////////////////////////////////////////////
void updateCamera() {
	if(freeCamOn){
		float r = sinf(freeCamAngles.y);
		freeCamDir = glm::vec3(1*r*sinf(freeCamAngles.x), -1*cosf(freeCamAngles.y), -1*r*cosf(freeCamAngles.x));

		freeCamDir = normalize(freeCamDir);
		freeCamLookAt = freeCamPos+freeCamDir;
	}else{
		eyePoint.x = cameraAngles.z * sinf( cameraAngles.x ) * sinf( cameraAngles.y );
		eyePoint.y = cameraAngles.z * -cosf( cameraAngles.y );
		eyePoint.z = cameraAngles.z * -cosf( cameraAngles.x ) * sinf( cameraAngles.y );
		eyePoint+=playerPos;
		lookAtPoint = playerPos;
	}
}

void updateFreeCamera(){
	if(moveUp){
		freeCamPos += normalize(freeCamDir)*2.0f;
	}
	if(moveDown){
		freeCamPos -= normalize(freeCamDir)*2.0f;
	}
	if(moveRight){
		//freeCamPos += normalize(cross(normalize(freeCamDir), upVector))*2.0f;
	}
	if(moveLeft){
		//freeCamPos -= normalize(cross(normalize(freeCamDir), upVector))*2.0f;
	}
	updateCamera();
}


bool registerOpenGLTexture(unsigned char *textureData,
                           unsigned int texWidth, unsigned int texHeight,
                           GLuint &textureHandle) {
    if(textureData == 0) {
        fprintf(stderr,"Cannot register texture; no data specified.");
        return false;
    }

    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

    return true;
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
	if( (key == GLFW_KEY_ESCAPE || key == 'Q') && action == GLFW_PRESS )
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if( (key == '1') && action == GLFW_PRESS ){
		freeCamOn = ! freeCamOn;
		moveUp = moveDown = moveRight = moveLeft = false;
		updateCamera();
	}
		

	if( action == GLFW_PRESS) {
		switch( key ) {
			
			case GLFW_KEY_W:
				moveUp = true;
				break;
			case GLFW_KEY_S:
				moveDown = true;
				break;
			case GLFW_KEY_A:
				moveLeft = true;
				break;
			case GLFW_KEY_D:
				moveRight = true;
				break;
		}
	}

	if( action == GLFW_RELEASE) {
		switch( key ) {
			
			case GLFW_KEY_W:
				moveUp = false;
				break;
			case GLFW_KEY_S:
				moveDown = false;
				break;
			case GLFW_KEY_A:
				moveLeft = false;
				break;
			case GLFW_KEY_D:
				moveRight = false;
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
		controlDown = (mods & GLFW_MOD_CONTROL);
	} else {
		leftMouseDown = false;
		mousePosition.x = -9999.0f;
		mousePosition.y = -9999.0f;
		controlDown = false;
	}
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
				if( (mousePosition.x - -9999.0f) < 0.001f ) {
					mousePosition.x = xpos;
					mousePosition.y = ypos;
				} else {

					if(freeCamOn){
						freeCamAngles.x += (xpos - mousePosition.x)*0.005f;
						freeCamAngles.y -= (ypos - mousePosition.y)*0.005f;

						if( freeCamAngles.y < 0 ) freeCamAngles.y = 0.0f + 0.001f;
						if( freeCamAngles.y >= M_PI ) freeCamAngles.y = M_PI - 0.001f;
					}else{
						if( !controlDown ) {
							cameraAngles.x += (xpos - mousePosition.x)*0.005f;
							cameraAngles.y += (ypos - mousePosition.y)*0.005f;

							if( cameraAngles.y < 0 ) cameraAngles.y = 0.0f + 0.001f;
							if( cameraAngles.y >= M_PI ) cameraAngles.y = M_PI - 0.001f;
						} else {
							double totChgSq = (xpos - mousePosition.x) + (ypos - mousePosition.y);
							cameraAngles.z += totChgSq*0.02f;

							if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
							if( cameraAngles.z >= 500.0f ) cameraAngles.z = 500.0f;
						}
					}

					
					updateCamera();

					mousePosition.x = xpos;
					mousePosition.y = ypos;
				}
			}
			// passive motion
			else {

			}
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
	double totChgSq = yOffset;
	cameraAngles.z += totChgSq*0.2f;

	if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
	if( cameraAngles.z >= 500.0f ) cameraAngles.z = 500.0f;

	updateCamera();
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
	GLFWwindow *window = glfwCreateWindow(640, 480, "Lab11: Collision Detection", NULL, NULL);
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

// setupTextures() /////////////////////////////////////////////////////////////
//
//      Load and register all the tetures for our program
//
////////////////////////////////////////////////////////////////////////////////
void setupTextures() {
	
}

void setupShaders() {
	textureShaderProgram = new CSCI441::ShaderProgram( "shaders/textureShader.v.glsl", "shaders/textureShader.f.glsl" );
	
	textureShaderUniforms.modelMtx          = textureShaderProgram->getUniformLocation( "modelMtx" );
	textureShaderUniforms.viewProjectionMtx = textureShaderProgram->getUniformLocation( "viewProjectionMtx" );
	textureShaderUniforms.tex               = textureShaderProgram->getUniformLocation( "tex" );
	textureShaderUniforms.color             = textureShaderProgram->getUniformLocation( "color" );
	
	textureShaderAttributes.vPos            = textureShaderProgram->getAttributeLocation( "vPos" );
	textureShaderAttributes.vTextureCoord   = textureShaderProgram->getAttributeLocation( "vTextureCoord" );


	customShaderProgram = new CSCI441::ShaderProgram( "shaders/modelShader.v.glsl", "shaders/modelShader.f.glsl" );

	customShaderUniforms.modelMtx          = customShaderProgram->getUniformLocation( "modelMtx" );
	customShaderUniforms.viewProjectionMtx = customShaderProgram->getUniformLocation( "viewProjectionMtx" );
	customShaderUniforms.color             = customShaderProgram->getUniformLocation( "color" );
	//customShaderUniforms.camera             = customShaderProgram->getUniformLocation( "color" );
	customShaderUniforms.light             = customShaderProgram->getUniformLocation( "lightPos" );
	
	customShaderAttributes.vPos            = customShaderProgram->getAttributeLocation( "vPosition" );
	//customShaderAttributes.vTextureCoord   = customShaderProgram->getAttributeLocation( "vTextureCoord" );
	customShaderAttributes.vNorm   = customShaderProgram->getAttributeLocation( "vNormal" );

}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {

	///////////////////////////////////////////
	//
	// Models

	mansionModel = new CSCI441::ModelLoader();
  	mansionModel->loadModelFile( mansionModelfile );
	skyboxModel = new CSCI441::ModelLoader();
  	skyboxModel->loadModelFile( skyboxModelfile );
	greenMarioModel = new CSCI441::ModelLoader();
  	greenMarioModel->loadModelFile( greenMarioModelFile );

	for(int i = 1; i <= WALKING_FRAME_COUNT; ++i)
	{
		std::stringstream ss;
		ss << i;
		string frameNumber = ss.str();
		
		greenmarioModelFrames.push_back(new CSCI441::ModelLoader());
		greenmarioModelfile = (greenmarioFilenameTemplate + frameNumber + ".obj").c_str();
		//cout << greenmarioModelfile << endl;
		greenmarioModelFrames.back()->loadModelFile(greenmarioModelfile);
	}


}




//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix ) {
	// use our texture shader program
	// set all our uniforms
	glm::mat4 modelMatrix(1.0f), vp = projectionMatrix * viewMatrix;
	glm::vec4 white(1,1,1,1);


	textureShaderProgram->useProgram();
	glUniformMatrix4fv(textureShaderUniforms.modelMtx, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(textureShaderUniforms.viewProjectionMtx, 1, GL_FALSE, &vp[0][0]);
	glUniform1ui(textureShaderUniforms.tex, GL_TEXTURE0);
	glUniform4fv(textureShaderUniforms.color, 1, &white[0]);
	mansionModel->draw( textureShaderAttributes.vPos, -1,  textureShaderAttributes.vTextureCoord);
	skyboxModel->draw( textureShaderAttributes.vPos, -1,  textureShaderAttributes.vTextureCoord);

	glm::mat4 playerMtx = glm::translate(glm::mat4(1.0f), playerPos);
	playerMtx = glm::rotate( playerMtx, -cameraAngles.x, upVector );
	glUniformMatrix4fv(textureShaderUniforms.modelMtx, 1, GL_FALSE, &playerMtx[0][0]);

	if(moving)
	{
		greenmarioModelFrames[currentFrame]->draw(textureShaderAttributes.vPos, -1,  textureShaderAttributes.vTextureCoord);
	}
	else
	{
		greenMarioModel->draw( textureShaderAttributes.vPos, -1,  textureShaderAttributes.vTextureCoord);
	}	
}


void updatePlayer() {
	//cout << "(" << playerPos.x << "," << playerPos.y << "," << playerPos.z << ")" << endl;
	
	if(!paused)
	{
		// get the sum of the directional inputs from the player
		playerDir = lookAtPoint-eyePoint;
		playerDir.y = 0.0;
		playerDir = normalize(playerDir);
		
		moving = false;
		if(moveUp){
			playerPos += playerDir * playerSpeed;
			moving = true;
		}
		if(moveDown){
			playerPos -= playerDir * playerSpeed;
			moving = true;
		}
		if(moveRight){
			playerPos += normalize(cross(playerDir, upVector))*playerSpeed;
			moving = true;
		}
		if(moveLeft){
			playerPos -= normalize(cross(playerDir, upVector))*playerSpeed;
			moving = true;
		}
		
		playerPos.y = DEFAULT_Y_VAL;
		// this is for the animation cycle if the player is walking
		if(moving == true){
			currentFrame++;
			if(currentFrame >= WALKING_FRAME_COUNT)
			{
				currentFrame = 0;
			}
		}else{
			currentFrame = 0;
		}
	}
	
	if(playerPos.x > finishUpperLeftX && playerPos.x < finishLowerRightX && playerPos.z > finishUpperLeftZ && playerPos.z < finishLowerRightZ)
	{
		if(!paused)
		{
			startTime = time(NULL);
			paused = true;
		}
		else
		{
			if(time(NULL) - startTime > 3)
			{
				playerPos.x = startX;
				playerPos.z = startZ;
				cameraAngles = originalCameraAngles;
				paused = false;
			}
		}
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
	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
	setupOpenGL();										// initialize all of the OpenGL specific information
	setupGLEW();											// initialize all of the GLEW specific information
	setupShaders();										// load our shaders into memory
	setupBuffers();										// load all our VAOs and VBOs into memory
	setupTextures();									// load all textures into memory

	updateCamera();		// set up our camera position

	CSCI441::setVertexAttributeLocations( textureShaderAttributes.vPos, -1, textureShaderAttributes.vTextureCoord );
	CSCI441::drawSolidSphere( 1, 16, 16 );	// strange hack I need to make spheres draw - don't have time to investigate why..it's a bug with my library
	CSCI441::drawSolidCone( 1.0, 2.0, 16, 16 );//I too noticed this bug, wish I had seen this comment^ sooner

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
		glm::mat4 projectionMatrix = glm::perspective( 45.0f, windowWidth / (float) windowHeight, 0.001f, 1000.0f );

		// set up our look at matrix to position our camera
		glm::mat4 viewMatrix = glm::lookAt( eyePoint,lookAtPoint, upVector );

		if(freeCamOn){
			viewMatrix = glm::lookAt( freeCamPos, freeCamLookAt, upVector );
		}

		// draw everything to the window
		// pass our view and projection matrices as well as deltaTime between frames
		renderScene( viewMatrix, projectionMatrix );

		glfwSwapBuffers(window);		// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen

		if(!freeCamOn)
		{
			updatePlayer();
		}	
		updateCamera();
		
		if(freeCamOn)
		{
			updateFreeCamera();
		}
		
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
