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

// 封装类
#include "vtkplot2d.h"
#include "drawable/vtkheatmap2d.h"
#include "drawable/vtkmarkergroup2d.h"

#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChartHistogram2D.h>
#include <vtkColorLegend.h>
#include <vtkColorTransferFunction.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlotPoints.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <vtkTextProperty.h>

#include <vector>
#include <utility>
#include <cmath>

/**
 * @brief 创建并显示带标记点的二维热力图窗口
 *
 * 封装完整的场景创建、窗口显示流程，一次调用即可显示结果。
 * 窗口由 Qt 管理生命周期，无需手动释放。
 */
void showHeatmapWithMarkers()
{
    vtkNew<vtkNamedColors> colors;
    vtkColor3d bgColor = colors->GetColor3d("SlateGray");
    vtkColor3d titleColor = colors->GetColor3d("Orange");
    vtkColor3d axisTitleColor = colors->GetColor3d("Orange");
    vtkColor3d axisLabelColor = colors->GetColor3d("Beige");
    vtkColor4ub legendBgColor = colors->GetColor4ub("Tomato");

    // 创建渲染窗口和控件
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->SetWindowName("HeatmapWithMarkers");
    renderWindow->SetMultiSamples(0);

    auto *widget = new QVTKOpenGLNativeWidget;
    widget->setRenderWindow(renderWindow);
    widget->resize(600, 600);
    widget->setWindowTitle("2D Heatmap with Markers");

    // 设置上下文视图
    vtkNew<vtkContextView> view;
    view->SetRenderWindow(renderWindow);
    view->GetRenderer()->SetBackground(bgColor.GetData());

    // 创建图表
    vtkNew<vtkChartHistogram2D> chart;

    chart->SetTitle("2D Heatmap with Markers");
    chart->GetTitleProperties()->SetFontSize(36);
    chart->GetTitleProperties()->SetColor(titleColor.GetData());

    for (int i = 0; i < 2; ++i) {
        chart->GetAxis(i)->GetTitleProperties()->SetFontSize(24);
        chart->GetAxis(i)->GetTitleProperties()->SetColor(axisTitleColor.GetData());
        chart->GetAxis(i)->GetLabelProperties()->SetColor(axisLabelColor.GetData());
        chart->GetAxis(i)->GetLabelProperties()->SetFontSize(18);
    }

    dynamic_cast<vtkColorLegend*>(chart->GetLegend())->DrawBorderOn();
    chart->GetLegend()->GetBrush()->SetColor(legendBgColor);

    view->GetScene()->AddItem(chart);

    // ---- 热力图数据 ----
    int size = 400;
    vtkNew<vtkImageData> imageData;
    imageData->SetExtent(0, size - 1, 0, size - 1, 0, 0);
    imageData->AllocateScalars(VTK_DOUBLE, 1);
    imageData->SetOrigin(100.0, -100.0, 0.0);
    imageData->SetSpacing(2.0, 1.0, 1.0);

    double *dPtr = static_cast<double*>(imageData->GetScalarPointer(0, 0, 0));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            dPtr[i * size + j] =
                std::sin(vtkMath::RadiansFromDegrees(double(2 * i))) *
                std::cos(vtkMath::RadiansFromDegrees(double(j)));
        }
    }
    chart->SetInputData(imageData);

    // HSV 颜色传输函数
    vtkNew<vtkColorTransferFunction> tf;
    tf->AddHSVSegment(0.0,    0.0, 1.0, 1.0, 0.3333, 0.3333, 1.0, 1.0);
    tf->AddHSVSegment(0.3333, 0.3333, 1.0, 1.0, 0.6666, 0.6666, 1.0, 1.0);
    tf->AddHSVSegment(0.6666, 0.6666, 1.0, 1.0, 1.0,   0.2,   1.0, 0.3);
    tf->Build();
    chart->SetTransferFunction(tf);

    // ---- 标记点辅助函数 ----
    auto addMarkers = [&](const char *name,
                          std::vector<std::pair<int, int>> pixels,
                          unsigned char r, unsigned char g, unsigned char b,
                          int style) {
        vtkNew<vtkTable> table;
        vtkNew<vtkFloatArray> colX;
        colX->SetName("X");
        table->AddColumn(colX);
        vtkNew<vtkFloatArray> colY;
        colY->SetName("Y");
        table->AddColumn(colY);

        table->SetNumberOfRows(static_cast<vtkIdType>(pixels.size()));
        for (size_t i = 0; i < pixels.size(); ++i) {
            table->SetValue(static_cast<vtkIdType>(i), 0,
                            100.0f + 2.0f * pixels[i].first);
            table->SetValue(static_cast<vtkIdType>(i), 1,
                            -100.0f + 1.0f * pixels[i].second);
        }

        vtkPlot *plot = chart->AddPlot(vtkChart::POINTS);
        plot->SetLabel(name);
        plot->SetInputData(table, 0, 1);
        plot->SetColor(r, g, b, 255);
        plot->SetWidth(3.0);
        vtkPlotPoints::SafeDownCast(plot)->SetMarkerSize(12.0);
        vtkPlotPoints::SafeDownCast(plot)->SetMarkerStyle(style);
    };

    // 三组标记点
    addMarkers("Group A", {{50, 100}, {100, 200}},
               255, 255, 255, vtkPlotPoints::DIAMOND);
    addMarkers("Group B", {{200, 300}, {250, 150}},
               255, 60, 60, vtkPlotPoints::CIRCLE);
    addMarkers("Group C", {{300, 50}, {150, 150}, {350, 250}},
               0, 255, 255, vtkPlotPoints::CROSS);

    chart->SetShowLegend(true);

    widget->show();
}

