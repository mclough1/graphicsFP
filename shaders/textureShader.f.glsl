#version 330 core

in vec2 texCoord;
in float distanceToPoint;
in vec3 lightToPoint;

out vec4 fragColorOut;

uniform sampler2D tex;

uniform vec3 lightDir;

void main() {
	float light = 1.0;
	float angle = acos(dot( lightToPoint, normalize(lightDir))); //angle between light and point
	  if ( distanceToPoint < 75 && angle < 1) {
		light = max(1, 2 * 10/distanceToPoint)	;
	  }
  fragColorOut = texture( tex, texCoord );
  fragColorOut = vec4(fragColorOut.rgb * light, fragColorOut.a);
  
}
