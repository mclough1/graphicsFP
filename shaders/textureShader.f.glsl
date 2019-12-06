/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

in vec2 texCoord;

uniform sampler2D textureMap;

out vec4 fragColorOut;

void main() {
    vec4 texel = texture(textureMap, texCoord);

    fragColorOut = texel;

    // fragColorOut = vec4(1, 1, 1, 1);
}