/**
 * @brief 使用封装类创建并显示带标记点的二维热力图窗口
 *
 * 通过 vtkPlot2D / vtkHeatmap2D / vtkMarkerGroup2D 封装类实现，
 * 与 showHeatmapWithMarkers() 显示相同内容。
 */
void showWrapperHeatmapWithMarkers()
{
    auto *plot2d = new vtkPlot2D;
    plot2d->resize(800, 600);
    plot2d->setWindowTitle("2D Heatmap with Markers (Wrapper)");
    plot2d->setBackground(QColor(112, 128, 144));  // SlateGray

    // ---- 热力图数据 (400x400 sin*cos) ----
    const int size = 400;
    QVector<double> heatmapData;
    heatmapData.reserve(size * size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            heatmapData.append(
                std::sin(i * 2.0 * M_PI / 180.0) *
                std::cos(j * M_PI / 180.0));
        }
    }

    // ---- 添加热力图 ----
    vtkHeatmap2D *heatmap = plot2d->addHeatmap2D(heatmapData, size, size, "Value");
    heatmap->setOrigin(100.0, -100.0);
    heatmap->setSpacing(2.0, 1.0);
    heatmap->setChartTitle("2D Heatmap with Markers");
    heatmap->setXAxisTitle("X Axis");
    heatmap->setYAxisTitle("Y Axis");
    heatmap->setValueRange(-1.0, 1.0);

    // ---- 标记组 A：白色菱形 ----
    vtkMarkerGroup2D *groupA = plot2d->addMarkerGroup(
        heatmap, "Group A", Qt::white, Marker2DStyle::Diamond, 12.0);
    groupA->addPoints({{200, 0}, {300, 100}});

    // ---- 标记组 B：红色圆形 ----
    vtkMarkerGroup2D *groupB = plot2d->addMarkerGroup(
        heatmap, "Group B", QColor(255, 60, 60), Marker2DStyle::Circle, 12.0);
    groupB->addPoints({{500, 200}, {600, 50}});

    // ---- 标记组 C：青色十字 ----
    vtkMarkerGroup2D *groupC = plot2d->addMarkerGroup(
        heatmap, "Group C", QColor(0, 255, 255), Marker2DStyle::Cross, 12.0);
    groupC->addPoints({{700, -50}, {400, 50}, {800, 150}});

    plot2d->show();
}

int main(int argc, char *argv[])
{
    // QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QApplication app(argc, argv);

    // 直接 VTK 示例（已验证）
    showHeatmapWithMarkers();

    // 封装类版本
    showWrapperHeatmapWithMarkers();

    return app.exec();
}
