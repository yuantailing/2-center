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

private slots:
    void on_pushButtonExportPoints_clicked();
    void on_pushButtonClear_clicked();
    void on_pushButtonCircle_clicked();
    void on_pushButtonRectangle_clicked();
    void on_pushButtonHyperbola_clicked();
    void on_checkBoxQuick_stateChanged(int arg1);

private:
    void reset_zoom();
    void recalculate();
    void test();

private:
    Ui::MainWindow *ui;
    bool leftButtonPressed;
    bool dragging;
    QPoint dragStart;
    qreal zoom;
    QPointF topleft;
    QVector<QPointF> S;
    qreal r;
    QVector<QPointF> centers;
};

#endif // MAINWINDOW_H
