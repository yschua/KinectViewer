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
  INT16 *colorDifferential;
  UINT16 *depthBuffer;
  INT16 *depthDifferential;
  INT16 *combinedDifferential;
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
  BYTE *getColorBufferReduced();
  UINT16 *getDepthBuffer();
  INT16 *getDepthDifferential();
  INT16 *getColorDifferential();
  INT16 *getCombinedDifferential();
  CameraSpacePoint *getCameraSpacePoints();
  ColorSpacePoint *getColorSpacePoints();
protected:
  void computeDepthDifferential();
  void computeColorDifferential();
  void computeCombinedDifferential();
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