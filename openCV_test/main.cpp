#include "DBScan/DBScan.h"

/* Sets pixels belonging to specific cluster id to be black c*/
void color_black(cv::Mat &image, vector_t vector) {
  // cluster id which members we would like to have black coloured
  int id = 1;
  
  for (int i = 0; i < image.rows; i++) {
    for (int j = 0; j < image.cols; j++) {
      if (vector[j+i*image.cols]->clusterId == id) {
        image.at<cv::Vec3b>(i, j)[0] = 0;
        image.at<cv::Vec3b>(i, j)[1] = 0; 
        image.at<cv::Vec3b>(i, j)[2] = 0;
      }
    }
  }
}



int main(int argc, char** argv) {
	cv::Mat image;

	/* Load image*/
	const std::string str = "C:\\Users\\stevu\\Documents\\Visual"
		" Studio 2015\\Projects\\openCV_test\\openCV_test\\etc\\small.png";
	
	image = cv::imread(str);
	if (!image.data) {
		std::cout << "File not found" << std::endl;
		return 1;
	}

	DBScan dbs;
  /* Convert points to vector*/
	vector_t vector = dbs.convertToDataPoint(image, image.rows, image.cols);
  
  /* Run clustering on the vector*/
    dbs.DBScanIteration(vector, 20, 7);

  /* Label specific cluster*/
    color_black(image, vector);

	/* Declare windows to show image*/
	cv::namedWindow("Original image");
	cv::imshow("Original image", image);

	cv::waitKey(0);
	return 0;
}