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

bool quick_case_only = false;

static int tmp_dc_case;
static Real tmp_dc_rotate_angle;
static std::vector<std::size_t> tmp_dc_division_left;

int dc_case = 0;
Float dc_rotate_angle = 0;
std::vector<std::size_t> dc_division_left;

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
    for (int j = 0; j * delta < M_PI - delta / 2; j++) {
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, j * delta);
        std::vector<std::pair<Coord, std::size_t> > paired(rotated_S.size());
        for (std::size_t i = 0; i < rotated_S.size(); i++) {
            paired[i] = std::make_pair(rotated_S[i], i);
        }
        std::sort(paired.begin(), paired.end(), [](std::pair<Coord, std::size_t> const &a, std::pair<Coord, std::size_t> const &b) {
            if (a.first.x != b.first.x) return a.first.x < b.first.x;
            if (a.first.y != b.first.y) return a.first.y < b.first.y;
            return a.second < b.second;
        });
        for (std::size_t i = 0; i < paired.size(); i++)
            rotated_S[i] = paired[i].first;
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
                tmp_dc_case = 1;
                tmp_dc_rotate_angle = j * delta;
                tmp_dc_division_left.clear();
                for (std::size_t k = 0; k < i; k++)
                    tmp_dc_division_left.push_back(paired[k].second);
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
    Float delta = M_PI / 3;
    for (int j0 = 0; j0 * delta < M_PI - delta / 2; j0++) {
        Real rotate_angle = j0 * delta;
        std::vector<Coord> rotated_S = S;
        rotate(rotated_S, rotate_angle);
        std::vector<std::pair<Coord, std::size_t> > paired(rotated_S.size());
        for (std::size_t i = 0; i < rotated_S.size(); i++) {
            paired[i] = std::make_pair(rotated_S[i], i);
        }
        std::sort(paired.begin(), paired.end(), [](std::pair<Coord, std::size_t> const &a, std::pair<Coord, std::size_t> const &b) {
            if (a.first.x != b.first.x) return a.first.x < b.first.x;
            if (a.first.y != b.first.y) return a.first.y < b.first.y;
            return a.second < b.second;
        });
        for (std::size_t i = 0; i < paired.size(); i++)
            rotated_S[i] = paired[i].first;
        BoundingBox bb = BoundingBox::from_vector(rotated_S);
        Real long_edge = std::max(bb.dx(), bb.dy());
        if (long_edge > r * 3)
            continue;
        Real de = r * 0.7057413;
        for (Real zx = bb.xmin + de / 2; zx == bb.xmin + de / 2 || zx <= bb.xmax - de / 2; zx += de) {
            for (Real zy = bb.ymin + de / 2; zy == bb.xmin + de / 2 || zy <= bb.ymax - de / 2; zy += de) {
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
                                rotate(centers, -rotate_angle);
                                tmp_dc_case = 3;
                                tmp_dc_rotate_angle = rotate_angle;
                                tmp_dc_division_left.clear();
                                for (int i = 0; i < v_pos; i++)
                                    tmp_dc_division_left.push_back(paired[Q0[i]].second);
                                for (int i = 0; i < h_pos; i++)
                                    tmp_dc_division_left.push_back(paired[Q1[i]].second);
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

bool DC(Real r, std::vector<Coord> const &S, std::vector<Coord> &centers) {
    if (DC_separated(r, S, centers)) return true;
    if (!quick_case_only && DC_close(r, S, centers)) return true;
    return false;
}

bool DC_check(Real r, std::vector<Coord> const &S, std::vector<Coord> const &centers) {
    for (Coord const &x: S) {
        bool found = false;
        for (Coord const &center: centers) {
            if ((x - center).norm2() <= r * r) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

void one_circle(std::vector<Coord> const &S, Real eps, Real &r_out, Coord &center_out) {
    r_out = 50.0;
    center_out = Coord(50, 50);
    if (S.size() == 0) {
        r_out = 0;
        center_out = Coord(0, 0);
    } else if (S.size() == 1) {
        r_out = 0;
        center_out = S[0];
    } else if (S.size() == 2) {
        center_out = (S[0] + S[1]) / 2;
        r_out = (S[0] - center_out).norm();
    } else {

    }
}

void fix_circle(std::vector<Coord> const &S, std::vector<Coord> const &centers, Real eps, Real &r_out, std::vector<Coord> &centers_out) { // centers和centers_out可以相同
    if (centers.empty())
        return;
    std::vector<std::vector<Coord> > Ss;
    Ss.resize(centers.size());
    for (Coord const &x: S) {
        Real mindist = 0;
        std::size_t minrank = 0;
        for (std::size_t i = 0; i < centers.size(); i++) {
            Real dist = (x - centers[i]).norm2();
            if (i == 0 || dist < mindist) {
                mindist = dist;
                minrank = i;
            }
        }
        Ss[minrank].push_back(x);
    }
    r_out = 0;
    centers_out.resize(centers.size());
    for (std::size_t i = 0; i < centers_out.size(); i++) {
        Real r_tmp;
        one_circle(Ss[i], eps, r_tmp, centers_out[i]);
        r_out = std::max(r_out, r_tmp);
    }
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
    if (S0.size() <= 2) {
        PCenterResult result;
        result.r = Real(0);
        result.centers.push_back(S0[0]);
        result.centers.push_back(S0[S0.size() - 1]);
        return result;
    }
    BoundingBox bb = BoundingBox::from_vector(S0);
    PCenterResult result;
    Real r_stop = std::sqrt(bb.dx() * bb.dx() + bb.dy() * bb.dy()) * eps / 4;
    Real lo = 0;
    Real hi = std::sqrt(bb.dx() * bb.dx() + bb.dy() * bb.dy()) / 2;
    std::vector<Coord> S;
    static std::default_random_engine random_engine = std::default_random_engine(0);
    std::uniform_real_distribution<Real> u(-r_stop, r_stop);
    for (Coord const &a: S0) {
        Real x = a.x + u(random_engine);
        Real y = a.y + u(random_engine);
        S.push_back(Coord(x, y));
    }
    dc_case = 0;
    std::vector<Coord> centers;
    result.r = 0;
    while (hi - lo > r_stop) {
        Real mi = (lo + hi) / 2;
        bool affirmative = false;
        for (int i = 0; i < 20; i++) {
            std::vector<Coord> S1;
            if (i == 0) {
                affirmative = DC(mi, S, centers);
            } else {
                Real mul = pow(2, i);
                for (Coord const &a: S0) {
                    Real x = a.x + u(random_engine) * mul;
                    Real y = a.y + u(random_engine) * mul;
                    S1.push_back(Coord(x, y));
                }
                affirmative = DC(mi, S1, centers);
            }
            if (!affirmative)
                break;
            if (i == 0) {
                if (DC_check(mi + r_stop, S, centers))
                    break;
            } else {
                if (DC_check(mi + r_stop, S1, centers))
                    break;
            }
        }
        std::cout << "r = " << mi << ", " << (affirmative ? "OK" : "FAIL") << std::endl;
        if (affirmative) {
            dc_case = tmp_dc_case;
            dc_rotate_angle = tmp_dc_rotate_angle;
            dc_division_left = tmp_dc_division_left;
            result.r = mi;
            result.centers = centers;
            hi = mi;
        } else {
            lo = mi;
        }
    }
    qDebug() << Kptree::get_stat_insert_called() << Kptree::get_stat_remove_called() << Kptree::get_stat_intersect_called();
    return result;
}
