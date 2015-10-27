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
  const int WIDTH;
  const int HEIGHT;
protected:
  PointCloud pointCloud;
  CameraParameters cameraParameters;
  Core::Timer timer;
public:
  ModelGenerator();
  ModelGenerator(int width, int height);
  ~ModelGenerator();
  int getNumVertices();
  PointCloud &getPointCloud();
  void updatePointCloud(PointCloud pointCloud);
  void updatePointCloud(const UINT16 *depthBuffer, const BYTE *colorBuffer);
  void updatePointCloud(const UINT16 *depthBuffer, const BYTE *colorBuffer, const glm::vec3 color);
  void loadModel();
};
