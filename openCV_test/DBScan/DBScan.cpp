#include "DBScan.h"

DBScan::DBScan() {
}


DBScan::~DBScan() {
   for (auto dp : m_allPoints) {
        delete[] dp;
    }
}

/*
* Converts cv image data to vector of pointers to DataPoint
* objects stored in current DBScan class  
*/
std::vector<DataPoint*> DBScan::convertToDataPoint(cv::Mat image, int rows, int cols) {
	m_imgCols = cols;
	m_imgRows = rows;

	for (auto i = 0; i < rows; i++) {
		for (auto j = 0; j < cols; j++) {
        auto *dp = new DataPoint(i, j, image.at<cv::Vec3b>(i, j)[2], 
                                      image.at<cv::Vec3b>(i, j)[1], 
                                      image.at<cv::Vec3b>(i, j)[0], 
                                      i * cols);
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


// ReSharper disable once CppMemberFunctionMayBeConst
void DBScan::assessNeighbour(DataPoint* dp, DataPoint* seed, DataPoint* center, vector_t& neighbours, double epsilon) {
    //std::cout << dp->x << " : " << dp->y << std::endl;
    if (!dp->clusterId && !dp->label && isInRadius(seed, center, dp, epsilon) == true) {
        dp->label = DataPoint::VISITED;
        neighbours.push_back(dp);
    }
}

/* Classic DBScan iteration steps over the set of datapoints*/
void DBScan::DBScanIteration(vector_t points, double epsilon, unsigned int maxClusterPoints) {
    for (int i = 0; i < points.size(); i++) {

        DataPoint *seedPoint = m_allPoints[i];

        if (seedPoint->clusterId) continue;

        seedPoint->clusterId = m_numClusters;
        m_labelledSet.push_back(seedPoint);

        vector_t neighbours;
        regionQuery(seedPoint, seedPoint, neighbours, epsilon);

        int j{ 0 };
        int assigned{ 1 };

        while (j < neighbours.size()) {
            
            neighbours[j]->clusterId = m_numClusters;
            assigned++;

            if (assigned < maxClusterPoints) {
                regionQuery(seedPoint, neighbours[j], neighbours, epsilon);
            }

            j++;
          
        }
        m_numClusters++;
        
    }
}

  
/* Returns all points within epsilon radius of a given point */
void DBScan::regionQuery(DataPoint* seed, DataPoint* center, vector_t& neighbours, double epsilon) {
    auto centerX{ center->x };
    auto centerY{ center->y }; //77 189
 
    if (movePossible(centerX, centerY + 1)) { // right
       // std::cout << center->linIndex + 1 << std::endl; //14820
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

bool DBScan::movePossible(int x, int y) const {


    if ((x >= 0 && x < m_imgRows) && (y >= 0 && y < m_imgCols)) {
      //  std::cout << x << "  " << y << std::endl;
        return true;
    }
    return false;
}

/*
  Unifies 2 vectors
*/
vector_t DBScan::mergeVectors(vector_t a, vector_t b) const
{
    struct PointerCompare {
        bool operator()(const DataPoint* c, const DataPoint* d) {
      return c->linIndex < d->linIndex;
    }
  };

    vector_t joinVector;

    std::sort(a.begin(), a.end(), PointerCompare());
    std::sort(b.begin(), b.end(), PointerCompare());

    std::set_union(a.begin(), a.end(), b.begin(), b.end(), back_inserter(joinVector),
                    PointerCompare());
  
	return joinVector;
}


