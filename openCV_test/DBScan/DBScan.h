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

class DBScan {
public:
	DBScan();
    ~DBScan();

	int m_imgCols;
	int m_imgRows;
	
    int m_minPoints;
    int m_numClusters {0};

  
	vector_t m_allPoints;
    vector_t m_labelledSet;
    vector_t m_candidateSet;


	vector_t convertToDataPoint(cv::Mat image, int rows, int cols);
	void DBScanIteration(std::vector<DataPoint*> points, double epsilon, unsigned int minPoints);


private:

	bool isInRadius(DataPoint* seed, DataPoint* center, DataPoint* potN, double epsilon) const;
    void assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t& vector, double epsilon);
    void regionQuery(DataPoint *seed, DataPoint *center, vector_t &neighbours, double epsilon);
	void expandCluster(DataPoint *investigated, vector_t neighbours,
						int cluster_id, double epsilon, unsigned int minPoints);
    vector_t mergeVectors(vector_t a, vector_t b) const;
    bool movePossible(int x, int y) const;

};

