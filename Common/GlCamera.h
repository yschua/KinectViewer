#pragma once
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>

// OpenGL Camera 
class GlCamera {
  glm::vec3 position;
  glm::vec3 viewDirection;
  glm::vec2 oldMousePosition;
  const glm::vec3 UP;
  const glm::vec3 RIGHT;
public:
  GlCamera();
  void setMousePosition(int x, int y);
  void mouseRotateCamera(const glm::vec2& newMousePosition);
  void mouseMoveCamera(const glm::vec2& newMousePosition);
  void moveCameraPosition(int x, int y, int z);
  glm::mat4 getViewMatrix() const;
  void resetView();
};
