#pragma once
#include "Primitives\CameraParameters.h"
#include <iostream>
#include <vector>
#include <Kinect.h>

class KinectCamera
{
  HRESULT hr;
  IKinectSensor *sensor;
  IMultiSourceFrameReader *reader;
  ICoordinateMapper *mapper;
  IMultiSourceFrame *frame;
  IColorFrameReference *colorFrameRef;
  IDepthFrameReference *depthFrameRef;
  IDepthFrame *depthFrame;
  IColorFrame *colorFrame;
  CameraSpacePoint *cameraSpacePoints;
  ColorSpacePoint *colorSpacePoints;
  CameraParameters cameraParameters;
  RGBQUAD *colorBuffer;
  BYTE *colorBufferReduced;
  UINT16 *depthBuffer;
public:
  const int MAX_DEPTH;
  const int COLOR_WIDTH;
  const int COLOR_HEIGHT;
  const int DEPTH_WIDTH;
  const int DEPTH_HEIGHT;
  const int DEPTH_SIZE;
  const int COLOR_SIZE;
public:
  KinectCamera();
  ~KinectCamera();
  void update();
  RGBQUAD *getColorBuffer();
  BYTE *getColorBufferReduced();
  UINT16 *getDepthBuffer();
  CameraSpacePoint *getCameraSpacePoints();
  ColorSpacePoint *getColorSpacePoints();
protected:
  static void checkError(HRESULT hr, char *name);
};

template<class Interface>
inline void SafeRelease(Interface *&pInterfaceToRelease)
{
  if (pInterfaceToRelease != NULL) {
    pInterfaceToRelease->Release();
    pInterfaceToRelease = NULL;
  }
}