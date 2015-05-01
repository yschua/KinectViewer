#include "Common\GlWindow.h"
#include "Common\KinectCamera.h"

int main(int argc, char *argv[])
{
  GlWindow glWindow(argc, argv);
  glWindow.show();

  //int test = 0;
  //KinectCamera kinectCamera;
  //while (true) {
  //  kinectCamera.update();
  //  UINT16 *thing = kinectCamera.getDepthBuffer();

  //  std::cout << thing[0] << std::endl;
  //  if (thing[0] == 0) {
  //    std::cout << "break here" << std::endl;
  //  }
  //}


  return 0;
}
