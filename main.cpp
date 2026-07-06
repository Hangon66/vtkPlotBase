/**
 * @file main.cpp
 * @brief 主程序 - 概率分布直方图示例
 */

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include "vtkplot2d.h"
#include "drawable/vtkhistogram.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ==================== 示例：概率分布直方图（双分布对比） ====================
    vtkPlot2D histWindow;
    histWindow.setWindowTitle("概率分布直方图 - Histogram");
    histWindow.resize(800, 600);

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // 生成正态分布数据（Box-Muller 变换）
    QVector<double> normalData;
    normalData.reserve(10000);
    for (int i = 0; i < 5000; ++i) {
        double u1 = (std::rand() + 1.0) / (RAND_MAX + 2.0);
        double u2 = (std::rand() + 1.0) / (RAND_MAX + 2.0);
        double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
        double z1 = std::sqrt(-2.0 * std::log(u1)) * std::sin(2.0 * M_PI * u2);
        normalData.append(z0);
        normalData.append(z1);
    }

    // 生成均匀分布数据 [-3, 3]
    QVector<double> uniformData;
    uniformData.reserve(10000);
    for (int i = 0; i < 10000; ++i) {
        uniformData.append(-3.0 + 6.0 * std::rand() / RAND_MAX);
    }

    // 添加正态分布直方图：50 分箱，青色半透明
    vtkHistogram *histNormal = histWindow.addHistogram(normalData, 50,
        QColor(0, 255, 255, 120), "N(0,1)");
    histNormal->setTitle("概率分布对比");
    histNormal->setXAxisTitle("数值");
    histNormal->setYAxisTitle("频次");

    // 添加均匀分布直方图：50 分箱，红色半透明
    histWindow.addHistogram(uniformData, 50,
        QColor(255, 80, 80, 120), "U[-3,3]");

    // 添加 x=0 处的红色虚线参考线
    histWindow.addHistogramRefLine(0.0, QColor(255, 0, 0), 2.0, "x=0");

    histWindow.show();

    return app.exec();
}
