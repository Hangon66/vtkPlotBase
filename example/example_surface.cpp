/**
 * @file example_surface.cpp
 * @brief 曲面示例 - 展示如何添加和自定义3D曲面
 * 
 * 本示例演示：
 * 1. 添加单色曲面
 * 2. 设置曲面颜色和不透明度
 * 3. 常见数学曲面的可视化
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
    
    // ==================== 示例1：抛物面 ====================
    // z = x^2 + y^2
    const int nx1 = 30;
    const int ny1 = 30;
    QVector<QVector3D> paraboloidPoints;
    
    for (int j = 0; j < ny1; ++j) {
        for (int i = 0; i < nx1; ++i) {
            double x = (i - nx1/2.0) * 0.2;
            double z = (j - ny1/2.0) * 0.2;
            double y = x * x + z * z;  // 抛物面方程
            paraboloidPoints.append(QVector3D(x - 3, y, z + 3));
        }
    }
    
    QString paraboloidId = w.addSurface(paraboloidPoints, nx1, ny1, Qt::cyan, 0.8);
    w.setSurfaceName(paraboloidId, "抛物面");
    
    // ==================== 示例2：双曲面 ====================
    // z = x^2 - y^2 (马鞍面)
    QVector<QVector3D> saddlePoints;
    
    for (int j = 0; j < ny1; ++j) {
        for (int i = 0; i < nx1; ++i) {
            double x = (i - nx1/2.0) * 0.2;
            double z = (j - ny1/2.0) * 0.2;
            double y = x * x - z * z;  // 马鞍面方程
            saddlePoints.append(QVector3D(x + 3, y, z + 3));
        }
    }
    
    QString saddleId = w.addSurface(saddlePoints, nx1, ny1, Qt::yellow, 0.8);
    w.setSurfaceName(saddleId, "马鞍面");
    
    // ==================== 示例3：正弦波曲面 ====================
    const int nx2 = 50;
    const int ny2 = 50;
    QVector<QVector3D> wavePoints;
    
    for (int j = 0; j < ny2; ++j) {
        for (int i = 0; i < nx2; ++i) {
            double x = (i - nx2/2.0) * 0.15;
            double z = (j - ny2/2.0) * 0.15;
            double y = sin(sqrt(x*x + z*z) * 2) * 0.5;  // 波浪曲面
            wavePoints.append(QVector3D(x, y - 2, z - 3));
        }
    }
    
    QString waveId = w.addSurface(wavePoints, nx2, ny2, Qt::magenta, 0.7);
    w.setSurfaceName(waveId, "波浪面");
    
    // ==================== 示例4：平面 ====================
    QVector<QVector3D> planePoints;
    
    for (int j = 0; j < 10; ++j) {
        for (int i = 0; i < 10; ++i) {
            double x = i * 0.5 - 2.5;
            double z = j * 0.5 - 2.5;
            double y = 0.5 * x + 0.3 * z;  // 倾斜平面
            planePoints.append(QVector3D(x - 3, y - 2, z - 3));
        }
    }
    
    QString planeId = w.addSurface(planePoints, 10, 10, Qt::green, 0.6);
    w.setSurfaceName(planeId, "斜平面");
    
    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
