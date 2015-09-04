#include "GlWindow.h"

GLuint program;
Camera glCamera;
KinectCamera kinectCamera;
HuffmanCompressor huffman;
StdHuffmanCompressor stdHuffman;
ModelGenerator model(DEPTH_WIDTH, DEPTH_HEIGHT);
Core::Timer timer;
ICP icp;
CameraParameters cameraParams;

int compressionMode = 1;
bool stdMode = true;
const int UNCOMPRESSED_SIZE = DEPTH_SIZE * 16 + COLOR_SIZE * 8;

GlWindow::GlWindow(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitWindowSize(800, 600);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("KinectViewer");
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_POINT_SMOOTH);
  // glPointSize(1.f);
  
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
  int diffIdx = 1;
  diff[0] = limitDepth(depth[0]);
  for (int i = 1; i < DEPTH_SIZE; i++)
    diff[diffIdx++] = limitDepth(depth[i]) - limitDepth(depth[i - 1]);
  diff[diffIdx++] = color[0] - limitDepth(depth[DEPTH_SIZE - 1]);
  for (int i = 1; i < COLOR_SIZE; i++)
    diff[diffIdx++] = color[i] - color[i - 1];
}

// Reconstruct depth data
void GlWindow::diffToData(const INT16 *diff, UINT16 *data)
{
  data[0] = diff[0];
  for (int i = 1; i < DEPTH_SIZE; i++)
    data[i] = diff[i] + data[i - 1];
}

// Reconstruct color data
void GlWindow::diffToData(const INT16 *diff, BYTE *data)
{
  data[0] = diff[0];
  for (int i = 1; i < COLOR_SIZE; i++)
    data[i] = diff[i] + data[i - 1];
}

// Reconstruct depth and color data
void GlWindow::diffToData(const INT16 *diff, UINT16 *depth, BYTE *color)
{
  int diffIdx = 1;
  depth[0] = diff[0];
  for (int i = 1; i < DEPTH_SIZE; i++)
    depth[i] = diff[diffIdx++] + depth[i - 1];
  color[0] = diff[diffIdx++] + depth[DEPTH_SIZE - 1];
  for (int i = 1; i < COLOR_SIZE; i++)
    color[i] = diff[diffIdx++] + color[i - 1];
}

// Perform huffman compression seperately on depth and color
void GlWindow::huffman1(const UINT16 *depthSend, const BYTE *colorSend,
                        UINT16 *depthReceive, BYTE *colorReceive, bool stdMode)
{
  static INT16 *depthDiffSend = new INT16[DEPTH_SIZE]; // deallocate
  static INT16 *colorDiffSend = new INT16[COLOR_SIZE]; // deallocate
  static INT16 *depthDiffReceive = new INT16[DEPTH_SIZE];
  static INT16 *colorDiffReceive = new INT16[COLOR_SIZE];
  Bitset depthTransmit, colorTransmit;

  dataToDiff(depthSend, depthDiffSend);
  dataToDiff(colorSend, colorDiffSend);

  if (!stdMode) {
    drawText("2a. Huffman (2 trees)", 0.f);

    huffman.compress(DEPTH_SIZE, depthDiffSend, depthTransmit);
    huffman.compress(COLOR_SIZE, colorDiffSend, colorTransmit);

    huffman.decompress(DEPTH_SIZE, depthTransmit, depthDiffReceive);
    huffman.decompress(COLOR_SIZE, colorTransmit, colorDiffReceive);
  } else {
    drawText("2b. Standard Huffman (2 trees)", 0.f);
    
    stdHuffman.compress(DATA_DEPTH, depthDiffSend, depthTransmit);
    stdHuffman.compress(DATA_COLOR, colorDiffSend, colorTransmit);

    stdHuffman.decompress(DATA_DEPTH, depthTransmit, depthDiffReceive);
    stdHuffman.decompress(DATA_COLOR, colorTransmit, colorDiffReceive);
  }

  float compressionRatio = UNCOMPRESSED_SIZE / (float)(depthTransmit.size() + colorTransmit.size());
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);

  diffToData(depthDiffReceive, depthReceive);
  diffToData(colorDiffReceive, colorReceive);
}

