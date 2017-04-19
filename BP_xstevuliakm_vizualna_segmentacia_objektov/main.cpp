#include <iostream>
#include "DBScan.h"
#include "KinectSensor.h"


void labelBorders(cv::Mat &image, vector_t vector) {
  for (int i = 0; i < image.rows; i++) {
    for (int j = 0; j < image.cols; j++) {
      if (vector[j+i*image.cols]->m_border == 1) {
        image.at<cv::Vec3b>(i, j)[0] = 204;
        image.at<cv::Vec3b>(i, j)[1] = 179;
        image.at<cv::Vec3b>(i, j)[2] = 51;
      }
    }
  }
}

int main(int argc, char** argv) {

    float scale = 0.4;
    KinectSensor sensor;

    cv::Mat m(1080, 1920, CV_8UC3);
    cv::Mat m1(1080, 1920, CV_8SC3);
    cv::Mat m2(1080, 1920, CV_16UC1);
    cv::Mat m3(1080, 1920, CV_16UC1);

    cv::Size size{static_cast<int>(1920 * scale), 
                static_cast<int>(1080 * scale) };
    
    while (true) {
        sensor.getColorData(m, m2);
       
        if (sensor.m_mapFlag == true) {
            cv::resize(m, m1, size);
            cv::resize(m2, m3, size);

            DBScan dbs{ m1.rows, m1.cols };

            dbs.convertToDataPoint(m1, m3);
            dbs.DBScanIteration(100, 5.0, 500, 4);

            labelBorders(m1, dbs.m_allPoints);
            break;
        }
    }

    cv::imshow("Segm image", m1);

    cv::waitKey(0);
	
    return 0;
}