#include "Common\GlWindow.h"
#include "Common\KinectCamera.h"
#include "Common\HuffmanCompressor.h"

int main(int argc, char *argv[])
{
  //GlWindow glWindow(argc, argv);
  //glWindow.show();

  Core::Timer timer;
  int test = 0;
  KinectCamera kinectCamera;
  HuffmanCompressor huffmanCompressor;
  while (true) {
    kinectCamera.update();
    UINT16 *thing = kinectCamera.getDepthBuffer();
    INT16 *diffData = kinectCamera.getDepthDifferential();

    //std::cout << thing[0] << std::endl;
    if (thing[0] == 0) {
      //timer.startTimer();
      huffmanCompressor.compress(512 * 424, diffData);
      huffmanCompressor.decompress();
      //timer.stopTimer();
      //std::cout << timer.getElapsedTime() / 1000 << std::endl;
      std::cout << "break here" << std::endl;
    }
  }


  return 0;
}
