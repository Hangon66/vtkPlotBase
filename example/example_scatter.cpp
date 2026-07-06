/**
 * @file example_scatter.cpp
 * @brief 演示 vtkPlot2D 二维散点图 + 线条叠加的用法
 */

#include "vtkplot2d.h"
#include "drawable/vtkscatterseries.h"
#include "drawable/vtklineseries.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QVector>
#include <QPointF>
#include <cmath>
#include <cstdlib>
#include <ctime>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QApplication a(argc, argv);

    vtkPlot2D w;
    w.setWindowTitle("Scatter Plot with Lines");
    w.resize(800, 600);
    w.setBackground(QColor(51, 51, 51));

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ---- 散点序列 A：随机圆形散点（青色）----
    QVector<QPointF> scatterA;
    scatterA.reserve(50);
    for (int i = 0; i < 50; ++i) {
        double x = -5.0 + 10.0 * std::rand() / RAND_MAX;
        double y = -5.0 + 10.0 * std::rand() / RAND_MAX;
        scatterA.append(QPointF(x, y));
    }
    vtkScatterSeries *seriesA = w.addScatterSeries(
        scatterA, QColor(0, 255, 255), Marker2DStyle::Circle, 10.0, "随机散点 A");

    // ---- 散点序列 B：随机方形散点（黄色）----
    QVector<QPointF> scatterB;
    scatterB.reserve(30);
    for (int i = 0; i < 30; ++i) {
        double x = -3.0 + 6.0 * std::rand() / RAND_MAX;
        double y = -3.0 + 6.0 * std::rand() / RAND_MAX;
        scatterB.append(QPointF(x, y));
    }
    vtkScatterSeries *seriesB = w.addScatterSeries(
        scatterB, QColor(255, 255, 0), Marker2DStyle::Square, 8.0, "随机散点 B");

    // ---- 线条序列：sin 曲线（白色线）----
    QVector<QPointF> linePoints;
    linePoints.reserve(200);
    for (int i = 0; i < 200; ++i) {
        double x = -5.0 + 10.0 * i / 199.0;
        double y = 5.0 * std::sin(x);
        linePoints.append(QPointF(x, y));
    }
    vtkLineSeries *line = w.addLineSeries(
        linePoints, QColor(255, 255, 255), 2.0, "sin(x) * 5");

    // 设置标题
    seriesA->chart()->SetTitle("散点图与线条叠加示例");
    seriesA->chart()->GetAxis(vtkAxis::BOTTOM)->SetTitle("X");
    seriesA->chart()->GetAxis(vtkAxis::LEFT)->SetTitle("Y");

    // 启用交互以支持平移/缩放
    w.setInteractionEnabled(true);

    w.show();
    return a.exec();
}
