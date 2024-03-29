#version 330 core

in vec3 vPos;
in vec2 vTextureCoord;

out vec2 texCoord;

out float distanceToPoint;
out float distanceToMoon;
out vec3 lightToPoint;

uniform vec3 lightPos;


uniform mat4 modelMtx;
uniform mat4 viewProjectionMtx;
uniform vec3 shake;

void main() {

  gl_Position = viewProjectionMtx * modelMtx * vec4(vPos+shake, 1.0);
  texCoord = vTextureCoord;
  
  distanceToMoon = distance(vec3(-10, 113, -291), vPos);
  
  lightToPoint = normalize(vPos - lightPos);
  distanceToPoint = distance(vPos, lightPos);
}
