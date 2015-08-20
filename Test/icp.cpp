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

double dist(Vector<4> p, Vector<4> q)
{
  return sqrt((p - q) * (p - q));
}

int main()
{
  // Initial set of points "x"
  int numPts = 7;
  vector<Vector<4> > x(numPts);
  vector<Vector<4> > y(numPts);
  vector<Vector<4> > xp(numPts);
  //x[0] = xp[0] = makeVector(2, 2, 0, 1);
  //x[1] = xp[1] = makeVector(3, 1, 1, 1);
  //x[2] = xp[2] = makeVector(4, 3, 2, 1);
  //x[3] = xp[3] = makeVector(4, 4, 3, 1);
  //x[4] = xp[4] = makeVector(2, 3, 1, 1);
  x[0] = xp[0] = makeVector(-3, 2, 0.1, 1);
  x[1] = xp[1] = makeVector(-2, 1, 0.2, 1);
  x[2] = xp[2] = makeVector(-1, -2, 0.3, 1);
  x[3] = xp[3] = makeVector(1, -2, 0.4, 1);
  x[4] = xp[4] = makeVector(2, -1, 0.5, 1);
  x[5] = xp[5] = makeVector(3, 1, 0.6, 1);
  x[6] = xp[6] = makeVector(4, 2, 0.7, 1);

  SE3<> Ti(makeVector(1.2, -2.2, 0.66, 0.553, 0.3575, 0.933));
  //Ti = makeVector(1, 2, 3, 0.6, 0.2, 0.1);
  cout << "Initial T:" << endl << Ti << endl;
  cout << "Destination points:" << endl;
  for (int i = 0; i < x.size(); i++) {
    cout << (y[i] = Ti * x[i]) << endl;
  }
  cout << endl;
  
  // ICP
  int iter = 8;
  SE3<> T;
  while (iter--) {
    cout << "*************************************" << endl; 
    WLS<6> w;
    for (int i = 0; i < xp.size(); i++) {
      double minDistance = 1e9;
      Vector<4> e;
      Matrix<3, 6> J = Zeros;
      // Find nearest point
      int corresPt;
      for (int j = 0; j < y.size(); j++) {
        double distance = dist(xp[i], y[j]);
        if (distance < minDistance) {
          minDistance = distance;
          e = y[j] - xp[i];
          corresPt = j;
        }
      }
      // Found e for nearest point
      cout << "Correspondence: " << i << "->" << corresPt << " e: " << minDistance << endl;

      // Compute Jacobian
      J(0, 0) = 1;
      J(0, 4) = xp[i][2]; // z
      J(0, 5) = -xp[i][1]; // -y

      J(1, 1) = 1;
      J(1, 3) = -xp[i][2]; // -z
      J(1, 5) = xp[i][0]; // x

      J(2, 2) = 1;
      J(2, 3) = xp[i][1]; // y
      J(2, 4) = -xp[i][0]; // -x

      w.add_mJ(e[0], J[0]);
      w.add_mJ(e[1], J[1]);
      w.add_mJ(e[2], J[2]);
    }
    cout << endl;

    w.compute();
    SE3<> update = SE3<>::exp(w.get_mu());
    T = update * T;
    cout << "T:" << endl << T << endl;
    //cout << "T:" << endl << T.get_rotation() << endl << T.get_translation() << endl;
    cout << "T * x:" << endl;
    for (int i = 0; i < x.size(); i++) {
      cout << (xp[i] = T * x[i]) << endl;
    }
    cout << endl;
    w.clear();
  }

  cout << "==============================" << endl;
  cout << "Actual transformation:" << endl;
  cout << SE3<>::ln(Ti) << endl;
  cout << endl;
  cout << "Estimated transformation:" << endl;
  cout << SE3<>::ln(T) << endl;
  cout << endl;

  system("pause");
  return 0;
}