// Perform huffman compression on whole frame
void GlWindow::huffman2(const UINT16 *depthSend, const BYTE *colorSend,
                        UINT16 *depthReceive, BYTE *colorReceive, bool stdMode)
{
  static INT16 *combineDiffSend = new INT16[FRAME_SIZE]; // deallocate
  static INT16 *combineDiffReceive = new INT16[FRAME_SIZE];
  Bitset transmitData;

  dataToDiff(depthSend, colorSend, combineDiffSend);

  if (!stdMode) {
    drawText("3a. Huffman (1 tree)", 0.f);

    huffman.compress(FRAME_SIZE, combineDiffSend, transmitData);
    huffman.decompress(FRAME_SIZE, transmitData, combineDiffReceive);
  } else {
    drawText("3b. Standard Huffman (1 tree)", 0.f);

    stdHuffman.compress(DATA_COMBINED, combineDiffSend, transmitData);
    stdHuffman.decompress(DATA_COMBINED, transmitData, combineDiffReceive);
  }

  float compressionRatio = UNCOMPRESSED_SIZE / (float)transmitData.size();
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);

  diffToData(combineDiffReceive, depthReceive, colorReceive);
}

void GlWindow::frameDiff(const UINT16 *depthSend, const BYTE *colorSend,
                            UINT16 *depthReceive, BYTE *colorReceive, bool stdMode)
{
  static INT16 *refFrameSend = new INT16[FRAME_SIZE];
  static INT16 *refFrameReceive = new INT16[FRAME_SIZE];
  static INT16 *frameDiffSend = new INT16[FRAME_SIZE];
  static INT16 *frameDiffReceive = new INT16[FRAME_SIZE];
  Bitset transmitData;

  static bool init = true;
  if (init) {
    memset(refFrameSend, 0, sizeof(INT16) * FRAME_SIZE);
    memset(refFrameReceive, 0, sizeof(INT16) * FRAME_SIZE);
    init = false;
  }

  int k = 0;
  for (int i = 0; i < DEPTH_SIZE; i++, k++) {
    frameDiffSend[k] = limitDepth(depthSend[i]) - refFrameSend[k];
    refFrameSend[k] = limitDepth(depthSend[i]);
  }
  for (int i = 0; i < COLOR_SIZE; i++, k++) {
    frameDiffSend[k] = colorSend[i] - refFrameSend[k];
    refFrameSend[k] = colorSend[i];
  }

  if (!stdMode) {
    drawText("4a. Huffman (difference frame)", 0.f);

    huffman.compress(FRAME_SIZE, frameDiffSend, transmitData);
    huffman.decompress(FRAME_SIZE, transmitData, frameDiffReceive);
  } else {
    drawText("4b. Standard Huffman (difference frame)", 0.f);

    stdHuffman.compress(DATA_DEPTH, frameDiffSend, transmitData);
    stdHuffman.decompress(DATA_DEPTH, transmitData, frameDiffReceive);
  }

  float compressionRatio = UNCOMPRESSED_SIZE / (float)transmitData.size();
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);

  k = 0;
  for (int i = 0; i < DEPTH_SIZE; i++, k++) {
    refFrameReceive[k] += frameDiffReceive[k];
    depthReceive[i] = refFrameReceive[k];
  }
  for (int i = 0; i < COLOR_SIZE; i++, k++) {
    refFrameReceive[k] += frameDiffReceive[k];
    colorReceive[i] = 255; //refFrameReceive[k];
  }
}

