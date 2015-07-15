#include "KinectCamera.h"

KinectCamera::KinectCamera() :
  COLOR_WIDTH(1920),
  COLOR_HEIGHT(1080),
  DEPTH_WIDTH(512),
  DEPTH_HEIGHT(424)
{
  hr = GetDefaultKinectSensor(&sensor);
  checkError(hr, "GetDefaultKinectSensor");
  hr = sensor->Open();
  checkError(hr, "IKinectSensor::Open()");
  hr = sensor->OpenMultiSourceFrameReader(FrameSourceTypes_Color | FrameSourceTypes_Depth, &reader);
  checkError(hr, "IKinectSensor::OpenMultiSourceFrameReader()");
  hr = sensor->get_CoordinateMapper(&mapper);
  checkError(hr, "IKinectSensor::get_CoordinateMapper()");

  colorBuffer = new RGBQUAD[COLOR_WIDTH * COLOR_HEIGHT];
  depthBuffer = new UINT16[DEPTH_WIDTH * DEPTH_HEIGHT];
  depthDifferential = new INT16[DEPTH_WIDTH * DEPTH_HEIGHT]; // this can overwrite depthBuffer
  cameraSpacePoints = new CameraSpacePoint[DEPTH_WIDTH * DEPTH_HEIGHT];
  colorSpacePoints = new ColorSpacePoint[DEPTH_WIDTH * DEPTH_HEIGHT];
}

KinectCamera::~KinectCamera()
{
  if (colorBuffer) {
    delete[] colorBuffer;
    colorBuffer = NULL;
  }
  if (depthBuffer) {
    delete[] depthBuffer;
    depthBuffer = NULL;
  }
  if (cameraSpacePoints) {
    delete[] cameraSpacePoints;
    cameraSpacePoints = NULL;
  }
  if (colorSpacePoints) {
    delete[] colorSpacePoints;
    colorSpacePoints = NULL;
  }
  SafeRelease(reader);
  SafeRelease(mapper);
  if (sensor)
    sensor->Close();
  SafeRelease(sensor);
}

CameraSpacePoint *customCamSpcPt = new CameraSpacePoint[512 * 424];

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
      //hr = depthFrame->AcessUnderlyingBuffer(&bufferSize, &depthBuffer);
      bufferSize = DEPTH_WIDTH * DEPTH_HEIGHT;
      hr = depthFrame->CopyFrameDataToArray(bufferSize, depthBuffer);
      checkError(hr, "IDepthFrame::CopyFrameDataToArray()");
    }
    computeDepthDifferential();
    SafeRelease(depthFrame);
    SafeRelease(depthFrameRef);

    // Acquire color frame
    colorFrame = nullptr; // Maybe not required
    hr = colorFrameRef->AcquireFrame(&colorFrame);
    if (SUCCEEDED(hr)) {
      // Retrive color data
      // TODO: Attempt to use AccessRawUnderlyingBuffer for color data
      //hr = colorFrame->AccessRawUnderlyingBuffer(&bufferSize, reinterpret_cast<BYTE**>(&colorBuffer));
      bufferSize = COLOR_WIDTH * COLOR_HEIGHT * sizeof(RGBQUAD);
      hr = colorFrame->CopyConvertedFrameDataToArray(bufferSize, reinterpret_cast<BYTE*>(colorBuffer), ColorImageFormat_Bgra);
      checkError(hr, "IColorFrame::CopyConvertedFrameDataToArray()");
    }
    SafeRelease(colorFrame);
    SafeRelease(colorFrameRef);

    // Color/Depth coordinate mapping
    bufferSize = DEPTH_WIDTH * DEPTH_HEIGHT;
    mapper->MapDepthFrameToColorSpace(bufferSize, depthBuffer, bufferSize, colorSpacePoints);
    //mapper->MapDepthFrameToCameraSpace(bufferSize, depthBuffer, bufferSize, cameraSpacePoints);

    // Custom mapping
    for (int y = 0; y < DEPTH_HEIGHT; y++) {
      for (int x = 0; x < DEPTH_WIDTH; x++) {
        int depthIndex = y * DEPTH_WIDTH + x;
        float depth = depthBuffer[depthIndex] / 1000.0f;
        glm::vec3 worldCoordinate = glm::vec3(x, y, 1) * cameraParameters.depthIntrinsicInv * depth;
        cameraSpacePoints[depthIndex] = { worldCoordinate.x, -worldCoordinate.y, worldCoordinate.z };
      }
    }
  }


  if (depthBuffer[100] != 0) {
    std::cout << "Ready" << std::endl;
  }
}

RGBQUAD *KinectCamera::getColorBuffer()
{
  return colorBuffer;
}

UINT16 *KinectCamera::getDepthBuffer()
{
  return depthBuffer;
}

INT16 *KinectCamera::getDepthDifferential()
{
  return depthDifferential;
}

CameraSpacePoint *KinectCamera::getCameraSpacePoints()
{
  return cameraSpacePoints;
}

ColorSpacePoint *KinectCamera::getColorSpacePoints()
{
  return colorSpacePoints;
}

void KinectCamera::computeDepthDifferential()
{
  depthDifferential[0] = depthBuffer[0];
  for (int i = 1; i < DEPTH_HEIGHT * DEPTH_WIDTH; i++) {
    depthDifferential[i] = (depthBuffer[i] - depthBuffer[i - 1]);
  }
}

void KinectCamera::checkError(HRESULT hr, char *name)
{
  if (FAILED(hr))
    std::cerr << "Error: " << name << std::endl;
}