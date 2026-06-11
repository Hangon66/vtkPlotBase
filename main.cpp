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
#include "vtkplot2d.h"
#include "drawable/vtkheatmap2d.h"

#include <QApplication>
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

void show3d() {
    vtkPlotBase *w = new vtkPlotBase();
    w->setWindowTitle("VTK Plot Demo");
    w->resize(800, 600);
    w->setTitle("VTK Plot Demo");
    // 设置坐标轴标题
    w->setAxisTitles("X", "Y", "Z");
    
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
    vtkCurve* helix = w->addCurve(helixPoints);
    helix->setName("11");
    helix->setLineWidth(2.0);

    // ==================== 标记点 ====================
    // 填充圆标记
    vtkMarker* originFilled = w->addFilledMarker(QVector3D(8, 8, 8));
    originFilled->setName("22");
    originFilled->setFilled(true);
    
    // 空心环标记
    vtkMarker* originHollow = w->addHollowMarker(QVector3D(5, 5, 5));
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
    
    vtkSurface* paraboloid = w->addSurface(paraboloidPoints, nx1, ny1);
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
    vtkHeatmap* sinc = w->addHeatmapSurface(sincPoints, nx, nz, "Sinc(r)");

    // 设置等高线数量
    sinc->setContourCount(5);

    // 显示图例
    w->setLegendVisible(true);
    
    w->show();
}

void show2d() {
    vtkPlot2D *w = new vtkPlot2D();
    w->setWindowTitle("2D Heatmap Demo");
    w->resize(600, 600);

    // 生成二维热力图数据：sin(x) * cos(y)
    const int size = 200;
    QVector<double> data;
    data.resize(size * size);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double x = i * 360.0 / size;
            double y = j * 360.0 / size;
            data[i * size + j] = std::sin(x * M_PI / 180.0) * std::cos(y * M_PI / 180.0);
        }
    }

    // 添加二维热力图
    vtkHeatmap2D* heatmap = w->addHeatmap2D(data, size, size, "2D Heatmap");
    heatmap->setChartTitle("2D Heatmap"); // 设置标题
    heatmap->setColorBarTitle("Color Bar");
    heatmap->setXAxisTitle("X Axis1");
    heatmap->setYAxisTitle("Y Axis1");
    heatmap->setName("2D Heatmap");

    // 设置原点和间距
    heatmap->setOrigin(0.0, 0.0);
    heatmap->setSpacing(360.0 / size, 360.0 / size);

    // 设置背景颜色
    w->setBackground(QColor(30, 30, 40));

    w->show();
}

/**
 * @brief 从 JSON 数据解析并显示热力图。
 *
 * 遍历 JSON 中的 charts 数组，根据 type 选择显示方式：
 * - surface 类型：使用 vtkPlotBase 的 3D 热力图曲面显示
 * - line 类型：暂不处理
 *
 * @param jsonData JSON 文件的原始字节数据。
 */
void testheatmapFromJson(const QByteArray &jsonData) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    if (doc.isNull()) {
        qWarning() << "JSON 解析失败:" << error.errorString();
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray charts = root["charts"].toArray();

    for (const QJsonValue &chartVal : charts) {
        QJsonObject chart = chartVal.toObject();
        QString type = chart["type"].toString();
        QString figure = chart["figure"].toString();

        // 只处理 surface 类型（三维热力图曲面）
        if (type != "surface") continue;

        // 提取坐标轴标签
        QString xLabel = chart["x_label"].toString();
        QString yLabel = chart["y_label"].toString();
        QString zLabel = chart["z_label"].toString();

        // 提取 x, y 轴数据
        QJsonArray xArr = chart["x"].toArray();
        QJsonArray yArr = chart["y"].toArray();
        QJsonArray zArr = chart["z"].toArray();

        int nx = xArr.size();  // x 方向点数
        int ny = yArr.size();  // y 方向点数

        // 构造三维点集：Y 为高度（z 数据值）
        QVector<QVector3D> points;
        points.reserve(nx * ny);
        for (int j = 0; j < ny; ++j) {
            QJsonArray rowArr = zArr[j].toArray();
            for (int i = 0; i < nx; ++i) {
                double xVal = xArr[i].toDouble();
                double yVal = rowArr[i].toDouble();  // Y 轴 = z 数据值（高度）
                double zVal = yArr[j].toDouble();    // Z 轴 = y 坐标
                points.append(QVector3D(xVal, yVal, zVal));
            }
        }

        // 创建三维热力图曲面窗口
        vtkPlotBase *w = new vtkPlotBase();
        w->setWindowTitle(figure);
        w->resize(800, 600);
        w->setTitle(figure);

        // 拉伸填满立方体模式：自动归一化几何体，坐标轴标签显示原始范围
        w->setAutoScaleMode(AutoScaleMode::StretchFill);

        // 设置坐标轴标题（曲面在 XZ 平面，高度为 Y）
        w->setAxisTitles(xLabel, zLabel, yLabel);

        // 添加热力图曲面，StretchFill 模式下自动处理归一化和标量重映射
        vtkHeatmap *heatmap = w->addHeatmapSurface(points, nx, ny, zLabel);
        heatmap->setContourCount(5);

        w->setLegendVisible(true);
        w->show();
    }
}

/**
 * @brief 读取 JSON 文件并调用热力图显示。
 *
 * 从程序运行目录下读取 distributed_network_charts.json 文件，
 * 交由 testheatmapFromJson 解析并显示。
 */
void testheatmap() {
    QFile file("distributed_network_charts.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开 JSON 文件:" << file.fileName();
        return;
    }
    testheatmapFromJson(file.readAll());
    file.close();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    testheatmap();

    return a.exec();    

}


