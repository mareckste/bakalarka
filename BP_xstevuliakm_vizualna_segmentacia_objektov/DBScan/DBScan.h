#pragma once

#include <Kinect.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "Defs.h"


class DBScan {
public:
	DBScan(int rows, int cols);
    ~DBScan();
    
    // processed image size
	int m_imgCols;
	int m_imgRows;

    // number of clusters generated during the clustering phase
    int m_numClusters;

    // pixel and cluster representation over current image
	vector_t m_allPoints;
    cluster_vec_t m_allClusters;


	vector_t convertToDataPoint(const cv::Mat& color, const cv::Mat& depth);
	void DBScanIteration(double epsilon, unsigned int maxClusterPoints);

private:

	bool isInRadius(DataPoint* seed, DataPoint* center, DataPoint* potN, double epsilon) const;
    void assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t& vector, double epsilon) const;
    void regionQuery(DataPoint *seed, DataPoint *center, vector_t &neighbours, double epsilon);
    bool movePossible(int x, int y) const;
    bool fromDifferentCluster(DataPoint* dp, int x, int y);
    bool checkBorder(DataPoint* pt);
    void setBorderPoints();

};

