/**
 * @file example_hollow_marker.cpp
 * @brief 空心环标记示例 - 展示如何添加和自定义空心环标记
 * 
 * 本示例演示：
 * 1. 添加空心环标记
 * 2. 设置标记颜色和大小
 * 3. 屏幕固定大小模式
 */

#include "vtkplotbase.h"

#include <QApplication>
#include <QVector>
#include <QVector3D>
#include <QColor>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    vtkPlotBase w;
    w.setWindowTitle("空心环标记示例 - Hollow Markers");
    w.resize(800, 600);
    
    // 设置坐标轴标题
    w.setAxisTitles("X", "Y", "Z");
    
    // 添加多个空心环标记
    // 默认：屏幕固定大小模式
    
    vtkMarker* marker1 = w.addHollowMarker(QVector3D(0, 0, 0), Qt::red, 15.0, 2.0);
    marker1->setName("点1(红)");
    
    vtkMarker* marker2 = w.addHollowMarker(QVector3D(2, 2, 0), Qt::green, 12.0, 2.0);
    marker2->setName("点2(绿)");
    
    vtkMarker* marker3 = w.addHollowMarker(QVector3D(-2, 1, 0), Qt::blue, 10.0, 2.0);
    marker3->setName("点3(蓝)");
    
    vtkMarker* marker4 = w.addHollowMarker(QVector3D(1, -2, 0), Qt::yellow, 8.0, 2.0);
    marker4->setName("点4(黄)");
    
    // 使用自动颜色
    vtkMarker* marker5 = w.addHollowMarker(QVector3D(-1, -1, 0));
    marker5->setName("点5(自动)");
    
    // 显示图例
    w.setLegendVisible(true);
    
    w.show();
    return a.exec();
}
