#include "DataPoint.h"

/*
 * Initializes current DataPoint object
 */
DataPoint::DataPoint(int x, int y, double d, int r, int g, int b, int width)
	:m_x(x), m_y(y), m_depth(d), m_r(r), m_g(g), m_b(b), m_clusterId(UNLABELED), m_label(UNLABELED),
    m_border(0)
{
  m_linIndex = y + width;
}
