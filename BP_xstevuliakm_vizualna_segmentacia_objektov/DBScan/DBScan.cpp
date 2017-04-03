#include "DBScan.h"
#include "Cluster.h"
#include <ctime>
#include <chrono>
#include <iostream>

/*
 * Inits the class by the size of the color
 */
DBScan::DBScan(int rows, int cols)
    :m_imgCols{ cols }, m_imgRows{ rows }, m_numClusters {1}
{
}


DBScan::~DBScan() {
   for (auto dp : m_allPoints)
        delete dp;

    for (auto cs : m_allClusters)
       delete cs;
}

/*
 * Converts cv color data to vector of pointers to DataPoint
 * objects stored in current DBScan class  
 */
std::vector<DataPoint*> DBScan::convertToDataPoint(const cv::Mat& color, const double *depth) {
	
    for (auto i = 0; i < m_imgRows; i++) {
		for (auto j = 0; j < m_imgCols; j++) {
            double depthPoint = depth[i * m_imgCols + j];

            if (depthPoint < 1) depthPoint = -1.0;

            auto *dp = new DataPoint
		        (
                    i,
                    j,
                    depthPoint,
                    color.at<cv::Vec3b>(i, j)[2],
                    color.at<cv::Vec3b>(i, j)[1], 
                    color.at<cv::Vec3b>(i, j)[0], 
                    i * m_imgCols
                );

		    m_allPoints.push_back(dp);
		}
	}
	return m_allPoints;
}

/* Measures distance between 2 points to expand neighbourhood
 * of investigated point
 */
bool DBScan::isInRadius(DataPoint *seed, DataPoint *center, DataPoint *potN, double epsilon, double depthThreshold) const {
    double distance_s, distance_c;

    if (center->m_depth && potN->m_depth) {
        double scale = abs(center->m_depth - potN->m_depth);

        if (scale > depthThreshold) return false;
    }

    distance_c = sqrt(pow(potN->m_r - center->m_r, 2) + pow(potN->m_g - center->m_g, 2)
                        + pow(potN->m_b - center->m_b, 2));

    distance_s = sqrt(pow(potN->m_r - seed->m_r, 2) + pow(potN->m_g - seed->m_g, 2)
                        + pow(potN->m_b - seed->m_b, 2));
          
     
    //std::cout << "Distance: " << distance_c + distance_s << std::endl;
    return ((distance_s + distance_c) <= epsilon ? true : false);
}


/*
 * Checks whether appropriate
 */
void DBScan::assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t& neighbours, 
                                double epsilon, double depthThreshold) const {
    if (!dp->m_clusterId && !dp->m_label && isInRadius(seed, center, dp, epsilon, depthThreshold) == true) {
        dp->m_label = DataPoint::VISITED;
        neighbours.push_back(dp);
    }
}

/*
 * Clustering DBScan iteration over set of RGB pixels
 */
void DBScan::DBScanIteration(double epsilon, double depthThreshold, unsigned int maxClusterPoints) {

    auto begin = std::chrono::steady_clock::now();

    for (int i = 0; i < m_allPoints.size(); i++) {
        DataPoint *seedPoint = m_allPoints[i];

        if (seedPoint->m_clusterId) continue;

        seedPoint->m_clusterId = m_numClusters;
        
        vector_t neighbours;
        neighbours.push_back(seedPoint);
        
        regionQuery(seedPoint, seedPoint, neighbours, epsilon, depthThreshold);

        int j{ 0 };
        unsigned int assigned{ 1 };

        while (j < neighbours.size()) {
            neighbours[j]->m_clusterId = m_numClusters;
            assigned++;

            if (assigned < maxClusterPoints) {
                regionQuery(seedPoint, neighbours[j], neighbours, epsilon, depthThreshold);
            }
            
            j++;
        }

        Cluster* currCluster = new Cluster(m_numClusters, neighbours);

        m_allClusters.push_back(currCluster);
        m_numClusters++;
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>
                                        (end - begin).count() << std::endl;
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> 
                                        (end - begin).count() << std::endl;
    setBorderPoints();
}

  
/*
 * Looks up for 4 neighbours whether possible
 * to form segment together
 */
void DBScan::regionQuery(DataPoint* seed, DataPoint* center, vector_t& neighbours, double epsilon, double depthThreshold) {
    auto centerX{ center->m_x };
    auto centerY{ center->m_y }; //77 189
 
    if (movePossible(centerX, centerY + 1)) { // right
        DataPoint *dp = m_allPoints[center->m_linIndex + 1];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon, depthThreshold);
    }

    if (movePossible(centerX - 1, centerY)) { // top
        DataPoint *dp = m_allPoints[center->m_linIndex - m_imgCols];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon, depthThreshold);
    }

    if (movePossible(centerX, centerY - 1)) { // left
        DataPoint *dp = m_allPoints[center->m_linIndex - 1];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon, depthThreshold);
    }

    if (movePossible(centerX + 1, centerY)) { // bottom
        DataPoint *dp = m_allPoints[center->m_linIndex + m_imgCols];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon, depthThreshold);
    }

}

/*
 * Enforces appropriate movement in grid
 */
bool DBScan::movePossible(int x, int y) const {
    if ((x >= 0 && x < m_imgRows) && (y >= 0 && y < m_imgCols)) {
        return true;
    }
    return false;
}

/*
 * Checks whether neighbour from different cluster
 */
bool DBScan::fromDifferentCluster(DataPoint* dp, int x, int y) {
    auto index = x * ( m_imgCols ) + y;
    auto point = m_allPoints[index];

    if (dp->m_clusterId == point->m_clusterId) {
        return false;
    }
    return true;  //(dp == point ? false : true);
}

/*
 * Assesses if the current point stands for m_border pixel 
 */
bool DBScan::checkBorder(DataPoint* pt) {
    if (movePossible(pt->m_x, pt->m_y + 1) && fromDifferentCluster(pt, pt->m_x, pt->m_y + 1)) {
        pt->m_border = 1;
        return true;
    }

    if (movePossible(pt->m_x - 1, pt->m_y) && fromDifferentCluster(pt, pt->m_x - 1, pt->m_y)) {
        pt->m_border = 1;
        return true;
    }

    if (movePossible(pt->m_x, pt->m_y - 1) && fromDifferentCluster(pt, pt->m_x, pt->m_y - 1)) {
        pt->m_border = 1;
        return true;
    }

    if (movePossible(pt->m_x + 1, pt->m_y) && fromDifferentCluster(pt, pt->m_x + 1, pt->m_y)) {
        pt->m_border = 1;
        return true;
    }

    return false;
}

/*
 * Iterates over all points and marks m_border pixels
 */
void DBScan::setBorderPoints() {
    for (auto pt: m_allPoints) {
        if (pt != nullptr) {
            checkBorder(pt);
        }
    }
}
