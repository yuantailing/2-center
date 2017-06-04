#ifndef CENTER_H
#define CENTER_H

// An implementation of Sharir M. A near-linear algorithm for the planar 2-center problem[J]. Discrete & Computational Geometry, 1997, 18(2): 125-134.

#include <vector>
#include "real.h"

struct PCenterResult {
    Real r;
    std::vector<Coord> centers;
};

extern bool quick_case_only;

extern int dc_case;
extern Float dc_rotate_angle;
extern std::vector<std::size_t> dc_division_left;

PCenterResult p_center(int p, std::vector<Coord> const &S, Real eps=Real(1e-4));

#endif // CENTER_H
