#pragma once
#include <Primitives\DataSize.h>
#include <Primitives\PointCloud.h>
#include <Primitives\CameraParameters.h>
#include <fstream>
#include <vector>
#include <TooN\TooN.h>
#include <TooN\se3.h>
#include <TooN\wls.h>
#include <Kinect.h>

using namespace TooN;

typedef std::vector<Vector<4> > Points;

class ICP {
  int iterations;
  Points x;
  Points y;
  Points xp;
  SE3<> transformation;
  CameraParameters cameraParams;
public:
  ICP();
  ~ICP();
  void clear();
  void loadPointsX(PointCloud &pointCloud);
  void loadPointsY(PointCloud &pointCloud);
  void computeTransformation();
  SE3<> getTransformation();
  void getDepthEstimate(UINT16 *depthEstimate);
protected:
  Points loadPoints(std::string filename);
  double computeDistance2(Vector<4> p, Vector<4> q);
  void findNearest(const Vector<4> &xp, Vector<4> &error, float &distance2);
  void createJacobian(const Vector<4> &xp, Matrix<3, 6> &jacobian);
  
};

