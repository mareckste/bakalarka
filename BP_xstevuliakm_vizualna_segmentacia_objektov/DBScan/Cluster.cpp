#include "Cluster.h"

/*
 * Initializes cluster
 */
Cluster::Cluster(int id, vector_t& points)
    : m_id{ id }, m_clusterSize{ static_cast<int>(points.size()) }
{
    addMemberPoints(points);
}

Cluster::~Cluster()
{
}

/*
 * Initializes cluster by its members
 */
void Cluster::addMemberPoints(vector_t& points) {
    for (auto &pt : points) {
        if (pt != nullptr)
            m_clusterMemberPoints.push_back(pt);

    }
}

void Cluster::computeColorAverages() {
    
    if (m_clusterSize > 0) {
        
        m_avgR = 0;
        m_avgG = 0;
        m_avgB = 0;
        
        for( auto &pt: m_clusterMemberPoints) {
            m_avgR += pt->m_r;
            m_avgG += pt->m_g;
            m_avgB += pt->m_b;
        }

        m_avgR /= m_clusterSize;
        m_avgG /= m_clusterSize;
        m_avgB /= m_clusterSize;
    }
}