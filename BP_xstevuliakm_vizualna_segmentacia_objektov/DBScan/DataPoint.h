/*________________________________________________________________________DATA_POINT________________________________________________________________________*/
#pragma once

class Cluster;                                                                      // circular depency

class DataPoint
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PUBLIC MEMBERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
     enum Label {
        UNLABELED,
        VISITED
    };
    
    DataPoint(int x, int y, double d, int r, int g, int b, int width);              // Constructor
  
    
	int m_x;                                                                        // Spatial coordinates
	int m_y;
    
    DataPoint* m_seed = nullptr;
    
    double m_depth;                                                                 // 3D coordinate
 
    
    int m_linIndex;                                                                 // Index in 1D array for comparison 

    
    Label m_label;                                                                  // Whether visited or not 

    
	int m_clusterId;                                                                // id of cluster the SEED belongs to 
    int m_border;                                                                   // whether border point

        
	int m_r;                                                                        // rgb channel values of the point
	int m_g;
	int m_b;
      
};
/*________________________________________________________________________DATA_POINT________________________________________________________________________*/