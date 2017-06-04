#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <vector>
#include <random>
#include <QtGlobal>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include "center.h"
#include "kptree.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    test();
    ui->setupUi(this);
    qsrand(0);
    leftButtonPressed = false;
    dragging = false;
    reset_zoom();
    for (int i = 0; i < 10; i++) {
        qreal x = (qreal)qrand() / RAND_MAX * 200.0 - 100.0;
        qreal y = (qreal)qrand() / RAND_MAX * 200.0 - 100.0;
        S.push_back(QPointF(x, y));
    }
    // recalculate();
    ui->pushButtonCircle->click();
    this->resize(800, 800);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setBrush(Qt::blue);
    for (QPointF const &p: S) {
        QPointF q = QPointF(p.x() - topleft.x(), -p.y() + topleft.y()) * zoom;
        painter.drawEllipse(q, 3., 3.);
    }
    painter.setPen(Qt::red);
    painter.setBrush(Qt::NoBrush);
    for (QPointF const &p: centers) {
        QPointF q = QPointF(p.x() - topleft.x(), -p.y() + topleft.y()) * zoom;
        painter.drawEllipse(q, r * zoom, r * zoom);
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
        if (!S.empty()) {
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
            if (mindist * zoom < 100) {
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
        } else {
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
    S.clear();
    reset_zoom();
    recalculate();
    update();
}

void MainWindow::on_pushButtonCircle_clicked() {
    S.clear();
    for (double theta = 0; theta < M_PI * 2 - M_PI / 40; theta += M_PI / 10) {
        S.push_back(QPointF(-50 + 70 * cos(theta), 70 * sin(theta)));
        S.push_back(QPointF(50 + 70 * cos(theta), 70 * sin(theta)));
    }
    reset_zoom();
    recalculate();
    update();
}

void MainWindow::on_pushButtonRectangle_clicked()
{
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

void MainWindow::reset_zoom() {
    topleft = QPointF(-266.67, 266.67);
    zoom = 1.5;
}

void MainWindow::recalculate() {
    std::vector<Coord> v;
    for (QPointF p: S) {
        v.push_back(Coord(Real(p.x()), Real(p.y())));
    }
    PCenterResult result;
    quick_case_only = ui->checkBoxQuick->isChecked();
    result = p_center(2, v, 2e-4);
    r = (qreal)result.r;
    centers.clear();
    for (Coord p: result.centers) {
        centers.push_back(QPointF((qreal)p.x, (qreal)p.y));
    }
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