void GlWindow::frameDiffICP(const UINT16 *depthSend, const BYTE *colorSend,
                         UINT16 *depthReceive, BYTE *colorReceive)
{
  drawText("5. Standard Huffman + ICP (difference frame)", 0.f);

  static UINT16 *currentDepth = new UINT16[DEPTH_SIZE]; 
  static UINT16 *estimateDepth = new UINT16[DEPTH_SIZE];
  static UINT16 *nextDepth = new UINT16[DEPTH_SIZE];
  static PointCloud estimatePtCloud;

  static INT16 *refFrameSend = new INT16[FRAME_SIZE];
  static INT16 *refFrameReceive = new INT16[FRAME_SIZE];
  static INT16 *frameDiffSend = new INT16[FRAME_SIZE];
  static INT16 *frameDiffReceive = new INT16[FRAME_SIZE];

  static bool init = true;
  if (init) {
    memset(currentDepth, 0, sizeof(UINT16) * DEPTH_SIZE);
    memset(refFrameSend, 0, sizeof(INT16) * FRAME_SIZE);
    memset(refFrameReceive, 0, sizeof(INT16) * FRAME_SIZE);
    estimatePtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);
    init = false;
  }

  for (int i = 0; i < COLOR_SIZE; i++) colorReceive[i] = 255;
  for (int i = 0; i < DEPTH_SIZE; i++) {
    nextDepth[i] = depthSend[i];
    estimateDepth[i] = 0;
  }

  // Convert to world coordinates
  model.updatePointCloud(nextDepth, colorReceive);
  icp.loadPointsY(model.getPointCloud());
  model.updatePointCloud(currentDepth, colorReceive);
  icp.loadPointsX(model.getPointCloud());

  icp.computeTransformation();
  SE3<> transform = icp.getTransformation();

  drawText("Iterations: " + std::to_string(icp.getIterations()) + " Error: " + std::to_string(icp.getError()), 0.2f);
  displayTransform(transform, 0.3f);

  // Apply transformation
  for (int i = 0; i < model.getPointCloud().numVertices; i++) {
    float wx = model.getPointCloud().vertices[i].position.x;
    float wy = model.getPointCloud().vertices[i].position.y;
    float wz = model.getPointCloud().vertices[i].position.z;

    Vector<4> p = makeVector(wx, wy, wz, 1);
    p = transform * p; 
    wx = p[0];
    wy = p[1];
    wz = p[2];

    estimatePtCloud.vertices[i] = { glm::vec3(wx, wy, wz), glm::vec3(1.f, 1.f, 1.f) };
  }

  // Convert estimated world coordinates back to depth
  for (int i = 0; i < estimatePtCloud.vertices.size(); i++) {
    glm::vec3 world = estimatePtCloud.vertices[i].position;
    glm::vec3 image = world * cameraParams.depthIntrinsic;

    int depth = (int)(world.z * 1000.f);
    int x = (int)(image.x / world.z + 0.5f);
    int y = (int)(image.y / world.z + 0.5f);

    int depthIndex = (DEPTH_HEIGHT - 1 - y) * DEPTH_WIDTH + (DEPTH_WIDTH - 1 - x);
    if (depthIndex >= 0 && depthIndex < DEPTH_SIZE)
      estimateDepth[depthIndex] = limitDepth(depth);
  }

  //for (int iy = 0; iy < DEPTH_HEIGHT; iy++) {
  //  for (int ix = 0; ix < DEPTH_WIDTH; ix++) {
  //    float wx = estimatePtCloud.vertices[i].position.x;
  //    float wy = estimatePtCloud.vertices[i].position.y;
  //    float wz = estimatePtCloud.vertices[i].position.z;

  //    glm::vec3 world = glm::vec3(wx, wy, wz);
  //    glm::vec3 image = world * cameraParams.depthIntrinsic;

  //    estimateDepth[DEPTH_SIZE - 1 - i] = (int)(image.x / ix * 1000 + 0.5f);
  //    
  //    if (estimateDepth[DEPTH_SIZE - 1 - i] < 0) {
  //      estimateDepth[DEPTH_SIZE - 1 - i] = 0;
  //    } else if (estimateDepth[DEPTH_SIZE - 1 - i] > 4500) {
  //      estimateDepth[DEPTH_SIZE - 1 - i] = 4500;
  //    }

  //    i++;
  //  }
  //}


  for (int i = 0; i < DEPTH_SIZE; i++) {
    frameDiffSend[i] = limitDepth(nextDepth[i]) - estimateDepth[i];
  }

  Bitset transmitData;
  stdHuffman.compress(DATA_DEPTH, frameDiffSend, transmitData);

  float compressionRatio = UNCOMPRESSED_SIZE / (float)transmitData.size();
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);
  
  for (int i = 0; i < DEPTH_SIZE; i++) {
    currentDepth[i] = nextDepth[i];
    depthReceive[i] = estimateDepth[i];
  }
  icp.clear();
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

void GlWindow::displayTransform(const SE3<> &transform, float offset)
{

  drawText("Transformation:", offset);
  offset += 0.1f;
  for (int i = 0; i < 3; i++, offset += 0.1f) {
    auto rotation = transform.get_rotation().get_matrix()[i];
    auto translation = transform.get_translation()[i];

    drawText(std::to_string(rotation[0]) + ' ' +
             std::to_string(rotation[1]) + ' ' +
             std::to_string(rotation[2]) + ' ' +
             std::to_string(translation), offset);
  }
}

