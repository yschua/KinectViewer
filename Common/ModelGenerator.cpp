#include "ModelGenerator.h"

ModelGenerator::ModelGenerator()
{
  pointCloud.vertices = std::vector<Vertex>(512 * 424); // fix magic number
}

ModelGenerator::~ModelGenerator()
{
  //delete[] indices;
}

void ModelGenerator::generate()
{
  createModel();
  loadModel();
}

int ModelGenerator::getNumVertices()
{
  return pointCloud.numVertices;
}

PointCloud ModelGenerator::getModel()
{
  return pointCloud;
}

void ModelGenerator::updateModel(const RGBQUAD *colorBuffer)
{
  timer.startTimer();
  int smallIndex;
  int modelIndex = 0;
  for (int y = 0; y < 1080; y += 2) { // fix magic number
    for (int x = 0; x < 1920; x += 2) { // fix magic number
      smallIndex = (1079 - y) * 1920 + (1919 - x);
      float r = colorBuffer[smallIndex].rgbRed / 255.0f;
      float g = colorBuffer[smallIndex].rgbGreen / 255.0f;
      float b = colorBuffer[smallIndex].rgbBlue / 255.0f;
      pointCloud.vertices[modelIndex++] = { glm::vec3(x, y, 0.0f), glm::vec3(r, g, b) };
    }
  }
  pointCloud.numVertices = pointCloud.vertices.size();
  timer.stopTimer();
  //std::cout << "Build point cloud: " << timer.getElapsedTime() << std::endl;

  timer.startTimer();
  glBufferData(GL_ARRAY_BUFFER, pointCloud.bufferSize(), &pointCloud.vertices[0], GL_DYNAMIC_DRAW);
  timer.stopTimer();

  //std::cout << "Load VBO: " << timer.getElapsedTime() << std::endl;
}

void ModelGenerator::updateDepthFrame(const UINT16 *depthBuffer)
{
  int index = 0;
  int smallIndex;
  for (int y = 0; y < 424; y++) {
    for (int x = 0; x < 512; x++) {
      smallIndex = (423 - y) * 512 + (511 - x);
      USHORT depth = depthBuffer[smallIndex];

      BYTE intensityInt = static_cast<BYTE>((depth >= 50) && (depth <= USHRT_MAX) ? (depth % 256) : 0);
      float intensity = intensityInt / 255.0f;
      pointCloud.vertices[index++] = { glm::vec3(x, y, depth), glm::vec3(intensity, intensity, intensity) };
    }
  }
  pointCloud.numVertices = pointCloud.vertices.size();
  
  glBufferData(GL_ARRAY_BUFFER, pointCloud.bufferSize(), &pointCloud.vertices[0], GL_DYNAMIC_DRAW);
}

void ModelGenerator::updatePointCloud(KinectCamera &kinectCamera)
{
  int pointCloudIndex = 0;
  CameraSpacePoint *cameraSpacePoints = kinectCamera.getCameraSpacePoints();
  ColorSpacePoint *colorSpacePoints = kinectCamera.getColorSpacePoints();

  for (int y = 0; y < kinectCamera.DEPTH_HEIGHT; ++y) {
    for (int x = 0; x < kinectCamera.DEPTH_WIDTH; ++x) {
      int depthIndex = (kinectCamera.DEPTH_HEIGHT - 1 - y) * kinectCamera.DEPTH_WIDTH + (kinectCamera.DEPTH_WIDTH - 1 - x);
      CameraSpacePoint cameraSpacePoint = cameraSpacePoints[depthIndex];
      ColorSpacePoint colorSpacePoint = colorSpacePoints[depthIndex];

      if (isnan(cameraSpacePoint.Z)) {
        pointCloud.vertices[pointCloudIndex] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) };
      } else {
        int colorX = static_cast<int>(std::floor(colorSpacePoint.X + 0.5f));
        int colorY = static_cast<int>(std::floor(colorSpacePoint.Y + 0.5f));
        if ((colorX >= 0) && (colorX < kinectCamera.COLOR_WIDTH) && (colorY >= 0) && (colorY < kinectCamera.COLOR_HEIGHT)) {
          RGBQUAD color = kinectCamera.getColorBuffer()[colorY * kinectCamera.COLOR_WIDTH + colorX];
          float r = color.rgbRed / 255.0f;
          float g = color.rgbGreen / 255.0f;
          float b = color.rgbBlue / 255.0f;
          float x = cameraSpacePoint.X;
          float y = cameraSpacePoint.Y;
          float z = cameraSpacePoint.Z;

          pointCloud.vertices[pointCloudIndex] = { glm::vec3(x, y, z), glm::vec3(r, g, b) };
        } else {
          pointCloud.vertices[pointCloudIndex] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) };
        }
      }
      pointCloudIndex++;
    }
  }
  pointCloud.numVertices = pointCloud.vertices.size();
  glBufferData(GL_ARRAY_BUFFER, pointCloud.bufferSize(), &pointCloud.vertices[0], GL_DYNAMIC_DRAW);
}

