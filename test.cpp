#include <cstdlib>
#include <cmath>
#include <iostream>
#include <GL/glut.h>
#include "center.h"

std::vector<Coord> S;
PCenterResult result;

void draw_circle(double cx, double cy, double r, int num_segments) {
    glBegin(GL_LINE_LOOP);
    for(int ii = 0; ii < num_segments; ii++) {
        double theta = 2.0f * 3.1415926f * double(ii) / double(num_segments);//get the current angle
        double x = r * std::cos(theta);
        double y = r * std::sin(theta);
        glVertex2d(x + cx, y + cy);
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(2.0);
    Coord center(Real(50.0), Real(50.0));
    Real scale(100.0);
    glColor3d(1.0, 0., 0.0);
    draw_circle((result.centers[0].x - center.x) / scale, (result.centers[0].y - center.y) / scale, result.r / scale, 60);
    draw_circle((result.centers[1].x - center.x) / scale, (result.centers[1].y - center.y) / scale, result.r / scale, 60);
    glColor3d(1.0, 1.0, 1.0);
    glBegin(GL_POINTS);
    for (Coord const &coord: S) {
        Coord p = (coord - center) / scale;
        glVertex2d((double)p.x, (double)p.y);
    }
    glEnd();
    glFlush();
};

int main(int argc, char *argv[]) {
    std::srand(1);
    int n = 20;
    S.resize(n);
    for (Coord &coord: S) {
        coord.x = Real((static_cast<double>(rand()) / RAND_MAX * 100));
        coord.y = Real((static_cast<double>(rand()) / RAND_MAX * 100));
    }
    result = p_center(2, S);
    std::cout << "r = " << result.r << std::endl;
    for (Coord const &p: result.centers) {
        std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
    }

    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowSize(400, 400);
    glutCreateWindow("Three Window");
    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
