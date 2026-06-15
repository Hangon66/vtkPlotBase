#include "vtkheatmap2d.h"
#include "vtkmarkergroup2d.h"

#include <vtkContextView.h>
#include <vtkRenderWindow.h>
#include <vtkChartHistogram2D.h>
#include <vtkColorLegend.h>
#include <vtkBrush.h>
#include <vtkMath.h>
#include <vtkChartLegend.h>
#include <vtkAxis.h>
#include <vtkTextProperty.h>

// ==================== 中文字体辅助 ====================

void vtkHeatmap2D::applyChineseFont(vtkTextProperty *prop)
{
    if (prop) {
        prop->SetFontFamily(VTK_FONT_FILE);
        prop->SetFontFile("C:/Windows/Fonts/msyh.ttc");  // 微软雅黑
    }
}

// ==================== 构造函数与析构函数 ====================

vtkHeatmap2D::vtkHeatmap2D(const QVector<double> &data, int rows, int cols,
                           const QString &colorBarTitle)
    : m_id(QUuid::createUuid().toString())
    , m_name("")
    , m_visible(true)
    , m_colorBarTitle(colorBarTitle)
    , m_rows(rows)
    , m_cols(cols)
    , m_view(nullptr)
{
    m_chart = vtkSmartPointer<vtkChartHistogram2D>::New();

    // 创建图像数据
    createImageData(data, rows, cols);

    // 创建颜色传输函数
    createDefaultTransferFunction();

    // 设置图表
    m_chart->SetInputData(m_imageData);
    m_chart->SetTransferFunction(m_transferFunction);

    // 设置图表标题
    if (!colorBarTitle.isEmpty()) {
        m_chart->SetTitle(colorBarTitle.toUtf8().constData());
        m_chart->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
        m_chart->GetTitleProperties()->SetFontSize(20);
        m_chart->GetTitleProperties()->SetBold(1);
        applyChineseFont(m_chart->GetTitleProperties());
    }

    // 设置颜色条标题与字体样式
    if (!colorBarTitle.isEmpty()) {
        vtkColorLegend *legend = dynamic_cast<vtkColorLegend*>(m_chart->GetLegend());
        if (legend) {
            legend->GetBrush()->SetColorF(0.2, 0.2, 0.25, 1.0);
            legend->SetTitle(colorBarTitle.toUtf8().constData());
            // 颜色条标题字体
            legend->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
            legend->GetLabelProperties()->SetFontSize(14);
            applyChineseFont(legend->GetLabelProperties());
            // 通过遍历子项找到内部 vtkAxis，设置刻度标签字体
            for (int i = 0; i < legend->GetNumberOfItems(); ++i) {
                vtkAxis *legendAxis = vtkAxis::SafeDownCast(legend->GetItem(i));
                if (legendAxis) {
                    legendAxis->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
                    legendAxis->GetTitleProperties()->SetFontSize(14);
                    applyChineseFont(legendAxis->GetTitleProperties());
                    legendAxis->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
                    legendAxis->GetLabelProperties()->SetFontSize(12);
                    applyChineseFont(legendAxis->GetLabelProperties());
                }
            }
        }
    }
}

vtkHeatmap2D::~vtkHeatmap2D()
{
    // 清理标记组
    clearMarkerGroups();

    if (m_view) {
        removeFromView(m_view);
    }
}

// ==================== 基类接口实现 ====================

void vtkHeatmap2D::setVisible(bool visible)
{
    m_visible = visible;
    m_chart->SetVisible(visible);
    render();
}

void vtkHeatmap2D::addToView(vtkContextView *view)
{
    if (!view) return;
    m_view = view;
    view->GetScene()->AddItem(m_chart);

    // 设置坐标轴字体为白色并增大字号
    for (int i = 0; i < 4; ++i) {
        vtkAxis *axis = m_chart->GetAxis(i);
        if (axis) {
            axis->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
            axis->GetTitleProperties()->SetFontSize(14);
            axis->GetTitleProperties()->SetBold(1);
            applyChineseFont(axis->GetTitleProperties());
            axis->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
            axis->GetLabelProperties()->SetFontSize(12);
            applyChineseFont(axis->GetLabelProperties());
        }
    }
}

void vtkHeatmap2D::removeFromView(vtkContextView *view)
{
    if (!view) return;
    view->GetScene()->RemoveItem(m_chart);
    m_view = nullptr;
}

void vtkHeatmap2D::render()
{
    if (m_view && m_view->GetRenderWindow()) {
        m_view->GetRenderWindow()->Render();
    }
}

// ==================== 二维热力图特有方法 ====================

void vtkHeatmap2D::updateData(const QVector<double> &data, int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    createImageData(data, rows, cols);
    m_chart->SetInputData(m_imageData);
    render();
}

void vtkHeatmap2D::setOrigin(double x, double y)
{
    if (m_imageData) {
        m_imageData->SetOrigin(x, y, 0.0);
        m_imageData->Modified();
        render();
    }
}

void vtkHeatmap2D::setSpacing(double dx, double dy)
{
    if (m_imageData) {
        m_imageData->SetSpacing(dx, dy, 1.0);
        m_imageData->Modified();
        render();
    }
}

