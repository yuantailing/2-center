#ifndef REAL_H
#define REAL_H

typedef double Real;

class Coord {
public:
    Coord() = default;
    Coord(Real x, Real y): x(x), y(y) { }
    Coord(Coord const &) = default;
    Coord &operator=(Coord const &) = default;
    Real x;
    Real y;
};

#endif // KPTREE_H
