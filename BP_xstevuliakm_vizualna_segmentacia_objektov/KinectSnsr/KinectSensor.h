/*________________________________________________________________________KINECT_SENSOR________________________________________________________________________*/
#pragma once
#include <Kinect.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

class KinectSensor {
    
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PUBLIC MEMBERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    KinectSensor();
    ~KinectSensor();

    /////////////////////////////////
    //variables
    /////////////////////////////////

    bool m_mapFlag = false;                                     // if frame obtained
    
    /////////////////////////////////
    //methods
    /////////////////////////////////

    void initSensor();
    void mapDepthData();
    void getMappedColorData(cv::Mat& mat);
    void getMappedDepthData(cv::Mat& outputMat);
    void getColorData(cv::Mat& outputMat, cv::Mat& depthMat);
    
private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PRIVATE MEMBERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    IKinectSensor* m_sensor = nullptr;                          // sensor
        
    IMultiSourceFrame* m_multiSourceFrame       = nullptr;      // frame source and reader 
    IMultiSourceFrameReader* m_multiSrcFrameRdr = nullptr;
        
    ICoordinateMapper* m_coordinateMapper = nullptr;            // mapper 
    
    int m_colorWidth    = 0;                                    // sizes of images retrieved from color and m_depth sensor
    int m_colorHeight   = 0;
    int m_depthWidth    = 0;
    int m_depthHeight   = 0;
    
    UINT8* m_colorBuffer    = nullptr;                          // buffers for frame data
    UINT16* m_depthBuffer   = nullptr;

    ColorSpacePoint* m_depthToRGBA = nullptr;                   // buffers for space point mappings 
    DepthSpacePoint* m_rgbaToDepth = nullptr;
};
/*________________________________________________________________________KINECT_SENSOR________________________________________________________________________*/
