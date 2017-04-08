#include "DBScan.h"
#include "Cluster.h"
#include <ctime>
#include <chrono>
#include <iostream>

// ==============================CODE======================================================================

/*
 * Inits the class by the size of the color
 */
DBScan::DBScan(int rows, int cols)
    :m_imgCols{ cols }, m_imgRows{ rows }, m_numClusters {1}
{
    m_allPoints.reserve(rows * cols);
    m_allClusters.reserve(40000);
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
    double depthPoint = -1;

    for (auto i = 0; i < m_imgRows; i++) {
		for (auto j = 0; j < m_imgCols; j++) {
            
		    if (depth != nullptr)
                depthPoint = depth[i * m_imgCols + j];

            if ((depthPoint < 1)) depthPoint = -1.0;

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
void DBScan::DBScanIteration(double epsilon, double depthThreshold, unsigned int numOfClusters, 
                        unsigned int mergingFactor) {

    auto minClusterPoints = (m_imgRows * m_imgCols) / numOfClusters;
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < m_allPoints.size(); i++) {
    
        DataPoint *seedPoint = m_allPoints[i];

        if (seedPoint->m_seed) continue;

        seedPoint->m_clusterId = m_numClusters;
        seedPoint->m_seed = seedPoint;

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

    int size = m_allClusters.size();

    printf("superpixels before : %d \n", size);

    for (int i = 0; i < mergingFactor; i++)
        DBSmerge(minClusterPoints, &size, numOfClusters);
    
    
    auto end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>
        (end - start).count()/1000000 << std::endl;

    int num = 0;
    for (auto& c: m_allClusters) {
        if (c->m_id != -1) num++;
    }
    printf("superpixels after: %d \n", num);
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

/*
 * Returns true when given point is neighbouring border point
 * from different segment
 */
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

/*
 * Checks if a given cluster has its average colours and spatial coordinates
 * recalculated
 */
bool DBScan::hasAvgValues(const Cluster *const cluster) {
    
    if (cluster->m_avgR < 0 && cluster->m_avgB < 0 && cluster->m_avgG < 0 &&
        cluster->m_avgX < 0 && cluster->m_avgY < 0) {
        return false;
    }

    return true;
}

/*
 * Gets datapoint at x, y position in the image
 */
DataPoint* DBScan::getPointAt(int x, int y) {
    auto index = x * (m_imgCols)+y;
    return m_allPoints[index];
}


inline Cluster* DBScan:: getClusterAt(int pos) {
    return m_allClusters[pos - 1];
}

/*
 * Computes merging distance2 and compares the distance with minimum
 * obtained by far
 */
void DBScan::computeMergingDistance(const Cluster*const currClust, int nX, int nY, double* min, Cluster** minc) {
    double dist_c, dist_a, dist_2;
    
    auto neighClustId = getPointAt(nX, nY)->m_seed->m_clusterId;
    auto neighClust = getClusterAt(neighClustId);

    
    if (!hasAvgValues(neighClust)) neighClust->computeAverages();

    dist_c = sqrt(pow(currClust->m_avgR-neighClust->m_avgR, 2) + pow(currClust->m_avgB - neighClust->m_avgB, 2)
        + pow(currClust->m_avgG - neighClust->m_avgG, 2));
    
    dist_a = sqrt(pow(currClust->m_avgX - neighClust->m_avgX, 2) + pow(currClust->m_avgY - neighClust->m_avgY, 2));

    dist_2 =  dist_c + dist_a;

   // printf("%lf\n", dist_2);

    if (dist_2 < *min) {
        *min = dist_2;
        *minc = neighClust;
    }
}

/*
 * Merges 2 segments together to form bigger segment
 */
void DBScan::mergeClusters(Cluster* cluster, Cluster* minc) {
    Cluster *huge = nullptr;
    Cluster *less = nullptr;
    DataPoint *lessSeed = nullptr;
    DataPoint *hugeSeed = nullptr;

    huge = (cluster->m_clusterSize > minc->m_clusterSize ? cluster : minc);
    less = (huge == cluster ? minc : cluster);

    hugeSeed = huge->m_clusterMemberPoints[0];
    less->m_id = -1;

    for (auto& pt: less->m_clusterMemberPoints) {
        pt->m_seed = hugeSeed;
        huge->m_clusterMemberPoints.push_back(pt);

    }
    huge->updateSize();
    huge->computeAverages();
}

/*
 * Mergin approach after initial segmentation
 */
void DBScan::DBSmerge(unsigned int minClusterPoints, int* size, int numOfClusters) {
    
    for (auto& currClust: m_allClusters) {
        if ((*size) == numOfClusters) return;

        if (currClust->m_id < 0) continue;
        
        if (currClust->m_clusterSize < minClusterPoints) {
            Cluster *minc = nullptr;
            double minDist = 100000;

            if (!hasAvgValues(currClust)) currClust->computeAverages();

            for (auto& pt: currClust->m_clusterMemberPoints) {
                                
                if (diffNeighbour(pt->m_x, pt->m_y + 1, pt)) {
                    (computeMergingDistance(currClust, pt->m_x, pt->m_y + 1,&minDist, &minc));
                }

                if (diffNeighbour(pt->m_x - 1, pt->m_y, pt)) {
                    (computeMergingDistance(currClust, pt->m_x - 1, pt->m_y, &minDist, &minc));
                }

                if (diffNeighbour(pt->m_x, pt->m_y - 1, pt)) {
                    (computeMergingDistance(currClust, pt->m_x, pt->m_y - 1, &minDist, &minc));
                }

                if (diffNeighbour(pt->m_x + 1, pt->m_y, pt)) {
                    (computeMergingDistance(currClust, pt->m_x + 1, pt->m_y, &minDist, &minc));
                }
            }

            if (minc != nullptr && minc!=currClust) {
                mergeClusters(currClust, minc);
                (*size)--;
            }
        }
    }
}
