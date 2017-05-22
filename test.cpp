#include <iostream>
#include <cstdlib>
#include "center.h"

int main() {
    srand(1);
    int n = 10;
    std::vector<Coord> S(n);
    for (Coord &coord: S) {
        coord.x = Real((static_cast<double>(rand()) / RAND_MAX * 100));
        coord.y = Real((static_cast<double>(rand()) / RAND_MAX * 100));
    }
    PCenterResult result = p_center(2, S);
    std::cout << "r = " << result.r << std::endl;
    for (Coord const &p: result.centers) {
        std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
    }
    return 0;
}
