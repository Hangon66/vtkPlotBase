/**
 * @file main.cpp
 * @brief 主程序 - 演示 vtkPlotBase 基本用法
 */

#include "vtkplotbase.h"

// drawable 类
#include "drawable/vtkcurve.h"
#include "drawable/vtkmarker.h"
#include "drawable/vtksurface.h"
#include "drawable/vtkheatmap.h"

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
    w.setWindowTitle("VTK Plot Demo");
    w.resize(800, 600);
    w.setTitle("VTK Plot Demo");
    // 设置坐标轴标题
    w.setAxisTitles("X", "Y", "Z");
    
    // ==================== 曲线 ====================
    QVector<QVector3D> helixPoints;
    for (int i = 0; i <= 360; ++i) {
        double t = i * M_PI / 180.0;
        double x = cos(t);
        double y = t / (2 * M_PI);
        double z = sin(t);
        helixPoints.append(QVector3D(x, y, z + 10));
    }
    
    // 添加曲线（自动颜色）
    vtkCurve* helix = w.addCurve(helixPoints);
    helix->setName("11");
    helix->setLineWidth(2.0);

    // ==================== 标记点 ====================
    // 填充圆标记
    vtkMarker* originFilled = w.addFilledMarker(QVector3D(8, 8, 8));
    originFilled->setName("22");
    originFilled->setFilled(true);
    
    // 空心环标记
    vtkMarker* originHollow = w.addHollowMarker(QVector3D(5, 5, 5));
    originHollow->setName("33");
    originHollow->setFilled(false);

    // ==================== 曲面 ====================
    // z = x^2 + y^2 抛物面
    const int nx1 = 30;
    const int ny1 = 30;
    QVector<QVector3D> paraboloidPoints;
    
    for (int j = 0; j < ny1; ++j) {
        for (int i = 0; i < nx1; ++i) {
            double x = (i - nx1/2.0) * 0.2;
            double z = (j - ny1/2.0) * 0.2;
            double y = x * x + z * z;
            paraboloidPoints.append(QVector3D(x - 3, y, z + 3));
        }
    }
    
    vtkSurface* paraboloid = w.addSurface(paraboloidPoints, nx1, ny1);
    paraboloid->setName("444");
    
    // ==================== Sinc函数曲面 ====================
    // sinc(r) = sin(r)/r，经典的信号处理函数
    const int nx = 60;
    const int nz = 60;
    QVector<QVector3D> sincPoints;
    
    for (int j = 0; j < nz; ++j) {
        for (int i = 0; i < nx; ++i) {
            double x = (i - nx/2.0) * 0.15;
            double z = (j - nz/2.0) * 0.15;
            double r = sqrt(x*x + z*z);
            double y = (r < 0.01) ? 1.0 : sin(r) / r;  // sinc函数
            sincPoints.append(QVector3D(x, y + 10, z + 5));
        }
    }
    
    // 添加热力图曲面，Y值（高度）映射为颜色
    vtkHeatmap* sinc = w.addHeatmapSurface(sincPoints, nx, nz, "Sinc(r)");

    // 设置等高线数量
    sinc->setContourCount(5);

    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
