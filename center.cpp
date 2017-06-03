#include "center.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include "kptree.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// S内所有点绕o顺时针旋转theta弧度
static void rotate(std::vector<Coord> &S, Float theta, Coord const &o=Coord(Real(0), Real(0))) {
    Float c = std::cos(theta);
    Float s = std::sin(theta);
    for (Coord &coord: S) {
        Coord d = coord - o;
        coord = o + Coord(d.x * c - d.y * s, d.x * s + d.y * c);
    }
}

static bool lt_by_x(Coord const &a, Coord const &b) {
    return a.x < b.x ? true : (a.x == b.x ? a.y < b.y : false);
}

bool DC_separated(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    Float delta = M_PI / 18;
    for (int j = 0; j * delta < M_PI; j++) {
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, j * delta);
        std::sort(rotated_S.begin(), rotated_S.end(), lt_by_x);
        BoundingBox bb = BoundingBox::from_vector(rotated_S);
        Real long_edge = std::max(bb.dx(), bb.dy());
        Kptree SL(r, rotated_S), SR(r, rotated_S);
        for (size_t i = 0; i < rotated_S.size(); i++)
            SR.insert(i);
        for (std::size_t i = 0; i <= rotated_S.size(); i++) {
            if (SL.has_intersection() && SR.has_intersection()) {
                centers.clear();
                centers.push_back(SL.center_avaliable());
                centers.push_back(SR.center_avaliable());
                rotate(centers, -j * delta);
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
    for (int j0 = 0; j0 * delta < M_PI; j0++) {
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, j0 * delta);
        std::sort(rotated_S.begin(), rotated_S.end(), lt_by_x);
        BoundingBox bb = BoundingBox::from_vector(rotated_S);
        Real long_edge = std::max(bb.dx(), bb.dy());
        if (long_edge > r * 3)
            continue;
        Real de = r / 2;
        for (Real zx = bb.xmin + r * 0.1015791152251; zx <= bb.xmax; zx += de) {
            for (Real zy = bb.ymin + r * 0.1015791154196; zy <= bb.ymax; zy += de) {
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
                            rotate(centers, -j0 * delta);
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
    // if (DC_close(r, S, centers)) return true;
    return false;
}

PCenterResult p_center(int p, std::vector<Coord> const &S0, Real eps) {
    if (p != 2)
        throw std::invalid_argument("");
    if (S0.empty()) {
        PCenterResult result;
        result.r = Real(0);
        Coord center(Real(0), Real(0));
        for (int i = 0; i < p; i++)
            result.centers.push_back(center);
        return result;
    }
    BoundingBox bb = BoundingBox::from_vector(S0);
    Real lo = 0;
    Real hi = std::sqrt(bb.dx() * bb.dx() + bb.dy() * bb.dy()) * 0.501;
    Real r_stop = hi / 2 * eps / 2;
    std::vector<Coord> S;
    std::default_random_engine e;
    std::uniform_real_distribution<Real> u(-r_stop, r_stop);
    for (Coord const &a: S0) {
        Real x = a.x + u(e);
        Real y = a.y + u(e);
        S.push_back(Coord(x, y));
    }
    std::vector<Coord> centers;
    PCenterResult result;
    result.r = 0;
    while (hi - lo > r_stop) {
        Real mi = (lo + hi) / 2;
        bool affirmative = DC(mi, S, centers);
        std::cout << "r = " << mi << ", " << (affirmative ? "OK" : "FAIL") << std::endl;
        if (affirmative) {
            result.r = mi;
            result.centers = centers;
            hi = mi;
        } else {
            lo = mi;
        }
    }
    return result;
}