void GlWindow::renderCallback()
{
  timer.startTimer(); // FPS timer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0, 0, 0, 1);

  // Sender computer obtains frame data
  kinectCamera.update();

  // Data processing and simulated transmission
  UINT16 *depthSend = kinectCamera.getDepthBuffer();
  BYTE *colorSend = kinectCamera.getColorBufferReduced();
  static UINT16 *depthReceive = new UINT16[DEPTH_SIZE];
  static BYTE *colorReceive = new BYTE[COLOR_SIZE];

  switch (compressionMode) {
    case 1: {
      drawText("1. No compression", 0.0f);
      for (int i = 0; i < DEPTH_SIZE; i++)
        depthReceive[i] = depthSend[i];
      for (int i = 0; i < COLOR_SIZE; i++)
        colorReceive[i] = colorSend[i];
      break;
    }
    case 2:
      huffman1(depthSend, colorSend, depthReceive, colorReceive, stdMode);
      break;
    case 3:
      huffman2(depthSend, colorSend, depthReceive, colorReceive, stdMode);
      break;
    case 4:
      frameDiff(depthSend, colorSend, depthReceive, colorReceive, stdMode);
      break;
    case 5:
      frameDiffICP(depthSend, colorSend, depthReceive, colorReceive);
      break;
    default: break;
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

int fileCount = 1;

void GlWindow::keyboardFuncCallback(unsigned char key, int xMouse, int yMouse)
{
  switch (key) {
    case 'c': {
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
    //case 'c': {
    //  std::ofstream file("depth-test.txt");
    //  UINT16 *depth = kinectCamera.getDepthBuffer();
    //  for (int i = 0; i < DEPTH_SIZE; i++) {
    //    file << depth[i] << std::endl;
    //  }
    //  file.close();
    //  //std::ofstream file("point-cloud" + std::to_string(fileCount++) + ".txt");
    //  file.open("depth-test-xyz.txt");
    //  PointCloud pointCloud = model.getPointCloud();
    //  for (int i = 0; i < pointCloud.numVertices; i++) {
    //    file << pointCloud.vertices[i].position.x << ' ' <<
    //    pointCloud.vertices[i].position.y << ' ' <<
    //    pointCloud.vertices[i].position.z << std::endl;
    //  }
    //  file.close();
    //INT16 *depth = kinectCamera.getDepthDifferential();
    //INT16 *color = kinectCamera.getColorDifferential();
    //std::ofstream file("depth-differential.txt", std::ios::app);
    //for (int i = 0; i < DEPTH_SIZE; i++)
    //  file << depth[i] << " ";
    //file.close();
    //file.open("color-differential.txt", std::ios::app);
    //for (int i = 0; i < COLOR_SIZE; i++)
    //  file << color[i] << " ";
    //file.close();
      break;
    }
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
    case 'z':
      glCamera.moveCameraPosition(0, 1, 0);
      break;
    case 'x':
      glCamera.moveCameraPosition(0, -1, 0);
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
      compressionMode = key - '0';
      break;
    case 'm':
      stdMode = !stdMode;
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

//void GlWindow::loadPointCloud()
//{
//  currentPtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);
//  nextPtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);
//  estimatePtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);
//
//  std::ifstream file;
//  file.open("point-cloud1.txt", std::ios::in);
//  float x, y, z;
//  for (int i = 0; file >> x >> y >> z; i++) {
//    int row = i % DEPTH_WIDTH;
//    int col = (i - row) / DEPTH_WIDTH;
//    //if (row >= ICP_ROW_START && row <= ICP_ROW_END && col >= ICP_COL_START && col <= ICP_COL_END)
//    if (i % 200 == 0)
//      currentPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 0.f, 0.f) };
//    else
//      currentPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 1.f, 1.f) };
//  }
//  file.close();
//  file.open("point-cloud2.txt", std::ios::in);
//  for (int i = 0; file >> x >> y >> z; i++) {
//    int row = i % DEPTH_WIDTH;
//    int col = (i - row) / DEPTH_WIDTH;
//    //if (row >= ICP_ROW_START && row <= ICP_ROW_END && col >= ICP_COL_START && col <= ICP_COL_END)
//    if (i % 200 == 0)
//      nextPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 0.f, 0.f) };
//    else
//      nextPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 1.f, 1.f) };
//  }
//  file.close();
//  std::cout << "Point clouds loaded" << std::endl;
//}