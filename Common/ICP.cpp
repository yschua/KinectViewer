#include "ICP.h"

ICP::ICP()
{
  //x = loadPoints("point-cloud1.txt");
  //xp = loadPoints("point-cloud1.txt");
  //y = loadPoints("point-cloud2.txt");
  //std::cout << "ICP points loaded" << std::endl;

  //x = loadPoints("test-points1.txt");
  //xp = loadPoints("test-points1.txt");
  //y = loadPoints("test-points2.txt");
}

ICP::~ICP()
{
}

void ICP::clear()
{
  x.clear();
  xp.clear();
  y.clear();
}

void ICP::loadPointsX(PointCloud &pointCloud)
{
  for (int i = 0; i < pointCloud.numVertices; i++) {
    if (i % 200 == 0) {
      float wx = pointCloud.vertices[i].position.x;
      float wy = pointCloud.vertices[i].position.y;
      float wz = pointCloud.vertices[i].position.z;
      if (wz != 0.f) {
        x.push_back(makeVector(wx, wy, wz, 1.f));
        xp.push_back(makeVector(wx, wy, wz, 1.f));
      }
    }
  }
}

void ICP::loadPointsY(PointCloud &pointCloud)
{
  for (int i = 0; i < pointCloud.numVertices; i++) {
    if (i % 200 == 0) {
      float wx = pointCloud.vertices[i].position.x;
      float wy = pointCloud.vertices[i].position.y;
      float wz = pointCloud.vertices[i].position.z;
      if (wz != 0.f) {
        y.push_back(makeVector(wx, wy, wz, 1.f));
      }
    }
  }
}

void ICP::computeTransformation()
{
  iterations = 0;
  float errorSum = 0.f;
  meanSquareError = 1.f;
  float eps = 0.000001f;
  transformation = SE3<>();

  while (meanSquareError > eps && iterations < 5) {
    ++iterations;
    errorSum = 0.f;
    WLS<6> w;

    // For each transformed point estimate
    for (int i = 0; i < xp.size(); i++) {
      float distance2;
      Vector<4> error;
      Matrix<3, 6> jacobian = Zeros;

      // Find nearest neighbour
      findNearest(xp[i], i, error, distance2);
      errorSum += distance2;
      createJacobian(xp[i], jacobian);

      // WLS estimation
      float c = 0.002;
      float weight = c / (c + error * error);
      for (int j = 0; j < 3; j++)
        w.add_mJ(error[j], jacobian[j], weight);
    }

    if ((meanSquareError = errorSum / xp.size()) > eps) {
      // Compute new transformation estimate
      w.compute();
      SE3<> update = SE3<>::exp(w.get_mu());
      transformation = update * transformation;

      // Apply transformation to original points
      for (int i = 0; i < x.size(); i++)
        xp[i] = transformation * x[i];

      w.clear();
    }
    //std::cout << "MSE: " << meanSquareError << std::endl;
  }
  //std::cout << "Iterations: " << iterations << std::endl;
  //std::cout << "Transformation: " << std::endl;
  //std::cout << transformation << std::endl;
}

SE3<> ICP::getTransformation()
{
  return transformation;
}

int ICP::getIterations()
{
  return iterations;
}

float ICP::getError()
{
  return meanSquareError;
}

Points ICP::loadPoints(std::string filename)
{
  Points points;
  std::ifstream file;
  file.open(filename, std::ios::in);
  float x, y, z;
  //for (int i = 0; file >> x >> y >> z; i++) {
  //  //if (i % 1000 == 0 && x && y && z) {
  //    points.push_back(makeVector(x, y, z, 1.f));
  //  //}
  //}

  int idx = 0;
  for (int row = 0; row < DEPTH_HEIGHT; row++) {
    for (int col = 0; col < DEPTH_WIDTH; col++) {
      file >> x >> y >> z;
      //if (row >= ICP_ROW_START && row <= ICP_ROW_END && col >= ICP_COL_START && col <= ICP_ROW_END)
      if (idx++ % 200 == 0)
        points.push_back(makeVector(x, y, z, 1.f));
    }
  }

  return points;
}

double ICP::computeDistance2(Vector<4> p, Vector<4> q)
{
  return (p - q) * (p - q);
}

void ICP::findNearest(const Vector<4> &xp, int index, Vector<4> &error, float &distance2)
{
  distance2 = 1e9;
  for (int i = 0; i < y.size(); i++) {
  //for (int i = max(index - 10, 0); i < min(index + 10, y.size()); i++) {
    float currentDistance = computeDistance2(xp, y[i]);
    if (currentDistance < distance2) {
      distance2 = currentDistance;
      error = y[i] - xp;
    }
  }
}

void ICP::createJacobian(const Vector<4> &xp, Matrix<3, 6> &J)
{
  J(0, 0) = 1;
  J(0, 4) = xp[2];  // z
  J(0, 5) = -xp[1]; // -y
  J(1, 1) = 1;
  J(1, 3) = -xp[2]; // -z
  J(1, 5) = xp[0];  // x
  J(2, 2) = 1;
  J(2, 3) = xp[1];  // y
  J(2, 4) = -xp[0]; // -x
}
