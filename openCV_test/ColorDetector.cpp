#include "ColorDetector.h"



using namespace cv;
using namespace std;


ColorDetector::ColorDetector() : mindist(100) 
{
	target[0] = target[1] = target[2] = 0;
}


ColorDetector::~ColorDetector()
{
}

void ColorDetector::setColorDistanceTreshold(int distance) {
	if (distance < 0)
		distance = 0;
	mindist = distance;
}

int ColorDetector::getColorDistanceTreshold() const {
	return mindist;
}

void ColorDetector::setTargetColor(uchar b, uchar g, uchar r) {
	target[0] = b;
	target[1] = g;
	target[2] = r;
}

void ColorDetector::setTargetColor(Vec3b color) {
	target = color;
}

Vec3b ColorDetector::getTargetColor() const {
	return target;
}

int ColorDetector::getDistance(const Vec3b & color) const {
	//return (abs(color[0] - target[0]) + abs(color[1] - target[1]) + abs(color[2] - target[2])) ;

	return norm<int, 3>(Vec3i(abs(color[0] - target[0]),
		abs(color[1] - target[1]), abs(color[2] - target[2])));
}

Mat ColorDetector::colorDetectorProces(const Mat & image) {
	result.create(image.rows, image.cols, CV_8U);  // realloc, then same size but one channel

	Mat_<Vec3b>::const_iterator it = image.begin<Vec3b>();
	Mat_<Vec3b>::const_iterator itend = image.end<Vec3b>();
	Mat_<uchar>::iterator itout = result.begin<uchar>();


	for (; it != itend; it++, itout++) {
		if (getDistance(*it) < mindist) {
			*itout = 255;
		}
		else
			*itout = 0;
	}
	return result;
}

