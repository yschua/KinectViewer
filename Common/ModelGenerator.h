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
  void updateModel(const RGBQUAD *colorBuffer);
  void updateDepthFrame(const UINT16 *depthBuffer);
  void updatePointCloud(KinectCamera &kinectCamera);
protected: // ModelGenerator.cpp is unable to access these functions when protected, why?
  void createModel();
  void loadModel();
};
