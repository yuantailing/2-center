#include "center.h"
#include <cassert>
#include <algorithm>

bool DC_separated(std::vector<Coord> const &S, std::vector<Coord> &centers) {
    return false;
}

bool DC_close(std::vector<Coord> const &S, std::vector<Coord> &centers) {
    return false;
}

bool DC(std::vector<Coord> const &S, std::vector<Coord> &centers) {
    if (DC_separated(S, centers)) return true;
    if (DC_close(S, centers)) return true;
    return false;
}

PCenterResult p_center(int p, std::vector<Coord> const &S, Real eps) {
    assert(p == 2);
    if (S.size() == 0) {
        PCenterResult result;
        result.r = Real(0);
        Coord center(Real(0), Real(0));
        for (int i = 0; i < p; i++)
            result.centers.push_back(center);
    }
    Real xmin = S.front().x, xmax = S.front().x;
    Real ymin = S.front().y, ymax = S.front().y;
    for (Coord const &coord: S) {
        xmin = std::min(xmin, coord.x);
        xmax = std::max(xmax, coord.x);
        ymin = std::min(ymin, coord.y);
        ymax = std::max(ymax, coord.y);
    }
    Real r = ((xmax - xmin) + (ymax - ymin)) / 2;
    Real dr = r / 2;
    Real r_stop = dr * eps;
    std::vector<Coord> centers;
    PCenterResult result;
    while (dr > r_stop) {
        bool positive = DC(S, centers);
        if (positive) {
            result.r = r;
            result.centers = centers;
            r -= dr;
        } else {
            r += dr;
        }
        dr /= 2;
    }
    return result;
}
