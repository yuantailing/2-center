#include "center.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include "kptree.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// S内所有点绕o顺时针旋转theta弧度
static void rotate(std::vector<Coord> &S, Float theta, Coord const &o=Coord(Real(0), Real(0))) {
    Float c = std::cos(theta);
    Float s = std::sin(theta);
    for (Coord &coord: S) {
        Coord d = coord - o;
        coord = o + Coord(d.x * c - d.y * s, d.x * s + d.y * c);
    }
}

static bool lt_by_x(Coord const &a, Coord const &b) {
    return a.x < b.x ? true : (a.x == b.x ? a.y < b.y : false);
}

bool DC_separated(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    Float delta = M_PI / 18;
    for (int j = 0; j * delta < M_PI; j++) {
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, j * delta);
        std::sort(rotated_S.begin(), rotated_S.end(), lt_by_x);
        BoundingBox bb = BoundingBox::from_vector(rotated_S);
        Real long_edge = std::max(bb.dx(), bb.dy());
        Kptree SL(r, rotated_S), SR(r, rotated_S);
        for (size_t i = 0; i < rotated_S.size(); i++)
            SR.insert(i);
        for (std::size_t i = 0; i <= rotated_S.size(); i++) {
            if (SL.has_intersection() && SR.has_intersection()) {
                centers.clear();
                centers.push_back(SL.center_avaliable());
                centers.push_back(SR.center_avaliable());
                rotate(centers, -j * delta);
                return true;
            }
            if (i < rotated_S.size()) {
                SL.insert(i);
                SR.remove(i);
            }
        }
        if (long_edge > r * 5)
            continue;
        for (Real lambdax = bb.xmin; lambdax <= bb.xmax; lambdax += 0.3 * r) {
            // TODO: r到3r之间的情况
        }
    }
    return false;
}

bool DC_close(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    Float delta = M_PI / 6;
    for (int j0 = 0; j0 * delta < M_PI; j0++) {
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, j0 * delta);
        std::sort(rotated_S.begin(), rotated_S.end(), lt_by_x);
        BoundingBox bb = BoundingBox::from_vector(rotated_S);
        Real long_edge = std::max(bb.dx(), bb.dy());
        if (long_edge > r * 3)
            continue;
        Real de = r / 2;
        for (Real zx = bb.xmin + r * 0.1015791152251; zx <= bb.xmax; zx += de) {
            for (Real zy = bb.ymin + r * 0.1015791154196; zy <= bb.ymax; zy += de) {
                Coord z(zx, zy);
                std::vector<std::size_t> Q0, Q1;
                for (std::size_t i = 0; i < rotated_S.size(); i++) {
                    Coord const &coord = rotated_S[i];
                    if (coord.y < z.y || (coord.y == z.y && coord.x < z.x))
                        Q0.push_back(i);
                    else
                        Q1.push_back(i);
                }
                std::sort(Q0.begin(), Q0.end(), [&](std::size_t a, std::size_t b) {
                    return to_left(rotated_S[b], z, rotated_S[a]);
                });
                std::sort(Q1.begin(), Q1.end(), [&](std::size_t a, std::size_t b) {
                    return to_left(z, rotated_S[b], rotated_S[a]);
                });
                struct Task {
                    int up, down, left, right;
                    Task(int up, int down, int left, int right):
                        up(up), down(down), left(left), right(right) { }
                };
                std::vector<Task> tasks;
                std::vector<Task> newTasks;
                tasks.push_back(Task(0, Q0.size(), 0, Q1.size()));

                while (!tasks.empty()) {
                    Kptree SL(r, rotated_S), SR(r, rotated_S);
                    int h_pos = (tasks[0].left + tasks[0].right) / 2;
                    int v_pos = 0;
                    for (std::size_t i = 0; i < (size_t)v_pos; i++)
                        SL.insert(Q0[i]);
                    for (std::size_t i = v_pos; i < Q0.size(); i++)
                        SR.insert(Q0[i]);
                    for (std::size_t i = 0; i < (size_t)h_pos; i++)
                        SL.insert(Q1[i]);
                    for (std::size_t i = h_pos; i < Q1.size(); i++)
                        SR.insert(Q1[i]);
                    newTasks.clear();
                    for (Task const &task: tasks) {
                        if (task.up > task.down || task.left > task.right)
                            continue;
                        int mid = (task.left + task.right) / 2;
                        while (h_pos > mid) {
                            h_pos--;
                            SL.remove(Q1[h_pos]);
                            SR.insert(Q1[h_pos]);
                        }
                        while (v_pos < task.up) {
                            SL.insert(Q0[v_pos]);
                            SR.remove(Q0[v_pos]);
                            v_pos++;
                        }
                        int YNNY = 0;
                        while (v_pos < task.down) {
                            bool Y0 = SL.has_intersection();
                            bool Y1 = SR.has_intersection();
                            if (Y0 && Y1) {
                                centers.clear();
                                centers.push_back(SL.center_avaliable());
                                centers.push_back(SR.center_avaliable());
                                rotate(centers, -j0 * delta);
                                return true;
                            }
                            if (!Y0 && !Y1) {
                                newTasks.push_back(Task(task.up, v_pos - 1, mid + 1, task.right));
                                newTasks.push_back(Task(v_pos + 1, task.down, task.left, mid - 1));
                                YNNY = 0;
                                break;
                            }
                            if (Y0 && !Y1)
                                YNNY = 1;
                            else
                                YNNY = 2;
                            SL.insert(Q0[v_pos]);
                            SR.remove(Q0[v_pos]);
                            v_pos++;
                        }
                        while (v_pos < task.down) {
                            SL.insert(Q0[v_pos]);
                            SR.remove(Q0[v_pos]);
                            v_pos++;
                        }
                        if (YNNY == 1)
                            newTasks.push_back(Task(task.up, task.down, mid + 1, task.right));
                        else if (YNNY == 2)
                            newTasks.push_back(Task(task.up, task.down, task.left, mid - 1));
                    }
                    tasks = newTasks;
                }
            }
        }
    }
    return false;
}

