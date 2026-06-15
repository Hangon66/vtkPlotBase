#include "vtkmarkergroup2d.h"

#include <vtkChartHistogram2D.h>
#include <vtkChart.h>
#include <vtkPlot.h>
#include <vtkFloatArray.h>

// ==================== 构造函数与析构函数 ====================

vtkMarkerGroup2D::vtkMarkerGroup2D(const QString &name,
                                   const QColor &color,
                                   Marker2DStyle style,
                                   double size)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name(name)
    , m_visible(true)
    , m_color(color)
    , m_style(style)
    , m_size(size)
    , m_lineWidth(3.0)
    , m_label(name)
    , m_table(vtkSmartPointer<vtkTable>::New())
    , m_plot(nullptr)
    , m_chart(nullptr)
{
}

vtkMarkerGroup2D::~vtkMarkerGroup2D()
{
    if (m_chart) {
        detachFromChart(m_chart);
    }
}

// ==================== 标识设置 ====================

void vtkMarkerGroup2D::setName(const QString &name)
{
    m_name = name;
    m_label = name;
    if (m_plot) {
        m_plot->SetLabel(name.toUtf8().constData());
    }
}

// ==================== 可见性 ====================

void vtkMarkerGroup2D::setVisible(bool visible)
{
    m_visible = visible;
    if (m_plot) {
        m_plot->SetVisible(visible);
    }
}

// ==================== 数据操作 ====================

void vtkMarkerGroup2D::addPoint(double x, double y)
{
    m_points.append(QPointF(x, y));
    if (!m_plot && m_chart) {
        createPlot();
    } else {
        rebuildTable();
    }
}

void vtkMarkerGroup2D::addPoints(const QVector<QPointF> &points)
{
    m_points.append(points);
    if (!m_plot && m_chart) {
        createPlot();
    } else {
        rebuildTable();
    }
}

void vtkMarkerGroup2D::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
    if (!m_plot && m_chart) {
        createPlot();
    } else {
        rebuildTable();
    }
}

void vtkMarkerGroup2D::clearPoints()
{
    m_points.clear();
    rebuildTable();
}

QVector<QPointF> vtkMarkerGroup2D::points() const
{
    return m_points;
}

int vtkMarkerGroup2D::pointCount() const
{
    return m_points.size();
}

// ==================== 外观设置 ====================

void vtkMarkerGroup2D::setColor(const QColor &color)
{
    m_color = color;
    if (m_plot) {
        m_plot->SetColor(color.red(), color.green(), color.blue(), color.alpha());
    }
}

void vtkMarkerGroup2D::setMarkerStyle(Marker2DStyle style)
{
    m_style = style;
    if (m_plot) {
        m_plot->SetMarkerStyle(static_cast<int>(style));
    }
}

void vtkMarkerGroup2D::setMarkerSize(double size)
{
    m_size = size;
    if (m_plot) {
        m_plot->SetMarkerSize(size);
    }
}

void vtkMarkerGroup2D::setLineWidth(double width)
{
    m_lineWidth = width;
    if (m_plot) {
        m_plot->SetWidth(width);
    }
}

void vtkMarkerGroup2D::setLabel(const QString &label)
{
    m_label = label;
    if (m_plot) {
        m_plot->SetLabel(label.toUtf8().constData());
    }
}

// ==================== 图表附着 ====================

void vtkMarkerGroup2D::attachToChart(vtkChartHistogram2D *chart)
{
    if (!chart || m_chart == chart) return;

    // 若已附着到其他图表，先分离
    if (m_chart && m_chart != chart) {
        detachFromChart(m_chart);
    }

    m_chart = chart;

    // 如果已有数据，立即创建 plot（匹配参考示例流程）
    if (!m_points.isEmpty()) {
        createPlot();
    }
}

void vtkMarkerGroup2D::detachFromChart(vtkChartHistogram2D *chart)
{
    if (!chart || m_chart != chart) return;

    // 从图表中移除散点图
    if (m_plot) {
        chart->RemovePlotInstance(m_plot);
        m_plot = nullptr;
    }

    m_chart = nullptr;
}

// ==================== 私有方法 ====================

void vtkMarkerGroup2D::createPlot()
{
    if (!m_chart || m_points.isEmpty()) return;

    // === 步骤 1：填充表数据（与参考示例一致）===
    int n = m_points.size();
    m_table = vtkSmartPointer<vtkTable>::New();

    vtkSmartPointer<vtkFloatArray> colX = vtkSmartPointer<vtkFloatArray>::New();
    colX->SetName("X");
    m_table->AddColumn(colX);

    vtkSmartPointer<vtkFloatArray> colY = vtkSmartPointer<vtkFloatArray>::New();
    colY->SetName("Y");
    m_table->AddColumn(colY);

    m_table->SetNumberOfRows(n);
    for (int i = 0; i < n; ++i) {
        m_table->SetValue(i, 0, static_cast<float>(m_points[i].x()));
        m_table->SetValue(i, 1, static_cast<float>(m_points[i].y()));
    }

    // === 步骤 2：AddPlot 创建散点图 ===
    vtkPlot *plot = m_chart->AddPlot(vtkChart::POINTS);
    m_plot = vtkPlotPoints::SafeDownCast(plot);

    if (m_plot) {
        // === 步骤 3：SetInputData ===
        m_plot->SetInputData(m_table, 0, 1);

        // === 步骤 4：设置标签 ===
        if (!m_label.isEmpty()) {
            m_plot->SetLabel(m_label.toUtf8().constData());
        }

        // === 步骤 5：设置颜色 ===
        m_plot->SetColor(m_color.red(), m_color.green(), m_color.blue(), 255);

        // === 步骤 6：设置线宽 ===
        m_plot->SetWidth(m_lineWidth);

        // === 步骤 7：设置标记大小和样式 ===
        m_plot->SetMarkerSize(m_size);
        m_plot->SetMarkerStyle(static_cast<int>(m_style));
    }
}

void vtkMarkerGroup2D::rebuildTable()
{
    if (!m_plot || m_points.isEmpty()) return;

    int n = m_points.size();
    m_table = vtkSmartPointer<vtkTable>::New();

    vtkSmartPointer<vtkFloatArray> colX = vtkSmartPointer<vtkFloatArray>::New();
    colX->SetName("X");
    m_table->AddColumn(colX);

    vtkSmartPointer<vtkFloatArray> colY = vtkSmartPointer<vtkFloatArray>::New();
    colY->SetName("Y");
    m_table->AddColumn(colY);

    m_table->SetNumberOfRows(n);
    for (int i = 0; i < n; ++i) {
        m_table->SetValue(i, 0, static_cast<float>(m_points[i].x()));
        m_table->SetValue(i, 1, static_cast<float>(m_points[i].y()));
    }

    m_plot->SetInputData(m_table, 0, 1);
}
