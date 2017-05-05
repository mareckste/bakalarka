/*________________________________________________________________________3D_DBSCAN________________________________________________________________________*/

#include "DBScan.h"
#include "Cluster.h"
#include <ctime>
#include <chrono>
#include <iostream>
#include <iterator>

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inits the class with the size of the points and clusters
// to avoid reallocation in std::vector
////////////////////////////////////////////////////////////////////////////////////////////////////////

DBScan::DBScan(int rows, int cols)
    :m_imgCols{ cols }, m_imgRows{ rows }, m_numClusters {1}
{
    m_allPoints  .reserve(rows * cols);
    m_allClusters.reserve(40000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////

DBScan::~DBScan() {
   for (auto dp : m_allPoints)
        delete dp;

    for (auto cs : m_allClusters)
       delete cs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Converts cv color data to vector of pointers to DataPoint
// to facilitate data manipalution
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::convertToDataPoint(const cv::Mat& color, const cv::Mat& depth) {
    std::cout << "Converting data" << std::endl;
    double depthPoint;

    for (auto i = 0; i < m_imgRows; i++) {
		for (auto j = 0; j < m_imgCols; j++) {
            
            if (depth.data == nullptr) depthPoint = UINT16_MAX;
            
		    else
                depthPoint = depth.at<UINT16>(i,j);

            if ((depthPoint == UINT16_MAX)) depthPoint = -1.0;

            DataPoint* dp = new DataPoint(
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Measures distance between 2 points to expand neighbourhood
// of investigated point
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DBScan::isInRadius(DataPoint *seed, DataPoint *center, DataPoint *potN, double epsilon, 
                            double depthThreshold) const {
    
    double distance_s, distance_c;

    if (center->m_depth && potN->m_depth) {
        double scale = abs(center->m_depth - potN->m_depth);

        if (scale > depthThreshold) return false;
    }

    distance_c = sqrt(    pow(potN->m_r - center->m_r, 2) 
                        + pow(potN->m_g - center->m_g, 2)
                        + pow(potN->m_b - center->m_b, 2)
                     );

    distance_s = sqrt(    pow(potN->m_r - seed->m_r, 2) 
                        + pow(potN->m_g - seed->m_g, 2)
                        + pow(potN->m_b - seed->m_b, 2)
                     );
       
    return ((distance_s + distance_c) <= epsilon ? true : false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if neighbour has not been marked yet by another one
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t* neighbours, 
                                double epsilon, double depthThreshold) const {
    
    if (
        !dp->m_seed && !dp->m_label && 
        isInRadius(seed, center, dp, epsilon, depthThreshold) == true
        ) {
        
        dp->m_label = DataPoint::VISITED;
        neighbours->push_back(dp);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3D DBSCAN iteration over the given set of RGB pixels
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::DBScanIteration(double epsilon, double depthThreshold, unsigned int numOfClusters, 
                        unsigned int mergingFactor) {

    std::cout << "Segmenting" << std::endl;
    unsigned int minClusterPoints = (m_imgRows * m_imgCols) / numOfClusters;

    for (int i = 0; i < m_allPoints.size(); i++) {
    
        DataPoint *seedPoint   = m_allPoints[i];

        if (seedPoint->m_seed) continue;

        seedPoint->m_clusterId = m_numClusters;
        seedPoint->m_seed      = seedPoint;

        vector_t* neighbours   = new vector_t;

        neighbours->reserve(200);
        neighbours->push_back(seedPoint);
        
        regionQuery(seedPoint, seedPoint, neighbours, epsilon, depthThreshold);

        unsigned j        = 0;
        unsigned assigned = 1;

        while (j < neighbours->size()) {
            
            neighbours->at(j)->m_seed = seedPoint;
            assigned++;

            if (assigned < minClusterPoints) 
                regionQuery(seedPoint, neighbours->at(j), neighbours, epsilon, depthThreshold);
                        
            j++;
        }

        Cluster* currCluster = new Cluster(m_numClusters, neighbours);

        m_allClusters.push_back(currCluster);
        m_numClusters++;
    }

    int size = m_allClusters.size();
    std::cout << "Superpixels before: " << size << std::endl;
    
    //////////////////////////////////////////////////////////////////////////////////////////
    // MERGE
    //////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "Merging phase" << std::endl;
    
    for (auto i = 0; i < mergingFactor; i++) {
        DBSmerge(minClusterPoints, &size, numOfClusters, 0);
        if (size == numOfClusters) break;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // POSTPROCESSING OPTIONAL
    //////////////////////////////////////////////////////////////////////////////////////////

    std::cout << "Refining phase" << std::endl;
   // DBSmerge(minClusterPoints, &size, numOfClusters, 1);
  
   ///////////////////////////////////////////////////////////////////////////////////////////
   // COUNT blob SUPERPIXELS FOR INFO
   ///////////////////////////////////////////////////////////////////////////////////////////
    unsigned int num = 0; unsigned int numsmall = 0;
    std::cout << "Counting clusters" << std::endl;

    for (auto& c: m_allClusters) {
        if (c->m_id != -1) {
            num++; 
            if (c->m_clusterSize < minClusterPoints / 10) 
                numsmall++;
        }
    }

    std::cout << "superpixels after: " << num << " small pts: " << numsmall << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////////
    // OPTIONAL BORDER POINTS FOR CV::IMAGESHOW
    //////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "Counting border points" << std::endl;
    setBorderPoints();

    //////////////////////////////////////////////////////////////////////////////////////////
    // RELABELING MODIFIED SUPERPIXELS ID INTO ASCENDING ORDER
    //////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "Relabeling id's" << std::endl;

    relabelPts();
  }
 
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Looks up for 4 neighbours whether possible
// to form superpixel together
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::regionQuery(DataPoint* seed, DataPoint* center, vector_t* neighbours, double epsilon, 
                            double depthThreshold) {

    unsigned int centerX = center->m_x;
    unsigned int centerY = center->m_y;

    
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

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enforces appropriate movement in grid
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DBScan::movePossible(int x, int y) const {
    
    if ((x >= 0 && x < m_imgRows) && (y >= 0 && y < m_imgCols)) {
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Checks whether neighbour from different cluster
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DBScan::fromDifferentCluster(DataPoint* dp, int x, int y) {
    
    DataPoint* potNeighb = getPointAt(x, y);

    if (dp->m_seed->m_clusterId == potNeighb->m_seed->m_clusterId)
        return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns true when given point is neighbouring border point
// from different segment
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DBScan::diffNeighbour(int x, int y, DataPoint *p) {
    
    if (movePossible(x, y) && fromDifferentCluster(p, x, y)) 
        return true;

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if the current point stands for m_border pixel 
////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Iterates over all points and marks m_border pixels
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::setBorderPoints() {

    for (auto& pt: m_allPoints)
        if (pt != nullptr)  checkBorder(pt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if the given cluster has its average colours and spatial coordinates
// recalculated
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DBScan::hasAvgValues(const Cluster *const cluster) {
    
    if (
        cluster->m_avgR < 0 && 
        cluster->m_avgB < 0 && 
        cluster->m_avgG < 0 &&
        cluster->m_avgX < 0 && 
        cluster->m_avgY < 0
        ) {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gets datapoint at x, y position in the image
////////////////////////////////////////////////////////////////////////////////////////////////////////

DataPoint* DBScan::getPointAt(int x, int y) {
    
    int index = x * (m_imgCols)+y;
    return m_allPoints[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Computes merging distance2 and compares the distance with minimum
// obtained by far
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::computeMergingDistance(const Cluster*const currClust, int nX, int nY, double* min, 
                                        Cluster** minc) {
    
    double   dist_c, dist_a, dist_d, dist_2;    
    int      neighClustId = getPointAt(nX, nY)->m_seed->m_clusterId;
    Cluster* neighClust   = m_allClusters[neighClustId - 1];

    
    if (!hasAvgValues(neighClust)) neighClust->computeAverages();

    dist_d = (abs(neighClust->m_avgD - currClust->m_avgD) > 25 ? 10000 : 0); //alpha
    
    dist_c =      (currClust->m_avgR - neighClust->m_avgR)*(currClust->m_avgR - neighClust->m_avgR)
                + (currClust->m_avgB - neighClust->m_avgB)*(currClust->m_avgB - neighClust->m_avgB)
                + (currClust->m_avgG - neighClust->m_avgG)*(currClust->m_avgG - neighClust->m_avgG);

    dist_a =      (currClust->m_avgX - neighClust->m_avgX)*(currClust->m_avgX - neighClust->m_avgX)
                + (currClust->m_avgY - neighClust->m_avgY)*(currClust->m_avgY - neighClust->m_avgY);

    dist_2 =  dist_c + 2*dist_a + dist_d;
    
    if (dist_2 < *min) {
        *min = dist_2;  *minc = neighClust;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Merges 2 superpixels together 
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::mergeClusters(Cluster* cluster, Cluster* minc) {
    Cluster *huge;
    Cluster *less;
    DataPoint *hugeSeed;

    huge = (cluster->m_clusterSize > minc->m_clusterSize ? cluster : minc);
    less = (huge == cluster ? minc : cluster);

    hugeSeed = huge->m_clusterMemberPoints->at(0);
    less->m_id = -1;

    for (int i = 0; i < less->m_clusterMemberPoints->size(); i++) {
        
        DataPoint *curr = less->m_clusterMemberPoints->at(i);
        
        curr->m_seed    = hugeSeed;
        huge->m_clusterMemberPoints->push_back(curr);
    }

    huge->updateSize();

    huge->m_avgR += less->m_avgR;
    huge->m_avgR /= 2;

    huge->m_avgG += less->m_avgG;
    huge->m_avgG /= 2;

    huge->m_avgB += less->m_avgB;
    huge->m_avgB /= 2;

    huge->m_avgD += less->m_avgD;
    huge->m_avgD /= 2;

    huge->m_avgX += less->m_avgX;
    huge->m_avgX /= 2;

    huge->m_avgY += less->m_avgY;
    huge->m_avgY /= 2;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mergin approach after initial segmentation
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::DBSmerge(unsigned int minClusterPoints, int* size, int numOfClusters, int ppFlag) {
    
    for (auto& currClust: m_allClusters) {
        if ((*size) == numOfClusters) return;

        if (currClust->m_id < 0) continue;
        
        if (currClust->m_clusterSize < (minClusterPoints /* 1.33*/)) {
            
            Cluster *minc  = nullptr; //6k
            double minDist = (!ppFlag ? 10000 : 100000);

            if (
                ppFlag && 
                (currClust->m_clusterSize > (minClusterPoints / 8))
                ) continue;

            if (!hasAvgValues(currClust)) 
                currClust->computeAverages();

            for (int i = 0; i < currClust->m_clusterMemberPoints->size(); i++) {
                
                DataPoint *pt = currClust->m_clusterMemberPoints->at(i);
                                
                if (diffNeighbour(pt->m_x, pt->m_y + 1, pt)) { // bottom
                    if (!ppFlag)
                        computeMergingDistance(currClust, pt->m_x, pt->m_y + 1,&minDist, &minc);
                    else {
                        getLargestNeighbour(currClust, pt->m_x, pt->m_y + 1, &minDist, &minc);
                    }
                }

                if (diffNeighbour(pt->m_x - 1, pt->m_y, pt)) { // left
                    if (!ppFlag)
                        computeMergingDistance(currClust, pt->m_x - 1, pt->m_y, &minDist, &minc);
                    else {
                        getLargestNeighbour(currClust, pt->m_x - 1, pt->m_y, &minDist, &minc);
                    }
                }

                if (diffNeighbour(pt->m_x, pt->m_y - 1, pt)) { // top
                    if (!ppFlag)
                        computeMergingDistance(currClust, pt->m_x, pt->m_y - 1, &minDist, &minc);
                    else {
                        getLargestNeighbour(currClust, pt->m_x, pt->m_y - 1, &minDist, &minc);
                    }
                }

                if (diffNeighbour(pt->m_x + 1, pt->m_y, pt)) { // right
                    if (!ppFlag)
                        computeMergingDistance(currClust, pt->m_x + 1, pt->m_y, &minDist, &minc);
                    else {
                        getLargestNeighbour(currClust, pt->m_x + 1, pt->m_y, &minDist, &minc);
                    }
                }
                
            }

            if (minc != nullptr && minc!=currClust) { // merge if minimum found
                mergeClusters(currClust, minc);
                (*size)--;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gets largest superpixel within neighbourhood of current superpixel
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::getLargestNeighbour(Cluster *currCluster, int nX, int nY, double* min_dist, 
                                    Cluster** minc) {
    
    unsigned int neighClustId = getPointAt(nX, nY)->m_seed->m_clusterId;
    Cluster* neighClust       = m_allClusters[neighClustId - 1];

    if ((*min_dist) > neighClust->m_clusterSize) {

        *min_dist = neighClust->m_clusterSize;
        *minc     = neighClust;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Relabels superpixel ids with ascdending order
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::relabelPts() {
    int counter = 1;

    for (auto &c:m_allClusters) {
        if (c->m_id != -1) {
            c->m_clusterMemberPoints->at(0)->m_clusterId = counter;
            counter++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Labels superpixel borders
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::labelBorders(cv::Mat &image) {
    std::cout << "Labeling" << std::endl;
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            if (m_allPoints[j + i*image.cols]->m_border == 1) {
                image.at<cv::Vec3b>(i, j)[0] = 204;
                image.at<cv::Vec3b>(i, j)[1] = 179;
                image.at<cv::Vec3b>(i, j)[2] = 51;
            }
        }
    }
    std::cout << "Labeling done" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Saves generated segmentation
////////////////////////////////////////////////////////////////////////////////////////////////////////

void DBScan::saveSegmentation(cv::Mat &image) {
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            image.at<UINT16>(i, j) = m_allPoints[j + i*image.cols]->m_seed->m_clusterId;
        }
    }

    cv::imwrite("segm.png", image);
}
/*________________________________________________________________________3D_DBSCAN________________________________________________________________________*/