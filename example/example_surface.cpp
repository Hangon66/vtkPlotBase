/**
 * @file example_surface.cpp
 * @brief 曲面示例 - 展示如何添加和自定义3D曲面
 * 
 * 本示例演示：
 * 1. 添加多个3D曲面
 * 2. 设置曲面颜色和不透明度
 * 3. 设置曲面可见性
 */

#include "vtkplotbase.h"

#include <QApplication>
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <cmath>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    vtkPlotBase w;
    w.setWindowTitle("曲面示例 - 3D Surfaces");
    w.resize(800, 600);
    
    // 设置坐标轴标题
    w.setAxisTitles("X", "Y", "Z");
    
    // ==================== 示例1：抛物面 z = x^2 + y^2 ====================
    const int nx1 = 30;
    const int ny1 = 30;
    QVector<QVector3D> paraboloidPoints;
    
    for (int j = 0; j < ny1; ++j) {
        for (int i = 0; i < nx1; ++i) {
            double x = (i - nx1/2.0) * 0.2;
            double z = (j - ny1/2.0) * 0.2;
            double y = x * x + z * z;
            paraboloidPoints.append(QVector3D(x - 3, y, z));
        }
    }
    
    vtkSurface* paraboloid = w.addSurface(paraboloidPoints, nx1, ny1, Qt::cyan, 0.8);
    paraboloid->setName("抛物面");
    
    // ==================== 示例2：平面 z = 0 ====================
    const int nx2 = 20;
    const int ny2 = 20;
    QVector<QVector3D> planePoints;
    
    for (int j = 0; j < ny2; ++j) {
        for (int i = 0; i < nx2; ++i) {
            double x = (i - nx2/2.0) * 0.3;
            double z = (j - ny2/2.0) * 0.3;
            double y = -2;  // 固定高度
            planePoints.append(QVector3D(x - 3, y, z + 5));
        }
    }
    
    vtkSurface* plane = w.addSurface(planePoints, nx2, ny2, Qt::gray, 0.6);
    plane->setName("平面");
    
    // ==================== 示例3：波浪面 ====================
    const int nx3 = 40;
    const int nz3 = 40;
    QVector<QVector3D> wavePoints;
    
    for (int j = 0; j < nz3; ++j) {
        for (int i = 0; i < nx3; ++i) {
            double x = (i - nx3/2.0) * 0.15;
            double z = (j - nz3/2.0) * 0.15;
            double y = sin(sqrt(x*x + z*z) * 2) * 0.5;
            wavePoints.append(QVector3D(x + 5, y, z));
        }
    }
    
    vtkSurface* wave = w.addSurface(wavePoints, nx3, nz3, Qt::magenta, 0.7);
    wave->setName("波浪面");
    
    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
