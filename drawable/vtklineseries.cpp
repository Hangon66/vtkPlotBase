#include "vtklineseries.h"

#include <vtkChartXY.h>
#include <vtkChart.h>
#include <vtkPlotLine.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkPen.h>
#include <vtkContextView.h>
#include <vtkRenderWindow.h>

// ==================== 构造函数与析构函数 ====================

vtkLineSeries::vtkLineSeries(vtkChartXY *chart, const QVector<QPointF> &points,
                             const QColor &color, double width,
                             const QString &name)
    : m_id(QUuid::createUuid().toString())
    , m_name(name)
    , m_visible(true)
    , m_color(color)
    , m_width(width)
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

    // 创建线条图
    vtkPlot *plot = m_chart->AddPlot(vtkChart::LINE);
    m_plot = vtkPlotLine::SafeDownCast(plot);
    if (m_plot) {
        m_plot->SetInputData(m_table, 0, 1);

        // 设置颜色和宽度
        if (m_plot->GetPen()) {
            m_plot->GetPen()->SetColorF(color.redF(), color.greenF(), color.blueF(), color.alphaF());
            m_plot->GetPen()->SetWidth(width);
        }

        // 图例标签
        if (!name.isEmpty()) {
            m_plot->SetLabel(name.toUtf8().constData());
        } else {
            m_plot->SetLegendVisibility(false);
        }
    }
}

vtkLineSeries::~vtkLineSeries()
{
    if (m_chart && m_plot) {
        vtkIdType idx = m_chart->GetPlotIndex(m_plot);
        if (idx >= 0) {
            m_chart->RemovePlot(idx);
        }
    }
}

// ==================== 基类接口实现 ====================

void vtkLineSeries::setVisible(bool visible)
{
    m_visible = visible;
    if (m_plot) {
        m_plot->SetVisible(visible);
    }
    render();
}

void vtkLineSeries::addToView(vtkContextView *view)
{
    if (!view) return;
    m_view = view;
}

void vtkLineSeries::removeFromView(vtkContextView *view)
{
    Q_UNUSED(view);
    m_view = nullptr;
}

void vtkLineSeries::render()
{
    if (m_view && m_view->GetRenderWindow()) {
        m_view->GetRenderWindow()->Render();
    }
}

// ==================== 属性操作 ====================

void vtkLineSeries::setColor(const QColor &color)
{
    m_color = color;
    if (m_plot && m_plot->GetPen()) {
        m_plot->GetPen()->SetColorF(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
    render();
}

void vtkLineSeries::setWidth(double width)
{
    m_width = width;
    if (m_plot && m_plot->GetPen()) {
        m_plot->GetPen()->SetWidth(width);
    }
    render();
}

void vtkLineSeries::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
    rebuildTable();
    if (m_plot && m_table) {
        m_plot->SetInputData(m_table, 0, 1);
    }
    render();
}

// ==================== 私有方法 ====================

void vtkLineSeries::rebuildTable()
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
