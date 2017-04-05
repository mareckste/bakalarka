#pragma once

class Cluster;

class DataPoint
{
public:
     enum Label {
        UNLABELED,
        UNVISITED,
        VISITED
    };
    //Constructor
    DataPoint(int x, int y, double d, int r, int g, int b, int width);
  
    //Spatial coordinates
	int m_x;
	int m_y;
    
    DataPoint* m_seed = nullptr;
    //3D coordinate
    double m_depth;
 
    //Index in 1D array for comparison 
    int m_linIndex;

    //Whether visited or not - NOISE otherwise
    Label m_label;

    //id of cluster the point belongs to
	int m_clusterId;
    int m_border; 

    //rgb channel values of the point
	int m_r;
	int m_g;
	int m_b;

  bool operator==(const DataPoint& o2) const {
      return m_clusterId == o2.m_clusterId;
  }

};

