#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <vector>
#include <random>
#include <QtGlobal>
#include <QMessageBox>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include "center.h"
#include "kptree.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->labelKptreeStat->hide();
    ui->checkBoxQuick->hide();

    qsrand(0);
    leftButtonPressed = false;
    dragging = false;
    reset_zoom();

    kp_last_insert_call = Kptree::get_stat_insert_called();
    kp_last_remove_call = Kptree::get_stat_remove_called();
    kp_last_intersect_call = Kptree::get_stat_intersect_called();

    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timer()));
    timer.setInterval(16);
    time_multiple = 20.;
    ticks = 0;

    this->setWindowTitle(QString("Planar 2-Center Problem"));
    on_pushButtonCircle_clicked();
    resize(800, 800);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event) {
    // defined colors
    QColor blue(0, 140, 255);
    QColor lblue(130, 200, 255);
    QColor red(255, 0, 0);
    QColor black(20, 20, 20);
    QPainter painter(this);
    auto draw_circle = [&](QPointF const &p, Real r) {
        QPointF q = QPointF(p.x() - topleft.x(), -p.y() + topleft.y()) * zoom;
        painter.drawEllipse(q, r, r);
    };
    auto paint_points_and_circles = [&](Real r, QVector<QPointF> S, QVector<QPointF> const &centers) {
        painter.setPen(red);
        painter.setBrush(red);
        for (QPointF const &p: centers)
            draw_circle(p, 3.);
        painter.setPen(black);
        painter.setBrush(black);
        for (QPointF const &p: S)
            draw_circle(p, 3.);
        painter.setPen(red);
        painter.setBrush(Qt::NoBrush);
        for (QPointF const &p: centers)
            draw_circle(p, r * zoom);
    };
    if (ui->comboBoxCenterNum->currentIndex() == 0) { // 1-center
        auto norm2 = [](QPointF const &p) { return p.x() * p.x() + p.y() * p.y(); };
        QVector<QPointF> S(this->S);
        std::shuffle(S.begin(), S.end(), std::default_random_engine(0));
        qreal const eps = 1e-6;
        int time_multiple = (int)(10 * this->time_multiple);
        int now = 0;
        int current = -1;
        QPointF d_current = S.empty() ? QPointF(0, 0) : S.first();
        qreal r2_current = 0;
        QVector<int> key_points;
        QPointF d;
        qreal r2;
        auto add_tick = [&](int current_) {
            now++;
            if (now * time_multiple <= ticks) {
                current = current_;
            }
        };
        auto set_key_points = [&](std::initializer_list<int> l) {
            now++;
            if (now * time_multiple <= ticks) {
                current = -1;
                key_points.clear();
                for (int idx: l)
                    key_points.push_back(idx);
                d_current = d;
                r2_current = r2;
            }
        };
        auto one_center = [&]() {
            if (S.empty()) {
                d = QPointF(0, 0);
                r2 = 0;
                set_key_points({});
                return;
            } else {
                d = S[0];
                r2 = 0;
                set_key_points({0});
                if (S.size() == 1) return;
            }
            d = (S[0] + S[1]) / 2;
            r2 = norm2(S[0] - d);
            add_tick(0);
            add_tick(1);
            set_key_points({0, 1});
            for (int i = 2; i < S.size(); i++) {
                add_tick(i);
                if (norm2(S[i] - d) <= r2) continue;
                d = (S[0] + S[i]) / 2;
                r2 = norm2(S[0] - d);
                set_key_points({0, i});
                for (int j = 1; j < i; j++) {
                    add_tick(j);
                    if (norm2(S[j] - d) <= r2) continue;
                    d = (S[i] + S[j]) / 2;
                    r2 = norm2(S[i] - d);
                    qreal r2eps = std::sqrt(r2) * (1 + eps);
                    r2eps *= r2eps;
                    set_key_points({i, j});
                    for (int k = 0; k < j; k++) {
                        add_tick(k);
                        if (norm2(S[k] - d) <= r2eps) continue;
                        QPointF A = S[i] - S[j];
                        QPointF B = S[i] - S[k];
                        qreal c1 = (norm2(S[i]) - norm2(S[j])) / 2;
                        qreal c2 = (norm2(S[i]) - norm2(S[k])) / 2;
                        qreal det = A.x() * B.y() - A.y() * B.x();
                        d.setX((c1 * B.y() - c2 * A.y()) / det);
                        d.setY((c2 * A.x() - c1 * B.x()) / det);
                        r2 = norm2(S[i] - d);
                        r2eps = std::sqrt(r2) * (1 + eps);
                        r2eps *= r2eps;
                        set_key_points({i, j, k});
                    }
                }
            }
            add_tick(S.size());
        };
        one_center();
        ui->horizontalSliderProgress->setMaximum((now + 8) * time_multiple);
        if (ticks == 0) {
            QVector<QPointF> centers_draw;
            centers_draw.push_back(d);
            paint_points_and_circles(std::sqrt(r2), S, centers_draw);
        } else {
            QVector<QPointF> S_draw;
            QVector<QPointF> centers_draw;
            for (int i = current + 1; i < S.size(); i++)
                S_draw.push_back(S[i]);
            centers_draw.push_back(d_current);
            paint_points_and_circles(std::sqrt(r2_current), S_draw, centers_draw);
            painter.setPen(Qt::red);
            painter.setBrush(Qt::red);
            for (int p: key_points)
                draw_circle(S[p], 5.);
            for (int i = 0; i <= current && i < S.size(); i++) {
                if (i == current) {
                    painter.setPen(Qt::black);
                    painter.setBrush(Qt::NoBrush);
                    draw_circle(S[i], 6.);
                } else {
                    painter.setPen(Qt::black);
                    painter.setBrush(Qt::NoBrush);
                    draw_circle(S[i], 4.);
                }
            }
        }
        QMainWindow::paintEvent(event);
        return;
    }
    if (ticks == 0 || S.size() <= 2 || centers.size() != 2 || dc_case == 0) {
        paint_points_and_circles(r, S, centers);
        QMainWindow::paintEvent(event);
        return;
    }
    auto rotated = [](QVector<QPointF> const &S, Float theta, QPointF const &o) {
        QVector<QPointF> res;
        res.reserve(S.size());
        Float c = std::cos(theta);
        Float s = std::sin(theta);
        for (QPointF const &coord: S) {
            QPointF d = coord - o;
            res.push_back(o + QPointF(d.x() * c - d.y() * s, d.x() * s + d.y() * c));
        }
        return res;
    };
    auto seperate = [](QVector<QPointF> &S, QVector<QPointF> &centers, qreal delta, int num_left) {
        for (int i = 0; i < S.size(); i++) {
                S[i].setX(S[i].x() + (i < num_left ? -delta : delta));
        }
        centers[0].setX(centers[0].x() - delta);
        centers[1].setX(centers[1].x() + delta);
    };

    int rotate_time = (int)(200 * time_multiple);
    int seperate_time = (int)(150 * time_multiple);
    int one_br_time = (int)(20 * time_multiple);

    std::vector<Coord> v;
    for (QPointF p: S)
        v.push_back(Coord(Real(p.x()), Real(p.y())));
    BoundingBox bb(BoundingBox::from_vector(v));
    Float angle = dc_rotate_angle * std::min(1., 1. / rotate_time * ticks);
    qreal max_seperate_distance = r * 1.2;
    qreal seperate_distance = max_seperate_distance * std::max(0., std::min(1., (ticks - rotate_time) * 1.0 / seperate_time));
    QPointF o((bb.xmax + bb.xmin) / 2, (bb.xmax + bb.xmin) / 2);
    QVector<QPointF> S_draw;
    for (std::size_t i = 0; i < dc_division_left.size(); i++)
        if (dc_division_left[i] == true)
            S_draw.push_back(S[(int)i]);
    int n_points_in_left = S_draw.size();
    for (std::size_t i = 0; i < dc_division_left.size(); i++)
        if (dc_division_left[i] == false)
            S_draw.push_back(S[(int)i]);
    S_draw = rotated(S_draw, angle, o);
    QVector<QPointF> center_draw = rotated(centers, angle, o);
    seperate(S_draw, center_draw, seperate_distance, n_points_in_left);
    auto lt_by_x = [](QPointF const &a, QPointF const &b) {
        return a.x() < b.x() ? true : (a.x() == b.x() ? a.y() < b.y() : false);
    };
    std::sort(S_draw.begin(), S_draw.begin() + n_points_in_left, lt_by_x);
    std::sort(S_draw.begin() + n_points_in_left, S_draw.end(), lt_by_x);
    int draw_type = 0;
    int draw_br_n = (ticks - rotate_time - seperate_time) / one_br_time;
    if (draw_br_n > 0) draw_type = 1;
    if (draw_br_n > S_draw.size()) {
        draw_br_n -= S_draw.size();
        draw_type = 2;
        if (draw_br_n > S_draw.size()) {
            draw_br_n = S_draw.size();
            draw_type = 3;
        }
    }
    painter.setPen(lblue);
    painter.setBrush(lblue);
    for (int i = 0; i < this->height(); i++) {
        qreal y = topleft.y() - i / zoom;
        int xminleftup = 0, xmaxleftup = width() - 1, xminrightup = 0, xmaxrightup = width() - 1;
        int xminleftdown = 0, xmaxleftdown = width() - 1, xminrightdown = 0, xmaxrightdown = width() - 1;
        for (int j = 0; j < S_draw.size(); j++) {
            if (j >= draw_br_n)
                break;
            QPointF o = S_draw[j];
            qreal xminup, xmaxup;
            qreal xmindown, xmaxdown;
            qreal delta = r * r - (o.y() - y) * (o.y() - y);
            if (delta < 0) {
                xminup = xmindown = o.x() + 1;
                xmaxup = xmaxdown = o.x() - 1;
            } else {
                delta = std::sqrt(delta);
                xminup = xmindown = o.x() - delta;
                xmaxup = xmaxdown = o.x() + delta;
            }
            if (o.y() <= y) {
                xminup = o.x() - r;
                xmaxup = o.x() + r;
            } else if (o.y() >= y) {
                xmindown = o.x() - r;
                xmaxdown = o.x() + r;
            }
            if (j < n_points_in_left) {
                xminleftup = std::max(xminleftup, (int)std::round((xminup - topleft.x()) * zoom));
                xmaxleftup = std::min(xmaxleftup, (int)std::round((xmaxup - topleft.x()) * zoom));
                xminleftdown = std::max(xminleftdown, (int)std::round((xmindown - topleft.x()) * zoom));
                xmaxleftdown = std::min(xmaxleftdown, (int)std::round((xmaxdown - topleft.x()) * zoom));
            } else {
                xminrightup = std::max(xminrightup, (int)std::round((xminup - topleft.x()) * zoom));
                xmaxrightup = std::min(xmaxrightup, (int)std::round((xmaxup - topleft.x()) * zoom));
                xminrightdown = std::max(xminrightdown, (int)std::round((xmindown - topleft.x()) * zoom));
                xmaxrightdown = std::min(xmaxrightdown, (int)std::round((xmaxdown - topleft.x()) * zoom));
            }
        }
        if (draw_type == 1) {
            if (xminleftup < xmaxleftup) {
                painter.drawLine(QPointF(xminleftup, i), QPointF(xmaxleftup, i));
            }
            if (xminrightup < xmaxrightup && draw_br_n > n_points_in_left) {
                painter.drawLine(QPointF(xminrightup, i), QPointF(xmaxrightup, i));
            }
        } else if (draw_type == 2) {
            if (xminleftdown < xmaxleftdown) {
                painter.drawLine(QPointF(xminleftdown, i), QPointF(xmaxleftdown, i));
            }
            if (xminrightdown < xmaxrightdown && draw_br_n > n_points_in_left) {
                painter.drawLine(QPointF(xminrightdown, i), QPointF(xmaxrightdown, i));
            }
        } else if (draw_type == 3) {
            painter.setOpacity(0.5);
            if (xminleftup < xmaxleftup) {
                painter.drawLine(QPointF(xminleftup, i), QPointF(xmaxleftup, i));
            }
            if (xminrightup < xmaxrightup && draw_br_n > n_points_in_left) {
                painter.drawLine(QPointF(xminrightup, i), QPointF(xmaxrightup, i));
            }
            if (xminleftdown < xmaxleftdown) {
                painter.drawLine(QPointF(xminleftdown, i), QPointF(xmaxleftdown, i));
            }
            if (xminrightdown < xmaxrightdown && draw_br_n > n_points_in_left) {
                painter.drawLine(QPointF(xminrightdown, i), QPointF(xmaxrightdown, i));
            }
            painter.setOpacity(1.);
            xminleftup = std::max(xminleftup, xminleftdown);
            xmaxleftup = std::min(xmaxleftup, xmaxleftdown);
            xminrightup = std::max(xminrightup, xminrightdown);
            xmaxrightup = std::min(xmaxrightup, xmaxrightdown);
            if (xminleftup < xmaxleftup) {
                painter.drawLine(QPointF(xminleftup, i), QPointF(xmaxleftup, i));
            }
            if (xminrightup < xmaxrightup && draw_br_n > n_points_in_left) {
                painter.drawLine(QPointF(xminrightup, i), QPointF(xmaxrightup, i));
            }
        }
    }
    for (int i = 0; i < draw_br_n; i++) {
        QPointF const &p(S_draw[i]);
        QPointF q = QPointF(p.x() - topleft.x(), -p.y() + topleft.y()) * zoom;
        Real r = this->r * this->zoom;
        painter.setOpacity(1.0);
        painter.setPen(blue);
        painter.setBrush(Qt::NoBrush);
        QRectF rect(q.x() - r, q.y() - r, r * 2, r * 2);
        if (draw_type == 1) {
            painter.drawArc(rect, 180 * 16, 180 * 16);
            painter.drawLine(QPointF(q.x() - r, q.y()), QPointF(q.x() - r, 0));
            painter.drawLine(QPointF(q.x() + r, q.y()), QPointF(q.x() + r, 0));
        } else if (draw_type == 2) {
            painter.drawArc(rect, 0 * 16, 180 * 16);
            painter.drawLine(QPointF(q.x() - r, q.y()), QPointF(q.x() - r, this->height()));
            painter.drawLine(QPointF(q.x() + r, q.y()), QPointF(q.x() + r, this->height()));
        } else {
            painter.drawEllipse(q, r, r);
            painter.drawLine(QPointF(q.x() - r, 0), QPointF(q.x() - r, this->height()));
            painter.drawLine(QPointF(q.x() + r, 0), QPointF(q.x() + r, this->height()));
        }
    }
    paint_points_and_circles(r, S_draw, center_draw);
    if ((draw_type == 1 || draw_type == 2) && draw_br_n > 0) {
        painter.setPen(Qt::red);
        painter.setBrush(Qt::red);
        draw_circle(S_draw[draw_br_n - 1], 5.);
    }
    QMainWindow::paintEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPointF p(event->pos());
    p = QPointF(p.x(), -p.y()) / zoom + topleft;
    if (event->button() == Qt::LeftButton) {
        leftButtonPressed = true;
        dragStart = event->pos();
    } else if (event->button() == Qt::RightButton) {
        if (!S.empty() && !prompt_stop()) {
            auto norm = [](QPointF r) { return std::sqrt(r.x() * r.x() + r.y() * r.y()); };
            int idx = 0;
            qreal mindist = 0;
            bool found = false;
            for (int i = 0; i < S.size(); i++) {
                qreal dist = norm(S[i] - p);
                if (!found || dist < mindist) {
                    idx = i;
                    mindist = dist;
                    found = true;
                }
            }
            if (mindist * zoom < 40) {
                S.remove(idx);
                recalculate();
                update();
            }
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (leftButtonPressed && (dragging || (dragStart - event->pos()).manhattanLength() > 20)) {
        QPoint p(event->pos() - dragStart);
        topleft -= QPointF(p.x(), -p.y()) / zoom;
        dragStart = event->pos();
        dragging = true;
        update();
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    QPointF p(event->pos());
    p = QPointF(p.x(), -p.y()) / zoom + topleft;
    if (event->button() == Qt::LeftButton) {
        leftButtonPressed = false;
        if (dragging) {
            QPoint p(event->pos() - dragStart);
            topleft -= QPointF(p.x(), -p.y()) / zoom;
            dragStart = event->pos();
            dragging = false;
            update();
        } else if (!prompt_stop()){
            auto norm = [](QPointF r) { return std::sqrt(r.x() * r.x() + r.y() * r.y()); };
            bool found = false;
            for (QPointF const &ps: S) {
                if (norm(ps - p) < 2.5 / zoom) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                S.push_back(p);
                if (centers.size() != 2 || (norm(p - centers[0]) > r && norm(p - centers[1]) > r)) {
                    recalculate();
                }
                update();
            }
        }
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    qreal zoom_old = zoom;
    zoom *= event->delta() > 0 ? 1.2 : 1. / 1.2;
    zoom = qMax(0.5, qMin(10., zoom));
    QPointF p(event->pos());
    p = QPointF(p.x(), -p.y());
    topleft += p / zoom_old - p / zoom;
    update();
    QMainWindow::wheelEvent(event);
}

void MainWindow::on_pushButtonExportPoints_clicked() {
    qDebug() << "n =" << S.size();
    for (QPointF const &x: S) {
        qDebug() << x.x() << x.y();
    }
}

void MainWindow::on_pushButtonClear_clicked() {
    on_pushButtonStop_clicked();
    S.clear();
    reset_zoom();
    recalculate();
    update();
}

void MainWindow::on_pushButtonRandom_clicked() {
    if (prompt_stop())
        return;
    S.clear();
    for (int i = 0; i < 10; i++) {
        qreal x = (qreal)qrand() / RAND_MAX * 200.0 - 100.0;
        qreal y = (qreal)qrand() / RAND_MAX * 200.0 - 100.0;
        S.push_back(QPointF(x, y));
    }
    reset_zoom();
    recalculate();
    update();
}

void MainWindow::on_pushButtonCircle_clicked() {
    if (prompt_stop())
        return;
    S.clear();
    for (double theta = 0; theta < M_PI * 2 - M_PI / 40; theta += M_PI / 10) {
        S.push_back(QPointF(-20 + 70 * cos(theta), -50 + 70 * sin(theta)));
        S.push_back(QPointF(20 + 70 * cos(theta), 50 + 70 * sin(theta)));
    }
    reset_zoom();
    recalculate();
    update();
}

void MainWindow::on_pushButtonRectangle_clicked() {
    if (prompt_stop())
        return;
    S.clear();
    for (int i = 0; i < 11; i++) {
        S.push_back(QPointF(i * 20 - 100, -40));
        S.push_back(QPointF(i * 20 - 100, 40));
    }
    for (int i = 0; i < 3; i++) {
        S.push_back(QPointF(-100, i * 20 - 20));
        S.push_back(QPointF(100, i * 20 - 20));
    }
    for (int i = -9; i < 6; i++) {
        if (i != -2 && i != 2)
            S.push_back(QPointF(0, i * 20));
    }
    reset_zoom();
    recalculate();
    update();
}

void MainWindow::on_pushButtonHyperbola_clicked() {
    if (prompt_stop())
        return;
    S.clear();
    for (int y = -100; y <= 100; y += 10)
        S.push_back(QPointF(std::sqrt(y * y * 1.5 + 1000), y));
    QPointF o(0, -70);
    S.push_back(o);
    for (int y = 5; y < 180; y += 14.3) {
        S.push_back(o + QPointF(std::sqrt(y) * 7, y));
        S.push_back(o + QPointF(-std::sqrt(y) * 7, y));
    }
    reset_zoom();
    recalculate();
    update();
}

void MainWindow::on_checkBoxQuick_stateChanged(int) {
    recalculate();
    update();
}

void MainWindow::on_pushButtonPlay_clicked() {
    if (ticks >= ui->horizontalSliderProgress->maximum())
        on_pushButtonStop_clicked();
    if ((std::size_t)S.size() > dc_division_left.size())
        recalculate();
    timer.start();
}

void MainWindow::on_pushButtonPause_clicked() {
    timer.stop();
}

void MainWindow::on_pushButtonStop_clicked() {
    timer.stop();
    ticks = 0;
    ui->horizontalSliderProgress->setValue(ticks);
    update();
}

void MainWindow::on_horizontalSliderProgress_sliderMoved(int position) {
    ticks = position;
    update();
}

void MainWindow::on_comboBoxCenterNum_currentIndexChanged(int) {
    on_pushButtonStop_clicked();
    recalculate();
}

void MainWindow::on_timer() {
    ticks += ui->horizontalScrollBarStep->value();
    ui->horizontalSliderProgress->setValue(ticks);
    if (ticks > ui->horizontalSliderProgress->maximum())
        timer.stop();
    update();
}

void MainWindow::reset_zoom() {
    topleft = QPointF(-266.67, 266.67);
    zoom = 1.5;
}

void MainWindow::recalculate() {
    if (ui->comboBoxCenterNum->currentIndex() == 0) { // 1-center
        ui->horizontalSliderProgress->setMaximum((int)(1000 * time_multiple));
        return;
    }
    std::vector<Coord> v;
    for (QPointF p: S) {
        v.push_back(Coord(Real(p.x()), Real(p.y())));
    }
    PCenterResult result;
    quick_case_only = ui->checkBoxQuick->isChecked();
    result = p_center(2, v, 1e-3);
    r = (qreal)result.r;
    centers.clear();
    for (Coord p: result.centers)
        centers.push_back(QPointF((qreal)p.x, (qreal)p.y));
    ui->labelKptreeStat->setText(QString("K(P) tree interface called times: insert %1, remove %2, intersect %3").
                                 arg(Kptree::get_stat_insert_called() - kp_last_insert_call).
                                 arg(Kptree::get_stat_remove_called() - kp_last_remove_call).
                                 arg(Kptree::get_stat_intersect_called() - kp_last_intersect_call));
    kp_last_insert_call = Kptree::get_stat_insert_called();
    kp_last_remove_call = Kptree::get_stat_remove_called();
    kp_last_intersect_call = Kptree::get_stat_intersect_called();
    int rotate_time = (int)(200 * time_multiple);
    int seperate_time = (int)(150 * time_multiple);
    int one_br_time = (int)(20 * time_multiple);
    ui->horizontalSliderProgress->setMaximum(rotate_time + seperate_time + one_br_time * (S.size() * 2 + 4));
}

bool MainWindow::prompt_stop() {
    if (ticks > 0) {
        if (ui->comboBoxCenterNum->currentIndex() == 0) {
            on_pushButtonStop_clicked();
            return false;
        } else {
            QMessageBox::information(this, "Tips", "Please click \"Stop\" before do this.");
            return true;
        }
    }
    return false;
}

static bool lt_by_x(Coord const &a, Coord const &b) {
    return a.x < b.x ? true : (a.x == b.x ? a.y < b.y : false);
}

void MainWindow::test() {
    qsrand(0);
    std::vector<Coord> A;
    A.push_back(Coord(-5.33667 , -2.66667));
    A.push_back(Coord(-2.00333 , 0.666667));
    A.push_back(Coord(-0.67 , -5.33333));
    A.push_back(Coord(217.997 , -149.333));
    A.push_back(Coord(217.997 , -141.333));
    Real r = 3.82813;
    sort(A.begin(), A.end(), lt_by_x);
    Kptree tree(r, A);
    for (std::size_t i = 3; i < 5; i++) {
        tree.insert(i);
    }
    tree.has_intersection();
}
