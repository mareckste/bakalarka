#pragma once

#include <Kinect.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <iterator> 
#include <string>
#include <cmath>
#include <algorithm>
#include "DataPoint.h"
#include "Cluster.h"

typedef std::vector<DataPoint *> vector_t;
typedef std::vector<Cluster *> cluster_vec_t;

class DBScan {
public:
	DBScan(int rows, int cols);
    ~DBScan();

	int m_imgCols;
	int m_imgRows;
	
    int m_minPoints;
    int m_numClusters;

  
	vector_t m_allPoints;
    cluster_vec_t m_allClusters;
    //vector_t m_candidateSet;


	vector_t convertToDataPoint(const cv::Mat& image);
	void DBScanIteration(vector_t points, double epsilon, unsigned int maxPoints);


private:

	bool isInRadius(DataPoint* seed, DataPoint* center, DataPoint* potN, double epsilon) const;
    void assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t& vector, double epsilon);
    void regionQuery(DataPoint *seed, DataPoint *center, vector_t &neighbours, double epsilon);
    bool movePossible(int x, int y) const;
    bool fromDifferentCluster(DataPoint* dp, int x, int y);
    bool checkBorder(DataPoint* pt);
    void setBorderPoints();

};

