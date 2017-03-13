#pragma once
#include <iostream> 
#include <vector>
#include <string>
#include "DataPoint.h"
#include "DBScan.h"

/*unused so far*/
class Cluster
{
public:
	Cluster(int id, vector_t points);

	int m_id;
    int m_clusterSize;

	vector_t m_clusterMemberPoints;

    void addMemberPoints(vector_t dp);
    

};

