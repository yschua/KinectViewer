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
}