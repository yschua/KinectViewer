#include "Camera.h"

Camera::Camera() :
  position(0.0f, 0.0f, 0.0f),
  viewDirection(0.0f, 0.0f, 1.0f),
  UP(0.0f, 1.0f, 0.0f),
  RIGHT(1.0f, 0.0f, 0.0f)
{
  viewAngle = glm::vec2(0.0f, 0.0f);
}

void Camera::setMousePosition(int x, int y)
{
  oldMousePosition.x = x;
  oldMousePosition.y = y;
}

void Camera::mouseUpdate(const glm::vec2& newMousePosition)
{
  glm::vec2 mouseDelta = newMousePosition - oldMousePosition;
  viewDirection = glm::mat3(glm::rotate(mouseDelta.x / 100.0f, -UP)) * viewDirection; // can shorten
  viewDirection = glm::mat3(glm::rotate(mouseDelta.y / 100.0f, RIGHT)) * viewDirection;

  oldMousePosition = newMousePosition;
  viewAngle += mouseDelta / 100.0f;
}

void Camera::moveCameraPosition(int x, int y, int z)
{
  float scale = 100.0f;
  glm::vec3 moveDirection(x / scale, y / scale, z / scale);
  moveDirection = glm::mat3(glm::rotate(viewAngle.x, -UP)) * moveDirection;
  moveDirection = glm::mat3(glm::rotate(viewAngle.y, RIGHT)) * moveDirection;
  position += moveDirection;
  //position.x += ((viewDirection.x * x) + (viewDirection.z * x)) / scale;
  //position.y += y / scale;
  //position.z += ((viewDirection.x * z) + (viewDirection.z * z)) / scale;
  //std::cout << "viewdirection: " << viewDirection.x << "," << viewDirection.y << "," << viewDirection.z << std::endl;
  //std::cout << "angle: " << viewAngle << std::endl;
}

glm::mat4 Camera::getViewMatrix() const
{
  return glm::lookAt(position, position + viewDirection, UP);
}

void Camera::resetView()
{
  position = glm::vec3(0.0f, 0.0f, 0.0f);
  viewDirection = glm::vec3(0.0f, 0.0f, 1.0f);
}