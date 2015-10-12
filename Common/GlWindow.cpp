#include "GlWindow.h"

GLuint program;
GlCamera glCamera;
KinectCamera kinectCamera;
HuffmanCompressor huffman;
StdHuffmanCompressor stdHuffman;
ModelGenerator model(DEPTH_WIDTH, DEPTH_HEIGHT);
CubeGenerator cube;
Core::Timer timer;
ICP icp;
CameraParameters cameraParams;

int rightButtonState;
int middleButtonState;

int compressionMode = 1;
bool stdMode = true;
bool flag = true;
bool record = false;
const int UNCOMPRESSED_SIZE = DEPTH_SIZE * 16 + COLOR_SIZE * 8;

GlWindow::GlWindow(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitWindowSize(800, 600);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
  glutCreateWindow("KinectViewer");
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_POINT_SMOOTH);
  // glPointSize(1.f);
  
  model.loadModel();
  cube.loadModel();

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
  //glutTimerFunc(66, timerCallback, 0);
  glutMouseFunc(mouseFuncCallback);
  glutMouseWheelFunc(mouseWheelFuncCallback);
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

    //huffman.decompress(DEPTH_SIZE, depthTransmit, depthDiffReceive);
    //huffman.decompress(COLOR_SIZE, colorTransmit, colorDiffReceive);
  } else {
    drawText("2b. Standard Huffman (2 trees)", 0.f);
    
    stdHuffman.compress(DATA_DEPTH, depthDiffSend, depthTransmit);
    stdHuffman.compress(DATA_COLOR, colorDiffSend, colorTransmit);

    //stdHuffman.decompress(DATA_DEPTH, depthTransmit, depthDiffReceive);
    //stdHuffman.decompress(DATA_COLOR, colorTransmit, colorDiffReceive);
  }

  float compressionRatio = UNCOMPRESSED_SIZE / (float)(depthTransmit.size() + colorTransmit.size());
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);

  if (record)
    recordData((stdMode) ? "rhuffman1std.txt" : "rhuffman1.txt", compressionRatio);

  diffToData(depthDiffReceive, depthReceive);
  diffToData(colorDiffReceive, colorReceive);

  for (int i = 0; i < DEPTH_SIZE; i++)
    depthReceive[i] = depthSend[i];
  for (int i = 0; i < COLOR_SIZE; i++)
    colorReceive[i] = colorSend[i];
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
    //huffman.decompress(FRAME_SIZE, transmitData, combineDiffReceive);
  } else {
    drawText("3b. Standard Huffman (1 tree)", 0.f);

    stdHuffman.compress(DATA_COMBINED, combineDiffSend, transmitData);
    //stdHuffman.decompress(DATA_COMBINED, transmitData, combineDiffReceive);
  }

  float compressionRatio = UNCOMPRESSED_SIZE / (float)transmitData.size();
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);

  if (record)
    recordData((stdMode) ? "rhuffman2std.txt" : "rhuffman2.txt", compressionRatio);

  diffToData(combineDiffReceive, depthReceive, colorReceive);

  for (int i = 0; i < DEPTH_SIZE; i++)
    depthReceive[i] = depthSend[i];
  for (int i = 0; i < COLOR_SIZE; i++)
    colorReceive[i] = colorSend[i];
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
    //huffman.decompress(FRAME_SIZE, transmitData, frameDiffReceive);
  } else {
    drawText("4b. Standard Huffman (difference frame)", 0.f);

    stdHuffman.compress(DATA_COMBINED, frameDiffSend, transmitData);
    //stdHuffman.decompress(DATA_COMBINED, transmitData, frameDiffReceive);
  }


  float compressionRatio = UNCOMPRESSED_SIZE / (float)transmitData.size();
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);

  if (record)
    recordData((stdMode) ? "rframediffstd.txt" : "rframediff.txt", compressionRatio);

  for (int i = 0; i < FRAME_SIZE; i++) frameDiffReceive[i] = frameDiffSend[i];

  k = 0;
  for (int i = 0; i < DEPTH_SIZE; i++, k++) {
    refFrameReceive[k] += frameDiffReceive[k];
    depthReceive[i] = refFrameReceive[k];
  }
  for (int i = 0; i < COLOR_SIZE; i++, k++) {
    refFrameReceive[k] += frameDiffReceive[k];
    colorReceive[i] = refFrameReceive[k];
  }
}

