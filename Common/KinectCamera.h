#pragma once
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
  RGBQUAD *colorBuffer;
  UINT16 *depthBuffer;
  INT16 *depthDifferential;
public:
  const int COLOR_WIDTH;
  const int COLOR_HEIGHT;
  const int DEPTH_WIDTH;
  const int DEPTH_HEIGHT;
public:
  KinectCamera();
  ~KinectCamera();
  void update();
  RGBQUAD *getColorBuffer();
  UINT16 *getDepthBuffer();
  INT16 *getDepthDifferential();
  CameraSpacePoint *getCameraSpacePoints();
  ColorSpacePoint *getColorSpacePoints();
protected:
  void computeDepthDifferential();
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