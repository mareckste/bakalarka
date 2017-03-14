#include "Cluster.h"


Cluster::Cluster(int id, vector_t points)
    : m_id{ id }, m_clusterSize{ static_cast<int>(points.size()) }
{
    addMemberPoints(points);
}

Cluster::~Cluster()
{
}

void Cluster::addMemberPoints(vector_t points) {
    for (auto &pt : points) {
        if (pt != nullptr)
            m_clusterMemberPoints.push_back(pt);

    }
}


