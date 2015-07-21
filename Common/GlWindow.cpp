#include "GlWindow.h"

// Class members
// Declare as global variables due to GLUT limitation
GLuint program;
Camera camera;
KinectCamera kinectCamera;
HuffmanCompressor huffmanCompressor;
ModelGenerator model(kinectCamera.DEPTH_WIDTH, kinectCamera.DEPTH_HEIGHT);
Core::Timer timer;
const int DEPTH_WIDTH = kinectCamera.DEPTH_WIDTH;
const int DEPTH_HEIGHT = kinectCamera.DEPTH_HEIGHT;
const int DEPTH_SIZE = 512 * 424;
const int COLOR_SIZE = 512 * 424 * 3;

int compressionMode = 1;

GlWindow::GlWindow(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitWindowSize(800, 600);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("KinectViewer");
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_POINT_SMOOTH);
  //glPointSize(1.0f);
  
  model.loadModel();

  Core::ShaderLoader shaderLoader;
  program = shaderLoader.createProgram("Shaders\\VertexShader.glsl",
                                       "Shaders\\FragmentShader.glsl");
}

GlWindow::~GlWindow()
{
  glUseProgram(0);
  glDeleteProgram(program);
}

void GlWindow::show()
{
  glutIdleFunc(renderCallback);
  //glutDisplayFunc(renderCallback);
  glutMouseFunc(mouseFuncCallback);
  glutMotionFunc(mouseMotionCallback);
  glutKeyboardFunc(keyboardFuncCallback);
  glutCloseFunc(closeCallback);
  glutMainLoop();
}

void GlWindow::renderCallback()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0, 0, 0, 1);

  timer.startTimer();
  // Sender computer obtains frame data
  kinectCamera.update();

  // Data processing and simulated transmission
  static UINT16 depth[DEPTH_SIZE];
  static UINT16 colorInt[COLOR_SIZE];
  static BYTE color[COLOR_SIZE];
  static UINT16 combinedData[DEPTH_SIZE + COLOR_SIZE];
  UINT16 *depthBuffer = kinectCamera.getDepthBuffer();
  BYTE *colorBuffer = kinectCamera.getColorBufferReduced();
  INT16 *depthDifferential = kinectCamera.getDepthDifferential();
  INT16 *colorDifferential = kinectCamera.getColorDifferential();
  INT16 *combinedDifferential = kinectCamera.getCombinedDifferential();

  if (compressionMode == 1) {
    for (int i = 0; i < DEPTH_SIZE; i++)
      depth[i] = depthBuffer[i];
    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = colorBuffer[i];
  } else if (compressionMode == 2) {
    int depthTransmitSize, colorTransmitSize;
    huffmanCompressor.compress(DEPTH_SIZE, depthDifferential);
    Bitset transmitData = huffmanCompressor.getTransmitData();
    depthTransmitSize = transmitData.size();
    //std::cout << "Depth size: " << depthTransmitSize << "\tCompresssion ratio: " << DEPTH_SIZE * 16 / (float)transmitData.size() << std::endl;
    huffmanCompressor.decompress(DEPTH_SIZE, transmitData, depth);
    
    huffmanCompressor.compress(COLOR_SIZE, colorDifferential);
    transmitData = huffmanCompressor.getTransmitData();
    colorTransmitSize = transmitData.size();
    //std::cout << "Color size: " << colorTransmitSize << "\tCompresssion ratio: " << COLOR_SIZE * 8 / (float)transmitData.size() << std::endl;
    huffmanCompressor.decompress(COLOR_SIZE, transmitData, colorInt);

    std::cout << "Total compression: " << DEPTH_SIZE * 32 / (float)(depthTransmitSize + colorTransmitSize) << std::endl;
    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = (BYTE)colorInt[i];
  } else if (compressionMode == 3) {
    

    huffmanCompressor.compress(DEPTH_SIZE + COLOR_SIZE, combinedDifferential);
    Bitset transmitData = huffmanCompressor.getTransmitData();
    std::cout << "Total compression: " << DEPTH_SIZE * 32 / (float)transmitData.size() << std::endl;
    huffmanCompressor.decompress(DEPTH_SIZE + COLOR_SIZE, transmitData, combinedData);

    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = (BYTE)combinedData[i];
    for (int i = 0; i < DEPTH_SIZE; i++)
      depth[i] = combinedData[i + COLOR_SIZE];
  }

  // Receiver computer renders received frame data
  model.updatePointCloud(depth, color);

  timer.stopTimer();
  std::cout << "Frame time: " << timer.getElapsedTime() / 1000 << std::endl;
  
  glm::mat4 modelMatrix = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
  //modelMatrix = glm::rotate(modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
  //modelMatrix = glm::translate(modelMatrix, glm::vec3(-256.0f, -212.0f, -1000.0f));
  glm::mat4 viewMatrix = camera.getViewMatrix();
  glm::mat4 projectionMatrix = glm::perspective(45.0f, 64.0f / 53.0f, 0.000001f, 10000.0f);
  glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
  
  glUseProgram(program);
  GLuint mvpMatrix = glGetUniformLocation(program, "mvpMatrix");
  glUniformMatrix4fv(mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
  glDrawArrays(GL_POINTS, 0, model.getNumVertices());

  glutSwapBuffers();
}

void GlWindow::mouseFuncCallback(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    camera.setMousePosition(x, y);
  }
}

void GlWindow::mouseMotionCallback(int x, int y)
{
  camera.mouseUpdate(glm::vec2(x, y));
  renderCallback();
}

void GlWindow::keyboardFuncCallback(unsigned char key, int xMouse, int yMouse)
{
  switch (key) {
    case ' ': {
      camera.resetView();
      break;
    }
    case 'w': {
      camera.moveCameraPosition(0, 0, 1);
      break;
    }
    case 'a': {
      camera.moveCameraPosition(1, 0, 0);
      break;
    }
    case 's': {
      camera.moveCameraPosition(0, 0, -1);
      break;
    }
    case 'd': {
      camera.moveCameraPosition(-1, 0, 0);
      break;
    }
    case '1': {
      compressionMode = 1;
      break;
    }
    case '2': {
      compressionMode = 2;
      break;
    }
    case '3': {
      compressionMode = 3;
      break;
    }
  }
}

void GlWindow::closeCallback()
{
  std::cout << "GLUT: Finished" << std::endl;
  glutLeaveMainLoop();
}