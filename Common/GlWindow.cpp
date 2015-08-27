#include "GlWindow.h"

// Class members
GLuint program;
Camera glCamera;
KinectCamera kinectCamera;
HuffmanCompressor huffmanCompressor;
StdHuffmanCompressor stdHuffmanCompressor;
ModelGenerator model(DEPTH_WIDTH, DEPTH_HEIGHT);
Core::Timer timer;
int compressionMode = 1;
bool stdToggle = false;

GlWindow::GlWindow(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitWindowSize(800, 600);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("KinectViewer");
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_POINT_SMOOTH);
  //glPointSize(1.f);
  
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

// Get differential for depth data
void GlWindow::dataToDiff(const UINT16 *data, INT16 *diff)
{
  diff[0] = limitDepth(data[0]);
  for (int i = 1; i < DEPTH_SIZE; i++)
    diff[i] = limitDepth(data[i]) - limitDepth(data[i - 1]);
}

// Get differential for color data
void GlWindow::dataToDiff(const BYTE *data, INT16 *diff)
{
  diff[0] = data[0];
  for (int i = 1; i < COLOR_SIZE; i++)
    diff[i] = data[i] - data[i - 1];
}

// Get differential for frame (depth + color)
void GlWindow::dataToDiff(const UINT16 *depth, const BYTE *color, INT16 *diff)
{
  int diffIdx = 0;
  diff[diffIdx++] = limitDepth(depth[0]);
  for (int i = 1; i < DEPTH_SIZE; i++)
    diff[diffIdx++] = limitDepth(depth[i]) - limitDepth(depth[i - 1]);
  diff[diffIdx++] = color[0] - limitDepth(depth[DEPTH_SIZE - 1]);
  for (int i = 1; i < COLOR_SIZE; i++)
    diff[diffIdx++] = color[i] - color[i - 1];
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
  timer.startTimer(); // FPS timer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0, 0, 0, 1);

  // Sender computer obtains frame data
  kinectCamera.update();

  // Data processing and simulated transmission
  // Send buffer
  UINT16 *depthSend = kinectCamera.getDepthBuffer();
  BYTE *colorSend = kinectCamera.getColorBufferReduced();

  static INT16 *depthDiffSend = new INT16[DEPTH_SIZE]; // deallocate
  static INT16 *colorDiffSend = new INT16[COLOR_SIZE]; // deallocate
  static INT16 *frameDiffSend = new INT16[FRAME_SIZE]; // deallocate

  INT16 *combinedDifferential = kinectCamera.getCombinedDifferential();

  // Receive buffer
  static UINT16 depthReceive[DEPTH_SIZE];
  static BYTE colorReceive[COLOR_SIZE];

  static INT16 depthDiffReceive[DEPTH_SIZE];
  static INT16 colorDiffReceive[COLOR_SIZE];

  static UINT16 colorInt[COLOR_SIZE]; // temporary for stdhuffman
  static UINT16 dataReceived[FRAME_SIZE]; // temporary for stdhuffman

  static INT16 frameReceive[FRAME_SIZE];

  static INT16 currentFrameSender[FRAME_SIZE];
  static INT16 currentFrameReceiver[FRAME_SIZE];
  static INT16 differenceFrame[FRAME_SIZE];

  // Send processing
  dataToDiff(depthSend, depthDiffSend);
  dataToDiff(colorSend, colorDiffSend);
  dataToDiff(depthSend, colorSend, frameDiffSend);

  const int UNCOMPRESSED_SIZE = DEPTH_SIZE * 16 + COLOR_SIZE * 8;
  
  if (compressionMode == 1) {
    /******************** NO COMPRESSION **************************/
    drawText("1. No compression", 0.0f);
    for (int i = 0; i < DEPTH_SIZE; i++)
      depthReceive[i] = depthSend[i];
    for (int i = 0; i < COLOR_SIZE; i++)
      colorReceive[i] = colorSend[i];

  } else if (compressionMode == 2) {
    /******************** HUFFMAN DIFFERENTIAL 1 **************************/
    drawText("2. Huffman (2 trees for each image)", 0.0f);

    int depthTransmitSize, colorTransmitSize;
    if (stdToggle) {
      drawText("                                                                  [STANDARDISED]", 0.0f);
      Bitset transmitData;
      stdHuffmanCompressor.compress(DATA_DEPTH, transmitData, depthDiffSend);
      depthTransmitSize = transmitData.size();
      stdHuffmanCompressor.decompress(DATA_DEPTH, transmitData, depthReceive);

      stdHuffmanCompressor.compress(DATA_COLOR, transmitData, colorDiffSend);
      colorTransmitSize = transmitData.size();
      stdHuffmanCompressor.decompress(DATA_COLOR, transmitData, colorInt);
    } else {
      // Depth image
      Bitset transmitData = huffmanCompressor.compress(DEPTH_SIZE, depthDiffSend);
      depthTransmitSize = transmitData.size();
      huffmanCompressor.decompress(DEPTH_SIZE, transmitData, depthDiffReceive);

      // Color image
      transmitData = huffmanCompressor.compress(COLOR_SIZE, colorDiffSend);
      colorTransmitSize = transmitData.size();
      huffmanCompressor.decompress(COLOR_SIZE, transmitData, colorDiffReceive);
    }

    // Display result
    float compression_ratio = UNCOMPRESSED_SIZE / (float)(depthTransmitSize + colorTransmitSize);
    drawText("Compress ratio: " + std::to_string(compression_ratio), 0.1f);

    // Reconstruct values
    depthReceive[0] = depthDiffReceive[0];
    for (int i = 1; i < DEPTH_SIZE; i++)
      depthReceive[i] = depthDiffReceive[i] + depthReceive[i - 1];
    for (int i = 1; i < COLOR_SIZE; i++)
      colorDiffReceive[i] = colorDiffReceive[i] + colorDiffReceive[i - 1];
    for (int i = 0; i < COLOR_SIZE; i++)
      colorReceive[i] = (BYTE)colorDiffReceive[i];

  } else if (compressionMode == 3) {
    /******************** HUFFMAN DIFFERENTIAL 2 **************************/
    drawText("3. Huffman (1 tree for both images)", 0.0f);

    Bitset transmitData;
    if (stdToggle) {
      drawText("                                                                  [STANDARDISED]", 0.0f);
      stdHuffmanCompressor.compress(DATA_COMBINED, transmitData, combinedDifferential);
      stdHuffmanCompressor.decompress(DATA_COMBINED, transmitData, dataReceived);
    } else {
      // Compress both images
      transmitData = huffmanCompressor.compress(FRAME_SIZE, frameDiffSend);

      // Decompress data
      huffmanCompressor.decompress(FRAME_SIZE, transmitData, frameReceive);
    }

    // Print result
    float compression_ratio = UNCOMPRESSED_SIZE / (float)transmitData.size();
    drawText("Compress ratio: " + std::to_string(compression_ratio), 0.1f);

    // Reconstruct values
    
    for (int i = 1; i < FRAME_SIZE; i++)
      frameReceive[i] = frameReceive[i] + frameReceive[i - 1];
    for (int i = 0; i < DEPTH_SIZE; i++)
      depthReceive[i] = frameReceive[i];
    for (int i = 0; i < COLOR_SIZE; i++)
      colorReceive[i] = (BYTE)frameReceive[i + DEPTH_SIZE];
    

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
    
    Bitset transmitData;
    if (stdToggle) {
      drawText("                                                                  [STANDARDISED]", 0.0f);
      stdHuffmanCompressor.compress(DATA_COMBINED, transmitData, differenceFrame);
      stdHuffmanCompressor.decompress(DATA_COMBINED, transmitData, dataReceived);
    } else {
      // Compress
      transmitData = huffmanCompressor.compress(FRAME_SIZE, differenceFrame);

      // Decompress
      huffmanCompressor.decompress(FRAME_SIZE, transmitData, frameReceive);
    }

    // Results
    float compression_ratio = UNCOMPRESSED_SIZE / (float)transmitData.size();
    drawText("Compress ratio: " + std::to_string(compression_ratio), 0.1f);

    // Reconstruct values
    for (int i = 0; i < FRAME_SIZE; i++)
      currentFrameReceiver[i] += dataReceived[i];
    for (int i = 0; i < DEPTH_SIZE; i++)
      depthReceive[i] = currentFrameReceiver[i];
    for (int i = 0; i < COLOR_SIZE; i++)
      colorReceive[i] = (BYTE)currentFrameReceiver[i + DEPTH_SIZE];
  
  }

  // Receiver computer renders received frame data
  model.updatePointCloud(depthReceive, colorReceive);
  shaderRender();

  timer.stopTimer();
  drawText("FPS: " + std::to_string(timer.getFPS()), 1.8f);
  glutSwapBuffers();
}

