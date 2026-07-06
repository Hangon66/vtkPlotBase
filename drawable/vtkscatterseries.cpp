#include "vtkscatterseries.h"

#include <vtkChartXY.h>
#include <vtkChart.h>
#include <vtkPlotPoints.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkPen.h>
#include <vtkContextView.h>
#include <vtkRenderWindow.h>

// ==================== 构造函数与析构函数 ====================

vtkScatterSeries::vtkScatterSeries(vtkChartXY *chart, const QVector<QPointF> &points,
                                   const QColor &color, Marker2DStyle style, double size,
                                   const QString &name)
    : m_id(QUuid::createUuid().toString())
    , m_name(name)
    , m_visible(true)
    , m_color(color)
    , m_style(style)
    , m_size(size)
    , m_points(points)
    , m_chart(chart)
    , m_plot(nullptr)
    , m_view(nullptr)
{
    if (!m_chart) return;

    // 创建数据表（X、Y 两列）
    m_table = vtkSmartPointer<vtkTable>::New();

    vtkSmartPointer<vtkFloatArray> colX = vtkSmartPointer<vtkFloatArray>::New();
    colX->SetName("X");
    colX->SetNumberOfValues(points.size());
    m_table->AddColumn(colX);

    vtkSmartPointer<vtkFloatArray> colY = vtkSmartPointer<vtkFloatArray>::New();
    colY->SetName("Y");
    colY->SetNumberOfValues(points.size());
    m_table->AddColumn(colY);

    // 填充数据
    rebuildTable();

    // 创建散点图
    vtkPlot *plot = m_chart->AddPlot(vtkChart::POINTS);
    m_plot = vtkPlotPoints::SafeDownCast(plot);
    if (m_plot) {
        m_plot->SetInputData(m_table, 0, 1);

        // 设置颜色
        m_plot->SetColor(color.red(), color.green(), color.blue(), color.alpha());

        // 设置标记样式和大小
        m_plot->SetMarkerStyle(static_cast<int>(style));
        m_plot->SetMarkerSize(size);

        // 图例标签
        if (!name.isEmpty()) {
            m_plot->SetLabel(name.toUtf8().constData());
        }
    }
}

vtkScatterSeries::~vtkScatterSeries()
{
    if (m_chart && m_plot) {
        m_chart->RemovePlotInstance(m_plot);
        m_plot = nullptr;
    }
}

// ==================== 基类接口实现 ====================

void vtkScatterSeries::setVisible(bool visible)
{
    m_visible = visible;
    if (m_plot) {
        m_plot->SetVisible(visible);
    }
    render();
}

void vtkScatterSeries::addToView(vtkContextView *view)
{
    if (!view) return;
    m_view = view;
}

void vtkScatterSeries::removeFromView(vtkContextView *view)
{
    Q_UNUSED(view);
    m_view = nullptr;
}

void vtkScatterSeries::render()
{
    if (m_view && m_view->GetRenderWindow()) {
        m_view->GetRenderWindow()->Render();
    }
}

// ==================== 属性操作 ====================

void vtkScatterSeries::setColor(const QColor &color)
{
    m_color = color;
    if (m_plot) {
        m_plot->SetColor(color.red(), color.green(), color.blue(), color.alpha());
    }
    render();
}

void vtkScatterSeries::setMarkerStyle(Marker2DStyle style)
{
    m_style = style;
    if (m_plot) {
        m_plot->SetMarkerStyle(static_cast<int>(style));
    }
    render();
}

void vtkScatterSeries::setMarkerSize(double size)
{
    m_size = size;
    if (m_plot) {
        m_plot->SetMarkerSize(size);
    }
    render();
}

void vtkScatterSeries::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
    rebuildTable();
    if (m_plot && m_table) {
        m_plot->SetInputData(m_table, 0, 1);
    }
    render();
}

// ==================== 私有方法 ====================

void vtkScatterSeries::rebuildTable()
{
    if (!m_table) return;

    int n = m_points.size();

    vtkFloatArray *colX = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("X"));
    vtkFloatArray *colY = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("Y"));

    if (colX) colX->SetNumberOfValues(n);
    if (colY) colY->SetNumberOfValues(n);

    for (int i = 0; i < n; ++i) {
        if (colX) colX->SetValue(i, static_cast<float>(m_points[i].x()));
        if (colY) colY->SetValue(i, static_cast<float>(m_points[i].y()));
    }
}
