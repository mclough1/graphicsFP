#version 330 core

in vec2 texCoord;
in float distanceToPoint;
in vec3 lightToPoint;
in float distanceToMoon;

out vec4 fragColorOut;

uniform sampler2D tex;

uniform vec3 lightDir;

void main() {
	
  fragColorOut = texture( tex, texCoord ) + vec4(1.0, 0.0, 0.0, 0.0);
  
}
