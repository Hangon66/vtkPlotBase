/**
 * @file example_curve.cpp
 * @brief 曲线示例 - 展示如何添加和自定义3D曲线
 * 
 * 本示例演示：
 * 1. 添加多条3D曲线
 * 2. 设置曲线颜色和线宽
 * 3. 设置曲线可见性
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
    w.setWindowTitle("曲线示例 - 3D Curves");
    w.resize(800, 600);
    
    // 设置坐标轴标题
    w.setAxisTitles("X", "Y", "Z");
    
    // ==================== 示例1：螺旋线 ====================
    QVector<QVector3D> helixPoints;
    for (int i = 0; i <= 360; ++i) {
        double t = i * M_PI / 180.0;
        double x = cos(t);
        double y = t / (2 * M_PI);  // 高度随角度增加
        double z = sin(t);
        helixPoints.append(QVector3D(x, y, z));
    }
    QString helixId = w.addCurve(helixPoints);
    w.setCurveColor(helixId, Qt::red);
    w.setCurveWidth(helixId, 2.0);
    w.setCurveName(helixId, "111");

    // 原点（白色大圆）
    QString originId1 = w.addFilledMarker(QVector3D(0, 0, 0));
    w.setFilledMarkerColor(originId1, Qt::white);
    w.setFilledMarkerRadius(originId1, 0.25);

    // 标记原点（红色，大圆环）
    QString originId = w.addHollowMarker(QVector3D(0, 0, 0));
    w.setMarkerColor(originId, Qt::red);
    w.setMarkerRadius(originId, 0.3);
    w.setMarkerLineWidth(originId, 2.0);
    w.setMarkerName(originId, "22");
    
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
    w.setSurfaceName(paraboloidId, "33");

    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
