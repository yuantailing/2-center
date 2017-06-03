#include "mainwindow.h"
#include "ui_mainwindow.h"
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
    dragging = false;
    topleft = QPointF(-266.67, 266.67);
    zoom = 1.5;
    for (int i = 0; i < 4; i++) {
        qreal x = (qreal)qrand() / RAND_MAX * 200.0 - 100.0;
        qreal y = (qreal)qrand() / RAND_MAX * 200.0 - 100.0;
        S.push_back(QPointF(x, y));
    }
    recalculate();
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
        painter.drawEllipse(q, 2., 2.);
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
        if (ui->checkBoxDragMode->isChecked()) {
            dragging = true;
            dragStart = event->pos();
        } else {
            S.push_back(p);
            recalculate();
            update();
        }
    } else if (event->button() == Qt::RightButton) {
        if (!S.empty()) {
            int idx = 0;
            qreal mindist = 0;
            bool found = false;
            for (int i = 0; i < S.size(); i++) {
                qreal dist = (S[i] - p).manhattanLength();
                if (!found || dist < mindist) {
                    idx = i;
                    mindist = dist;
                    found = true;
                }
            }
            if (mindist * zoom < 20) {
                S.remove(idx);
                recalculate();
                update();
            }
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (dragging) {
        QPoint p(event->pos() - dragStart);
        topleft -= QPointF(p.x(), -p.y()) / zoom;
        dragStart = event->pos();
        update();
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (dragging) {
            QPoint p(event->pos() - dragStart);
            topleft -= QPointF(p.x(), -p.y()) / zoom;
            dragStart = event->pos();
            dragging = false;
            update();
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

void MainWindow::recalculate() {
    std::vector<Coord> v;
    for (QPointF p: S) {
        v.push_back(Coord(Real(p.x()), Real(p.y())));
    }
    PCenterResult result;
    result = p_center(2, v);
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
