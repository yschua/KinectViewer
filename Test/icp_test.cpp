#include <iostream>
#include <glm\common.hpp>
#include <glm\glm.hpp>
#include <vector>
#include <cmath>
#include <tuple>
#include <algorithm>
#include <stdint.h>
#include <TooN\TooN.h>
#include <TooN\se3.h>
#include <TooN\wls.h>

using namespace std;
using namespace TooN;

int numPts = 7;
vector<Vector<4> > x(numPts);
vector<Vector<4> > y(numPts);
vector<Vector<4> > xp(numPts);

double dist2(Vector<4> p, Vector<4> q)
{
  return (p - q) * (p - q);
}

void findNearest(const Vector<4> &xp, double &distance, Vector<4> &e, int &corresPt)
{
  double minDistance2 = 1e9;
  for (int j = 0; j < y.size(); j++) {
    double distance2 = dist2(xp, y[j]);
    if (distance2 < minDistance2) {
      minDistance2 = distance2;
      e = y[j] - xp;
      corresPt = j;
    }
  }
  distance = minDistance2;
}

void getJacobian(Matrix<3, 6>& J, const Vector<4> &xp)
{
  J(0, 0) = 1;
  J(0, 4) = xp[2]; // z
  J(0, 5) = -xp[1]; // -y

  J(1, 1) = 1;
  J(1, 3) = -xp[2]; // -z
  J(1, 5) = xp[0]; // x

  J(2, 2) = 1;
  J(2, 3) = xp[1]; // y
  J(2, 4) = -xp[0]; // -x
}

int main()
{
  // Initial set of points "x"
  x[0] = xp[0] = makeVector(-3, 2, 0.1, 1);
  x[1] = xp[1] = makeVector(-2, 1, 0.2, 1);
  x[2] = xp[2] = makeVector(-1, -2, 0.3, 1);
  x[3] = xp[3] = makeVector(1, -2, 0.4, 1);
  x[4] = xp[4] = makeVector(2, -1, 0.5, 1);
  x[5] = xp[5] = makeVector(3, 1, 0.6, 1);
  x[6] = xp[6] = makeVector(4, 2, 0.7, 1);

  // Actual transformation
  SE3<> Ti(makeVector(1.2, -2.2, 0.66, 0.553, 0.3575, 0.933));
  cout << "Initial T:" << endl << Ti << endl;
  cout << "Destination points:" << endl;
  for (int i = 0; i < x.size(); i++) {
    cout << (y[i] = Ti * x[i]) << endl;
  }
  cout << endl;
  
  // ICP
  int iter = 0;
  float errorSum = 0.0f, meanSqErr = 1.0f, eps = 0.001f;
  SE3<> T;
  while (meanSqErr > eps) {
    iter++;
    errorSum = 0.0f;
    cout << "*************************************" << endl; 
    WLS<6> w;
    for (int i = 0; i < xp.size(); i++) {
      double minDistance2 = 1e9;
      Vector<4> e;
      Matrix<3, 6> J = Zeros;

      // Find nearest point
      int corresPt;
      //for (int j = 0; j < y.size(); j++) {
      //  double distance2 = dist2(xp[i], y[j]);
      //  if (distance2 < minDistance2) {
      //    minDistance2 = distance2;
      //    e = y[j] - xp[i];
      //    corresPt = j;
      //  }
      //}
      findNearest(xp[i], minDistance2, e, corresPt);

      // Found e for nearest point
      cout << "Correspondence: " << i << "->" << corresPt << " e: " << minDistance2 << endl;
      errorSum += minDistance2;

      // Compute Jacobian
      //J(0, 0) = 1;
      //J(0, 4) = xp[i][2]; // z
      //J(0, 5) = -xp[i][1]; // -y

      //J(1, 1) = 1;
      //J(1, 3) = -xp[i][2]; // -z
      //J(1, 5) = xp[i][0]; // x

      //J(2, 2) = 1;
      //J(2, 3) = xp[i][1]; // y
      //J(2, 4) = -xp[i][0]; // -x

      getJacobian(J, xp[i]);

      // WLS estimation
      w.add_mJ(e[0], J[0]);
      w.add_mJ(e[1], J[1]);
      w.add_mJ(e[2], J[2]);
    }
    cout << endl;

    meanSqErr = errorSum / numPts;

    if (meanSqErr > eps) {
      w.compute();
      SE3<> update = SE3<>::exp(w.get_mu());
      T = update * T;
      cout << "T:" << endl << T << endl;
      cout << "T * x:" << endl;
      for (int i = 0; i < x.size(); i++) {
        cout << (xp[i] = T * x[i]) << endl;
      }
      cout << endl;
      w.clear();
    }
  }

  cout << "==============================" << endl;
  cout << "Actual transformation:" << endl;
  cout << SE3<>::ln(Ti) << endl;
  cout << endl;
  cout << "Estimated transformation:" << endl;
  cout << SE3<>::ln(T) << endl;
  cout << endl;
  cout << "Iterations: " << iter << endl;

  system("pause");
  return 0;
}