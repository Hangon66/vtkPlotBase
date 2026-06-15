/**
 * @file example_heatmap2d_markers.cpp
 * @brief 演示 vtkPlot2D 二维热力图 + 多组标记点的用法
 */

#include "vtkplot2d.h"
#include "drawable/vtkheatmap2d.h"
#include "drawable/vtkmarkergroup2d.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVector>
#include <cmath>

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
    w.setWindowTitle("2D Heatmap with Markers");
    w.resize(800, 600);

    // 生成二维热力图数据：sin(x) * cos(y)
    const int size = 400;
    QVector<double> data;
    data.reserve(size * size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            data.append(
                std::sin(i * 2.0 * M_PI / 180.0) *
                std::cos(j * M_PI / 180.0));
        }
    }

    // 添加二维热力图
    vtkHeatmap2D *heatmap = w.addHeatmap2D(data, size, size, "Value");
    heatmap->setOrigin(100.0, -100.0);
    heatmap->setSpacing(2.0, 1.0);
    heatmap->setChartTitle("2D Heatmap with Markers");
    heatmap->setXAxisTitle("X Axis");
    heatmap->setYAxisTitle("Y Axis");
    heatmap->setValueRange(-1.0, 1.0);

    // 设置背景颜色
    w.setBackground(QColor(112, 128, 144));  // SlateGray

    // ---- 标记组 A：白色菱形 ----
    vtkMarkerGroup2D *groupA = w.addMarkerGroup(
        heatmap, "Group A", Qt::white, Marker2DStyle::Diamond, 12.0);
    groupA->addPoints({{200, 0}, {300, 100}});

    // ---- 标记组 B：红色圆形 ----
    vtkMarkerGroup2D *groupB = w.addMarkerGroup(
        heatmap, "Group B", QColor(255, 60, 60), Marker2DStyle::Circle, 12.0);
    groupB->addPoints({{500, 200}, {600, 50}});

    // ---- 标记组 C：青色十字 ----
    vtkMarkerGroup2D *groupC = w.addMarkerGroup(
        heatmap, "Group C", QColor(0, 255, 255), Marker2DStyle::Cross, 12.0);
    groupC->addPoints({{700, -50}, {400, 50}, {800, 150}});

    w.show();
    return a.exec();
}
