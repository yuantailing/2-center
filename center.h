#ifndef CENTER_H
#define CENTER_H

// An implementation of Sharir M. A near-linear algorithm for the planar 2-center problem[J]. Discrete & Computational Geometry, 1997, 18(2): 125-134.

#include <vector>
#include "real.h"

struct PCenterResult {
    Real r;
    std::vector<Coord> centers;
};

PCenterResult p_center(int p, std::vector<Coord> const &S, Real eps=Real(1e-6));

#endif // CENTER_H
