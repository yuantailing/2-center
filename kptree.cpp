#include "kptree.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

Kptree::Kptree(Real r, std::vector<Coord> const &S):
    r(r),
    S(S) {
    flag.resize(S.size());
    for (std::size_t i = 0; i < flag.size(); i++)
        flag[i] = false;
}

Kptree::~Kptree() { }

void Kptree::insert(std::size_t idx) {
    flag[idx] = true;
}

void Kptree::remove(std::size_t idx) {
    flag[idx] = false;
}

bool Kptree::has_intersection() const {
    try {
        center_avaliable();
    } catch (std::logic_error const &e) {
        return false;
    }
    return true;
}

Coord Kptree::center_avaliable() const {
#if 0
    IntersectionResult arcs = intersection_arcs_with_outer_circles(true);
    if (arcs.arcs.empty())
        return Coord(Real(0), Real(0));
    else
        return arcs.arcs.front().o + arcs.arcs.front().oa;
#else
    // 在未实现intersection_arcs_with_outer_circles时的替代方案
    for (std::size_t i = 0; i < S.size(); i++) {
        if (!flag[i]) continue;
        Coord o(S[i]);
        Coord a(Real(0), Real(0)), b(Real(0), Real(0));
        bool is_circle = true;
        bool failed = false;
        for (std::size_t j = 0; j < S.size(); j++) {
            if (!flag[j] || j == i) continue;
            Coord delta(S[j] - S[i]);
            Real delta_norm = delta.norm();
            Coord unit = delta / delta_norm;
            unit = unit / unit.norm();
            Coord line(-unit.y, unit.x);
            Real line_norm2 = r * r - delta_norm * delta_norm / 4;
            if (line_norm2 < 0) {
                throw std::invalid_argument("");
            }
            line = line * std::sqrt(line_norm2);
            Coord c(o + delta / 2 - line), d(o + delta / 2 + line);
            if (is_circle) {
                a = c;
                b = d;
                is_circle = false;
            } else {
                if (not_to_right(o, a, c) && not_to_right(o, c, b)) a = c;
                else if (!(not_to_right(o, c, a) && not_to_right(o, a, d))) failed = true;
                if (not_to_right(o, a, d) && not_to_right(o, d, b)) b = d;
                else if (!(not_to_right(o, c, b) && not_to_right(o, b, d))) failed = true;
            }
        }
        if (failed)
            continue;
        if (is_circle)
            return o;
        return a;
    }
    throw std::invalid_argument("");
#endif
}

IntersectionResult Kptree::intersection_arcs_with_outer_circles(bool) const {
    IntersectionResult result;
    return result;
}