void GlWindow::frameDiffICP(const UINT16 *depthSend, const BYTE *colorSend,
                         UINT16 *depthReceive, BYTE *colorReceive)
{
  drawText("5. Standard Huffman + ICP (difference frame)", 0.f);

  static UINT16 *refDepthSend = new UINT16[DEPTH_SIZE];
  static BYTE *refColorSend = new BYTE[COLOR_SIZE];

  static UINT16 *estimateDepth = new UINT16[DEPTH_SIZE];
  static BYTE *estimateColor = new BYTE[COLOR_SIZE];

  static INT16 *frameDiffSend = new INT16[FRAME_SIZE];
  static INT16 *frameDiffReceive = new INT16[FRAME_SIZE];

  static PointCloud estimatePtCloud;

  static UINT16 *nextDepth = new UINT16[DEPTH_SIZE];
  static BYTE *nextColor = new BYTE[COLOR_SIZE];

  static bool init = true;
  if (init) {
    memset(refDepthSend, 0, sizeof(UINT16) * DEPTH_SIZE);
    memset(refColorSend, 0, sizeof(BYTE) * COLOR_SIZE);
    estimatePtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);
    init = false;
  }

  for (int i = 0; i < DEPTH_SIZE; i++) {
    estimateDepth[i] = refDepthSend[i];
    nextDepth[i] = limitDepth(depthSend[i]);
  }
  for (int i = 0; i < COLOR_SIZE; i++) {
    estimateColor[i] = 0;
  }

  // Convert to world coordinates
  model.updatePointCloud(nextDepth, colorSend); // new frame
  icp.loadPointsY(model.getPointCloud());
  model.updatePointCloud(refDepthSend, refColorSend); // previous frame
  icp.loadPointsX(model.getPointCloud());

  // Compute transformation
  icp.computeTransformation();
  SE3<> transform = icp.getTransformation();

  drawText("Iterations: " + std::to_string(icp.getIterations()) + " Error: " + std::to_string(icp.getError()), 0.2f);
  displayTransform(transform, 0.3f);

  // Apply transformation
  for (int i = 0; i < model.getPointCloud().numVertices; i++) {
    glm::vec3 world = model.getPointCloud().vertices[i].position;
    glm::vec3 color = model.getPointCloud().vertices[i].color;

    Vector<4> p = makeVector(world.x, world.y, world.z, 1);
    p = transform * p;
    world = glm::vec3(p[0], p[1], p[2]);

    estimatePtCloud.vertices[i] = { world, color };
  }

  // Convert estimated world coordinates back to depth
  for (int i = 0; i < estimatePtCloud.vertices.size(); i++) {
    glm::vec3 world = estimatePtCloud.vertices[i].position;
    glm::vec3 image = world * cameraParams.depthIntrinsic;
    glm::vec3 color = estimatePtCloud.vertices[i].color;

    int depth = (int)(world.z * 1000.f + 0.5f);
    int x = (int)(image.x / world.z + 0.5f);
    int y = (int)(image.y / world.z + 0.5f);

    int depthIndex = (DEPTH_HEIGHT - 1 - y) * DEPTH_WIDTH + (DEPTH_WIDTH - 1 - x);
    if (depthIndex >= 0 && depthIndex < DEPTH_SIZE) {
      estimateDepth[depthIndex] = limitDepth(depth);
      estimateColor[depthIndex * 3] = (int)(color.r * 255.f + 0.5f);
      estimateColor[depthIndex * 3 + 1] = (int)(color.g * 255.f + 0.5f);
      estimateColor[depthIndex * 3 + 2] = (int)(color.b * 255.f + 0.5f);
    }
  }

  // Compute frame difference
  int k = 0;
  for (int i = 0; i < DEPTH_SIZE; i++, k++) {
    frameDiffSend[k] = nextDepth[i] - estimateDepth[i];
    refDepthSend[i] = nextDepth[i];
  }
  for (int i = 0; i < COLOR_SIZE; i++, k++) {
    frameDiffSend[k] = colorSend[i] - estimateColor[i];
    refColorSend[i] = colorSend[i];
  }

  Bitset transmitData;
  stdHuffman.compress(DATA_COMBINED, frameDiffSend, transmitData);
  //stdHuffman.decompress(DATA_COMBINED, transmitData, frameDiffReceive);

  float compressionRatio = (UNCOMPRESSED_SIZE + 16 * 32) / (float)(transmitData.size());
  drawText("Compress ratio: " + std::to_string(compressionRatio), 0.1f);

  if (record)
    recordData("ricp.txt", compressionRatio);

  k = 0;
  for (int i = 0; i < DEPTH_SIZE; i++, k++) {
    depthReceive[i] = estimateDepth[i] + frameDiffReceive[k];
  }
  for (int i = 0; i < COLOR_SIZE; i++, k++) {
    colorReceive[i] = estimateColor[i] + frameDiffReceive[k];
  }

  icp.clear();
}

