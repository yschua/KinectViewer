#include "Camera.h"

Camera::Camera() :
  position(0.0f, 0.0f, 0.0f),
  viewDirection(0.0f, 0.0f, 1.0f),
  UP(0.0f, 1.0f, 0.0f)
{

}

void Camera::setMousePosition(int x, int y)
{
  oldMousePosition.x = x;
  oldMousePosition.y = y;
}

void Camera::mouseUpdate(const glm::vec2& newMousePosition)
{
  glm::vec2 mouseDelta = newMousePosition - oldMousePosition;
  viewDirection = glm::mat3(glm::rotate(mouseDelta.x / 100, UP)) * viewDirection;
  oldMousePosition = newMousePosition;
}

glm::mat4 Camera::getViewMatrix() const
{
  return glm::lookAt(position, position + viewDirection, UP);
}