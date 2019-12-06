/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

in vec3 vPos;
in float alpha;

out alphaData {
    float al;
} alphad;

uniform mat4 mvMatrix;



void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************
    
    gl_Position = mvMatrix * vec4(vPos, 1.0);

    alphad.al = alpha;

}
