#include "DBScan.h"
#include "Cluster.h"
#include <ctime>
#include <chrono>
#include <iostream>

/*
 * Inits the class by the size of the image
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
 * Converts cv image data to vector of pointers to DataPoint
 * objects stored in current DBScan class  
 */
std::vector<DataPoint*> DBScan::convertToDataPoint(const cv::Mat& image) {
	
    for (auto i = 0; i < m_imgRows; i++) {
		for (auto j = 0; j < m_imgCols; j++) {
        auto *dp = new DataPoint(i, j, image.at<cv::Vec3b>(i, j)[2], 
                                      image.at<cv::Vec3b>(i, j)[1], 
                                      image.at<cv::Vec3b>(i, j)[0], 
                                      i * m_imgCols);
		m_allPoints.push_back(dp);
		}
	}
	return m_allPoints;
}

/* Measures distance between 2 points to expand neighbourhood
 * of investigated point
 */
bool DBScan::isInRadius(DataPoint *seed, DataPoint *center, DataPoint *potN, double epsilon) const {
    double distance_s, distance_c;

    distance_c = sqrt(pow(potN->r - center->r, 2) + pow(potN->g - center->g, 2)
                        + pow(potN->b - center->b, 2));
    distance_s = sqrt(pow(potN->r - seed->r, 2) + pow(potN->g - seed->g, 2)
                        + pow(potN->b - seed->b, 2));
     
    //std::cout << "Distance: " << distance_c + distance_s << std::endl;
    return ((distance_s + distance_c) <= epsilon ? true : false);
}


/*
 * Checks whether appropriate
 */
void DBScan::assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t& neighbours, 
                                double epsilon) const {
    if (!dp->clusterId && !dp->label && isInRadius(seed, center, dp, epsilon) == true) {
        dp->label = DataPoint::VISITED;
        neighbours.push_back(dp);
    }
}

/*
 * Clustering DBScan iteration over set of RGB pixels
 */
void DBScan::DBScanIteration(double epsilon, unsigned int maxClusterPoints) {

    auto begin = std::chrono::steady_clock::now();

    for (int i = 0; i < m_allPoints.size(); i++) {
        DataPoint *seedPoint = m_allPoints[i];

        if (seedPoint->clusterId) continue;

        seedPoint->clusterId = m_numClusters;
        
        vector_t neighbours;
        neighbours.push_back(seedPoint);
        
        regionQuery(seedPoint, seedPoint, neighbours, epsilon);

        int j{ 0 };
        unsigned int assigned{ 1 };

        while (j < neighbours.size()) {
            neighbours[j]->clusterId = m_numClusters;
            assigned++;

            if (assigned < maxClusterPoints) {
                regionQuery(seedPoint, neighbours[j], neighbours, epsilon);
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
void DBScan::regionQuery(DataPoint* seed, DataPoint* center, vector_t& neighbours, double epsilon) {
    auto centerX{ center->x };
    auto centerY{ center->y }; //77 189
 
    if (movePossible(centerX, centerY + 1)) { // right
        DataPoint *dp = m_allPoints[center->linIndex + 1];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon);
    }

    if (movePossible(centerX - 1, centerY)) { // top
        DataPoint *dp = m_allPoints[center->linIndex - m_imgCols];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon);
    }

    if (movePossible(centerX, centerY - 1)) { // left
        DataPoint *dp = m_allPoints[center->linIndex - 1];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon);
    }

    if (movePossible(centerX + 1, centerY)) { // bottom
        DataPoint *dp = m_allPoints[center->linIndex + m_imgCols];
        if (dp != nullptr)
            assessNeighbour(dp, seed, center, neighbours, epsilon);
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

    if (dp->clusterId == point->clusterId) {
        return false;
    }
    return true;  //(dp == point ? false : true);
}

/*
 * Assesses if the current point stands for border pixel 
 */
bool DBScan::checkBorder(DataPoint* pt) {
    if (movePossible(pt->x, pt->y + 1) && fromDifferentCluster(pt, pt->x, pt->y + 1)) {
        pt->border = 1;
        return true;
    }

    if (movePossible(pt->x - 1, pt->y) && fromDifferentCluster(pt, pt->x - 1, pt->y)) {
        pt->border = 1;
        return true;
    }

    if (movePossible(pt->x, pt->y - 1) && fromDifferentCluster(pt, pt->x, pt->y - 1)) {
        pt->border = 1;
        return true;
    }

    if (movePossible(pt->x + 1, pt->y) && fromDifferentCluster(pt, pt->x + 1, pt->y)) {
        pt->border = 1;
        return true;
    }

    return false;
}

/*
 * Iterates over all points and marks border pixels
 */
void DBScan::setBorderPoints() {
    for (auto pt: m_allPoints) {
        if (pt != nullptr) {
            checkBorder(pt);
        }
    }
}
