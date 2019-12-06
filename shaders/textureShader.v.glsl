/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #B
in vec3 vPosition;
in vec2 texCoordIn;
out vec2 texCoord;


// TODO #A
uniform mat4 mvpMatrix;

// TODO #G1

void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************
    
    // TODO #C
    gl_Position = mvpMatrix*vec4(vPosition, 1);


    texCoord = texCoordIn;
}
