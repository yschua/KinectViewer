#include "KinectCamera.h"

KinectCamera::KinectCamera() :
  MAX_DEPTH(4500), // maximum depth value to store in depth image
  COLOR_WIDTH(1920),
  COLOR_HEIGHT(1080),
  DEPTH_WIDTH(512),
  DEPTH_HEIGHT(424),
  DEPTH_SIZE(DEPTH_WIDTH * DEPTH_HEIGHT),
  COLOR_SIZE(COLOR_WIDTH * COLOR_HEIGHT)
{
  hr = GetDefaultKinectSensor(&sensor);
  checkError(hr, "GetDefaultKinectSensor");
  hr = sensor->Open();
  checkError(hr, "IKinectSensor::Open()");
  hr = sensor->OpenMultiSourceFrameReader(FrameSourceTypes_Color | FrameSourceTypes_Depth, &reader);
  checkError(hr, "IKinectSensor::OpenMultiSourceFrameReader()");
  hr = sensor->get_CoordinateMapper(&mapper);
  checkError(hr, "IKinectSensor::get_CoordinateMapper()");

  colorBuffer = new RGBQUAD[COLOR_SIZE];
  colorBufferReduced = new BYTE[DEPTH_SIZE * 3];
  colorDifferential = new INT16[DEPTH_SIZE * 3];

  depthBuffer = new UINT16[DEPTH_SIZE];
  depthDifferential = new INT16[DEPTH_SIZE];

  combinedFrame = new INT16[DEPTH_SIZE * 4];
  combinedDifferential = new INT16[DEPTH_SIZE * 4];

  //cameraSpacePoints = new CameraSpacePoint[DEPTH_WIDTH * DEPTH_HEIGHT];
  //colorSpacePoints = new ColorSpacePoint[DEPTH_WIDTH * DEPTH_HEIGHT];
}

KinectCamera::~KinectCamera()
{
  if (colorBuffer) {
    delete[] colorBuffer;
    colorBuffer = NULL;
  }
  if (colorBufferReduced) {
    delete[] colorBufferReduced;
    colorBufferReduced = NULL;
  }
  if (colorDifferential) {
    delete[] colorDifferential;
    colorDifferential = NULL;
  }
  if (depthBuffer) {
    delete[] depthBuffer;
    depthBuffer = NULL;
  }
  if (depthDifferential) {
    delete[] depthDifferential;
    depthDifferential = NULL;
  }
  if (combinedFrame) {
    delete[] combinedFrame;
    combinedFrame = NULL;
  }
  if (combinedDifferential) {
    delete[] combinedDifferential;
    combinedDifferential = NULL;
  }
  SafeRelease(reader);
  SafeRelease(mapper);
  if (sensor)
    sensor->Close();
  SafeRelease(sensor);
}

