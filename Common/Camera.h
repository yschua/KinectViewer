#pragma once
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>

// OpenGL Camera 
class Camera {
  glm::vec3 position;
  glm::vec3 viewDirection;
  glm::vec2 oldMousePosition;
  const glm::vec3 UP;
  float viewAngle;
public:
  Camera();
  void setMousePosition(int x, int y);
  void mouseUpdate(const glm::vec2& newMousePosition);
  void moveCameraPosition(int x, int y, int z);
  glm::mat4 getViewMatrix() const;
};
