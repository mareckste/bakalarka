/*________________________________________________________________________MAIN_RUNNER________________________________________________________________________*/
#include <iostream>
#include "DBScan.h"
#include "KinectSensor.h"
/************************************************************************************************/

int main(int argc, char** argv) {

    float scale = 0.4f;
    KinectSensor sensor;

    cv::Mat m(1080, 1920, CV_8UC3);
    cv::Mat m1(1080, 1920, CV_8SC3);
    cv::Mat m2(1080, 1920, CV_16UC1);
    cv::Mat m3(1080, 1920, CV_16UC1);
    cv::Mat segm(static_cast<int>(1080 * scale), static_cast<int>(1920 * scale), CV_16UC1);

    cv::Size size{static_cast<int>(1920 * scale), 
                static_cast<int>(1080 * scale) };
    
    while (true) {
        sensor.getColorData(m, m2);
       
        if (sensor.m_mapFlag == true) {                     // if frame obtained
            
            
            cv::resize(m, m1, size);                        // scale image
            cv::resize(m2, m3, size);
            
            DBScan dbs{ m1.rows, m1.cols };                 // init dbscan
          //m3.data = nullptr;                              // no depth

            dbs.convertToDataPoint(m1, m3);                 // convert data
            dbs.DBScanIteration(20, 5.0, 1700, 3);          // run segmentation
            dbs.saveSegmentation(segm);                     // save and label
            dbs.labelBorders(m1);

            break;
        }
    }

    cv::imshow("Segm image", m1);                           // draw result

    cv::waitKey(0);
	
    return 0;
}
/************************************************************************************************/
/*________________________________________________________________________MAIN_RUNNER________________________________________________________________________*/