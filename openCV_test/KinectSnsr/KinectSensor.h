#pragma once
#include <Kinect.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <memory>
#include <vector>

#include <cstdlib>
#include <cstdio>
#include <cstring>
class KinectSensor {
public:
    KinectSensor();
    ~KinectSensor();

    void initSensor();
    void getKinectDataFrames();
    void getDepthData();
    void getColorData(cv::Mat& mat);

private:
    IKinectSensor* m_sensor = nullptr;
    ICoordinateMapper* m_coordinateMapper = nullptr;
   // IDepthFrameReader* m_dReader = nullptr;
   // IColorFrameReader* m_cReader = nullptr;
    IMultiSourceFrame* m_multiSourceFrame = nullptr;
    IMultiSourceFrameReader* m_multiSrcFrameRdr = nullptr;

   // uint32_t* m_colorBuffer = nullptr;
   // uint16_t *m_depthBuffer = nullptr;

    int m_colorWidth, m_colorHeight, m_depthWidth, m_deptHeight;
    BYTE* m_colorBuffer = nullptr;
    ColorSpacePoint* m_depthToRGBA = nullptr;
};

