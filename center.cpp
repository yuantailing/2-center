#include "center.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "kptree.h"


// S内所有点绕o顺时针旋转theta弧度
void rotate(std::vector<Coord> S, Float theta, Coord const &o=Coord(Real(0), Real(0))) {
    Float c = std::cos(theta);
    Float s = std::sin(theta);
    for (Coord &coord: S) {
        Coord d = coord - o;
        coord = o + Coord(d.x * c - d.y * s, d.x * s + d.y * c);
    }
}

bool DC_separated(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    Float delta = M_PI / 180;
    for (int j = 0; j * delta < M_PI; j++) {
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, j * delta);
        BoundingBox bb = BoundingBox::from_vector(rotated_S);
        Real long_edge = std::max(bb.dx(), bb.dy());
        std::sort(rotated_S.begin(), rotated_S.end(), [](Coord const &a, Coord const &b) {
            return a.x < b.x;
        });
        Kptree SL(r, rotated_S), SR(r, rotated_S);
        for (size_t i = 0; i < rotated_S.size(); i++)
            SR.insert(i);
        for (std::size_t i = 0; i <= rotated_S.size(); i++) {
            if (SL.has_intersection() && SR.has_intersection()) {
                centers.clear();
                centers.push_back(SL.center_avaliable());
                centers.push_back(SR.center_avaliable());
                return true;
            }
            if (i < rotated_S.size()) {
                SL.insert(i);
                SR.remove(i);
            }
        }
        if (long_edge > r * 5)
            continue;
        for (Real lambdax = bb.xmin; lambdax <= bb.xmax; lambdax += 0.3 * r) {
            // TODO: r到3r之间的情况
        }
    }
    return false;
}

bool DC_close(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    Float delta = M_PI / 6;
    for (int j = 0; j * delta < M_PI; j++) {
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, j * delta);
        BoundingBox bb = BoundingBox::from_vector(rotated_S);
        Real long_edge = std::max(bb.dx(), bb.dy());
        if (long_edge > r * 3)
            continue;
        Real de = r / 2;
        for (Real zx = bb.xmin; zx <= bb.xmax; zx += de) {
            for (Real zy = bb.ymin; zy <= bb.ymax; zy += de) {
                Coord z(zx, zy);
                std::vector<std::size_t> Q0, Q1;
                for (std::size_t i = 0; i < rotated_S.size(); i++) {
                    Coord const &coord = rotated_S[i];
                    if (coord.y < z.y || (coord.y == z.y && coord.x < z.x))
                        Q0.push_back(i);
                    else
                        Q1.push_back(i);
                }
                std::sort(Q0.begin(), Q0.end(), [&](std::size_t a, std::size_t b) {
                    return to_left(rotated_S[b], z, rotated_S[a]);
                });
                std::sort(Q1.begin(), Q1.end(), [&](std::size_t a, std::size_t b) {
                    return to_left(z, rotated_S[b], rotated_S[a]);
                });
                for (std::size_t i = 0; i <= Q0.size(); i++) {
                    Kptree SL(r, rotated_S), SR(r, rotated_S);
                    for (std::size_t j = 0; j < i; j++)
                        SL.insert(Q0[j]);
                    for (std::size_t j = i; j < Q0.size(); j++)
                        SR.insert(Q0[j]);
                    for (int idx: Q1)
                        SR.insert(idx);
                    for (std::size_t j = 0; j <= Q1.size(); j++) {
                        if (SL.has_intersection() && SR.has_intersection()) {
                            centers.clear();
                            centers.push_back(SL.center_avaliable());
                            centers.push_back(SR.center_avaliable());
                            return true;
                        }
                        if (j < Q1.size()) {
                            SL.insert(Q1[j]);
                            SR.remove(Q1[j]);
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool DC(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    if (DC_separated(r, S, centers)) return true;
    if (DC_close(r, S, centers)) return true;
    return false;
}

PCenterResult p_center(int p, std::vector<Coord> const &S, Real eps) {
    assert(p == 2);
    if (S.empty()) {
        PCenterResult result;
        result.r = Real(0);
        Coord center(Real(0), Real(0));
        for (int i = 0; i < p; i++)
            result.centers.push_back(center);
        return result;
    }
    BoundingBox bb = BoundingBox::from_vector(S);
    Real r = (bb.dx() + bb.dy()) / 2;
    Real dr = r / 2;
    Real r_stop = dr * eps;
    std::vector<Coord> centers;
    PCenterResult result;
    result.r = 0;
    while (dr > r_stop) {
        bool affirmative = DC(r, S, centers);
        std::cout << "r = " << r << ", " << (affirmative ? "OK" : "FAIL") << std::endl;
        if (affirmative) {
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