void KinectCamera::update()
{
  hr = reader->AcquireLatestFrame(&frame);
  if (SUCCEEDED(hr)) {
    UINT bufferSize;

    // Acquire frame reference
    hr = frame->get_DepthFrameReference(&depthFrameRef);
    checkError(hr, "IMultiSourceFrame::get_DepthFrameReference()");
    hr = frame->get_ColorFrameReference(&colorFrameRef);
    checkError(hr, "IMultiSourceFrame::get_ColorFrameReference()");
    SafeRelease(frame);

    // Acquire depth frame
    depthFrame = nullptr; // Maybe not required
    hr = depthFrameRef->AcquireFrame(&depthFrame);
    if (SUCCEEDED(hr)) {
      // Retrieve depth data
      bufferSize = DEPTH_WIDTH * DEPTH_HEIGHT;
      hr = depthFrame->CopyFrameDataToArray(bufferSize, depthBuffer);
      checkError(hr, "IDepthFrame::CopyFrameDataToArray()");
    }
    SafeRelease(depthFrame);
    SafeRelease(depthFrameRef);

    // Acquire color frame
    colorFrame = nullptr; // Maybe not required
    hr = colorFrameRef->AcquireFrame(&colorFrame);
    if (SUCCEEDED(hr)) {
      // Retrive color data
      bufferSize = COLOR_WIDTH * COLOR_HEIGHT * sizeof(RGBQUAD);
      hr = colorFrame->CopyConvertedFrameDataToArray(bufferSize, reinterpret_cast<BYTE*>(colorBuffer), ColorImageFormat_Bgra);
      checkError(hr, "IColorFrame::CopyConvertedFrameDataToArray()");
    }
    SafeRelease(colorFrame);
    SafeRelease(colorFrameRef);

    // Kinect SDK color/Depth coordinate mapping
    //bufferSize = DEPTH_WIDTH * DEPTH_HEIGHT;
    //mapper->MapDepthFrameToColorSpace(bufferSize, depthBuffer, bufferSize, colorSpacePoints);
    //mapper->MapDepthFrameToCameraSpace(bufferSize, depthBuffer, bufferSize, cameraSpacePoints);

    // Custom mapping
    // World coordinates are calculated and used to do depth/color mapping to
    // find the relevant color pixels, which are stored in colorBufferReduced
    for (int y = 0; y < DEPTH_HEIGHT; y++) {
      for (int x = 0; x < DEPTH_WIDTH; x++) {
        int depthIndex = y * DEPTH_WIDTH + x;
        float depth = depthBuffer[depthIndex] / 1000.0f;
        glm::vec3 worldCoordinate = glm::vec3(x, y, 1) * cameraParameters.depthIntrinsicInv * depth;
        glm::vec3 colorCoordinate = glm::vec3(glm::vec4(worldCoordinate, 1) * cameraParameters.depthToColor);
        glm::vec3 colorSpace = colorCoordinate * cameraParameters.colorIntrinsic / colorCoordinate.z;

        int colorX = static_cast<int>(std::floor(colorSpace.x + 0.5f));
        int colorY = static_cast<int>(std::floor(colorSpace.y + 0.5f));
        if ((colorX >= 0) && (colorX < COLOR_WIDTH) && (colorY >= 0) && (colorY < COLOR_HEIGHT)) {
          RGBQUAD color = colorBuffer[(colorY * COLOR_WIDTH + colorX)];
          colorBufferReduced[depthIndex * 3] = color.rgbRed;
          colorBufferReduced[depthIndex * 3 + 1] = color.rgbGreen;
          colorBufferReduced[depthIndex * 3 + 2] = color.rgbBlue;
        } else {
          colorBufferReduced[depthIndex * 3] = 128;
          colorBufferReduced[depthIndex * 3 + 1] = 128;
          colorBufferReduced[depthIndex * 3 + 2] = 128;
        }
      }
    }
  }
  computeCombinedFrame();
  computeCombinedDifferential();
}

RGBQUAD *KinectCamera::getColorBuffer()
{
  return colorBuffer;
}

BYTE *KinectCamera::getColorBufferReduced()
{
  return colorBufferReduced;
}

UINT16 *KinectCamera::getDepthBuffer()
{
  return depthBuffer;
}



INT16 *KinectCamera::getCombinedFrame()
{
  return combinedFrame;
}

INT16 *KinectCamera::getCombinedDifferential()
{
  return combinedDifferential;
}

//CameraSpacePoint *KinectCamera::getCameraSpacePoints()
//{
//  return cameraSpacePoints;
//}
//
//ColorSpacePoint *KinectCamera::getColorSpacePoints()
//{
//  return colorSpacePoints;
//}



void KinectCamera::computeCombinedFrame()
{
  for (int i = 0; i < DEPTH_SIZE; i++) {
    combinedFrame[i] = depthBuffer[i];
  }
  for (int i = 0; i < DEPTH_SIZE * 3; i++) {
    combinedFrame[i + DEPTH_SIZE] = colorBufferReduced[i];
  }
}

void KinectCamera::computeCombinedDifferential()
{
  combinedDifferential[0] = colorBufferReduced[0];
  for (int i = 1; i < DEPTH_SIZE * 3; i++) {
    combinedDifferential[i] = colorBufferReduced[i] - colorBufferReduced[i - 1];
  }
  combinedDifferential[DEPTH_SIZE * 3] = min(depthBuffer[0], MAX_DEPTH) - colorBufferReduced[DEPTH_SIZE * 3 - 1];
  for (int i = 1; i < DEPTH_SIZE; i++) {
    combinedDifferential[DEPTH_SIZE * 3 + i] = (min(depthBuffer[i], MAX_DEPTH) - min(depthBuffer[i - 1], MAX_DEPTH));
  }
}

void KinectCamera::checkError(HRESULT hr, char *name)
{
  if (FAILED(hr))
    std::cerr << "Error: " << name << std::endl;
}