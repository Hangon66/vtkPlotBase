/**
 * @file example_heatmap2d.cpp
 * @brief 演示 vtkPlot2D 二维热力图的基本用法
 */

#include "vtkplot2d.h"

#include <QApplication>
#include <QVector>
#include <cmath>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    vtkPlot2D w;
    w.setWindowTitle("2D Heatmap Demo");
    w.resize(600, 600);

    // 生成二维热力图数据：sin(x) * cos(y)
    const int size = 200;
    QVector<double> data;
    data.resize(size * size);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double x = i * 360.0 / size;
            double y = j * 360.0 / size;
            data[i * size + j] = std::sin(x * M_PI / 180.0) * std::cos(y * M_PI / 180.0);
        }
    }

    // 添加二维热力图
    vtkHeatmap2D* heatmap = w.addHeatmap2D(data, size, size, "sin(x)·cos(y)");
    heatmap->setName("2D Heatmap");

    // 设置标题与坐标轴描述
    heatmap->setChartTitle("2D Heatmap");
    heatmap->setColorBarTitle("Value");
    heatmap->setXAxisTitle("X Axis");
    heatmap->setYAxisTitle("Y Axis");

    // 设置原点和间距
    heatmap->setOrigin(0.0, 0.0);
    heatmap->setSpacing(360.0 / size, 360.0 / size);

    // 设置背景颜色
    w.setBackground(QColor(30, 30, 40));

    w.show();
    return a.exec();
}
