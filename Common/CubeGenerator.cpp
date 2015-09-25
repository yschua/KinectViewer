#include "CubeGenerator.h"

CubeGenerator::CubeGenerator()
{
  std::vector<Vertex> vertices;

  // Top face (green)
  glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
  vertices.push_back({ glm::vec3(1.f, 1.f, -1.f), green });
  vertices.push_back({ glm::vec3(-1.f, 1.f, -1.f), green });
  vertices.push_back({ glm::vec3(-1.f, 1.f, 1.f), green });
  vertices.push_back({ glm::vec3(1.f, 1.f, 1.f), green });

  // Bottom face (orange)
  glm::vec3 orange = glm::vec3(1.f, 0.5f, 0.f);
  vertices.push_back({ glm::vec3(1.f, -1.f, 1.f), orange });
  vertices.push_back({ glm::vec3(-1.f, -1.f, 1.f), orange });
  vertices.push_back({ glm::vec3(-1.f, -1.f, -1.f), orange });
  vertices.push_back({ glm::vec3(1.f, -1.f, -1.f), orange });

  // Front face (red)
  glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
  vertices.push_back({ glm::vec3(1.f, 1.f, 1.f), red });
  vertices.push_back({ glm::vec3(-1.f, 1.f, 1.f), red });
  vertices.push_back({ glm::vec3(-1.f, -1.f, 1.f), red });
  vertices.push_back({ glm::vec3(1.f, -1.f, 1.f), red });

  // Back face (yellow)
  glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
  vertices.push_back({ glm::vec3(1.f, -1.f, -1.f), yellow });
  vertices.push_back({ glm::vec3(-1.f, -1.f, -1.f), yellow });
  vertices.push_back({ glm::vec3(-1.f, 1.f, -1.f), yellow });
  vertices.push_back({ glm::vec3(1.f, 1.f, -1.f), yellow });

  // Left face (blue)
  glm::vec3 blue = glm::vec3(0.f, 0.f, 1.f);
  vertices.push_back({ glm::vec3(-1.f, 1.f, 1.f), blue });
  vertices.push_back({ glm::vec3(-1.f, 1.f, -1.f), blue });
  vertices.push_back({ glm::vec3(-1.f, -1.f, -1.f), blue });
  vertices.push_back({ glm::vec3(-1.f, -1.f, 1.f), blue });

  // Right face (magenta)
  glm::vec3 magenta = glm::vec3(1.f, 0.f, 1.f);
  vertices.push_back({ glm::vec3(1.f, 1.f, -1.f), magenta });
  vertices.push_back({ glm::vec3(1.f, 1.f, 1.f), magenta });
  vertices.push_back({ glm::vec3(1.f, -1.f, 1.f), magenta });
  vertices.push_back({ glm::vec3(1.f, -1.f, -1.f), magenta });

  for (int i = 0; i < vertices.size(); i++) {
    vertices[i].position.z += 4.f;
  }

  pointCloud.vertices = vertices;
}

CubeGenerator::~CubeGenerator()
{
}

void CubeGenerator::reset()
{
  std::vector<Vertex> vertices;

  // Top face (green)
  glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
  vertices.push_back({ glm::vec3(1.f, 1.f, -1.f), green });
  vertices.push_back({ glm::vec3(-1.f, 1.f, -1.f), green });
  vertices.push_back({ glm::vec3(-1.f, 1.f, 1.f), green });
  vertices.push_back({ glm::vec3(1.f, 1.f, 1.f), green });

  // Bottom face (orange)
  glm::vec3 orange = glm::vec3(1.f, 0.5f, 0.f);
  vertices.push_back({ glm::vec3(1.f, -1.f, 1.f), orange });
  vertices.push_back({ glm::vec3(-1.f, -1.f, 1.f), orange });
  vertices.push_back({ glm::vec3(-1.f, -1.f, -1.f), orange });
  vertices.push_back({ glm::vec3(1.f, -1.f, -1.f), orange });

  // Front face (red)
  glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
  vertices.push_back({ glm::vec3(1.f, 1.f, 1.f), red });
  vertices.push_back({ glm::vec3(-1.f, 1.f, 1.f), red });
  vertices.push_back({ glm::vec3(-1.f, -1.f, 1.f), red });
  vertices.push_back({ glm::vec3(1.f, -1.f, 1.f), red });

  // Back face (yellow)
  glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
  vertices.push_back({ glm::vec3(1.f, -1.f, -1.f), yellow });
  vertices.push_back({ glm::vec3(-1.f, -1.f, -1.f), yellow });
  vertices.push_back({ glm::vec3(-1.f, 1.f, -1.f), yellow });
  vertices.push_back({ glm::vec3(1.f, 1.f, -1.f), yellow });

  // Left face (blue)
  glm::vec3 blue = glm::vec3(0.f, 0.f, 1.f);
  vertices.push_back({ glm::vec3(-1.f, 1.f, 1.f), blue });
  vertices.push_back({ glm::vec3(-1.f, 1.f, -1.f), blue });
  vertices.push_back({ glm::vec3(-1.f, -1.f, -1.f), blue });
  vertices.push_back({ glm::vec3(-1.f, -1.f, 1.f), blue });

  // Right face (magenta)
  glm::vec3 magenta = glm::vec3(1.f, 0.f, 1.f);
  vertices.push_back({ glm::vec3(1.f, 1.f, -1.f), magenta });
  vertices.push_back({ glm::vec3(1.f, 1.f, 1.f), magenta });
  vertices.push_back({ glm::vec3(1.f, -1.f, 1.f), magenta });
  vertices.push_back({ glm::vec3(1.f, -1.f, -1.f), magenta });

  for (int i = 0; i < vertices.size(); i++) {
    vertices[i].position.z += 4.f;
  }

  pointCloud.vertices = vertices;
}