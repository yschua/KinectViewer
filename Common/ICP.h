#pragma once
#include <Primitives\DataSize.h>
#include <fstream>
#include <vector>
#include <TooN\TooN.h>
#include <TooN\se3.h>
#include <TooN\wls.h>

using namespace TooN;

typedef std::vector<Vector<4> > Points;

class ICP {
  int iterations;
  Points x;
  Points y;
  Points xp;
  SE3<> transformation;
public:
  ICP();
  ~ICP();
  void computeTransformation();
  SE3<> getTransformation();
protected:
  Points loadPoints(std::string filename);
  double computeDistance2(Vector<4> p, Vector<4> q);
  void findNearest(const Vector<4> &xp, Vector<4> &error, float &distance2);
  void createJacobian(const Vector<4> &xp, Matrix<3, 6> &jacobian);
  
};

