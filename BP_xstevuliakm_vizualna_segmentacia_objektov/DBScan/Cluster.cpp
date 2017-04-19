#include "Cluster.h"

/*
 * Initializes cluster
 */
Cluster::Cluster(int id, vector_t* points)
    : m_id{ id }, 
    m_clusterSize{ static_cast<int>(points->size()) },
    m_clusterMemberPoints{ points }     
{
}

Cluster::~Cluster() {
    delete m_clusterMemberPoints;
}


void Cluster::computeAverages() {
    
    if (m_clusterSize > 0) {
        
        m_avgR = 0;
        m_avgG = 0;
        m_avgB = 0;

        m_avgX = 0;
        m_avgY = 0;
        
        for (int i = 0; i < m_clusterMemberPoints->size(); i++) {
            m_avgR += m_clusterMemberPoints->at(i)->m_r;
            m_avgG += m_clusterMemberPoints->at(i)->m_g;
            m_avgB += m_clusterMemberPoints->at(i)->m_b;

            m_avgX += m_clusterMemberPoints->at(i)->m_x;
            m_avgY += m_clusterMemberPoints->at(i)->m_y;
        }

        m_avgR /= m_clusterSize;
        m_avgG /= m_clusterSize;
        m_avgB /= m_clusterSize;

        m_avgX /= m_clusterSize;
        m_avgY /= m_clusterSize;
    }
}

void Cluster::updateSize() {
    m_clusterSize = m_clusterMemberPoints->size();
}