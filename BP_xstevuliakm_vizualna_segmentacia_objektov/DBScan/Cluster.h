#pragma once
#include "Defs.h"

class Cluster
{
public:
    
    Cluster(int id, vector_t& points);
    ~Cluster();

    // cluster id
    int m_id;
    DataPoint* m_seed = nullptr;

    double m_avgR = -1;
    double m_avgG = -1;
    double m_avgB = -1;

    double m_avgX = -1;
    double m_avgY = -1;

    // how many datapoints this cluster contains
    int m_clusterSize;

    // adjacent data points
    vector_t m_clusterMemberPoints;

    void addMemberPoints(vector_t& dp);
    void computeColorAverages();
};