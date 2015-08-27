#include "Common\GlWindow.h"
#include "Common\KinectCamera.h"
#include "Common\HuffmanCompressor.h"

GlWindow *glWindow;

int main(int argc, char *argv[])
{
  glWindow = new GlWindow(argc, argv);
  glWindow->show();

 // GlWindow glWindow(argc, argv);
 

 // glWindow.show();

 

  /*Core::Timer timer;
  KinectCamera kinectCamera;
  HuffmanCompressor huffmanCompressor;
  while (true) {
    kinectCamera.update();
    UINT16 *depth = kinectCamera.getDepthBuffer();
    INT16 *diffData = kinectCamera.getDepthDifferential();

    //std::cout << thing[0] << std::endl;
    if (depth[0] == 0) {
      timer.startTimer();
      huffmanCompressor.compress(512 * 424, diffData);
      //huffmanCompressor.decompress();
      timer.stopTimer();
      std::cout << timer.getElapsedTime() / 1000 << " ms" << std::endl;
      //std::cout << std::endl;
    }
  }*/


  return 0;
}
