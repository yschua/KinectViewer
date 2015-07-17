#pragma once
#include "Primitives\PointCloud.h"
#include "Primitives\Vertex.h"
#include "Core\Timer.h"
#include "Common\KinectCamera.h"
#include <vector>
#include <gl\glew.h>
#include <glm\glm.hpp>

class ModelGenerator {
  PointCloud pointCloud;
  GLushort *indices;
  int numIndices;
  Core::Timer timer;
public:
  ModelGenerator();
  ~ModelGenerator();
  void generate();
  int getNumVertices();
  PointCloud getModel();
  void updatePointCloud(KinectCamera &kinectCamera);
protected:
  void loadModel();
};
