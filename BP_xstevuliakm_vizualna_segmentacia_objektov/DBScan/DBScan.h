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


	vector_t convertToDataPoint(const cv::Mat& color, const double* depth);
	void DBScanIteration(double epsilon, double depthThreshold, unsigned int numOfClusters);

private:

	bool isInRadius(DataPoint* seed, DataPoint* center, DataPoint* potN, double epsilon, double depthThreshold) const;
    void assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t& neighbours, double epsilon, double depthThreshold) const;
    void regionQuery(DataPoint* seed, DataPoint* center, vector_t& neighbours, double epsilon, double depthThreshold);
    bool movePossible(int x, int y) const;
    DataPoint* getPointAt(int x, int y);
    Cluster* getClusterAt(int pos);
    bool fromDifferentCluster(DataPoint* dp, int x, int y);
    bool diffNeighbour(int x, int y, DataPoint* p);
    bool checkBorder(DataPoint* pt);
    void setBorderPoints();
    static bool hasAvgValues(const Cluster*const cluster);
    void computeMergingDistance(const Cluster*const currClust, int nX, int nY, double* min, Cluster** minc);
    static void mergeClusters(Cluster* cluster, Cluster* minc);
    void DBSmerge(unsigned int minClusterPoints);

};

