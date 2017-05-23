#include "kptree.h"
#include <cassert>

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
    for (std::size_t i = 0; i < S.size(); i++) {
        if (!flag[i]) continue;
        for (std::size_t j = 0; j < S.size(); j++) {
            if (!flag[j]) continue;
            Coord delta = S[i] - S[j];
            if (delta.x * delta.x + delta.y * delta.y > r * r)
                return false;
        }
    }
    return true;
}

Coord Kptree::center_avaliable() const {
    return Coord(Real(0), Real(0));
}
