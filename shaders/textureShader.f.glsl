#version 330 core

in vec2 texCoord;
in float distanceToPoint;
in vec3 lightToPoint;
in float distanceToMoon;

out vec4 fragColorOut;

uniform sampler2D tex;

uniform vec3 lightDir;

void main() {
	float light = 1.0;
	float moon = 1/distanceToMoon;
	float angle = acos(dot( lightToPoint, normalize(lightDir))); //angle between light and point
	  if ( distanceToPoint < 75 && angle < 1) {
		light = max(1, 2 * 10/distanceToPoint)	;
	  }
	vec3 ambient = 0.3*vec3(0.9, 0.9, 1.0);
  fragColorOut = texture( tex, texCoord );
  fragColorOut = vec4(fragColorOut.rgb * ambient * light + (vec3(90, 40, 0.0)* moon/2), fragColorOut.a);
  
}
