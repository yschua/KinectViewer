#include "GlWindow.h"

// Class members
// Declare as global variables due to GLUT limitation
GLuint program;
Camera camera;
KinectCamera kinectCamera;
HuffmanCompressor huffmanCompressor;
StdHuffmanCompressor stdHuffmanCompressor;
ModelGenerator model(kinectCamera.DEPTH_WIDTH, kinectCamera.DEPTH_HEIGHT);
Core::Timer timer;
int compressionMode = 1;
bool stdToggle = false;

// ICP test objects
PointCloud currentPtCloud, nextPtCloud, estimatePtCloud;
int icpToggle = 0;
ICP icp;

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

  ////////////////////////////////////////////////////////////////////////
  // ICP test initializer
  loadPointCloud();
  icp.computeTransformation();
  SE3<> transform = icp.getTransformation();

  for (int i = 0; i < currentPtCloud.vertices.size(); i++) {
    float x = currentPtCloud.vertices[i].position.x;
    float y = currentPtCloud.vertices[i].position.y;
    float z = currentPtCloud.vertices[i].position.z;
    Vector<4> p = makeVector(x, y, z, 1);
    p = transform * p;

    x = p[0]; y = p[1]; z = p[2];

    int row = i % DEPTH_WIDTH;
    int col = (i - row) / DEPTH_WIDTH;
    //if (row >= ICP_ROW_START && row <= ICP_ROW_END && col >= ICP_COL_START && col <= ICP_COL_END)
      //estimatePtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 0.f, 0.f) };
    //else
      estimatePtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 1.f, 1.f) };
  }
  ////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////
  // Depth test //
  std::ifstream file;
  file.open("depth-test-xyz.txt", std::ios::in);
  std::ofstream outfile("output.txt");
  std::vector<glm::vec3> ptCld;
  
  for (int y = 0; y < DEPTH_HEIGHT; y++) {
    for (int x = 0; x < DEPTH_WIDTH; x++) {
      float wx, wy, wz;
      file >> wx >> wy >> wz;
      glm::vec3 world = glm::vec3(wx, wy, wz);

      CameraParameters cameraParams;
      glm::vec3 image = world * cameraParams.depthIntrinsic;
      outfile << ((wz == 0) ?  0 : (int)(image.x / x * 1000 + 0.5)) << std::endl;

    }
  }
  file.close();
  outfile.close();
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

  UINT16 *depth = kinectCamera.getDepthBuffer();
  BYTE *color = kinectCamera.getColorBufferReduced();

  ////////////////////////////////////////////////////////////////////////
  // ICP point cloud rendering
  if (stdToggle) {
    for (int i = 0; i < currentPtCloud.vertices.size(); i++) {
      currentPtCloud.vertices[i].color.b = 1;
      currentPtCloud.vertices[i].color.g = 1;
      nextPtCloud.vertices[i].color.b = 1;
      nextPtCloud.vertices[i].color.g = 1;
    }
  }
  //icpToggle = 3;
  if (icpToggle == 0)
    model.updatePointCloud(currentPtCloud);
  else if (icpToggle == 1)
    model.updatePointCloud(nextPtCloud);
  else if (icpToggle == 2)
    model.updatePointCloud(estimatePtCloud);
  ////////////////////////////////////////////////////////////////////////


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

int fileCount = 1;

void GlWindow::keyboardFuncCallback(unsigned char key, int xMouse, int yMouse)
{
  switch (key) {
    case 'c': {
      std::ofstream file("depth-test.txt");
      UINT16 *depth = kinectCamera.getDepthBuffer();
      for (int i = 0; i < DEPTH_SIZE; i++) {
        file << depth[i] << std::endl;
      }
      file.close();

      //std::ofstream file("point-cloud" + std::to_string(fileCount++) + ".txt");
      file.open("depth-test-xyz.txt");
      PointCloud pointCloud = model.getPointCloud();
      for (int i = 0; i < pointCloud.numVertices; i++) {
        file << pointCloud.vertices[i].position.x << ' ' <<
        pointCloud.vertices[i].position.y << ' ' <<
        pointCloud.vertices[i].position.z << std::endl;
      }
      file.close();


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
    //  INT16 *color = kinectCamera.getColorDifferential();
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
    case 'z': {
      camera.moveCameraPosition(0, 1, 0);
      break;
    }
    case 'x': {
      camera.moveCameraPosition(0, -1, 0);
      break;
    }
    case '1': {
      compressionMode = 1;
      icpToggle = 0;
      break;
    }
    case '2': {
      //compressionMode = 2;
      icpToggle = 1;
      break;
    }
    case '3': {
      //compressionMode = 3;
      icpToggle = 2;
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
    case 'm': {
      stdToggle = !stdToggle;
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

void GlWindow::loadPointCloud()
{
  currentPtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);
  nextPtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);
  estimatePtCloud.vertices = std::vector<Vertex>(DEPTH_SIZE);

  std::ifstream file;
  file.open("point-cloud1.txt", std::ios::in);
  float x, y, z;
  for (int i = 0; file >> x >> y >> z; i++) {
    int row = i % DEPTH_WIDTH;
    int col = (i - row) / DEPTH_WIDTH;
    //if (row >= ICP_ROW_START && row <= ICP_ROW_END && col >= ICP_COL_START && col <= ICP_COL_END)
    if (i % 200 == 0)
      currentPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 0.f, 0.f) };
    else
      currentPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 1.f, 1.f) };
  }
  file.close();
  file.open("point-cloud2.txt", std::ios::in);
  for (int i = 0; file >> x >> y >> z; i++) {
    int row = i % DEPTH_WIDTH;
    int col = (i - row) / DEPTH_WIDTH;
    //if (row >= ICP_ROW_START && row <= ICP_ROW_END && col >= ICP_COL_START && col <= ICP_COL_END)
    if (i % 200 == 0)
      nextPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 0.f, 0.f) };
    else
      nextPtCloud.vertices[i] = { glm::vec3(x, y, z), glm::vec3(1.f, 1.f, 1.f) };
  }
  file.close();
  std::cout << "Point clouds loaded" << std::endl;
}