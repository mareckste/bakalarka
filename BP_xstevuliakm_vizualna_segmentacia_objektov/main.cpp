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
	//cv::Mat image, imagecopy;

	//// load image
	//const std::string str = "C:\\Users\\stevu\\Disk Google\\skola\\6.semester"
 //   "\\BP2_INFO_B\\superpixel_image_segmentation"
 //   "\\BP_xstevuliakm_vizualna_segmentacia_objektov\\etc\\b1.jpg";
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
 //   dbs.convertToDataPoint(image, nullptr);
 //   dbs.DBScanIteration(50, 10000, 10, 3);

 //   // m_label m_border pixels
 //   labelBorders(image, dbs.m_allPoints);

	//// show results
 //   cv::imshow("original image", imagecopy);
	//cv::imshow("segmented image", image);// 512 424

    KinectSensor sensor;
    cv::Mat m(1080, 1920, CV_8UC3);
    cv::Size size{1920, 1080};
    cv::Mat m1(1080, 1920, CV_8SC3);
    double *depthBuff = new double[1920 * 1080];
    while (true) {
        sensor.getColorData(m, depthBuff);
       // memcpy(m1.data, sensor.getDepthMatrix(), 1920 * 1080);
       // cv::imshow("Original image", m);
        //m.copyTo(m1);
       /* DBScan dbs{ m.rows, m.cols };
        dbs.convertToDataPoint(m);
        dbs.DBScanIteration(50, 150);
        labelBorders(m, dbs.m_allPoints);*/
        if (sensor.m_mapFlag == true) {
         

            DBScan dbs{ m.rows, m.cols };
            printf("CLUSTERING\n");
            //cv::resize(m, m1, size);
            dbs.convertToDataPoint(m, depthBuff);
            dbs.DBScanIteration(50, 10.0, 500, 3);
            labelBorders(m, dbs.m_allPoints);
            
            
            delete[] depthBuff;
            break;
        }
        

    }

    cv::imshow("Segm image", m);

    cv::waitKey(0);
	return 0;
}