void ModelGenerator::createModel()
{
  pointCloud.vertices[0] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f) };
  pointCloud.vertices[1] = { glm::vec3(1920.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f) };
  pointCloud.vertices[2] = { glm::vec3(0.0f, 1080.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f) };

  pointCloud.vertices[3] = { glm::vec3(0.0f, 1080.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
  pointCloud.vertices[4] = { glm::vec3(1920.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
  pointCloud.vertices[5] = { glm::vec3(1920.0f, 1080.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) };

  //vertices.push_back({ glm::vec3(0.0f, 0.0f, 0.0f),
  //                     glm::vec3(1.0f, 1.0f, 1.0f) });
  //vertices.push_back({ glm::vec3(1.0f, 0.0f, 0.0f),
  //                     glm::vec3(1.0f, 1.0f, 1.0f) });
  //vertices.push_back({ glm::vec3(0.5f, 1.0f, 0.0f),
  //                     glm::vec3(1.0f, 1.0f, 1.0f) });
  //
  //vertices.push_back({ glm::vec3(0.0f, 0.0f, 0.0f),
  //                     glm::vec3(0.0f, 0.0f, 1.0f) });
  //vertices.push_back({ glm::vec3(0.5f, 0.5f, -0.7f),
  //                     glm::vec3(0.0f, 0.0f, 1.0f) });
  //vertices.push_back({ glm::vec3(0.5f, 1.0f, 0.0f),
  //                     glm::vec3(0.0f, 0.0f, 1.0f) });
  //
  //vertices.push_back({ glm::vec3(0.0f, 0.0f, 0.0f),
  //                     glm::vec3(0.0f, 1.0f, 0.0f) });
  //vertices.push_back({ glm::vec3(1.0f, 0.0f, 0.0f),
  //                     glm::vec3(0.0f, 1.0f, 0.0f) });
  //vertices.push_back({ glm::vec3(0.5f, 0.5f, -0.7f),
  //                     glm::vec3(0.0f, 1.0f, 0.0f) });
  //
  //vertices.push_back({ glm::vec3(0.5f, 0.5f, -0.7f),
  //                     glm::vec3(1.0f, 0.0f, 0.0f) });
  //vertices.push_back({ glm::vec3(1.0f, 0.0f, 0.0f),
  //                     glm::vec3(1.0f, 0.0f, 0.0f) });
  //vertices.push_back({ glm::vec3(0.5f, 1.0f, 0.0f),
  //                     glm::vec3(1.0f, 0.0f, 0.0f) });

  pointCloud.numVertices = 6;

  //indices = new GLushort[3]{ 0, 1, 2 };
  //numIndices = 3 * sizeof(indices[0]);
}

void ModelGenerator::loadModel()
{
  // Vertex Buffer
  GLuint vertexBufferObject;
  glGenBuffers(1, &vertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER, pointCloud.bufferSize(), &pointCloud.vertices[0], GL_DYNAMIC_DRAW);

  // Position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(pointCloud.vertices[0]), 0);

  // Color attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(pointCloud.vertices[0]),
                        (void *)(sizeof(pointCloud.vertices[0].position)));

  // Index Buffer (useful for vertex duplicates)
  //GLuint indexBufferObject;
  //glGenBuffers(1, &indexBufferObject);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices, indices, GL_STATIC_DRAW);
}