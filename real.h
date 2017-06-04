#ifndef REAL_H
#define REAL_H

#include <cmath>
#include <vector>

typedef double Real; // 用于表示坐标

typedef Real Float; // 用于表示角度

class Coord {
public:
    Coord() = default;
    Coord(Real x, Real y): x(x), y(y) { }
    Coord(Coord const &) = default;
    Coord &operator =(Coord const &) = default;
    Real x;
    Real y;
    inline Real norm2() const { return x * x + y * y; }
    inline Real norm() const { return std::sqrt(norm2()); }
    inline Coord operator +(Coord const &o) const { return Coord(x + o.x, y + o.y); }
    inline Coord operator -(Coord const &o) const { return Coord(x - o.x, y - o.y); }
    inline Coord operator *(Real r) const { return Coord(x * r, y * r); }
    inline Coord operator /(Real r) const { return Coord(x / r, y / r); }
    inline bool operator ==(Coord const &o) const { return x == o.x && y == o.y; }
};

// 包围盒
class BoundingBox {
public:
    Real xmin, xmax;
    Real ymin, ymax;
    static BoundingBox from_vector(std::vector<Coord> const &S);
    Real dx() const { return xmax - xmin; }
    Real dy() const { return ymax - ymin; }
};

// 判断s是否在pq的左边
inline bool to_left(Coord p, Coord q, Coord s) {
    Real x = p.x * q.y - p.y * q.x + q.x * s.y - q.y * s.x + s.x * p.y - s.y * p.x;
    return x > 0;
}

// 判断s是否在pq的左边
inline bool not_to_right(Coord p, Coord q, Coord s) {
    Real x = p.x * q.y - p.y * q.x + q.x * s.y - q.y * s.x + s.x * p.y - s.y * p.x;
    return x >= 0;
}


#endif // KPTREE_H
