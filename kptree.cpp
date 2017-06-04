#include "kptree.h"
#include <cmath>
#include <iostream>
#include <functional>
#include <stdexcept>
#include <stack>

int Kptree::stat_insert_called = 0;
int Kptree::stat_remove_called = 0;
int Kptree::stat_intersect_called = 0;

Kptree::Kptree(Real r, std::vector<Coord> const &S):
    r(r),
    S(S)
{
    for (std::size_t i = 1; i < S.size(); i++) {
        if (S[i].x < S[i - 1].x)
            throw std::invalid_argument("Kptree point set must be sorted by x");
    }
    flag.resize(S.size());
    for (std::size_t i = 0; i < flag.size(); i++)
        flag[i] = false;

    // construct kptree;
    n = 1;
    while (n < S.size())
        n *= 2;
    nodes.resize(n + S.size() + 1);
    inves.resize(n + S.size() + 1);
    for (KptreeNode &node: nodes) {
        node.narc = 0;
        node.jump = 0;
    }
    for (KptreeNode &node: inves) {
        node.narc = 0;
        node.jump = 0;
    }
}

Kptree::~Kptree() { }

void Kptree::insert(std::size_t idx)
{
    stat_insert_called++;
    std::size_t cur = idx + n;
    nodes[cur].narc = 1;
    inves[cur].narc = 1;
    update_both(cur);
    flag[idx] = true;
}

void Kptree::remove(std::size_t idx)
{
    stat_remove_called++;
    std::size_t cur = idx + n;
    nodes[cur].narc = 0;
    inves[cur].narc = 0;
    update_both(cur);
    flag[idx] = false;
}

bool Kptree::has_intersection_kp() const {
    return compute_center().first;
}

bool Kptree::has_intersection_force() const {
    try {
        center_avaliable_force();
    } catch (std::logic_error const &e) {
        return false;
    }
    return true;
}

bool Kptree::has_intersection() const {
    bool res_kp = has_intersection_kp();
    return res_kp;
    bool res_force = has_intersection_force();
    //if (res_kp == true && res_force == false) {
    if (res_kp != res_force) {
        qDebug() << res_kp << res_force;
        int cnt = 0;
        for (std::size_t i = n; i < n + n; i++) {
            if (nodes[i].narc == 1)
                cnt++;
        }
        qdebug();
        qDebug() << "r =" << r;
        std::exit(1);
    }
    return has_intersection_kp();
}

Coord Kptree::center_avaliable_kp() const {
    return compute_center().second;
}

