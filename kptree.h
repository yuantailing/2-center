#ifndef KPTREE_H
#define KPTREE_H

#include <vector>
#include "real.h"


struct ArcRFixed {
    Coord o;
    Coord oa;
    Coord ob;
};

struct IntersectionOnArc {
    std::size_t idx;
    bool is_in;
    Coord oc;
};

struct IntersectionResult {
public:
    std::vector<ArcRFixed> arcs; // KP树上的圆的交集区域（用一组弧表示）
    std::vector<std::size_t> in_kp_at_arc0; // 以arcs[0]的左端点为圆心，能覆盖的点集
    std::vector<std::vector<IntersectionOnArc> > intersection_on_arcs; // 每条弧进出的点集
};

class Kptree {
public:
    Kptree(Real r, std::vector<Coord> const &S); // S应已按x排序
    ~Kptree();
    void insert(std::size_t idx);
    void remove(std::size_t idx);
    bool has_intersection() const;
    Coord center_avaliable() const;
    IntersectionResult intersection_arcs_with_outer_circles(bool only_self=false) const; // only_self：只计算KP树的交集，不计算与其它圆的交点
private:
    Kptree(Kptree const &) = delete;
    Kptree &operator =(Kptree const &) = delete;
    Real r;
    std::vector<Coord> const &S;
    std::vector<bool> flag;
};

#endif // KPTREE_H
