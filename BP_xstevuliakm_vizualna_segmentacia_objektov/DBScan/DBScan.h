/*________________________________________________________________________3D_DBSCAN________________________________________________________________________*/
#pragma once

#include <Kinect.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "Defs.h"

class DBScan {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PUBLIC MEMBERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DBScan(int rows, int cols);
    ~DBScan();

	void convertToDataPoint(const cv::Mat& color, const cv::Mat& depth);          //converting data and segmentation
	void DBScanIteration(double epsilon, double depthThreshold, unsigned int numOfClusters, unsigned int mergingFactor);
    void labelBorders(cv::Mat& image);
    void saveSegmentation(cv::Mat& image);
    
private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PRIVATE MEMBERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////
    //variables
    /////////////////////////////////

    int m_imgCols;                                                               // image size
    int m_imgRows;

    int m_numClusters;                                                           // num of superpixels

    vector_t m_allPoints;                                                        // all pixels and superpixels
    cluster_vec_t m_allClusters;

    /////////////////////////////////
    //methods
    /////////////////////////////////

    void DBSmerge(unsigned int minClusterPoints, int* size, int numOfClusters, int ppFlag);
	bool isInRadius(DataPoint* seed, DataPoint* center, DataPoint* potN, double epsilon, double depthThreshold) const;
    void assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t* neighbours, double epsilon, double depthThreshold) const;
    void regionQuery(DataPoint* seed, DataPoint* center, vector_t* neighbours, double epsilon, double depthThreshold);
    bool movePossible(int x, int y) const;
    DataPoint* getPointAt(int x, int y);
    bool fromDifferentCluster(DataPoint* dp, int x, int y);
    bool diffNeighbour(int x, int y, DataPoint* p);
    bool checkBorder(DataPoint* pt);
    void setBorderPoints();
    static bool hasAvgValues(const Cluster*const cluster);
    void computeMergingDistance(const Cluster*const currClust, int nX, int nY, double* min, Cluster** minc);
    static void mergeClusters(Cluster* cluster, Cluster* minc);
    void getLargestNeighbour(Cluster* cluster, int i, int m_y, double* min_dist, Cluster** minc);
    void relabelPts();
};
/*________________________________________________________________________3D_DBSCAN________________________________________________________________________*/