/*________________________________________________________________________DEFINITIONS________________________________________________________________________*/
#pragma once
#include <vector>
#include "DataPoint.h"

class Cluster;                                      // to avoid circular depency DBScan <-> Cluster

typedef std::vector<DataPoint *> vector_t;
typedef std::vector<Cluster *> cluster_vec_t;
/*________________________________________________________________________DEFINITIONS________________________________________________________________________*/