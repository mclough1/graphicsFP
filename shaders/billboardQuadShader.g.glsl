/*
 *   Geometry Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #A: set primitive input
layout(points) in;

// TODO #B: set primitive output
layout(triangle_strip, max_vertices = 4) out; 

in alphaData {
    float al;
} inAlpha[];

out alphaData {
    float al;
} outAlpha;


uniform mat4 projMatrix;

// TODO I: add varying output
out vec2 texCoord;
out float fade;


void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************
    // TODO #C: add first point




    gl_Position = projMatrix * (gl_in[0].gl_Position+vec4(-1,-1, 0, 0));
    texCoord = vec2(0,0);
    outAlpha.al = inAlpha[0].al;
    // TODO #D: emit!
    EmitVertex();
    
    
    // TODO #F: add 2nd point
    gl_Position = projMatrix * (gl_in[0].gl_Position+vec4(-1,1, 0, 0));
    texCoord = vec2(1,0);
    EmitVertex();
    
    
    // TODO #G: add 3rd point
    gl_Position = projMatrix * (gl_in[0].gl_Position+vec4(1,-1, 0, 0));
    texCoord = vec2(0,1);
    EmitVertex();
    

    // TODO #H: add 4th point
    gl_Position = projMatrix * (gl_in[0].gl_Position+vec4(1,1, 0, 0));
    texCoord = vec2(1,1);


    EmitVertex();

    

    // TODO #E: end!
    EndPrimitive();


    
}
