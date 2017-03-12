#pragma once
#include <iostream> 
#include <vector>
#include <string>
#include "DataPoint.h"

/*unused so far*/
class Cluster
{
public:
	Cluster(int id);

	int id;
	
	std::vector<DataPoint> dataPoints;
};

