#include "DBScan.h"
#include "Cluster.h"
#include <ctime>
#include <chrono>
#include <iostream>
/*
 * 
    auto end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>
                                        (end - begin).count() << std::endl;
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> 
                                        (end - begin).count() << std::endl;
 */


// ==============================CODE======================================================================

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
    
    if (!dp->m_seed && !dp->m_label && isInRadius(seed, center, dp, epsilon, depthThreshold) == true) {
        dp->m_label = DataPoint::VISITED;
        neighbours.push_back(dp);
    }
}

/*
 * Clustering DBScan iteration over set of RGB pixels
 */
void DBScan::DBScanIteration(double epsilon, double depthThreshold, unsigned int numOfClusters) {
    auto minClusterPoints = (m_imgRows * m_imgCols) / numOfClusters;

    for (int i = 0; i < m_allPoints.size(); i++) {
    
        DataPoint *seedPoint = m_allPoints[i];

        if (seedPoint->m_seed) continue;

        seedPoint->m_clusterId = m_numClusters;
        vector_t neighbours;
        neighbours.push_back(seedPoint);
        
        regionQuery(seedPoint, seedPoint, neighbours, epsilon, depthThreshold);

        unsigned j = 0;
        unsigned assigned = 1;

        while (j < neighbours.size()) {
            
            neighbours[j]->m_seed = seedPoint;
            assigned++;

            if (assigned < minClusterPoints) 
                regionQuery(seedPoint, neighbours[j], neighbours, epsilon, depthThreshold);
                        
            j++;
        }

        Cluster* currCluster = new Cluster(m_numClusters, neighbours);

        m_allClusters.push_back(currCluster);
        m_numClusters++;
    }
    
   // mergeClusters(minClusterPoints);
    setBorderPoints();
}

  
/*
 * Looks up for 4 neighbours whether possible
 * to form segment together
 */
void DBScan::regionQuery(DataPoint* seed, DataPoint* center, vector_t& neighbours, double epsilon, double depthThreshold) {
    auto centerX{ center->m_x };
    auto centerY{ center->m_y }; 
    
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


DataPoint* DBScan::getPointAt(int x, int y) {
    auto index = x * (m_imgCols)+y;
    return m_allPoints[index];
}


inline Cluster* DBScan:: getClusterAt(int pos) {
    return m_allClusters.at(pos - 1);
}

/*
 * Checks whether neighbour from different cluster
 */
bool DBScan::fromDifferentCluster(DataPoint* dp, int x, int y) {
    
    auto potNeighb = getPointAt(x, y);

    if (dp->m_seed->m_clusterId == potNeighb->m_seed->m_clusterId) {
        return false;
    }
    return true;
}


bool DBScan::diffNeighbour(int x, int y, DataPoint *p) {
    
    if (movePossible(x, y) && fromDifferentCluster(p, x, y)) 
        return true;

    return false;
}

/*
 * Assesses if the current point stands for m_border pixel 
 */
bool DBScan::checkBorder(DataPoint* pt) {
    
    if (diffNeighbour(pt->m_x, pt->m_y + 1, pt)) {
        pt->m_border = 1;
        return true;
    }

    if (diffNeighbour(pt->m_x - 1, pt->m_y, pt)) {
        pt->m_border = 1;
        return true;
    }

    if (diffNeighbour(pt->m_x, pt->m_y - 1, pt)) {
        pt->m_border = 1;
        return true;
    }

    if (diffNeighbour(pt->m_x + 1, pt->m_y, pt)) {
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


bool DBScan::hasAvgColors(const Cluster *const cluster) {
    
    if (cluster->m_avgR < 0 && cluster->m_avgB < 0 && cluster->m_avgG < 0) {
        return false;
    }

    return true;
}

double DBScan::computeMergingDistance(const Cluster *const currClust, int nX, int nY) {
    double dist_c, dist_a;
    auto neighClustId = getPointAt(nX, nY)->m_seed->m_clusterId;
    auto neighClust = getClusterAt(neighClustId);

    if (!hasAvgColors(neighClust)) neighClust->computeColorAverages();

    dist_c = sqrt(pow(currClust->m_avgR-neighClust->m_avgR, 2) + pow(currClust->m_avgB - neighClust->m_avgB, 2)
        + pow(currClust->m_avgG - neighClust->m_avgG, 2));
    
    dist_a = sqrt(pow(currClust->m_avgR - neighClust->m_avgR, 2) + pow(currClust->m_avgB - neighClust->m_avgB, 2)
        + pow(currClust->m_avgG - neighClust->m_avgG, 2));

    return dist_c + dist_a;
}


void DBScan::mergeClusters(unsigned int minClusterPoints) {
    
    for (auto& currClust: m_allClusters) {

        if (currClust->m_id < 0) continue;
        
        if (currClust->m_clusterSize < minClusterPoints) {
            Cluster *minc = nullptr;
            int minDist = 100000;

            if (!hasAvgColors(currClust)) currClust->computeColorAverages();

            for (auto& pt: currClust->m_clusterMemberPoints) {
                int dist_2;
                
                if (diffNeighbour(pt->m_x, pt->m_y + 1, pt)) {
                    
                }

                if (diffNeighbour(pt->m_x - 1, pt->m_y, pt)) {

                }

                if (diffNeighbour(pt->m_x, pt->m_y - 1, pt)) {

                }

                if (diffNeighbour(pt->m_x + 1, pt->m_y, pt)) {

                }
            }
        }
    }
}
