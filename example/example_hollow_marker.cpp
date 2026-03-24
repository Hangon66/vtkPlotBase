/**
 * @file example_hollow_marker.cpp
 * @brief 空心标记示例 - 展示如何添加和自定义空心环标记
 * 
 * 本示例演示：
 * 1. 添加空心环标记
 * 2. 设置标记颜色、线宽和半径
 * 3. 标记关键数据点
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
    w.setWindowTitle("空心标记示例 - Hollow Markers");
    w.resize(800, 600);
    
    // 设置坐标轴标题
    w.setAxisTitles("X", "Y", "Z");
    
    // ==================== 示例：在3D空间中标记关键点 ====================
    
    // 标记原点（红色，大圆环）
    QString originId = w.addHollowMarker(QVector3D(0, 0, 0));
    w.setHollowMarkerColor(originId, Qt::red);
    w.setHollowMarkerRadius(originId, 0.3);
    w.setHollowMarkerWidth(originId, 2.0);
    w.setMarkerName(originId, "原点");
    
    // 标记X轴方向点（绿色）
    QString xPosId = w.addHollowMarker(QVector3D(3, 0, 0));
    w.setHollowMarkerColor(xPosId, Qt::green);
    w.setHollowMarkerRadius(xPosId, 0.2);
    w.setMarkerName(xPosId, "X+");
    
    // 标记Y轴方向点（蓝色）
    QString yPosId = w.addHollowMarker(QVector3D(0, 3, 0));
    w.setHollowMarkerColor(yPosId, Qt::blue);
    w.setHollowMarkerRadius(yPosId, 0.2);
    w.setMarkerName(yPosId, "Y+");
    
    // 标记Z轴方向点（黄色）
    QString zPosId = w.addHollowMarker(QVector3D(0, 0, 3));
    w.setHollowMarkerColor(zPosId, Qt::yellow);
    w.setHollowMarkerRadius(zPosId, 0.2);
    w.setMarkerName(zPosId, "Z+");
    
    // ==================== 示例：标记立方体顶点 ====================
    double size = 2.0;
    QVector<QVector3D> cubeVertices = {
        QVector3D(-size, -size, -size),
        QVector3D( size, -size, -size),
        QVector3D(-size,  size, -size),
        QVector3D( size,  size, -size),
        QVector3D(-size, -size,  size),
        QVector3D( size, -size,  size),
        QVector3D(-size,  size,  size),
        QVector3D( size,  size,  size)
    };
    
    for (int i = 0; i < cubeVertices.size(); ++i) {
        QString id = w.addHollowMarker(cubeVertices[i]);
        w.setHollowMarkerColor(id, Qt::cyan);
        w.setHollowMarkerRadius(id, 0.15);
        w.setHollowMarkerWidth(id, 1.5);
    }
    
    // ==================== 示例：标记螺旋线上的点 ====================
    QString prevId;
    for (int i = 0; i <= 360; i += 45) {
        double t = i * M_PI / 180.0;
        double x = 2 * cos(t);
        double y = i / 60.0;
        double z = 2 * sin(t);
        
        QString id = w.addHollowMarker(QVector3D(x, y, z));
        w.setHollowMarkerColor(id, QColor::fromHsv(i, 255, 255));
        w.setHollowMarkerRadius(id, 0.1);
        
        // 连接相邻点形成曲线
        if (!prevId.isEmpty()) {
            // 可以在这里添加曲线连接
        }
        prevId = id;
    }
    
    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
