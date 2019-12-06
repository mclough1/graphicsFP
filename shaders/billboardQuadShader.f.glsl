/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO J: add varying input
in vec2 texCoord;

in alphaData {
    float al;
} alpha;

out vec4 fragColorOut;

// TODO K: add texture uniform
uniform sampler2D tex;


void main() {

    //*****************************************
    //******* Final Color Calculations ********
    //*****************************************
    
    // TODO L: load texel
    float fade = 0.0;
    if(alpha.al<20){
        fade = alpha.al/20.0;
    }else{
        fade = 1.0;
    }

    fragColorOut = texture(tex, texCoord) * vec4(1.0, 1.0, 1.0, fade);
    //fragColorOut.w = fragColorOut.w*fade;
}
