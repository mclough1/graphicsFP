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

#include <CSCI441/modelLoader3.hpp> // to load in OBJ models
#include <CSCI441/objects3.hpp>
#include <CSCI441/ShaderProgram3.hpp>
#include <CSCI441/TextureUtils.hpp>

#include "include/Enemy.h"

using namespace std;

//******************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;
bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles( -1.57f, 2.01f, 40.0f );
glm::vec3 eyePoint(   0.0f, 0.0f, 0.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );



string mansionStr = "models/Luigis_Mansion.obj";
string skyboxStr = "models/Skybox.obj";

const char* mansionModelfile = mansionStr.c_str();
const char* skyboxModelfile = skyboxStr.c_str();
CSCI441::ModelLoader* mansionModel = NULL;
CSCI441::ModelLoader* skyboxModel = NULL;

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
std::vector< Enemy* > enemies;
const GLfloat GROUND_SIZE = 18;
const GLfloat ENEMY_RADIUS = 1.0;
const GLint NUM_ENEMIES = 5;

//player values, from directional/positional to game state vaalues
glm::vec3 playerLoc = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 playerVel = glm::vec3(0.0f, 0.0f, 0.0f);
float pushVel = 0.0;
float pushAcc = 0.03;
glm::vec3 pushDir = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 playerDir = glm::vec3(0.0f, 0.0f, 0.0f);
bool moveUp, moveDown, moveRight, moveLeft;
float playerSpeed = 0.5;
bool playerAlive = true;
bool playerWon = false;
float animoffset, animstate;


//colors
glm::vec4 white(1,1,1,1);
glm::vec4 orange(1,0.5,0,1);
glm::vec4 black(0,0,0,1);



//******************************************************************************
//
// Helper Functions

// convertSphericalToCartesian() ///////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraAngles is updated.
//
////////////////////////////////////////////////////////////////////////////////
void convertSphericalToCartesian() {
	eyePoint.x = cameraAngles.z * sinf( cameraAngles.x ) * sinf( cameraAngles.y );
	eyePoint.y = cameraAngles.z * -cosf( cameraAngles.y );
	eyePoint.z = cameraAngles.z * -cosf( cameraAngles.x ) * sinf( cameraAngles.y );
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
					convertSphericalToCartesian();

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

	convertSphericalToCartesian();
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
	//model->enableAutoGenerateNormals();
  	mansionModel->loadModelFile( mansionModelfile );
	skyboxModel = new CSCI441::ModelLoader();
	//model->enableAutoGenerateNormals();
  	skyboxModel->loadModelFile( skyboxModelfile );




}