void GlWindow::cubeDemo(const UINT16 *depth)
{
  static UINT16 *refDepth = new UINT16[DEPTH_SIZE];
  static BYTE *refColor = new BYTE[COLOR_SIZE];

  static bool init = true;
  static int count = 0;
  if (init) {
    memset(refDepth, 0, sizeof(UINT16) * DEPTH_SIZE);
    memset(refColor, 0, sizeof(BYTE) * COLOR_SIZE);
    init = false;
  }
  count++;
  if (count == 100) {
    std::cout << "reset" << std::endl;
    cube.reset();
  }
  // Convert to world coordinates
  model.updatePointCloud(depth, refColor); // new frame
  icp.loadPointsY(model.getPointCloud());
  model.updatePointCloud(refDepth, refColor); // previous frame
  icp.loadPointsX(model.getPointCloud());

  for (int i = 0; i < DEPTH_SIZE; i++)
    refDepth[i] = depth[i];

  // Compute transformation
  if (count > 1)
    icp.computeTransformation();
  SE3<> transform = icp.getTransformation();

  Matrix<3> flipY;
  flipY[0] = makeVector(1, 0, 0);
  flipY[1] = makeVector(0, -1, 0);
  flipY[2] = makeVector(0, 0, 1);
  Matrix<3> mat = transform.get_rotation().get_matrix();
  mat = flipY * mat * flipY;
  transform.get_rotation() = SO3<>(mat);

  transform.get_translation()[2] *= 2; // scale z displacement

  displayTransform(transform, 0.3f);
  icp.clear();
  //std::cout << transform.ln() << std::endl;
  
  // Apply transformation
  for (int i = 0; i < cube.getNumVertices(); i++) {
    glm::vec3 world = cube.getPointCloud().vertices[i].position;

    Vector<4> p = makeVector(world.x, world.y, world.z, 1);
    p =  transform * p;
    cube.getPointCloud().vertices[i].position = glm::vec3(p[0], p[1], p[2]);
  }

  glBufferData(GL_ARRAY_BUFFER, cube.getPointCloud().bufferSize(), &cube.getPointCloud().vertices[0], GL_DYNAMIC_DRAW);

  glm::mat4 modelMatrix = glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));
  // modelMatrix = glm::rotate(modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
  //modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, 0.f, 0.f));
  glm::mat4 viewMatrix = glCamera.getViewMatrix();
  glm::mat4 projectionMatrix = glm::perspective(45.0f, 64.0f / 53.0f, 0.1f, 10.0f);
  glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

  glUseProgram(program);
  GLuint mvpMatrix = glGetUniformLocation(program, "mvpMatrix");
  glUniformMatrix4fv(mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
  glDrawArrays(GL_QUADS, 0, cube.getNumVertices());
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

void GlWindow::recordData(std::string filename, float value)
{
  std::ofstream file(filename, std::ios::app);
  file << value << std::endl;
  file.close();
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

void GlWindow::timerCallback(int value)
{
  renderCallback();
  glutTimerFunc(83, timerCallback, value);
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
    case 1:
      drawText("1. No compression", 0.0f);
      for (int i = 0; i < DEPTH_SIZE; i++)
        depthReceive[i] = depthSend[i];
      for (int i = 0; i < COLOR_SIZE; i++)
        colorReceive[i] = colorSend[i];
      break;
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
    default:
      break;
  }

  // Receiver computer renders received frame data
  if (compressionMode <= 5) {
    model.updatePointCloud(depthReceive, colorReceive);
    shaderRender();
  } else {
    cubeDemo(depthSend);
  }

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

void GlWindow::mouseWheelFuncCallback(int button, int dir, int x, int y)
{
  if (dir > 0) {
    glCamera.moveCameraPosition(0, 0, 2);
  } else if (dir < 0) {
    glCamera.moveCameraPosition(0, 0, -2);
  }
}

void GlWindow::mouseFuncCallback(int button, int state, int x, int y)
{
  if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
    rightButtonState = 1;
    glCamera.setMousePosition(x, y);
  } else {
    rightButtonState = 0;
  }

  if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
    middleButtonState = 1;
    glCamera.setMousePosition(x, y);
  } else {
    middleButtonState = 0;
  }
}

void GlWindow::mouseMotionCallback(int x, int y)
{
  if (rightButtonState) {
    glCamera.mouseRotateCamera(glm::vec2(x, y));
    renderCallback();
  }

  if (middleButtonState) {
    glCamera.mouseMoveCamera(glm::vec2(x, y));
  }
}

void GlWindow::keyboardFuncCallback(unsigned char key, int xMouse, int yMouse)
{
  switch (key) {
    case ' ':
      glCamera.resetView();
      cube.reset();
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
      compressionMode = key - '0';
      break;
    case 'r':
      record = true;
      break;
    case 's':
      stdMode = !stdMode;
      break;
    case 'f':
      flag = !flag;
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
