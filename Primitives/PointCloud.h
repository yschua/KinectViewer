#pragma once
#include "Vertex.h"
#include <vector>
#include <gl\glew.h>

struct PointCloud {
  std::vector<Vertex> vertices;
  GLuint numVertices; // don't use this, use vertices.size()
  GLsizeiptr bufferSize() const
  {
    return numVertices * sizeof(vertices[0]);
  }
};