#include "DBScan/DBScan.h"

/* Sets pixels belonging to specific cluster m_id to be black c*/
void color_black(cv::Mat &image, vector_t vector) {
  // cluster m_id which members we would like to have black coloured
  
  
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
	cv::Mat image;

	/* Load image*/
	const std::string str = "C:\\Users\\stevu\\Documents\\Visual"
		" Studio 2015\\Projects\\openCV_test\\openCV_test\\etc\\b0.jpg";
	
	image = cv::imread(str);
	if (!image.data) {
		std::cout << "File not found" << std::endl;
		return 1;
	}

	DBScan dbs{ image.rows, image.cols };
  /* Convert points to vector*/
	vector_t vector = dbs.convertToDataPoint(image);
  
  /* Run clustering on the vector*/
    dbs.DBScanIteration(vector, 100, 2000);

    std::cout << "CLUSTERING DONE: " << dbs.m_numClusters << " CLUSTERS" << std::endl;

    color_black(image, vector);

	/* Declare windows to show image*/
	cv::namedWindow("Original image");
	cv::imshow("Original image", image);

	cv::waitKey(0);
	return 0;
}