void GlWindow::shaderRender()
{
  glm::mat4 modelMatrix = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
  // modelMatrix = glm::rotate(modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
  // modelMatrix = glm::translate(modelMatrix, glm::vec3(-256.0f, -212.0f, -1000.0f));
  glm::mat4 viewMatrix = glCamera.getViewMatrix();
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
    glCamera.setMousePosition(x, y);
  }
}

void GlWindow::mouseMotionCallback(int x, int y)
{
  glCamera.mouseUpdate(glm::vec2(x, y));
  renderCallback();
}

void GlWindow::keyboardFuncCallback(unsigned char key, int xMouse, int yMouse)
{
  switch (key) {
    //case 'c': {
    //  INT16 *depth = kinectCamera.getDepthDifferential();
    //  INT16 *color = kinectCamera.getColorDifferential();
    //  std::ofstream file("depth-differential.txt", std::ios::app);
    //  for (int i = 0; i < DEPTH_SIZE; i++)
    //    file << depth[i] << " ";
    //  file.close();

    //  file.open("color-differential.txt", std::ios::app);
    //  for (int i = 0; i < COLOR_SIZE; i++)
    //    file << color[i] << " ";
    //  file.close();
    //  break;
    //}
    //////// temporary /////////
    //case 'q': {
    //  INT16 *depth = kinectCamera.getDepthDifferential();
    //  INT16 *colorReceive = kinectCamera.getColorDifferential();
    //  INT16 *combined = kinectCamera.getCombinedDifferential();
    //  INT16 *dataReceived = new INT16[FRAME_SIZE];

    //  Bitset data = stdHuffmanCompressor.compress(DATA_DEPTH, depth);
    //  stdHuffmanCompressor.decompress(DATA_DEPTH, data, dataReceived);

    //  std::ofstream file("debug-output.txt");
    //  for (int i = 0; i < 500; i++)
    //    file << depth[i] << " ";
    //  file << std::endl << std::endl;
    //  for (int i = 0; i < 500; i++)
    //    file << dataReceived[i] << " ";
    //  file.close();

    //  delete[] dataReceived;
    //  break;
    //}
    //////// temporary /////////
    case ' ':
      glCamera.resetView();
      break;
    case 'w':
      glCamera.moveCameraPosition(0, 0, 1);
      break;
    case 'a':
      glCamera.moveCameraPosition(1, 0, 0);
      break;
    case 's':
      glCamera.moveCameraPosition(0, 0, -1);
      break;
    case 'd':
      glCamera.moveCameraPosition(-1, 0, 0);
      break;
    case '1':
      compressionMode = 1;
      break;
    case '2':
      compressionMode = 2;
      break;
    case '3':
      compressionMode = 3;
      break;
    case '4':
      compressionMode = 4;
      break;
    case '5':
      compressionMode = 5;
      break;
    case 'm':
      stdToggle = !stdToggle;
      break;
    case 27: // ESC
      closeCallback();
      break;
  }
}

void GlWindow::closeCallback()
{
  std::cout << "GLUT: Finished" << std::endl;
  glutLeaveMainLoop();
}