#pragma once
#include "Primitives\PointCloud.h"
#include "Primitives\Vertex.h"
#include "Primitives\CameraParameters.h"
#include "Core\Timer.h"
#include "Common\KinectCamera.h"
#include <vector>
#include <gl\glew.h>
#include <glm\glm.hpp>

class ModelGenerator {
  PointCloud pointCloud;
  CameraParameters cameraParameters;
  Core::Timer timer;
  const int WIDTH;
  const int HEIGHT;
public:
  ModelGenerator(int width, int height);
  ~ModelGenerator();
  int getNumVertices();
  PointCloud &getPointCloud();
  void updatePointCloud(PointCloud pointCloud);
  void updatePointCloud(UINT16 *depthBuffer, BYTE *colorBuffer);
  void loadModel();
};
