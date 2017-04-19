#include "KinectSensor.h"
#include <iostream>
#include <memory>
#include <vector>


KinectSensor::KinectSensor() {
    initSensor();
}

/*
 * Destructs all member arrays used during the frame processing stage
 */
KinectSensor::~KinectSensor() {

    if (m_coordinateMapper) m_coordinateMapper->Release();
    
    if (m_sensor) {
        m_sensor->Close();
        m_sensor->Release();
    }

    if (m_multiSrcFrameRdr) m_multiSrcFrameRdr->Release();
    if (m_depthToRGBA) delete[] m_depthToRGBA;
    if (m_colorBuffer) delete[] m_colorBuffer;
    if (m_depthBuffer) delete[] m_depthBuffer;
    if (m_rgbaToDepth) delete[] m_rgbaToDepth;
    if (m_depthMatrix) delete[] m_depthMatrix;
}

/*
 * Initializes Kinect sensor and the class member buffers
 */
void KinectSensor::initSensor() {
    short successFlag = 0;

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
    IFrameDescription* desc;
    
    if (
        SUCCEEDED(m_sensor->get_ColorFrameSource(&colorFrameSource)) && 
        SUCCEEDED(colorFrameSource->get_FrameDescription(&desc))
        ) {

        desc->get_Width(&m_colorWidth);
        desc->get_Height(&m_colorHeight);

        if (desc) desc->Release();
        if (colorFrameSource) colorFrameSource->Release();
        successFlag++;
    }

    IDepthFrameSource* depthFrameSource;
    
    if (
        SUCCEEDED(m_sensor->get_DepthFrameSource(&depthFrameSource)) && 
        SUCCEEDED(depthFrameSource->get_FrameDescription(&desc))
        ) {
        
        desc->get_Width(&m_depthWidth);
        desc->get_Height(&m_depthHeight);

        if (desc) desc->Release();
        if (depthFrameSource) depthFrameSource->Release();
        successFlag++;
    }

    if (successFlag > 1) {
        m_depthToRGBA = new ColorSpacePoint[m_depthWidth * m_depthHeight];
        m_rgbaToDepth = new DepthSpacePoint[m_colorWidth * m_colorHeight];
        m_colorBuffer = new BYTE[m_colorWidth * m_colorHeight * 4];
        m_depthBuffer = new UINT16[m_depthWidth * m_depthHeight];
        m_depthMatrix = new int[m_colorWidth * m_colorHeight];

        if (m_depthToRGBA && m_rgbaToDepth && m_colorBuffer && m_depthBuffer) {
            std::cout << "Initialization successfull\nColor resolution = " << m_colorWidth << " x " << m_colorHeight <<
                "\nDepth resolution = " << m_depthWidth << " x " << m_depthHeight << std::endl;
        }
        else {
            std::cout << "Memory init FAILED" << std::endl;
            exit(2);
        }
    }
    else {
        std::cout << "Init process failed" << std::endl;
        exit(3);
    }
}

/*
 * Initializes coordinate mapper and space point arrays to map 
 * back and forth between m_depth and color
 */
void KinectSensor::mapDepthData() {
    IDepthFrame *depthFrame = nullptr;
    IDepthFrameReference *depthFrameRef = nullptr;

    if (
        SUCCEEDED(m_multiSourceFrame->get_DepthFrameReference(&depthFrameRef)) &&
        SUCCEEDED(depthFrameRef->AcquireFrame(&depthFrame))
        ) {
        
        m_mapFlag = true;

        depthFrame->CopyFrameDataToArray(m_depthWidth * m_depthHeight,  m_depthBuffer);

        m_coordinateMapper->MapDepthFrameToColorSpace(
            m_depthWidth * m_depthHeight,
            m_depthBuffer,
            m_depthWidth * m_depthHeight,
            m_depthToRGBA
        );
        
        m_coordinateMapper->MapColorFrameToDepthSpace(
            m_depthWidth * m_depthHeight,
            m_depthBuffer,
            m_colorWidth * m_colorHeight,
            m_rgbaToDepth
        );

        if (depthFrameRef) depthFrameRef->Release();
        if (depthFrame) depthFrame->Release();
    }
}

/*
 * Maps depthframe to colorspace 
 * and initializes the cv Matrix with color values
 */
void KinectSensor::getMappedColorData(cv::Mat& outputMat) {
    IColorFrame* colorFrame = nullptr;
    IColorFrameReference* colorFrameRef = nullptr;

    if (SUCCEEDED(m_multiSrcFrameRdr->AcquireLatestFrame(&m_multiSourceFrame))) {
        if (m_multiSourceFrame) {
            mapDepthData();

            if (
                SUCCEEDED(m_multiSourceFrame->get_ColorFrameReference(&colorFrameRef)) &&
                SUCCEEDED(colorFrameRef->AcquireFrame(&colorFrame)) &&
                SUCCEEDED(colorFrame->CopyConvertedFrameDataToArray(m_colorWidth * m_colorHeight * 4,
                    m_colorBuffer, ColorImageFormat_Bgra))
                ) {

                for (int i = 0; i < m_depthHeight; ++i) {
                    for (int j = 0; j < m_depthWidth; ++j) {
                        ColorSpacePoint p = m_depthToRGBA[i * m_depthWidth + j];

                        if (p.X < 0 || p.Y < 0 || p.X > m_colorWidth || p.Y > m_colorHeight) {
                            outputMat.at<cv::Vec3b>(i, j)[2] = 255;
                            outputMat.at<cv::Vec3b>(i, j)[1] = 255;
                            outputMat.at<cv::Vec3b>(i, j)[0] = 255;
                        }
                        else {
                            int index = static_cast<int>(p.X) + m_colorWidth * static_cast<int>(p.Y);
                            outputMat.at<cv::Vec3b>(i, j)[2] = m_colorBuffer[4 * index + 2];
                            outputMat.at<cv::Vec3b>(i, j)[1] = m_colorBuffer[4 * index + 1];
                            outputMat.at<cv::Vec3b>(i, j)[0] = m_colorBuffer[4 * index];
                        }
                    }
                }

                if (colorFrameRef) colorFrameRef->Release();
                if (colorFrame) colorFrame->Release();
                if (m_multiSourceFrame) m_multiSourceFrame->Release();
            }
        }
    }
}

