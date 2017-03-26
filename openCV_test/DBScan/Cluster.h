#pragma once
#include "Defs.h"

class Cluster
{
public:
    
    Cluster(int id, vector_t points);
    ~Cluster();

    // cluster id
    int m_id;

    // how many datapoints this cluster contains
    int m_clusterSize;

    // adjacent data points
    vector_t m_clusterMemberPoints;

    void addMemberPoints(vector_t dp);
};