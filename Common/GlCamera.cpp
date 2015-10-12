#include "GlCamera.h"

GlCamera::GlCamera() :
  position(0.0f, 0.0f, 0.0f),
  viewDirection(0.0f, 0.0f, 1.0f),
  UP(0.0f, 1.0f, 0.0f),
  RIGHT(1.0f, 0.0f, 0.0f)
{
}

void GlCamera::setMousePosition(int x, int y)
{
  oldMousePosition.x = x;
  oldMousePosition.y = y;
}

void GlCamera::mouseRotateCamera(const glm::vec2& newMousePosition)
{
  float scale = 100.f;
  glm::vec2 mouseDelta = newMousePosition - oldMousePosition;

  viewDirection = glm::mat3(glm::rotate(mouseDelta.x / scale, -UP)) * viewDirection;
  viewDirection = glm::mat3(glm::rotate(mouseDelta.y / scale, RIGHT)) * viewDirection;

  oldMousePosition = newMousePosition;
}

void GlCamera::mouseMoveCamera(const glm::vec2& newMousePosition)
{
  float scale = 4.f;
  glm::vec2 mouseDelta = newMousePosition - oldMousePosition;

  moveCameraPosition(mouseDelta.x / scale, mouseDelta.y / scale, 0);

  oldMousePosition = newMousePosition;
}

void GlCamera::moveCameraPosition(int x, int y, int z)
{
  float scale = 100.0f;
  glm::vec3 moveDirection(x / scale, y / scale, z / scale);

  glm::vec3 forwardVector = viewDirection - position;
  forwardVector = glm::normalize(forwardVector);

  position += forwardVector * moveDirection.z;
  viewDirection += forwardVector * moveDirection.z;

  glm::vec3 leftVector = glm::cross(UP, forwardVector);
  position += leftVector * moveDirection.x;
  viewDirection += leftVector * moveDirection.x;

  position += UP * moveDirection.y;
  viewDirection += UP * moveDirection.y;
}

glm::mat4 GlCamera::getViewMatrix() const
{
  return glm::lookAt(position, position + viewDirection, UP);
}

void GlCamera::resetView()
{
  position = glm::vec3(0.0f, 0.0f, 0.0f);
  viewDirection = glm::vec3(0.0f, 0.0f, 1.0f);
}