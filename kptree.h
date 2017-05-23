#ifndef KPTREE_H
#define KPTREE_H

#include <vector>
#include "real.h"


class Kptree {
public:
    Kptree(Real r, std::vector<Coord> const &S);
    ~Kptree();
    void insert(std::size_t idx);
    void remove(std::size_t idx);
    bool has_intersection() const;
    Coord center_avaliable() const;
private:
    Kptree(Kptree const &) = delete;
    Kptree &operator =(Kptree const &) = delete;
    Real r;
    std::vector<Coord> const &S;
    std::vector<bool> flag;
};

#endif // KPTREE_H
