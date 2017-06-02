#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void recalculate();
    void test();

private:
    Ui::MainWindow *ui;
    bool dragging;
    QPoint dragStart;
    qreal zoom;
    QPointF topleft;
    QVector<QPointF> S;
    qreal r;
    QVector<QPointF> centers;
};

#endif // MAINWINDOW_H
