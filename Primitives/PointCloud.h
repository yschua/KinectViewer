#pragma once
#include "Vertex.h"
#include <vector>
#include <gl\glew.h>

struct PointCloud {
  std::vector<Vertex> vertices;
  GLuint numVertices; // don't use this, use vertices.size()
  GLsizeiptr bufferSize() const
  {
    return vertices.size() * sizeof(vertices[0]);
  }
};