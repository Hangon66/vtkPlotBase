/**
 * @file example_bitmap.cpp
 * @brief 演示 vtkPlot2D 二值比特图（二维码风格）的用法
 */

#include "vtkplot2d.h"

#include <QApplication>
#include <QVector>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    vtkPlot2D w;
    w.setWindowTitle("Binary Bitmap Demo");
    w.resize(600, 600);

    // 21x21 QR 码风格图案（1=黑, 0=白）
    // 包含三个定位图案（左上/右上/左下角 7x7）和随机数据区
    const int rows = 21, cols = 21;
    double qr[21][21] = {
        {1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1},
        {1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
        {1,0,1,1,1,0,1,0,0,1,1,0,0,0,1,0,1,1,1,0,1},
        {1,0,1,1,1,0,1,0,1,0,0,1,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,1,0,0,1,0,0,1,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0},
        {1,0,1,0,1,1,1,1,0,0,1,1,0,1,1,0,1,0,0,1,0},
        {0,1,0,1,0,0,0,1,1,0,1,0,1,0,0,1,0,1,1,0,1},
        {1,0,1,1,0,1,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0},
        {0,1,0,0,1,0,0,1,0,1,1,0,0,1,1,0,0,1,0,0,1},
        {1,1,0,1,1,0,1,0,1,0,0,1,1,0,1,1,0,1,0,1,0},
        {0,0,0,0,0,0,0,0,1,0,1,1,0,0,0,1,0,0,1,0,1},
        {1,1,1,1,1,1,1,0,0,1,0,0,1,0,1,0,1,1,0,1,0},
        {1,0,0,0,0,0,1,0,1,1,0,1,0,1,0,0,1,0,1,0,1},
        {1,0,1,1,1,0,1,0,0,0,1,0,1,1,0,1,0,1,0,0,0},
        {1,0,1,1,1,0,1,0,1,0,1,1,0,0,1,1,0,0,1,1,0},
        {1,0,1,1,1,0,1,0,0,1,0,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,1,0,1,0,0,1,0,1,0,0,0,1,0,1,0},
        {1,1,1,1,1,1,1,0,0,1,1,0,1,0,1,1,0,1,1,0,1},
    };

    // 将二维数组转为一维向量
    QVector<double> data;
    data.reserve(rows * cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            data.append(qr[i][j]);

    // 添加二值热力图，不传 colorBarTitle 以隐藏颜色条
    vtkHeatmap2D* heatmap = w.addHeatmap2D(data, rows, cols);

    // 设置离散色表：0=白, 1=黑
    w.setHeatmap2DDiscreteColorMap(heatmap, {Qt::white, Qt::black});

    // 确保颜色条隐藏（针对 vtkChartHistogram2D 默认显示行为）
    w.setHeatmap2DColorBarVisible(heatmap, false);

    // 设置标题
    heatmap->setChartTitle("QR-Code Style Bitmap");
    heatmap->setName("Binary Bitmap");

    // 禁用交互，固定视图
    w.setInteractionEnabled(false);

    // 设置背景颜色
    w.setBackground(QColor(30, 30, 40));

    w.show();
    return a.exec();
}
