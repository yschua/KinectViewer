#include "Common\GlWindow.h"
#include "Common\KinectCamera.h"
#include "Common\HuffmanCompressor.h"

GlWindow *glWindow;

//struct Thing {
//  int value;
//  std::string code;
//  int length;
//
//  Thing(int v, std::string c, int l) : value(v), code(c), length(l) {}
//};
//
//bool myFunc(Thing a, Thing b) {
//  if (a.length == b.length) {
//    return abs(a.value) < abs(b.value);
//  }
//  return a.length < b.length;
//}

int main(int argc, char *argv[])
{
  glWindow = new GlWindow(argc, argv);
  glWindow->show();

  //std::ifstream file("maptbl-combined.txt", std::ios::in);
  //int v;
  //std::string c;
  //std::vector<Thing> vec;
  //while (file >> v >> c) {
  //  vec.push_back(Thing(v, c, c.length()));
  //}
  //file.close();

  //std::sort(vec.begin(), vec.end(), myFunc);

  //std::ofstream ofile("maptbl-sorted.txt", std::ios::out);
  //for (int i = 0; i < vec.size(); i++) {
  //  ofile << vec[i].value << '\t' << vec[i].code << std::endl;
  //}
  //ofile.close();

  return 0;
}