bool DC_bf(Real r, std::vector<Coord> const &S0, std::vector<Coord> &centers) {
    std::vector<Coord> S(S0);
    std::sort(S.begin(), S.end(), lt_by_x);
    auto circle_n_circle = [](Coord const &a, Coord const &b, Real r, bool &flag, Coord &p1, Coord &p2) {
        Coord delta = (b - a) / 2;
        Real delta_norm2 = delta.norm2();
        Real r2 = r * r;
        Real line_norm2 = r2 - delta_norm2;
        if (line_norm2 < 0 || delta_norm2 == 0) { // 重合情况可以视为只有一个圆，不需要加判断点，与无交相同
            flag = false;
            return;
        }
        Coord unit = delta / std::sqrt(delta_norm2);
        Coord line = Coord(-unit.y, unit.x) * std::sqrt(line_norm2);
        p1 = a + delta + line;
        p2 = a + delta - line;
        flag = true;
    };
    for (std::size_t i = 1; i < S.size(); i++) {
        bool flag;
        Coord p1, p2;
        circle_n_circle(S[0], S[i], r, flag, p1, p2);
        if (flag) {

        }
    }
    return false;
}

bool DC(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    // return DC_bf(r, S, centers);
    if (DC_separated(r, S, centers)) return true;
    if (DC_close(r, S, centers)) return true;
    return false;
}

PCenterResult p_center(int p, std::vector<Coord> const &S0, Real eps) {
    if (p != 2)
        throw std::invalid_argument("");
    if (S0.empty()) {
        PCenterResult result;
        result.r = Real(0);
        Coord center(Real(0), Real(0));
        for (int i = 0; i < p; i++)
            result.centers.push_back(center);
        return result;
    }
    BoundingBox bb = BoundingBox::from_vector(S0);
    Real lo = 0;
    Real hi = std::sqrt(bb.dx() * bb.dx() + bb.dy() * bb.dy()) * 0.501;
    Real r_stop = hi / 2 * eps / 2;
    std::vector<Coord> S;
    std::default_random_engine e;
    std::uniform_real_distribution<Real> u(-r_stop, r_stop);
    for (Coord const &a: S0) {
        Real x = a.x + u(e);
        Real y = a.y + u(e);
        S.push_back(Coord(x, y));
    }
    std::vector<Coord> centers;
    PCenterResult result;
    result.r = 0;
    while (hi - lo > r_stop) {
        Real mi = (lo + hi) / 2;
        bool affirmative = DC(mi, S, centers);
        std::cout << "r = " << mi << ", " << (affirmative ? "OK" : "FAIL") << std::endl;
        if (affirmative) {
            result.r = mi;
            result.centers = centers;
            hi = mi;
        } else {
            lo = mi;
        }
    }
    return result;
}
