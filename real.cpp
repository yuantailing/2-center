#include "real.h"

BoundingBox BoundingBox::from_vector(std::vector<Coord> const &S) {
    BoundingBox bb;
    if (S.empty()) {
        bb.xmin = bb.xmax = bb.ymin = bb.ymax = Real(0);
    } else {
        bb.xmin = bb.xmax = S.front().x;
        bb.ymin = bb.ymax = S.front().y;
    }
    for (Coord const &coord: S) {
        bb.xmin = std::min(bb.xmin, coord.x);
        bb.xmax = std::max(bb.xmax, coord.x);
        bb.ymin = std::min(bb.ymin, coord.y);
        bb.ymax = std::max(bb.ymax, coord.y);
    }
    return bb;
}
