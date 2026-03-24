#include "vtkplotbase.h"

#include <QApplication>
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <vtkMath.h>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    vtkPlotBase w;
    w.setWindowTitle("VTK 3D Heatmap Surface Plot");
    w.resize(800, 600);
    
    // 设置坐标轴标题（曲面在ZX平面，高度为Y）
    w.setAxisTitles("X", "Height", "Z");
    
    // 创建曲面网格数据 (sin(r)/r 曲面，在ZX平面)
    const int nx = 50;  // X方向网格数
    const int nz = 50;  // Z方向网格数
    QVector<QVector3D> surfacePoints;
    
    for (int j = 0; j < nz; ++j) {
        for (int i = 0; i < nx; ++i) {
            double x = (i - nx/2.0) * 0.1;
            double z = (j - nz/2.0) * 0.1;
            double r = sqrt(x*x + z*z);
            double y = (r < 0.01) ? 1.0 : sin(r) / r;  // sinc函数作为高度
            surfacePoints.append(QVector3D(x, y, z));
        }
    }
    
    // Add heatmap surface (Y value as color - height)
    QString surfaceId = w.addHeatmapSurface(surfacePoints, nx, nz, "Height");
    
    w.show();
    return a.exec();
}
