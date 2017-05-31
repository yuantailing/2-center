#include "kptree.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <stack>
using namespace std;

Kptree::Kptree(Real r, std::vector<Coord> const &S):
    r(r),
    S(S)
{
    for (std::size_t i = 1; i < S.size(); i++) {
        if (S[i].x < S[i - 1].x)
            throw std::invalid_argument("Kptree node must be sorted by x");
    }
    flag.resize(S.size());
    for (std::size_t i = 0; i < flag.size(); i++)
        flag[i] = false;

    // construct kptree;
    n = 1;
    while (n < S.size())
        n *= 2;
    nodes.resize(n * 2);
    for (KptreeNode &node: nodes)
        node.narc = 0;
}

Kptree::~Kptree() { }

void Kptree::insert(std::size_t idx)
{
    std::size_t cur = idx + n;
    nodes[cur].narc = 1;
    update(cur);
    flag[idx] = true;
}

void Kptree::remove(std::size_t idx)
{
    std::size_t cur = idx + n;
    nodes[cur].narc = 0;
    update(cur);
    flag[idx] = false;
}

void Kptree::update(std::size_t cur) {
    if (!cur) return;
    if (isleaf(cur)) {
        nodes[cur].lx = nodes[cur].rx = cur - n;
        nodes[cur].larc = nodes[cur].rarc = cur - n;
    } else {
        std::size_t l = lchild(cur);
        std::size_t r = rchild(cur);
        if (nodes[l].narc == 0) {
            nodes[cur] = nodes[r];
        } else if (nodes[r].narc == 0) {
            nodes[cur] = nodes[l];
        } else {
            nodes[cur].lx = nodes[r].lx;
            nodes[cur].rx = nodes[l].rx;
            while (1) {
                if (nodes[l].narc == 1 && nodes[r].narc == 1) {
                    nodes[cur].narc = 2;
                    nodes[cur].larc = nodes[r].larc;
                    nodes[cur].rarc = nodes[l].larc;
                    nodes[cur].ip = intersection(S[nodes[cur].rarc], S[nodes[cur].larc]);
                    break;
                } else if (nodes[l].narc == 1) {
                    Coord L = intersection(S[nodes[l].larc], S[nodes[r].larc]);
                    // Coord R = intersection(S[nodes[l].larc], S[nodes[r].rarc]);
                    if (L.x < nodes[r].ip.x) {
                        r = rchild(r);
                    } else { // if (R.x > nodes[r].ip.x) {
                        r = lchild(r);
                    }
                } else if (nodes[r].narc == 1) {
                    Coord L = intersection(S[nodes[l].larc], S[nodes[r].larc]);
                    // Coord R = intersection(S[nodes[l].rarc], S[nodes[r].larc]);
                    if (L.x < nodes[l].ip.x) {
                        l = rchild(l);
                    } else { // if (R.x > nodes[l].ip.x) {
                        l = lchild(l);
                    }

                } else {
                    Coord LL = intersection(S[nodes[l].larc], S[nodes[r].larc]);
                    Coord LR = intersection(S[nodes[l].larc], S[nodes[r].rarc]);
                    Coord RL = intersection(S[nodes[l].rarc], S[nodes[r].larc]);
                    // Coord RR = intersection(S[nodes[l].rarc], S[nodes[r].rarc]);
                    if (LL.x < nodes[l].ip.x && LL.x < nodes[r].ip.x) {
                        r = rchild(r);
                    } else if (LR.x < nodes[l].ip.x && LR.x > nodes[r].ip.x) {
                        l = rchild(l);
                        r = lchild(r);
                    } else if (RL.x > nodes[l].ip.x && RL.x < nodes[r].ip.x) {
                        l = lchild(l);
                        r = rchild(r);
                    } else { // if (RR.x > nodes[l].ip.x && RR.x > nodes[r].ip.x) {
                        l = lchild(l);
                    }
                }
            }
        }
    }
    update(parent(cur));
}

bool Kptree::has_intersection() const
{
    try
    {
        center_avaliable();
    }
    catch (std::logic_error const &e)
    {
        return false;
    }
    return true;
}

Coord Kptree::center_avaliable() const
{
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

IntersectionResult Kptree::intersection_arcs_with_outer_circles(bool) const
{
    IntersectionResult result;
    return result;
}

Coord Kptree::intersection(Coord const &Si, Coord const &Sj) {
    Real x = Si.x + r;
    Real dy2 = r * r - (Sj.x - x) * (Sj.x - x);
    if (dy2 <= 0) {
        return Coord(x, std::max(Si.y, Sj.y));
    } else {
        Real y = Sj.y - std::sqrt(dy2);
        if (y > Si.y) {
            return Coord(x, y);
        }
    }
    x = Sj.x - r;
    Real y = Si.y - std::sqrt(dy2);
    if (y > Sj.y) {
        return Coord(x, y);
    }
    Coord delta(Sj - Si);
    Real delta_norm = delta.norm();
    Coord unit = delta / delta_norm;
    unit = unit / unit.norm();
    Coord line(-unit.y, unit.x);
    Real line_norm2 = r * r - delta_norm * delta_norm / 4;
    if (line_norm2 < 0) {
        return Si + delta / 2;
    }
    line = line * std::sqrt(line_norm2);
    return Si + delta / 2 - line;
}
