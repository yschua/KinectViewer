#include "GlWindow.h"

// Class members
// Declare as global variables due to GLUT limitation
GLuint program;
Camera camera;
KinectCamera kinectCamera;
ModelGenerator model;

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
  
  model.generate();

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
  glutDisplayFunc(renderCallback);
  glutMouseFunc(mouseFuncCallback);
  glutMotionFunc(mouseMotionCallback);
  glutKeyboardFunc(keyboardFuncCallback);
  glutCloseFunc(closeCallback);
  glutMainLoop();
}

bool once = true; // temporary

void GlWindow::renderCallback()
{
  kinectCamera.update();

  //if (kinectCamera.depthBuffer[100] > 0 && once) {
  //std::cout << "Capture" << std::endl;
  //model.updateModel(kinectCamera.colorBuffer);
  //model.updateDepthFrame(kinectCamera.depthBuffer);
  
  model.updatePointCloud(kinectCamera);
  //once = false; // temporary
  //}
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0, 0, 0, 1);
  glUseProgram(program);

  glm::mat4 modelMatrix = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
  //modelMatrix = glm::rotate(modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
  //modelMatrix = glm::translate(modelMatrix, glm::vec3(-256.0f, -212.0f, -1000.0f));
  glm::mat4 viewMatrix = camera.getViewMatrix();
  glm::mat4 projectionMatrix = glm::perspective(45.0f, 64.0f / 53.0f, 0.000001f, 10000.0f);
  glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

  GLuint mvpMatrix = glGetUniformLocation(program, "mvpMatrix");
  glUniformMatrix4fv(mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
  glDrawArrays(GL_POINTS, 0, model.getNumVertices());

  glutSwapBuffers();
}

void GlWindow::mouseFuncCallback(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    once = true; // temporary
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
  }
}

void GlWindow::closeCallback()
{
  std::cout << "GLUT: Finished" << std::endl;
  glutLeaveMainLoop();
}