void vtkHeatmap2D::setValueRange(double minVal, double maxVal)
{
    if (m_transferFunction) {
        m_transferFunction->RemoveAllPoints();
        // 彩虹色映射：蓝 → 青 → 绿 → 黄 → 红（与三维热力图一致）
        m_transferFunction->AddRGBPoint(minVal, 0.0, 0.0, 1.0);
        m_transferFunction->AddRGBPoint(minVal + (maxVal - minVal) * 0.25, 0.0, 1.0, 1.0);
        m_transferFunction->AddRGBPoint(minVal + (maxVal - minVal) * 0.50, 0.0, 1.0, 0.0);
        m_transferFunction->AddRGBPoint(minVal + (maxVal - minVal) * 0.75, 1.0, 1.0, 0.0);
        m_transferFunction->AddRGBPoint(maxVal, 1.0, 0.0, 0.0);
        m_transferFunction->Build();
        m_chart->SetTransferFunction(m_transferFunction);
        render();
    }
}

void vtkHeatmap2D::setColorBarTitle(const QString &title)
{
    m_colorBarTitle = title;
    if (m_chart && m_chart->GetLegend()) {
        vtkColorLegend *legend = dynamic_cast<vtkColorLegend*>(m_chart->GetLegend());
        if (legend) {
            legend->SetTitle(title.toUtf8().constData());
            legend->GetBrush()->SetColorF(0.2, 0.2, 0.25, 1.0);
            legend->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
            legend->GetLabelProperties()->SetFontSize(14);
        }
        render();
    }
}

void vtkHeatmap2D::setChartTitle(const QString &title)
{
    if (m_chart) {
        m_chart->SetTitle(title.toUtf8().constData());
        m_chart->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
        m_chart->GetTitleProperties()->SetFontSize(20);
        m_chart->GetTitleProperties()->SetBold(1);
        applyChineseFont(m_chart->GetTitleProperties());
        render();
    }
}

void vtkHeatmap2D::setXAxisTitle(const QString &title)
{
    if (m_chart) {
        vtkAxis *axis = m_chart->GetAxis(vtkAxis::BOTTOM);
        if (axis) {
            axis->SetTitle(title.toUtf8().constData());
            axis->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
            axis->GetTitleProperties()->SetFontSize(14);
            axis->GetTitleProperties()->SetBold(1);
            applyChineseFont(axis->GetTitleProperties());
        }
        render();
    }
}

void vtkHeatmap2D::setYAxisTitle(const QString &title)
{
    if (m_chart) {
        vtkAxis *axis = m_chart->GetAxis(vtkAxis::LEFT);
        if (axis) {
            axis->SetTitle(title.toUtf8().constData());
            axis->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
            axis->GetTitleProperties()->SetFontSize(14);
            axis->GetTitleProperties()->SetBold(1);
            applyChineseFont(axis->GetTitleProperties());
        }
        render();
    }
}

// ==================== 标记组操作 ====================

vtkMarkerGroup2D* vtkHeatmap2D::addMarkerGroup(const QString &name,
                                               const QColor &color,
                                               Marker2DStyle style,
                                               double size)
{
    vtkMarkerGroup2D *group = new vtkMarkerGroup2D(name, color, style, size);
    group->attachToChart(m_chart);
    m_markerGroups.append(group);
    render();
    return group;
}

void vtkHeatmap2D::removeMarkerGroup(vtkMarkerGroup2D *group)
{
    if (group) {
        group->detachFromChart(m_chart);
        m_markerGroups.removeAll(group);
        delete group;
        render();
    }
}

void vtkHeatmap2D::clearMarkerGroups()
{
    for (auto group : m_markerGroups) {
        group->detachFromChart(m_chart);
        delete group;
    }
    m_markerGroups.clear();
    render();
}

QList<vtkMarkerGroup2D*> vtkHeatmap2D::getMarkerGroups() const
{
    return m_markerGroups;
}

// ==================== 私有方法 ====================

void vtkHeatmap2D::createImageData(const QVector<double> &data, int rows, int cols)
{
    m_imageData = vtkSmartPointer<vtkImageData>::New();
    m_imageData->SetExtent(0, cols - 1, 0, rows - 1, 0, 0);
    m_imageData->AllocateScalars(VTK_DOUBLE, 1);

    double *ptr = static_cast<double *>(m_imageData->GetScalarPointer(0, 0, 0));
    for (int i = 0; i < rows && i * cols < data.size(); ++i) {
        for (int j = 0; j < cols && i * cols + j < data.size(); ++j) {
            ptr[i * cols + j] = data[i * cols + j];
        }
    }
}

void vtkHeatmap2D::createDefaultTransferFunction()
{
    m_transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    // 彩虹色映射：蓝 → 青 → 绿 → 黄 → 红（与三维热力图一致）
    m_transferFunction->AddRGBPoint(0.00, 0.0, 0.0, 1.0);   // 蓝
    m_transferFunction->AddRGBPoint(0.25, 0.0, 1.0, 1.0);   // 青
    m_transferFunction->AddRGBPoint(0.50, 0.0, 1.0, 0.0);   // 绿
    m_transferFunction->AddRGBPoint(0.75, 1.0, 1.0, 0.0);   // 黄
    m_transferFunction->AddRGBPoint(1.00, 1.0, 0.0, 0.0);   // 红
    m_transferFunction->Build();
}
