#ifndef _ENEMY_H_
#define _ENEMY_H_ 1

#include <glm/glm.hpp>

#include <GL/glew.h>

class Enemy {
public:

	// CONSTRUCTORS / DESTRUCTORS
  Enemy();
  Enemy( glm::vec3 loc, glm::vec3 dir, double r );

	// MISCELLANEOUS
  glm::vec3 location;
  glm::vec3 direction;
  float velocity;

  enum State {ALIVE, DEAD};

	State state;

  void draw( glm::mat4 modelMtx, GLint uniform_modelMtx_loc, GLint uniform_color_loc ) const;
  void moveForward(glm::vec3 target);
  void moveBackward();
  double getRadius() const;

private:
  double _radius;
  double _rotation;
  
  glm::vec4 _color;
};

#endif	// _ENEMY_H_
