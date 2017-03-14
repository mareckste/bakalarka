#pragma once
#include "Defs.h"

class Cluster
{
public:
    
    Cluster(int id, vector_t points);
    ~Cluster();

    int m_id;
    int m_clusterSize;

    vector_t m_clusterMemberPoints;

    void addMemberPoints(vector_t dp);


};

