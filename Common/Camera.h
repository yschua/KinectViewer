#pragma once
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

class Camera {
  glm::vec3 position;
  glm::vec3 viewDirection;
  glm::vec2 oldMousePosition;
  const glm::vec3 UP;
public:
  Camera();
  void setMousePosition(int x, int y);
  void mouseUpdate(const glm::vec2& newMousePosition);
  glm::mat4 getViewMatrix() const;
};
