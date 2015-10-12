#pragma once
#include "Core\ShaderLoader.h"
#include "Core\Timer.h"
#include "Common\GlCamera.h"
#include "Common\ModelGenerator.h"
#include "Common\CubeGenerator.h"
#include "Common\KinectCamera.h"
#include "Common\HuffmanCompressor.h"
#include "Common\StdHuffmanCompressor.h"
#include "Common\ICP.h"
#include "Primitives\Vertex.h"
#include "Primitives\DataSize.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <gl\glew.h>
#include <gl\freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>

#define NOMINMAX
#define limitDepth(depth) std::min(int(depth), int(MAX_DEPTH))

class GlWindow {
public:
  GlWindow(int argc, char **argv);
  ~GlWindow();
  void show(void);
protected:
  static void dataToDiff(const UINT16 *data, INT16 *diff);
  static void dataToDiff(const BYTE *data, INT16 *diff);
  static void dataToDiff(const UINT16 *depth, const BYTE *color, INT16 *diff);

  static void diffToData(const INT16 *diff, UINT16 *data);
  static void diffToData(const INT16 *diff, BYTE *data);
  static void diffToData(const INT16 *diff, UINT16 *depth, BYTE *color);

  static void huffman1(const UINT16 *depthSend, const BYTE *colorSend,
                       UINT16 *depthReceive, BYTE *colorReceive, bool stdMode);
  static void huffman2(const UINT16 *depthSend, const BYTE *colorSend,
                       UINT16 *depthReceive, BYTE *colorReceive, bool stdMode);
  static void frameDiff(const UINT16 *depthSend, const BYTE *colorSend,
                       UINT16 *depthReceive, BYTE *colorReceive, bool stdMode);
  static void frameDiffICP(const UINT16 *depthSend, const BYTE *colorSend,
                       UINT16 *depthReceive, BYTE *colorReceive);
  static void cubeDemo(const UINT16 *depth);

  static void shaderRender();
  static void drawText(std::string text, float offset);
  static void displayTransform(const SE3<> &transform, float offset);
  static void recordData(std::string, float);

  static void timerCallback(int value);
  static void renderCallback();
  static void mouseWheelFuncCallback(int, int, int, int);
  static void mouseFuncCallback(int button, int state, int x, int y);
  static void mouseMotionCallback(int x, int y);
  static void keyboardFuncCallback(unsigned char key, int xMouse, int yMouse);
  static void closeCallback();
private:
  // ICP test functions
  static void loadPointCloud();
};

