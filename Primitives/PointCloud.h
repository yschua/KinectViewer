#pragma once
#include "Vertex.h"
#include <vector>
#include <gl\glew.h>

struct PointCloud {
  std::vector<Vertex> vertices;
  GLuint numVertices;
  GLsizeiptr bufferSize() const
  {
    return numVertices * sizeof(vertices[0]);
  }
};