void populateEnemies() {
    srand( time(NULL) );
    const float RANGE_X = GROUND_SIZE*2;
    const float RANGE_Z = GROUND_SIZE*2;
    for(int i = 0; i < NUM_ENEMIES; i++) {
        // TODO: Populate our enemy locations
        Enemy* m = new Enemy( glm::vec3( rand()/(float)RAND_MAX * RANGE_X - RANGE_X/2.0f, 0.0f, (RANGE_Z * (i/(float)NUM_ENEMIES)) - RANGE_Z/2.0f),
                            	glm::vec3( rand()/(float)RAND_MAX - 0.5, 0.0, rand()/(float)RAND_MAX - 0.5 ),
                            	ENEMY_RADIUS * (rand()/(float)RAND_MAX+0.25) );
        enemies.push_back( m );
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
	
}

void moveEnemies() {
	// Move every enemy forward along its heading
	for(Enemy* enemy: enemies){
		enemy->moveForward(playerLoc);
	}
}

void enemiesFallOff() {
	// Check if any enemy passes beyond any wall, and kill them
	for(Enemy* enemy: enemies){
		if((enemy->location.x > GROUND_SIZE) || (enemy->location.x < -GROUND_SIZE) || (enemy->location.z > GROUND_SIZE) || (enemy->location.z < -GROUND_SIZE)){
			enemy->state = Enemy::State::DEAD;
		}
	}

	// if all of the enemies are dead then player wins
	playerWon = true;
	for(Enemy* enemy: enemies){
		if(enemy->state == Enemy::State::ALIVE){
			playerWon = false;
			return;
		}
	}
	
}

void collideEnemiesWithEachother() {
	
	// check for inter-enemy collisions
	// warning this isn't perfect...enemies can get caught and
	// continually bounce back-and-forth in place off
	// each other
	for(int i =0; i<NUM_ENEMIES-1; i++){
		for(int j = i+1; j<NUM_ENEMIES; j++){
			if(distance(enemies[i]->location, enemies[j]->location) < enemies[i]->getRadius() + enemies[j]->getRadius()){
				enemies[i]->moveBackward();
				enemies[j]->moveBackward();
				glm::vec3 norm = glm::normalize(enemies[i]->location-enemies[j]->location);
				enemies[i]->direction = enemies[i]->direction - 2*glm::dot(enemies[i]->direction, norm)*norm;
				enemies[j]->direction = enemies[j]->direction - 2*glm::dot(enemies[j]->direction, -1.0f*norm)*norm*-1.0f;
				float swapVel = enemies[i]->velocity;
				enemies[i]->velocity = enemies[j]->velocity;
				enemies[j]->velocity = swapVel;
			}
		}
	}

}

void collideEnemiesWithPlayer() {
	
	// check for enemy collisions with player
	// warning this isn't perfect...they can also get caught and
	// continually bounce back-and-forth in place off
	// each other
	for(int i =0; i<NUM_ENEMIES; i++){
		
		if(distance(enemies[i]->location, playerLoc) < enemies[i]->getRadius() + 1 && enemies[i]->state == Enemy::State::ALIVE){
			enemies[i]->moveBackward();
			glm::vec3 norm = glm::normalize(enemies[i]->location-playerLoc);
			enemies[i]->direction = enemies[i]->direction - 2*glm::dot(enemies[i]->direction, norm)*norm;
			if(playerVel.x == 0&&playerVel.z == 0 ){
				pushDir = normalize(-1.0f * enemies[i]->direction);
			}else{
				pushDir = normalize(normalize(playerVel) - 2*glm::dot(normalize(playerVel), -1.0f*norm)*norm*-1.0f);
			}
			
			enemies[i]->velocity = 0.6;
			pushVel = 0.5;
		}
		
	}

}

void updatePlayer() {
	
	// if the player died, then they fall down
	if(!playerAlive){
		playerVel.y-=pushAcc;
		playerLoc += playerVel;
		return;

	}

	// if the player is outside the platform then kill them
	if((playerLoc.x > GROUND_SIZE) || (playerLoc.x < -GROUND_SIZE) || (playerLoc.z > GROUND_SIZE) || (playerLoc.z < -GROUND_SIZE)){
			playerAlive = false;
			playerVel = glm::vec3(0.0f, 0.0f, 0.0f);
			return;
	}
	
	// just make sure the pushed velocity is >= 0
	if(pushVel <= 0){
		pushVel = 0;
	}

	// player velocity is a combination of the input controlls or the most recent bounce, which decays
	playerVel = pushDir*pushVel;

	//decay the push velocity
	pushVel-=pushAcc;

	// get the sum of the directional inputs from the player
	playerDir = glm::vec3(0.0f, 0.0f, 0.0f);
	
	bool moving = false;
	if(moveUp){
		playerDir += glm::vec3(playerSpeed, 0.0f, 0.0f);
		moving = true;
	}
	if(moveDown){
		playerDir += glm::vec3(-playerSpeed, 0.0f, 0.0f);
		moving = true;
	}
	if(moveRight){
		playerDir += glm::vec3(0.0f, 0.0f, playerSpeed);
		moving = true;
	}
	if(moveLeft){
		playerDir += glm::vec3(0.0f, 0.0f, -playerSpeed);
		moving = true;
	}

	// this is for the animation cycle that bounce the character up and down
	if(moving == true){
		animstate++;
		if(animstate>8&&animstate<16){
			animoffset = 0.3;
		}else if(animstate>=16){
			animstate = 0.0;
			animoffset = 0.0;
		}
	}else{
		animstate = 0.0;
		animoffset = 0.0;
	}

	// when hit the player is effectively stunned and thier inputs do not matter, the player moves in the direction of the bounce
	// if there is no current push velocity then add the player input to the player velocity
	if(pushVel <= 0){
		playerVel += playerDir;
	}
	
	// step the player
	playerLoc += playerVel;

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
	populateEnemies();								// generate enemies

	convertSphericalToCartesian();		// set up our camera position

	CSCI441::setVertexAttributeLocations( textureShaderAttributes.vPos, -1, textureShaderAttributes.vTextureCoord );
	CSCI441::drawSolidSphere( 1, 16, 16 );	// strange hack I need to make spheres draw - don't have time to investigate why..it's a bug with my library
	CSCI441::drawSolidCone( 1.0, 2.0, 16, 16 );//I too noticed this bug, wish I had seen this comment^ sooner

	bool completionMessagePrinted = false;
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
		if(!playerAlive){
			//death cam
			viewMatrix = glm::lookAt( glm::vec3(playerLoc.x*1.1, 3.0f, playerLoc.z*1.1), glm::vec3(playerLoc.x, -3.0f, playerLoc.z), upVector );
		}
		

		// draw everything to the window
		// pass our view and projection matrices as well as deltaTime between frames
		renderScene( viewMatrix, projectionMatrix );

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen

		/*// while the player is alive, move the enemies
		if(playerAlive){
			moveEnemies();
			enemiesFallOff();
			collideEnemiesWithEachother();
			collideEnemiesWithPlayer();
		}
		// while the game is still playing, the player can move the character
		if(!playerWon){
			updatePlayer();
		}

		//ones an outcome of the game has happened, only one message is displayed accordingly
		if(!playerAlive&&!completionMessagePrinted){
			cout<<"\n\n\nOh no, you died. Game Over."<<endl;
			completionMessagePrinted = true;
		}else if(playerWon&&!completionMessagePrinted){
			cout<<"\n\n\nHey, you won, good job!"<<endl;
			completionMessagePrinted = true;
		}*/
		
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
