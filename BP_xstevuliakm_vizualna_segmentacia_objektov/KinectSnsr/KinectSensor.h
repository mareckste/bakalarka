#pragma once
#include <Kinect.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

class KinectSensor {
public:
    KinectSensor();
    ~KinectSensor();

    void initSensor();
    void mapDepthData() const;
    void getMappedColorData(cv::Mat& mat);
    void getMappedDepthData(cv::Mat& outputMat);
    void getColorData(cv::Mat& outputMat, cv::Mat& depthMat);
    int *getDepthMatrix() const;
private:
    // sensor
    IKinectSensor* m_sensor = nullptr;
    
    // frame source and reader 
    IMultiSourceFrame* m_multiSourceFrame       = nullptr;
    IMultiSourceFrameReader* m_multiSrcFrameRdr = nullptr;

    // mapper
    ICoordinateMapper* m_coordinateMapper = nullptr;
    
    // sizes of images retrieved from color and m_depth sensor
    int m_colorWidth    = 0;
    int m_colorHeight   = 0;
    int m_depthWidth    = 0;
    int m_depthHeight   = 0;

    // buffers for frame data
    UINT8* m_colorBuffer    = nullptr;
    UINT16* m_depthBuffer   = nullptr;

    // matrix with m_depth values
    int* m_depthMatrix = nullptr;

    // buffers for space point mappings 
    ColorSpacePoint* m_depthToRGBA = nullptr;
    DepthSpacePoint* m_rgbaToDepth = nullptr;
};

