#include "KinectSensor.h"

KinectSensor::KinectSensor() {
    initSensor();
}


KinectSensor::~KinectSensor() {

    if (m_depthToRGBA) delete[] m_depthToRGBA;

    if (m_coordinateMapper) m_coordinateMapper->Release();
    
    if (m_sensor) {
        m_sensor->Close();
        m_sensor->Release();
    }

    if (m_multiSrcFrameRdr) m_multiSrcFrameRdr->Release();
    
}

void KinectSensor::initSensor() {

    if (FAILED(GetDefaultKinectSensor(&m_sensor))) {
        std::cout << "Failed to init Kinect sensor" << std::endl;
        exit(1);
    }

    if (m_sensor) {
        m_sensor->get_CoordinateMapper(&m_coordinateMapper);
        
        m_sensor->Open();
        m_sensor->OpenMultiSourceFrameReader(FrameSourceTypes_Depth | FrameSourceTypes_Color, &m_multiSrcFrameRdr);
    }
    
}

void KinectSensor::getKinectDataFrames() {
    m_multiSrcFrameRdr->AcquireLatestFrame(&m_multiSourceFrame);

    if (m_multiSourceFrame) getDepthData();
}

void KinectSensor::getDepthData() {
    IDepthFrame *depthFrame = nullptr;
    IDepthFrameReference *depthFrameRef = nullptr;


    m_multiSourceFrame->get_DepthFrameReference(&depthFrameRef);
    depthFrameRef->AcquireFrame(&depthFrame);
    

    int height;
    int width;
    IFrameDescription *depthFrameDesc = nullptr;
    depthFrame->get_FrameDescription(&depthFrameDesc);
    depthFrameDesc->get_Height(&height);
    depthFrameDesc->get_Width(&width);
    m_depthToRGBA = new ColorSpacePoint[height * width];

    // process DF data
    unsigned int frameSize;
    UINT16 *buff;
    depthFrame->AccessUnderlyingBuffer(&frameSize, &buff);

    m_coordinateMapper->MapDepthFrameToColorSpace(width * height, buff, width * height, m_depthToRGBA);

    if (depthFrameRef) depthFrameRef->Release();
    if (depthFrame) depthFrame->Release();
}

cv::Mat* KinectSensor::getColorData() const {
    IColorFrame* colorFrame = nullptr;
    IColorFrameReference* colorFrameRef = nullptr;
    HRESULT hr;

    if (m_multiSourceFrame) {
        hr = m_multiSourceFrame->get_ColorFrameReference(&colorFrameRef);


        if (SUCCEEDED(hr)) {
            hr = colorFrameRef->AcquireFrame(&colorFrame);
        }

        if (colorFrameRef) colorFrameRef->Release();

        int height;
        int width;
        IFrameDescription* colorFrameDesc = nullptr;
        colorFrame->get_FrameDescription(&colorFrameDesc);
        colorFrameDesc->get_Height(&height);
        colorFrameDesc->get_Width(&width);

        BYTE* rgbImage = new BYTE[width * height * 4];
        colorFrame->CopyConvertedFrameDataToArray(width * height * 4, rgbImage, ColorImageFormat_Rgba);


        cv::Mat* outputMat = new cv::Mat(height, width, CV_8UC3);

        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                ColorSpacePoint p = m_depthToRGBA[i * width + j];

                if (p.X < 0 || p.Y < 0 || p.X > width || p.Y > height) {
                    outputMat->at<cv::Vec3b>(i, j)[2] = 255;
                    outputMat->at<cv::Vec3b>(i, j)[1] = 255;
                    outputMat->at<cv::Vec3b>(i, j)[0] = 255;
                }
                else {
                    int index = static_cast<int>(p.X) + width * static_cast<int>(p.Y);
                    outputMat->at<cv::Vec3b>(i, j)[2] = rgbImage[4 * index + 0];
                    outputMat->at<cv::Vec3b>(i, j)[1] = rgbImage[4 * index + 1];
                    outputMat->at<cv::Vec3b>(i, j)[0] = rgbImage[4 * index + 2];
                }
            }
        }
        if (rgbImage) delete[] rgbImage;
        if (colorFrame) colorFrame->Release();
        if (m_depthToRGBA) delete[] m_depthToRGBA;

        return outputMat;
    }
    return nullptr;
}
