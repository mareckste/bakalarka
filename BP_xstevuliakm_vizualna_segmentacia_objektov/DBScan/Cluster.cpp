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

void Cluster::computeAverages() {
    
    if (m_clusterSize > 0) {
        
        m_avgR = 0;
        m_avgG = 0;
        m_avgB = 0;

        m_avgX = 0;
        m_avgY = 0;
        
        for( auto &pt: m_clusterMemberPoints) {
            m_avgR += pt->m_r;
            m_avgG += pt->m_g;
            m_avgB += pt->m_b;

            m_avgX += pt->m_x;
            m_avgY += pt->m_y;
        }

        m_avgR /= m_clusterSize;
        m_avgG /= m_clusterSize;
        m_avgB /= m_clusterSize;

        m_avgX /= m_clusterSize;
        m_avgY /= m_clusterSize;
    }
}

void Cluster::updateSize() {
    m_clusterSize = m_clusterMemberPoints.size();
}