Coord Kptree::center_avaliable_force() const {
    bool no_point = true;
    for (std::size_t i = 0; i < S.size(); i++) {
        if (!flag[i]) continue;
        no_point = false;
        Coord o(S[i]);
        Coord a(Real(0), Real(0)), b(Real(0), Real(0));
        bool is_circle = true;
        bool failed = false;
        for (std::size_t j = 0; j < S.size(); j++) {
            if (!flag[j] || j == i) continue;
            Coord delta(S[j] - S[i]);
            Real delta_norm = delta.norm();
            Coord unit = delta_norm ? delta / delta_norm : Coord(0, 1);
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
    if (no_point)
        return Coord(Real(0), Real(0));
    throw std::invalid_argument("");
}

Coord Kptree::center_avaliable() const
{
    return center_avaliable_kp();
}

IntersectionResult Kptree::intersection_arcs_with_outer_circles(bool) const
{
    IntersectionResult result;
    return result;
}

void Kptree::update(std::size_t cur, bool up) {
    std::vector<KptreeNode> &nodes(up ? this->nodes : this->inves);
    while (cur) {
        if (isleaf(cur, n)) {
            nodes[cur].lx = nodes[cur].rx = cur - n;
            nodes[cur].larc = nodes[cur].rarc = cur - n;
            Coord const &o(S[cur - n]);
            nodes[cur].ip = Coord(o.x, up ? o.y - this->r : o.y + this->r);
        } else {
            std::size_t l = lchild(cur);
            std::size_t r = rchild(cur);
            if (nodes[l].narc == 0) {
                nodes[cur].narc = nodes[r].narc;
                nodes[cur].lx = nodes[r].lx;
                nodes[cur].rx = nodes[r].rx;
                nodes[cur].jump = 2;
            } else if (nodes[r].narc == 0) {
                nodes[cur].narc = nodes[l].narc;
                nodes[cur].lx = nodes[l].lx;
                nodes[cur].rx = nodes[l].rx;
                nodes[cur].jump = 1;
            } else {
                nodes[cur].lx = nodes[r].lx;
                nodes[cur].rx = nodes[l].rx;
                nodes[cur].jump = 0;
                if (S[nodes[cur].lx].x - this->r > S[nodes[cur].rx].x + this->r) {
                    nodes[cur].narc = 2;
                } else {
                    while (1) {
                        if (nodes[l].jump != 0) {
                            l = nodes[l].jump == 1 ? lchild(l) : rchild(l);
                        } else if (nodes[r].jump != 0) {
                            r = nodes[r].jump == 1 ? lchild(r) : rchild(r);
                        } else if (nodes[l].narc == 1 && nodes[r].narc == 1) {
                            nodes[cur].narc = 2;
                            std::size_t a = nodes[l].larc;
                            std::size_t b = nodes[r].larc;
                            nodes[cur].ip = br_n_br_same(S[a], S[b], this->r, up);
                            if (nodes[cur].ip.x < S[a].x && nodes[cur].ip.x < S[b].x) {
                                if (S[a].y < S[b].y) {
                                    nodes[cur].larc = a;
                                    nodes[cur].rarc = b;
                                } else {
                                    nodes[cur].larc = b;
                                    nodes[cur].rarc = a;
                                }
                                if (!up) std::swap(nodes[cur].larc, nodes[cur].rarc);
                            } else if (nodes[cur].ip.x > S[a].x && nodes[cur].ip.x > S[b].x) {
                                if (S[a].y < S[b].y) {
                                    nodes[cur].larc = b;
                                    nodes[cur].rarc = a;
                                } else {
                                    nodes[cur].larc = a;
                                    nodes[cur].rarc = b;
                                }
                                if (!up) std::swap(nodes[cur].larc, nodes[cur].rarc);
                            } else {
                                nodes[cur].larc = nodes[r].larc;
                                nodes[cur].rarc = nodes[l].larc;
                            }
                            nodes[cur].arc_swapped = nodes[cur].larc != nodes[r].larc;
                            break;
                        } else if (nodes[l].narc == 1) {
                            Coord L = br_n_br_same(S[nodes[l].larc], S[nodes[r].larc], this->r, up);
                            // Coord R = br_n_br_same(S[nodes[l].larc], S[nodes[r].rarc], this->r, up);
                            if (L.x < nodes[r].ip.x) {
                                r = nodes[r].arc_swapped ? lchild(r) : rchild(r);
                            } else { // if (R.x > nodes[r].ip.x) {
                                r = nodes[r].arc_swapped ? rchild(r) : lchild(r);
                            }
                        } else if (nodes[r].narc == 1) {
                            Coord L = br_n_br_same(S[nodes[l].larc], S[nodes[r].larc], this->r, up);
                            // Coord R = br_n_br_same(S[nodes[l].rarc], S[nodes[r].larc], this->r, up);
                            if (L.x < nodes[l].ip.x) {
                                l = nodes[l].arc_swapped ? lchild(l) : rchild(l);
                            } else { // if (R.x > nodes[l].ip.x) {
                                l = nodes[l].arc_swapped ? rchild(l) : lchild(l);
                            }
                        } else {
                            Coord LL = br_n_br_same(S[nodes[l].larc], S[nodes[r].larc], this->r, up);
                            Coord LR = br_n_br_same(S[nodes[l].larc], S[nodes[r].rarc], this->r, up);
                            Coord RL = br_n_br_same(S[nodes[l].rarc], S[nodes[r].larc], this->r, up);
                            // Coord RR = br_n_br_same(S[nodes[l].rarc], S[nodes[r].rarc], this->r, up);
                            if (br_toleft(S[nodes[l].larc], LL, nodes[l].ip, true, up) && br_toleft(S[nodes[r].larc], LL, nodes[r].ip, true, up)) { // (LL.x < nodes[l].ip.x && LL.x < nodes[r].ip.x) {
                                r = nodes[r].arc_swapped ? lchild(r) : rchild(r);
                            } else if (br_toleft(S[nodes[l].larc], LR, nodes[l].ip, true, up) && br_toleft(S[nodes[r].rarc], LR, nodes[r].ip, false, up)) { // (LR.x < nodes[l].ip.x && LR.x > nodes[r].ip.x) {
                                l = nodes[l].arc_swapped ? lchild(l) : rchild(l);
                                r = nodes[r].arc_swapped ? rchild(r) : lchild(r);
                            } else if (br_toleft(S[nodes[l].rarc], RL, nodes[l].ip, false, up) && br_toleft(S[nodes[r].larc], RL, nodes[r].ip, true, up)) { // (RL.x > nodes[l].ip.x && RL.x < nodes[r].ip.x) {
                                l = nodes[l].arc_swapped ? rchild(l) : lchild(l);
                                r = nodes[r].arc_swapped ? lchild(r) : rchild(r);
                            } else { // if (RR.x > nodes[l].ip.x && RR.x > nodes[r].ip.x) {
                                l = nodes[l].arc_swapped ? rchild(l) : lchild(l);
                            }
                        }
                    }
                }
            }
        }
        cur = parent(cur);
    }
}

std::pair<bool, Coord> Kptree::compute_center() const {
    stat_intersect_called++;
    if (nodes[root()].narc == 0)
        return std::make_pair(true, Coord(0, 0));
    if (S[nodes[root()].lx].x - r > S[nodes[root()].rx].x + r)
        return std::make_pair(false, Coord(0, 0));
    std::size_t cur = root(); // cursor in K+(P)
    Real lmost = S[nodes[root()].lx].x - this->r, rmost = S[nodes[root()].rx].x + this->r;
    auto between_most = [&](Real x) { return lmost <= x && x <= rmost; };
    while (!isleaf(cur, n)) {
        if (nodes[cur].jump != 0) {
            cur = nodes[cur].jump == 1 ? lchild(cur) : rchild(cur);
            continue;
        }
        Coord search_point = nodes[cur].ip;
        //qDebug() << "search_point" << cur;
        std::size_t invcur = root(); // cursor in K-(P)
        while (!isleaf(invcur, n)) {
            if (inves[invcur].jump != 0) {
                invcur = inves[invcur].jump == 1 ? lchild(invcur) : rchild(invcur);
            } else if (std::abs(S[inves[invcur].larc].x - S[inves[invcur].rarc].x) < 1e-5) { // 有精度问题
                if (S[inves[invcur].larc].y < S[inves[invcur].rarc].y) {
                    invcur = inves[invcur].arc_swapped ? lchild(invcur) : rchild(invcur);
                } else {
                    invcur = inves[invcur].arc_swapped ? rchild(invcur) : lchild(invcur);
                }
            } else if (search_point.x < inves[invcur].ip.x) {
                invcur = inves[invcur].arc_swapped ? lchild(invcur) : rchild(invcur);
            } else {
                invcur = inves[invcur].arc_swapped ? rchild(invcur) : lchild(invcur);
            }
        }
        //qDebug() << "invcur" << invcur;
        auto in_brdown = [](Coord const &o, Real r, Coord const &p) { // P是否在Br-(o)内
            if (r * r >= (p - o).norm2())
                return true;
            return o.x - r <= p.x && p.x <= o.x + r && p.y <= o.y;
        };
        auto br_intersect = [&](Coord const &up, Coord const &down, Real r, Coord x, bool left) { //
            auto res = brup_n_brdown(up, down, r);
            //qDebug() << std::get<0>(res);
            if (std::get<0>(res) == false)
                return false;
            return (between_most(std::get<1>(res).x) && br_toleft(up, std::get<1>(res), x, left, true)) ||
                    (between_most(std::get<2>(res).x) && br_toleft(up, std::get<2>(res), x, left, true));
        };
        //qDebug() << nodes[cur].larc << inves[invcur].larc;
        if (between_most(search_point.x) && in_brdown(S[inves[invcur].larc], this->r, search_point)) {
            return std::make_pair(true, search_point);
        } else if (br_intersect(S[nodes[cur].larc], S[inves[invcur].larc], this->r, search_point, true)) {
            rmost = std::min(rmost, search_point.x);
            cur = nodes[cur].arc_swapped ? lchild(cur) : rchild(cur);
        } else if (br_intersect(S[nodes[cur].rarc], S[inves[invcur].larc], this->r, search_point, false)) {
            lmost = std::max(lmost, search_point.x);
            cur = nodes[cur].arc_swapped ? rchild(cur) : lchild(cur);
        } else {
            return std::make_pair(false, Coord(0, 0));
        }
    }
    std::size_t invcur = root();
    while (!isleaf(invcur, n)) {
        if (inves[invcur].jump != 0) {
            invcur = inves[invcur].jump == 1 ? lchild(invcur) : rchild(invcur);
            continue;
        }
        auto in_brup = [](Coord const &o, Real r, Coord const &p) {
            if (r * r >= (p - o).norm2())
                return true;
            return o.x - r <= p.x && p.x <= o.x + r && p.y >= o.y;
        };
        auto br_intersect = [&](Coord const &up, Coord const &down, Real r, Coord x, bool left) { //
            auto res = brup_n_brdown(up, down, r);
            if (std::get<0>(res) == false)
                return false;
            return (between_most(std::get<1>(res).x) && br_toleft(down, std::get<1>(res), x, left, false)) ||
                    (between_most(std::get<2>(res).x) && br_toleft(down, std::get<2>(res), x, left, false));
        };
        Coord const &search_point(inves[invcur].ip);
        if (std::abs(S[inves[invcur].larc].x - S[inves[invcur].rarc].x) < 1e-5) { // 有精度问题
            if (S[inves[invcur].larc].y < S[inves[invcur].rarc].y) {
                invcur = inves[invcur].arc_swapped ? lchild(invcur) : rchild(invcur);
            } else {
                invcur = inves[invcur].arc_swapped ? rchild(invcur) : lchild(invcur);
            }
        } else if (search_point.x < lmost) {
            invcur = inves[invcur].arc_swapped ? rchild(invcur) : lchild(invcur);
        } else if (search_point.x > rmost) {
            invcur = inves[invcur].arc_swapped ? lchild(invcur) : rchild(invcur);
        } else if (between_most(search_point.x) && in_brup(S[nodes[cur].larc], this->r, search_point)) {
            return std::make_pair(true, search_point);
        } else if (br_intersect(S[nodes[cur].larc], S[inves[invcur].larc], this->r, search_point, true)) {
            rmost = std::min(rmost, search_point.x);
            invcur = inves[invcur].arc_swapped ? lchild(invcur) : rchild(invcur);
        } else if (br_intersect(S[nodes[cur].larc], S[inves[invcur].rarc], this->r, search_point, false)) {
            lmost = std::max(lmost, search_point.x);
            invcur = inves[invcur].arc_swapped ? rchild(invcur) : lchild(invcur);
        } else {
            return std::make_pair(false, Coord(0, 0));
        }
    }
    auto res = brup_n_brdown(S[nodes[cur].larc], S[inves[invcur].larc], this->r);
    if (std::get<0>(res) == false)
        return std::make_pair(false, Coord(0, 0));
    if ((lmost <= std::get<1>(res).x && std::get<1>(res).x <= rmost) || (lmost <= std::get<2>(res).x && std::get<2>(res).x <= rmost))
        return std::make_pair(true, std::get<1>(res));
    return std::make_pair(false, Coord(0, 0));
}

Coord Kptree::br_n_br_same(Coord const &Si, Coord const &Sj, Real r, bool up) { // 如果不相交，返回极限；如果相交，返回交点；如果重合，返回最左下的点
    if (Si.x > Sj.x)
        return br_n_br_same(Sj, Si, r, up);
    if (!up) {
        Coord Si2(Si.x, -Si.y);
        Coord Sj2(Sj.x, -Sj.y);
        Coord res = br_n_br_same(Si2, Sj2, r, true);
        return Coord(res.x, -res.y);
    }
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
    Coord unit = delta_norm ? delta / delta_norm : Coord(0, 1);
    unit = unit / unit.norm();
    Coord line(-unit.y, unit.x);
    Real line_norm2 = r * r - delta_norm * delta_norm / 4;
    if (line_norm2 < 0) {
        return Si + delta / 2;
    }
    line = line * std::sqrt(line_norm2);
    return Si + delta / 2 - line;
}

std::tuple<bool, Coord, Coord> Kptree::brup_n_brdown(Coord const &up, Coord const &down, Real r) { // 若相交，返回2个交点。若x重合，返回左上右下或右上左下；若xy重合，返回左右两点。
    if (up.x > down.x) {
        Coord u(-up.x, up.y);
        Coord d(-down.x, down.y);
        auto result = brup_n_brdown(u, d, r);
        return std::make_tuple(std::get<0>(result), Coord(-std::get<2>(result).x, std::get<2>(result).y), Coord(-std::get<1>(result).x, std::get<1>(result).y));
    }
    if (down.x - up.x - r > r) {
        return std::make_tuple(false, Coord(0, 0), Coord(0, 0));
    }
    Real delta_y2 = r * r - (down.x - up.x - r) * (down.x - up.x - r);
    if (up.y - down.y < 0 || (up.y - down.y) * (up.y - down.y) <= delta_y2) {
        Real delta_y = std::sqrt(delta_y2);
        return std::make_tuple(true, Coord(down.x - r, up.y - delta_y), Coord(up.x + r, down.y + delta_y));
    }
    Coord delta(down - up);
    Real delta_norm = delta.norm();
    Coord unit = delta_norm ? delta / delta_norm : Coord(0, 1);
    unit = unit / unit.norm();
    Coord line(-unit.y, unit.x);
    Real line_norm2 = r * r - delta_norm * delta_norm / 4;
    if (line_norm2 < 0) {
        return std::make_tuple(false, Coord(0, 0), Coord(0, 0));
    }
    line = line * std::sqrt(line_norm2);
    return std::make_tuple(true, up + delta / 2 - line, up + delta / 2 + line);
}

bool Kptree::br_toleft(Coord const &o, Coord const &a, Coord const &b, bool left, bool up) { // Br+(O)上的两点a、b，判断a是否在b的左边（或重合）
    if (!up) {
        return br_toleft(Coord(o.x, -o.y), Coord(a.x, -a.y), Coord(b.x, -b.y), left, true);
    }
    if (left) {
        if ((a.x < o.x) == (b.x < o.x)) return (a.y >= b.y) == (a.x < o.x);
        return a.x <= b.x;
    } else {
        if ((a.x < o.x) == (b.x < o.x)) return (a.y >= b.y) == (a.x > o.x);
        return a.x >= b.x;
    }
}
