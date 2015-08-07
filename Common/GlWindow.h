#pragma once
#include "Core\ShaderLoader.h"
#include "Core\Timer.h"
#include "Common\Camera.h"
#include "Common\ModelGenerator.h"
#include "Common\KinectCamera.h"
#include "Common\HuffmanCompressor.h"
#include "Common\StdHuffmanCompressor.h"
#include "Primitives\Vertex.h"
#include <iostream>
#include <fstream>
#include <gl\glew.h>
#include <gl\freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>

class GlWindow {
public:
  GlWindow(int argc, char **argv);
  ~GlWindow();
  void show(void);
protected:
  static void shaderRender();
  static void drawText(std::string text, float offset);
  static void renderCallback();
  static void mouseFuncCallback(int button, int state, int x, int y);
  static void mouseMotionCallback(int x, int y);
  static void keyboardFuncCallback(unsigned char key, int xMouse, int yMouse);
  static void closeCallback();
};

