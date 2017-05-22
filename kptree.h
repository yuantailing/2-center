#ifndef KPTREE_H
#define KPTREE_H

#include "real.h"

class KptreeNode {
    Coord coord;
};

class Kptree {
public:
    Kptree(Real r);
    ~Kptree();
    KptreeNode *insert(Coord coord);
    void remove(KptreeNode *node);
    bool has_intersection();
private:
    Kptree(Kptree const &) = delete;
    Kptree &operator=(Kptree const &) = delete;
};

#endif // KPTREE_H
