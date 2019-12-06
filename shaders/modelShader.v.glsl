/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #B
in vec3 vPosition;
in vec3 vNormal;


// TODO #F1
out vec4 theColor;
out vec3 v;
out vec3 n;
//out vec3 cam;
out vec4 l;

// TODO #A
uniform mat4 modelMtx;
uniform mat4 viewProjectionMtx;
uniform vec3 lightPos;
uniform vec4 color;
//uniform vec3 camPos;



void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************
    
    vec3 newVertex = vPosition;
    gl_Position = viewProjectionMtx * modelMtx * vec4(newVertex, 1.0);
    vec4 vt = viewProjectionMtx * modelMtx *vec4(newVertex, 1);

    v = vec3(vt.x, vt.y, vt.z);
    n = vNormal;

    //cam = camPos;
    theColor = color;
    l= vec4(lightPos, 1.0);
    
}