/*
 * Maps color frame to m_depth space
 * and initializes the cv Matrix with color values
 */
void KinectSensor::getMappedDepthData(cv::Mat& outputMat) {
    IDepthFrame* depthFrame = nullptr;
    IDepthFrameReference* depthFrameRef = nullptr;

    if (SUCCEEDED(m_multiSrcFrameRdr->AcquireLatestFrame(&m_multiSourceFrame))) {
        if (m_multiSourceFrame) {
            mapDepthData();

            if (
                SUCCEEDED(m_multiSourceFrame->get_DepthFrameReference(&depthFrameRef)) &&
                SUCCEEDED(depthFrameRef->AcquireFrame(&depthFrame))
                )

                for (int i = 0; i < m_colorHeight; ++i) {
                    for (int j = 0; j < m_colorWidth; ++j) {
                        DepthSpacePoint p = m_rgbaToDepth[i * m_colorWidth + j];

                        if (p.X < 0 || p.Y < 0 || p.X > m_depthWidth || p.Y > m_depthHeight) {
                            outputMat.at<cv::Vec3b>(i, j)[2] = 255;
                            outputMat.at<cv::Vec3b>(i, j)[1] = 255;
                            outputMat.at<cv::Vec3b>(i, j)[0] = 255;
                        }
                        else {
                            int index = static_cast<int>(p.X + 0.5f) + m_depthWidth * static_cast<int>(p.Y + 0.5f);
                            outputMat.at<cv::Vec3b>(i, j)[2] = static_cast<BYTE> (255 * (m_depthBuffer[index] - 0) / 3200.0);
                            outputMat.at<cv::Vec3b>(i, j)[1] = static_cast<BYTE> (255 * (m_depthBuffer[index] - 0) / 3200.0);
                            outputMat.at<cv::Vec3b>(i, j)[0] = static_cast<BYTE> (255 * (m_depthBuffer[index] - 0) / 3200.0);
                        }
                    }
                }
            if (depthFrameRef) depthFrameRef->Release();
            if (depthFrame) depthFrame->Release();
            if (m_multiSourceFrame) m_multiSourceFrame->Release();
        }
    }
}

void KinectSensor::getColorData(cv::Mat& outputMat, cv::Mat& depthMat) {
    IColorFrame* colorFrame = nullptr;
    IColorFrameReference* colorFrameRef = nullptr;

    if (SUCCEEDED(m_multiSrcFrameRdr->AcquireLatestFrame(&m_multiSourceFrame))) {
        if (m_multiSourceFrame) {
            mapDepthData();

            if (
                SUCCEEDED(m_multiSourceFrame->get_ColorFrameReference(&colorFrameRef)) &&
                SUCCEEDED(colorFrameRef->AcquireFrame(&colorFrame)) &&
                SUCCEEDED(colorFrame->CopyConvertedFrameDataToArray(m_colorWidth * m_colorHeight * 4,
                    m_colorBuffer, ColorImageFormat_Bgra))
                ) {

                for (int i = 0; i < m_colorHeight; ++i) {
                    for (int j = 0; j < m_colorWidth; ++j) {
                        DepthSpacePoint p = m_rgbaToDepth[i * m_colorWidth + j];

                        if (p.X < 0 || p.Y < 0 || p.X > m_depthWidth || p.Y > m_depthHeight) {
                            //depthMat.data[i * m_colorWidth + j] = -1;
                            depthMat.at<UINT16>(i, j) = -1;
                        }
                        else {
                            int index = static_cast<int>(p.X + 0.5f) + m_depthWidth * static_cast<int>(p.Y + 0.5f);
                            // m_depthMatrix[i * m_colorWidth + j] = static_cast<int> (255 * (m_depthBuffer[index] - 0) / 3200.0);
                            //depthMat.data[i * m_colorWidth + j] = m_depthBuffer[index];
                            depthMat.at<UINT16>(i, j) = m_depthBuffer[index];
                        }

                        outputMat.at<cv::Vec3b>(i, j)[0] = m_colorBuffer[4 * (i * m_colorWidth + j) + 0];
                        outputMat.at<cv::Vec3b>(i, j)[1] = m_colorBuffer[4 * (i * m_colorWidth + j) + 1];
                        outputMat.at<cv::Vec3b>(i, j)[2] = m_colorBuffer[4 * (i * m_colorWidth + j) + 2];
                    }
                }

                if (colorFrameRef) colorFrameRef->Release();
                if (colorFrame) colorFrame->Release();
                if (m_multiSourceFrame) m_multiSourceFrame->Release();
            }
        }
    }
}

int* KinectSensor::getDepthMatrix() const {
    if (m_depthMatrix != nullptr)
        return m_depthMatrix;
    
    return nullptr;
}
