#include <iostream>
#include "DBScan.h"
#include "KinectSensor.h"


void labelBorders(cv::Mat &image, vector_t vector) {
  for (int i = 0; i < image.rows; i++) {
    for (int j = 0; j < image.cols; j++) {
      if (vector[j+i*image.cols]->border == 1) {
        image.at<cv::Vec3b>(i, j)[0] = 204;
        image.at<cv::Vec3b>(i, j)[1] = 179;
        image.at<cv::Vec3b>(i, j)[2] = 51;
      }
    }
  }
}

int main(int argc, char** argv) {
	cv::Mat image, imagecopy;

//	 load image
	//const std::string str = "c:\\users\\stevu\\documents\\visual"
 //                               " studio 2015\\projects\\opencv_test\\opencv_test\\etc\\lion.jpg";
	//
	//image = cv::imread(str);
	//if (!image.data) {
	//	std::cout << "file not found" << std::endl;
	//	return 1;
	//}

 //   image.copyTo(imagecopy);

	//DBScan dbs{ image.rows, image.cols };
 //   
 //  // convert points to vector and run clustering
 //   dbs.convertToDataPoint(image);
 //   dbs.DBScanIteration(50, 150);

 //   std::cout << "clustering done: " << dbs.m_numClusters << " clusters" << std::endl;

 //   // label border pixels
 //   labelBorders(image, dbs.m_allPoints);

	//// show results
 //   cv::imshow("original image", imagecopy);
	//cv::imshow("segmented image", image);// 512 424

    KinectSensor sensor;
    cv::Mat m(1080, 1920, CV_8UC3);
    cv::Mat m1(1080, 1920, CV_8UC1);
    while (true) {
        sensor.getColorData(m, m1);
       // memcpy(m1.data, sensor.getDepthMatrix(), 1920 * 1080);
       // cv::imshow("Original image", m);
        //m.copyTo(m1);
       /* DBScan dbs{ m.rows, m.cols };
        dbs.convertToDataPoint(m);
        dbs.DBScanIteration(50, 150);
        labelBorders(m, dbs.m_allPoints);*/
        cv::imshow("Segm image", m);
        cv::imshow("Depth image", m1);

        cv::waitKey(2);
    }
	return 0;
}