#include "DBScan.h"

DBScan::DBScan()
{
}


DBScan::~DBScan()
{
}

/*
* Converts cv image data to vector of pointers to DataPoint
* objects stored in current DBScan class  
*/
std::vector<DataPoint*> DBScan::convertToDataPoint(cv::Mat image, int rows, int cols)
{
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
bool DBScan::isInRadius(DataPoint *investigated, DataPoint *potNeighbour, double epsilon) const
{
    double distance_s, distance_c;

    distance_c = sqrt(pow(investigated->r - potNeighbour->r, 2) + pow(investigated->g - potNeighbour->g, 2)
                        + pow(investigated->b - potNeighbour->b, 2));
    distance_s = sqrt(pow(investigated->x - potNeighbour->x,2) + pow(investigated->y - potNeighbour->y, 2));

     // std::cout << "Distance: " << distance_c + distance_s << std::endl;
    return ((distance_s + distance_c) <= epsilon ? true : false);
}

/* Classic DBScan iteration steps over the set of datapoints*/
void DBScan::DBScanIteration(vector_t points, double epsilon, unsigned int maxClusterPoints)
{
    for (int i=0; i < points.size(); i++) {
        
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
                regionQuery(seedPoint, neighbours[i], neighbours, epsilon);
            }
            
            j++;
        }
        m_numClusters++;
     }
    /* if (currentPoint->label == DataPoint::VISITED) continue;

    currentPoint->label = m_numClusters;
    std::vector<DataPoint*> neighbours = regionQuery(currentPoint, epsilon);
    if (neighbours.size() < maxClusterPoints) {
    currentPoint->label = NOISE;
    }
    else {
    this->m_numClusters++;
    expandCluster(currentPoint, neighbours, m_numClusters, epsilon, maxClusterPoints);
    }*/
}

/* Returns all points within epsilon radius of a given point */
void DBScan::regionQuery(DataPoint* seed, DataPoint* center, vector_t &neighbours, double epsilon)
{
  for (int i = 0; i < m_allPoints.size(); i++) {
    DataPoint *currentPoint = m_allPoints[i];
    if (isInRadius(seed, currentPoint, epsilon) == true)
      neighbours.push_back(currentPoint);
  }
}

/* Expands cluster by seed
*  point and its neighbourhood
*/
void DBScan::expandCluster(DataPoint *investigated, vector_t neighbours, 
							int cluster_id, double epsilon, unsigned int minPoints)
{
	for (int i = 0; i < neighbours.size(); i++) {
        DataPoint* currentPoint = neighbours[i];

		if (currentPoint->label != DataPoint::VISITED) {
            currentPoint->label = DataPoint::VISITED;
			std::vector<DataPoint*> neighbours_s = regionQuery(currentPoint,, epsilon);
			if (neighbours_s.size() >= minPoints) {
                neighbours =  mergeVectors(neighbours, neighbours_s);
			}
		}
		if (currentPoint->clusterId == UNLABELLED) {
			currentPoint->clusterId = cluster_id;
            //printf("i: %d id: %d\n", i, cluster_id);
		}
	}
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

