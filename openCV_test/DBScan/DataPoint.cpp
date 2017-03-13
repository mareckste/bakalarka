#include "DataPoint.h"



DataPoint::DataPoint(int x, int y, int r, int g, int b, int width)
	:x(x), y(y), r(r), g(g), b(b), clusterId(UNLABELED), label(UNLABELED),
    border(0)
{
  linIndex = y + width;
}
