/**
 * @file main.cpp
 * @brief 主程序 - 二维热力图标记点示例
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
#include "drawable/vtkheatmap2d.h"
#include "drawable/vtkmarkergroup2d.h"
#include "vtkplotbase.h"
#include "drawable/vtkheatmap.h"
#include <cmath>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    vtkPlotBase w;
    w.setWindowTitle("热力图示例 - Heatmap Surface");
    w.resize(800, 600);
    
    // 设置坐标轴标题（曲面在ZX平面，高度为Y）
    w.setAxisTitles("X", "Height", "Z");
    w.setHoverDisplayEnabled(true);
    w.setHoverTolerance(0.01);

    
    // ==================== 示例1：Sinc函数曲面 ====================
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
            sincPoints.append(QVector3D(x, y, z));
        }
    }
    
    // 添加热力图曲面，Y值（高度）映射为颜色
    vtkHeatmap* sinc = w.addHeatmapSurface(sincPoints, nx, nz, "Sinc(r)");
    
    // 设置等高线数量
    sinc->setContourCount(8);

    w.show();
    return app.exec();
}
