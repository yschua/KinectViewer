#include "Camera.h"

Camera::Camera() :
  position(0.0f, 0.0f, 0.0f),
  viewDirection(0.0f, 0.0f, 1.0f),
  UP(0.0f, 1.0f, 0.0f),
  RIGHT(1.0f, 0.0f, 0.0f)
{
  viewAngle = glm::vec2(0.0f, 0.0f);
  yaw = 90.f;
  pitch = 0.f;
}

void Camera::setMousePosition(int x, int y)
{
  oldMousePosition.x = x;
  oldMousePosition.y = y;
}

void Camera::mouseRotateCamera(const glm::vec2& newMousePosition)
{
  float scale = 100.f;

  glm::vec2 mouseDelta = newMousePosition - oldMousePosition;

  yaw += mouseDelta.x / scale;
  pitch += mouseDelta.y / scale;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  //viewDirection = glm::normalize(front);

  viewDirection = glm::mat3(glm::rotate(mouseDelta.x / scale, -UP)) * viewDirection;
  viewDirection = glm::mat3(glm::rotate(mouseDelta.y / scale, RIGHT)) * viewDirection;

  std::cout << viewDirection.x << ' ' << viewDirection.y << ' ' << viewDirection.z << std::endl;
  viewAngle += mouseDelta / 500.0f;

  oldMousePosition = newMousePosition;
}

void Camera::mouseMoveCamera(const glm::vec2& newMousePosition)
{
  glm::vec2 mouseDelta = newMousePosition - oldMousePosition;

  float scale = 4.f;
  moveCameraPosition(mouseDelta.x / scale, mouseDelta.y / scale, 0);

  oldMousePosition = newMousePosition;
}

void Camera::moveCameraPosition(int x, int y, int z)
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

  //moveDirection = glm::mat3(glm::rotate(viewAngle.x, -UP)) * moveDirection;
  //moveDirection = glm::mat3(glm::rotate(viewAngle.y, RIGHT)) * moveDirection;
  //position += moveDirection;


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
  viewAngle = glm::vec2(0.0f, 0.0f);
}