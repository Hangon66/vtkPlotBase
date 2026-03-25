/**
 * @file example_heatmap.cpp
 * @brief 热力图示例 - 展示如何添加热力图曲面和等高线投影
 * 
 * 本示例演示：
 * 1. 添加热力图曲面（高度映射颜色）
 * 2. 设置等高线数量和可见性
 * 3. 设置颜色条标题和位置
 * 4. sinc函数和peaks函数可视化
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
    w.setWindowTitle("热力图示例 - Heatmap Surface");
    w.resize(800, 600);
    
    // 设置坐标轴标题（曲面在ZX平面，高度为Y）
    w.setAxisTitles("X", "Height", "Z");
    
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
    QString sincId = w.addHeatmapSurface(sincPoints, nx, nz, "Sinc(r)");
    
    // 设置等高线数量
    w.setHeatmapContourCount(sincId, 8);
    
    // ==================== 示例2：Peaks函数曲面（注释状态，可取消注释测试） ====================
    // MATLAB经典的peaks函数
    /*
    const int nx2 = 50;
    const int nz2 = 50;
    QVector<QVector3D> peaksPoints;
    
    for (int j = 0; j < nz2; ++j) {
        for (int i = 0; i < nx2; ++i) {
            double x = (i - nx2/2.0) * 0.2;
            double z = (j - nz2/2.0) * 0.2;
            
            // peaks函数
            double y = 3 * pow(1-x, 2) * exp(-x*x - (z+1)*(z+1))
                     - 10 * (x/5 - pow(x, 3) - pow(z, 5)) * exp(-x*x - z*z)
                     - 1.0/3 * exp(-(x+1)*(x+1) - z*z);
            
            peaksPoints.append(QVector3D(x, y, z));
        }
    }
    
    QString peaksId = w.addHeatmapSurface(peaksPoints, nx2, nz2, "Peaks");
    w.setHeatmapContourCount(peaksId, 10);
    */
    
    // ==================== 示例3：高斯分布曲面（注释状态） ====================
    /*
    const int nx3 = 50;
    const int nz3 = 50;
    QVector<QVector3D> gaussPoints;
    
    double cx = 0, cz = 0;  // 中心点
    double sigma = 1.0;     // 标准差
    
    for (int j = 0; j < nz3; ++j) {
        for (int i = 0; i < nx3; ++i) {
            double x = (i - nx3/2.0) * 0.15;
            double z = (j - nz3/2.0) * 0.15;
            
            // 二维高斯分布
            double r2 = (x-cx)*(x-cx) + (z-cz)*(z-cz);
            double y = exp(-r2 / (2 * sigma * sigma));
            
            gaussPoints.append(QVector3D(x, y, z));
        }
    }
    
    QString gaussId = w.addHeatmapSurface(gaussPoints, nx3, nz3, "Gaussian");
    w.setHeatmapContourCount(gaussId, 6);
    */
    
    // ==================== 控制等高线可见性 ====================
    // w.setHeatmapContourVisible(sincId, false);  // 隐藏等高线
    
    // ==================== 控制颜色条可见性 ====================
    // w.setHeatmapColorBarVisible(false);  // 隐藏颜色条
    
    // ==================== 设置曲面不透明度 ====================
    // w.setHeatmapSurfaceOpacity(sincId, 0.7);
    
    w.show();
    return a.exec();
}
