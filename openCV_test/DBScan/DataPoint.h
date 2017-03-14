#pragma once

class DataPoint
{
public:
     enum Label {
        UNLABELED,
        UNVISITED,
        VISITED
    };
  //Constructor
	DataPoint(int x, int y, int r, int g, int b, int width);
  
  //Spatial coordinates
	int x;
	int y;
  
  //Index in 1D array for comparison 
    int linIndex;

  //Whether visited or not - NOISE otherwise
    Label label;

  //ID of cluster the point belongs to
	int clusterId;
    int border; 

  //RGB channel values of the point
	int r;
	int g;
	int b;

  bool operator==(const DataPoint& o2) const {
      return clusterId == o2.clusterId;
  }

};

