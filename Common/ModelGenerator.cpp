#include "ModelGenerator.h"

ModelGenerator::ModelGenerator() :
  WIDTH(0),
  HEIGHT(0)
{
}

ModelGenerator::ModelGenerator(int width, int height) :
  WIDTH(width),
  HEIGHT(height)
{
  pointCloud.vertices = std::vector<Vertex>(width * height);
}

ModelGenerator::~ModelGenerator()
{
}

int ModelGenerator::getNumVertices()
{
  return pointCloud.vertices.size();
}

PointCloud &ModelGenerator::getPointCloud()
{
  return pointCloud;
}

// Directly passes world coordinates
void ModelGenerator::updatePointCloud(PointCloud pointCloud)
{
  for (int i = 0; i < pointCloud.vertices.size(); i++) {
    this->pointCloud.vertices[i] = pointCloud.vertices[i];
  }
  this->pointCloud.numVertices = pointCloud.vertices.size();
  glBufferData(GL_ARRAY_BUFFER, this->pointCloud.bufferSize(), &this->pointCloud.vertices[0], GL_DYNAMIC_DRAW);
}

// World coordinates have to be recalculated, color data is used as received
void ModelGenerator::updatePointCloud(const UINT16 *depthBuffer, const BYTE *colorBuffer)
{
  int pointCloudIndex = 0;
  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      int depthIndex = (HEIGHT - 1 - y) * WIDTH + (WIDTH - 1 - x);
      float depth = depthBuffer[depthIndex] / 1000.0f;
      glm::vec3 worldCoordinate = glm::vec3(x, y, 1) * cameraParameters.depthIntrinsicInv * depth;

      if (isnan(worldCoordinate.z)) {
        pointCloud.vertices[pointCloudIndex] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) };
      } else {
        float r = colorBuffer[depthIndex * 3] / 255.0f;
        float g = colorBuffer[depthIndex * 3 + 1] / 255.0f;
        float b = colorBuffer[depthIndex * 3 + 2] / 255.0f;
        pointCloud.vertices[pointCloudIndex] = { glm::vec3(worldCoordinate.x, worldCoordinate.y, worldCoordinate.z), glm::vec3(r, g, b) };
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
