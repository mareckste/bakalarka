/*________________________________________________________________________CLUSTER________________________________________________________________________*/
#pragma once
#include "Defs.h"

class Cluster
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PUBLIC MEMBERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Cluster(int id, vector_t* points);
    ~Cluster();

    /////////////////////////////////
    //variables
    /////////////////////////////////
    
    int m_id;                                                   // superpixel id
    DataPoint* m_seed = nullptr;                                // seed point of superpixels

    double m_avgR = -1;                                         // average rgb values for superpixels
    double m_avgG = -1;
    double m_avgB = -1;
    double m_avgD = -1;                                         // average depth for superpixel

    double m_avgX = -1;                                         // centroid of superpixel
    double m_avgY = -1;

    
    int m_clusterSize;                                          // how many datapoints in superpixel

    vector_t* m_clusterMemberPoints;                            // all points within superpixel
    
    /////////////////////////////////
    //methods
    /////////////////////////////////
    
    void computeAverages();
    void updateSize();
};
/*________________________________________________________________________CLUSTER________________________________________________________________________*/