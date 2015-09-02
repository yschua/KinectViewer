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
      x.push_back(makeVector(wx, wy, wz, 1.f));
      xp.push_back(makeVector(wx, wy, wz, 1.f));
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
      y.push_back(makeVector(wx, wy, wz, 1.f));
    }
  }
}

void ICP::computeTransformation()
{
  iterations = 0;
  float errorSum = 0.f;
  float meanSquareError = 1.f;
  float eps = 1e-6f;

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
      findNearest(xp[i], error, distance2);
      errorSum += distance2;
      createJacobian(xp[i], jacobian);

      // WLS estimation
      for (int j = 0; j < 3; j++)
        w.add_mJ(error[j], jacobian[j]);
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

void ICP::getDepthEstimate(UINT16 *depthEstimate)
{
  int i = 0;
  for (int iy = 0; iy < DEPTH_HEIGHT; iy++) {
    for (int ix = 0; ix < DEPTH_WIDTH; ix++) {
      float wx = x[i][0];
      float wy = x[i][1];
      float wz = x[i][2];

      glm::vec3 world = glm::vec3(wx, wy, wz);
      glm::vec3 image = world * cameraParams.depthIntrinsic;

      depthEstimate[i] = (wz == 0) ? 0 : (int)(image.x / ix * 1000 + 0.5f);
      i++;
    }
  }
  std::cout << std::endl;
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

void ICP::findNearest(const Vector<4> &xp, Vector<4> &error, float &distance2)
{
  distance2 = 1e9;
  for (int i = 0; i < y.size(); i++) {
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
