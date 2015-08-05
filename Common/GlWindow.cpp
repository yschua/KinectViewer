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
const int FRAME_SIZE = DEPTH_SIZE + COLOR_SIZE;
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
  glutMouseFunc(mouseFuncCallback);
  glutMotionFunc(mouseMotionCallback);
  glutKeyboardFunc(keyboardFuncCallback);
  glutCloseFunc(closeCallback);
  glutMainLoop();
}

void GlWindow::drawText(std::string text, float offset)
{
  glUseProgram(0);
  glLoadIdentity();
  glRasterPos2f(-0.9f, 0.9f - offset);
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
  for (int i = 0; i < text.length(); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void GlWindow::renderCallback()
{
  timer.startTimer();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0, 0, 0, 1);
  
  // Sender computer obtains frame data
  kinectCamera.update();

  // Data processing and simulated transmission
  static UINT16 depth[DEPTH_SIZE];
  static UINT16 colorInt[COLOR_SIZE];
  static BYTE color[COLOR_SIZE];
  static UINT16 dataReceived[FRAME_SIZE];
  static INT16 currentFrameSender[FRAME_SIZE];
  static INT16 currentFrameReceiver[FRAME_SIZE];
  static INT16 differenceFrame[FRAME_SIZE];
  UINT16 *depthBuffer = kinectCamera.getDepthBuffer();
  BYTE *colorBuffer = kinectCamera.getColorBufferReduced();
  INT16 *depthDifferential = kinectCamera.getDepthDifferential();
  INT16 *colorDifferential = kinectCamera.getColorDifferential();
  INT16 *combinedDifferential = kinectCamera.getCombinedDifferential();
  const int UNCOMPRESSED_SIZE = DEPTH_SIZE * 16 + COLOR_SIZE * 8;
  
  if (compressionMode == 1) {
    /******************** NO COMPRESSION **************************/
    drawText("1. No compression", 0.0f);
    for (int i = 0; i < DEPTH_SIZE; i++)
      depth[i] = depthBuffer[i];
    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = colorBuffer[i];

  } else if (compressionMode == 2) {
    /******************** HUFFMAN DIFFERENTIAL 1 **************************/
    drawText("2. Huffman (2 trees for each image)", 0.0f);

    int depthTransmitSize, colorTransmitSize;
    // Compress depth image
    huffmanCompressor.compress(DEPTH_SIZE, depthDifferential);
    Bitset transmitData = huffmanCompressor.getTransmitData();
    depthTransmitSize = transmitData.size();
    huffmanCompressor.decompress(DEPTH_SIZE, transmitData, depth);
    
    // Compress color image
    huffmanCompressor.compress(COLOR_SIZE, colorDifferential);
    transmitData = huffmanCompressor.getTransmitData();
    colorTransmitSize = transmitData.size();

    // Print result
    float compression_ratio = UNCOMPRESSED_SIZE / (float)(depthTransmitSize + colorTransmitSize);
    drawText("Compress ratio: " + std::to_string(compression_ratio), 0.1f);

    // Decompress data
    huffmanCompressor.decompress(COLOR_SIZE, transmitData, colorInt);

    // Reconstruct values
    for (int i = 1; i < DEPTH_SIZE; i++)
      depth[i] = depth[i] + depth[i - 1];
    for (int i = 1; i < COLOR_SIZE; i++)
      colorInt[i] = colorInt[i] + colorInt[i - 1];
    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = (BYTE)colorInt[i];

  } else if (compressionMode == 3) {
    /******************** HUFFMAN DIFFERENTIAL 2 **************************/
    drawText("3. Huffman (1 tree for both images)", 0.0f);

    // Compress both images
    huffmanCompressor.compress(FRAME_SIZE, combinedDifferential);
    Bitset transmitData = huffmanCompressor.getTransmitData();

    // Print result
    float compression_ratio = UNCOMPRESSED_SIZE / (float)transmitData.size();
    drawText("Compress ratio: " + std::to_string(compression_ratio), 0.1f);

    // Decompress data
    huffmanCompressor.decompress(FRAME_SIZE, transmitData, dataReceived);

    // Reconstruct values
    for (int i = 1; i < FRAME_SIZE; i++)
      dataReceived[i] = dataReceived[i] + dataReceived[i - 1];
    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = (BYTE)dataReceived[i];
    for (int i = 0; i < DEPTH_SIZE; i++)
      depth[i] = dataReceived[i + COLOR_SIZE];

  } else if (compressionMode == 4) {
    /******************** HUFFMAN FRAME DIFFERENTIAL  **************************/
    drawText("4. Huffman (difference between frames)", 0.0f);

    // Initialize reference frame
    static bool once = true;
    if (once) {
      memset(currentFrameSender, 0, sizeof(INT16) * FRAME_SIZE);
      memset(currentFrameReceiver, 0, sizeof(INT16) * FRAME_SIZE);
      once = false;
    }

    // Compute frame difference
    INT16 *nextFrame = kinectCamera.getCombinedFrame();
    for (int i = 0; i < FRAME_SIZE; i++) {
      differenceFrame[i] = nextFrame[i] - currentFrameSender[i];
      currentFrameSender[i] = nextFrame[i];
    }
    
    // Compress
    huffmanCompressor.compress(FRAME_SIZE, differenceFrame);
    Bitset transmitData = huffmanCompressor.getTransmitData();

    // Results
    float compression_ratio = UNCOMPRESSED_SIZE / (float)transmitData.size();
    drawText("Compress ratio: " + std::to_string(compression_ratio), 0.1f);

    // Decompress
    huffmanCompressor.decompress(FRAME_SIZE, transmitData, dataReceived);

    // Reconstruct values
    for (int i = 0; i < FRAME_SIZE; i++)
      currentFrameReceiver[i] += dataReceived[i];
    for (int i = 0; i < DEPTH_SIZE; i++)
      depth[i] = currentFrameReceiver[i];
    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = currentFrameReceiver[i + DEPTH_SIZE];
  
  } else if (compressionMode == 5) {
    /**************** HUFFMAN FRAME DIFFERENTIAL (COLOR ONLY)  *********************/
    drawText("5. Huffman (difference between frames, color only)", 0.0f);

    // Initialize reference frame
    static bool once = true;
    if (once) {
      memset(currentFrameSender, 0, sizeof(INT16) * COLOR_SIZE);
      memset(currentFrameReceiver, 0, sizeof(INT16) * COLOR_SIZE);
      once = false;
    }

    // Compute frame difference
    for (int i = 0; i < COLOR_SIZE; i++) {
      differenceFrame[i] = colorBuffer[i] - currentFrameSender[i];
      currentFrameSender[i] = colorBuffer[i];
    }

    // Compress
    huffmanCompressor.compress(COLOR_SIZE, differenceFrame);
    Bitset transmitData = huffmanCompressor.getTransmitData();

    // Results
    float compression_ratio = COLOR_SIZE * 8 / (float)transmitData.size();
    drawText("Compress ratio (color only): " + std::to_string(compression_ratio), 0.1f);

    // Decompress
    huffmanCompressor.decompress(FRAME_SIZE, transmitData, dataReceived);

    // Reconstruct
    for (int i = 0; i < COLOR_SIZE; i++)
      currentFrameReceiver[i] += dataReceived[i];
    for (int i = 0; i < DEPTH_SIZE; i++)
      depth[i] = 10;
    for (int i = 0; i < COLOR_SIZE; i++)
      color[i] = currentFrameReceiver[i];
  }

  // Receiver computer renders received frame data
  model.updatePointCloud(depth, color);
  shaderRender();

  timer.stopTimer();
  drawText("FPS: " + std::to_string(timer.getFPS()), 1.8f);
  glutSwapBuffers();
}

void GlWindow::shaderRender()
{
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
    case '4': {
      compressionMode = 4;
      break;
    }
    case '5': {
      compressionMode = 5;
      break;
    }
    case 27:
      closeCallback();
      break;
  }
}

void GlWindow::closeCallback()
{
  std::cout << "GLUT: Finished" << std::endl;
  glutLeaveMainLoop();
}