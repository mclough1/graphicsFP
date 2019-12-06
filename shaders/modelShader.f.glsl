/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #F3
in vec4 theColor;
in vec3 v;
in vec3 n;
//in vec3 cam;
in vec4 l;


// TODO #D
out vec4 fragColorOut;

void main() {

    //*****************************************
    //******* Final Color Calculations ********
    //*****************************************




    vec3 lightvec = normalize(vec3(l.x, l.y, l.z)-v);
    float angle = dot(lightvec, normalize(n));
    vec4 diffuse;
    if(angle<0.3){
        diffuse = 0.25*theColor;
    }else if(angle<0.5){
        diffuse = 0.5*theColor;
    }else if(angle<0.8){
        diffuse = 0.75*theColor;
    }else{
        diffuse = theColor;
    }
    
   


    diffuse.w = 1.0;


    fragColorOut = diffuse;
}
