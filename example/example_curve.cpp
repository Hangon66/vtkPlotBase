/**
 * @file example_curve.cpp
 * @brief 曲线示例 - 展示如何添加和自定义3D曲线
 * 
 * 本示例演示：
 * 1. 添加多条3D曲线
 * 2. 设置曲线颜色和线宽
 * 3. 设置曲线可见性
 */

#include "../vtkplotbase.h"

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
    w.setCurveName(helixId, "螺旋线");
    
    // ==================== 示例2：正弦曲线 ====================
    QVector<QVector3D> sinePoints;
    for (int i = 0; i <= 100; ++i) {
        double x = i * 0.1 - 5.0;
        double y = sin(x);
        double z = 0;  // 在XY平面
        sinePoints.append(QVector3D(x, y, z));
    }
    QString sineId = w.addCurve(sinePoints);
    w.setCurveColor(sineId, Qt::cyan);
    w.setCurveWidth(sineId, 2.0);
    w.setCurveName(sineId, "正弦曲线");
    
    // ==================== 示例3：余弦曲线（Z方向偏移） ====================
    QVector<QVector3D> cosinePoints;
    for (int i = 0; i <= 100; ++i) {
        double x = i * 0.1 - 5.0;
        double y = cos(x);
        double z = 2;  // Z方向偏移
        cosinePoints.append(QVector3D(x, y, z));
    }
    QString cosineId = w.addCurve(cosinePoints);
    w.setCurveColor(cosineId, Qt::yellow);
    w.setCurveWidth(cosineId, 2.0);
    w.setCurveName(cosineId, "余弦曲线");
    
    // ==================== 示例4：抛物线 ====================
    QVector<QVector3D> parabolaPoints;
    for (int i = 0; i <= 100; ++i) {
        double x = i * 0.1 - 5.0;
        double y = x * x / 5.0;  // y = x^2 / 5
        double z = -2;  // Z方向偏移
        parabolaPoints.append(QVector3D(x, y, z));
    }
    QString parabolaId = w.addCurve(parabolaPoints);
    w.setCurveColor(parabolaId, Qt::green);
    w.setCurveWidth(parabolaId, 2.0);
    w.setCurveName(parabolaId, "抛物线");
    
    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
