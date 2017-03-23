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
    if (m_depthToRGBA) delete[] m_depthToRGBA;
    if (m_colorBuffer) delete[] m_colorBuffer;
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

    IColorFrameSource* colorFrameSource;
    m_sensor->get_ColorFrameSource(&colorFrameSource);

    IFrameDescription* desc;
    colorFrameSource->get_FrameDescription(&desc);
    
    desc->get_Width(&m_colorWidth);
    desc->get_Height(&m_colorHeight);

    if (desc) desc->Release();

    IDepthFrameSource* depthFrameSource;
    m_sensor->get_DepthFrameSource(&depthFrameSource);

    depthFrameSource->get_FrameDescription(&desc);

    desc->get_Width(&m_depthWidth);
    desc->get_Height(&m_deptHeight);

    if (desc) desc->Release();
    if (colorFrameSource) colorFrameSource->Release();
    if (depthFrameSource) depthFrameSource->Release();

    m_depthToRGBA = new ColorSpacePoint[m_depthWidth * m_deptHeight];
    m_colorBuffer = new BYTE[m_colorWidth * m_colorHeight * 4];

    std::cout << "Initialization successfull\n Color resolution = " << m_colorWidth << " x " << m_colorHeight <<
        "\nDepth resolution = " << m_depthWidth << " x " << m_deptHeight << std::endl;
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
    if (depthFrame == nullptr) return;

    // process DF data
    unsigned int frameSize;
    UINT16 *buff;
    depthFrame->AccessUnderlyingBuffer(&frameSize, &buff);

    m_coordinateMapper->MapDepthFrameToColorSpace(
        m_depthWidth * m_deptHeight, 
        buff,
        m_depthWidth * m_deptHeight,
        m_depthToRGBA
    );

    if (depthFrameRef) depthFrameRef->Release();
    if (depthFrame) depthFrame->Release();
}

void KinectSensor::getColorData(cv::Mat& outputMat) {
    IColorFrame* colorFrame = nullptr;
    IColorFrameReference* colorFrameRef = nullptr;
    HRESULT hr;

    m_multiSrcFrameRdr->AcquireLatestFrame(&m_multiSourceFrame);

    if (m_multiSourceFrame) {
        getDepthData();

        hr = m_multiSourceFrame->get_ColorFrameReference(&colorFrameRef);

        if (SUCCEEDED(hr)) {
            hr = colorFrameRef->AcquireFrame(&colorFrame);
        }

        if (colorFrameRef) colorFrameRef->Release();

        colorFrame->CopyConvertedFrameDataToArray(m_colorWidth * m_colorHeight * 4, m_colorBuffer, ColorImageFormat_Bgra);
        
        for (int i = 0; i < m_deptHeight; ++i) {
            for (int j = 0; j < m_depthWidth; ++j) {
                ColorSpacePoint p = m_depthToRGBA[i * m_depthWidth + j];

                if (p.X < 0 || p.Y < 0 || p.X >= m_colorWidth || p.Y >= m_colorHeight) {
                    outputMat.at<cv::Vec3b>(i, j)[2] = 255;
                    outputMat.at<cv::Vec3b>(i, j)[1] = 255;
                    outputMat.at<cv::Vec3b>(i, j)[0] = 255;
                }
                else {
                    int index = static_cast<int>(p.X) + m_colorWidth * static_cast<int>(p.Y);
                    outputMat.at<cv::Vec3b>(i, j)[2] = m_colorBuffer[4 * index + 2];
                    outputMat.at<cv::Vec3b>(i, j)[1] = m_colorBuffer[4 * index + 1];
                    outputMat.at<cv::Vec3b>(i, j)[0] = m_colorBuffer[4 * index ];
                }
                
            }
        }
        //if (rgbImage) delete[] rgbImage;
        if (colorFrame) colorFrame->Release();
        if (m_multiSourceFrame) m_multiSourceFrame->Release();
    }
}
