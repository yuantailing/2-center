#ifndef KPTREE_H
#define KPTREE_H

#include <vector>
#include <QDebug>
#include "real.h"


struct ArcRFixed
{
    Coord o;
    Coord oa;
    Coord ob;
};

struct IntersectionOnArc
{
    std::size_t idx;
    bool is_in;
    Coord oc;
};

struct IntersectionResult
{
public:
    std::vector<ArcRFixed> arcs; // KP树上的圆的交集区域（用一组弧表示）
    std::vector<std::vector<IntersectionOnArc> > intersection_on_arcs; // 每条弧进出的点集
};

struct KptreeNode
{
    int narc;
    std::size_t lx, rx; // range left, right;
    std::size_t larc, rarc;
    Coord ip; // intersect point;
    void qdebug() const { qDebug() << narc << ip.x << ip.y << lx << rx << larc << rarc; }
};

class Kptree
{
public:
    Kptree(Real r, std::vector<Coord> const &S); // S应已按x排序
    ~Kptree();
    void insert(std::size_t idx);
    void remove(std::size_t idx);
    bool has_intersection() const;
    Coord center_avaliable() const;
    IntersectionResult intersection_arcs_with_outer_circles(bool only_self=false) const; // only_self：只计算KP树的交集，不计算与其它圆的交点
public:
    Kptree(Kptree const &) = delete;
    Kptree &operator =(Kptree const &) = delete;
    Real r;
    std::vector<Coord> const &S;
    std::vector<bool> flag;

    inline static std::size_t root() { return 1; }
    inline static std::size_t lchild(std::size_t x) { return x << 1; }
    inline static std::size_t rchild(std::size_t x) { return (x << 1) | 1; }
    inline static std::size_t parent(std::size_t x) { return x >> 1; }
    inline static bool isleaf(std::size_t x, std::size_t n) { return x >= n; }

    static Coord intersection(Coord const &Si, Coord const &Sj, Real r, bool up);
    void update(std::size_t cur, bool up);
    void update_both(std::size_t cur) { update(cur, true); update(cur, false); }
    void qdebug() const { qDebug() << nodes.size(); for (KptreeNode const &node: nodes) node.qdebug(); }

    std::size_t n;
    std::size_t h;
    std::vector<KptreeNode> nodes;
    std::vector<KptreeNode> inves;
};

#endif // KPTREE_H
