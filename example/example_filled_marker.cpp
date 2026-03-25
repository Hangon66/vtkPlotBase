/**
 * @file example_filled_marker.cpp
 * @brief 填充标记示例 - 展示如何添加和自定义填充圆标记
 * 
 * 本示例演示：
 * 1. 添加填充圆标记
 * 2. 设置标记颜色和半径
 * 3. 用标记表示数据点密度
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
    w.setWindowTitle("填充标记示例 - Filled Markers");
    w.resize(800, 600);
    
    // 设置坐标轴标题
    w.setAxisTitles("X", "Y", "Z");
    
    // ==================== 示例1：坐标轴方向标记 ====================
    
    // 原点（白色大圆）
    QString originId = w.addFilledMarker(QVector3D(0, 0, 0));
    w.setFilledMarkerColor(originId, Qt::white);
    w.setFilledMarkerRadius(originId, 0.25);
    
    // X轴正方向（红色）
    QString xPosId = w.addFilledMarker(QVector3D(4, 0, 0));
    w.setFilledMarkerColor(xPosId, Qt::red);
    w.setFilledMarkerRadius(xPosId, 0.2);
    
    // Y轴正方向（绿色）
    QString yPosId = w.addFilledMarker(QVector3D(0, 4, 0));
    w.setFilledMarkerColor(yPosId, Qt::green);
    w.setFilledMarkerRadius(yPosId, 0.2);
    
    // Z轴正方向（蓝色）
    QString zPosId = w.addFilledMarker(QVector3D(0, 0, 4));
    w.setFilledMarkerColor(zPosId, Qt::blue);
    w.setFilledMarkerRadius(zPosId, 0.2);
    
    // ==================== 示例2：随机散点分布 ====================
    // 模拟3D数据点分布
    for (int i = 0; i < 50; ++i) {
        double x = (rand() % 100 - 50) / 10.0;
        double y = (rand() % 100 - 50) / 10.0;
        double z = (rand() % 100 - 50) / 10.0;
        
        QString id = w.addFilledMarker(QVector3D(x, y, z));
        
        // 根据高度设置颜色
        QColor color = QColor::fromHsv(static_cast<int>((y + 5) * 25), 255, 255);
        w.setFilledMarkerColor(id, color);
        w.setFilledMarkerRadius(id, 0.1);
    }
    
    // ==================== 示例3：球面上的点 ====================
    double radius = 3.0;
    for (int phi = 0; phi < 360; phi += 30) {
        for (int theta = -90; theta <= 90; theta += 30) {
            double phiRad = phi * M_PI / 180.0;
            double thetaRad = theta * M_PI / 180.0;
            
            double x = radius * cos(thetaRad) * cos(phiRad);
            double y = radius * sin(thetaRad);
            double z = radius * cos(thetaRad) * sin(phiRad);
            
            QString id = w.addFilledMarker(QVector3D(x, y, z));
            
            // 根据纬度设置颜色
            QColor color = QColor::fromHsv((theta + 90) * 2, 255, 255);
            w.setFilledMarkerColor(id, color);
            w.setFilledMarkerRadius(id, 0.08);
        }
    }
    
    // ==================== 示例4：大小变化的点 ====================
    // 演示用半径表示数据大小
    for (int i = 0; i < 10; ++i) {
        double x = i - 4.5;
        double y = 0;
        double z = -3;
        
        QString id = w.addFilledMarker(QVector3D(x, y, z));
        w.setFilledMarkerColor(id, Qt::magenta);
        w.setFilledMarkerRadius(id, 0.05 + i * 0.03);  // 半径递增
    }
    
    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
