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
	//cv::Mat image, imageCopy;

	//// load image
	//const std::string str = "C:\\Users\\stevu\\Documents\\Visual"
 //                               " Studio 2015\\Projects\\openCV_test\\openCV_test\\etc\\b0.jpg";
	//
	//image = cv::imread(str);
	//if (!image.data) {
	//	std::cout << "File not found" << std::endl;
	//	return 1;
	//}

 //   image.copyTo(imageCopy);

	//DBScan dbs{ image.rows, image.cols };
 //   
 //   //convert points to vector and run clustering
 //   dbs.convertToDataPoint(image);
 //   dbs.DBScanIteration(50, 150);

 //   std::cout << "CLUSTERING DONE: " << dbs.m_numClusters << " CLUSTERS" << std::endl;

 //   // label border pixels
 //   labelBorders(image, dbs.m_allPoints);

	//// show results
 //   cv::imshow("Original image", imageCopy);
	//cv::imshow("Segmented image", image);

    KinectSensor sensor;
    cv::Mat m(424, 512, CV_8UC3);

    while (true) {
        sensor.getColorData(m);
        cv::imshow("Original image", m);

        cv::waitKey(1);
    }
	return 0;
}