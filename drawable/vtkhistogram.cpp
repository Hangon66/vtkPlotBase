#include "vtkhistogram.h"

#include <vtkChartXY.h>
#include <vtkChart.h>
#include <vtkAxis.h>
#include <vtkPlotArea.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkBrush.h>
#include <vtkPen.h>
#include <vtkTextProperty.h>
#include <vtkContextView.h>
#include <vtkRenderWindow.h>

#include <algorithm>

// ==================== 构造函数与析构函数 ====================

vtkHistogram::vtkHistogram(vtkChartXY *chart, const QVector<double> &data, int numBins,
                           const QColor &color, const QString &name)
    : m_id(QUuid::createUuid().toString())
    , m_name(name)
    , m_visible(true)
    , m_numBins(numBins)
    , m_color(color)
    , m_chart(chart)
    , m_areaPlot(nullptr)
    , m_view(nullptr)
{
    if (!m_chart) return;

    // 创建数据表（三列：bin_center、baseline=0、frequency）
    m_table = vtkSmartPointer<vtkTable>::New();

    vtkSmartPointer<vtkFloatArray> binCenterArr = vtkSmartPointer<vtkFloatArray>::New();
    binCenterArr->SetName("bin_center");
    binCenterArr->SetNumberOfValues(numBins);
    m_table->AddColumn(binCenterArr);

    vtkSmartPointer<vtkFloatArray> baselineArr = vtkSmartPointer<vtkFloatArray>::New();
    baselineArr->SetName("baseline");
    baselineArr->SetNumberOfValues(numBins);
    baselineArr->Fill(0.0);
    m_table->AddColumn(baselineArr);

    vtkSmartPointer<vtkFloatArray> freqArr = vtkSmartPointer<vtkFloatArray>::New();
    freqArr->SetName("frequency");
    freqArr->SetNumberOfValues(numBins);
    m_table->AddColumn(freqArr);

    // 分箱统计并写入数据表
    computeHistogram(data, numBins);

    // 创建面积图：baseline -> frequency 之间填充
    vtkPlot *plot = m_chart->AddPlot(vtkChart::AREA);
    m_areaPlot = vtkPlotArea::SafeDownCast(plot);
    if (m_areaPlot) {
        m_areaPlot->SetInputData(m_table);
        m_areaPlot->SetInputArray(0, "bin_center");
        m_areaPlot->SetInputArray(1, "baseline");
        m_areaPlot->SetInputArray(2, "frequency");

        // 设置半透明填充颜色
        if (m_areaPlot->GetBrush()) {
            m_areaPlot->GetBrush()->SetColorF(
                color.redF(), color.greenF(), color.blueF(), color.alphaF());
        }
        // 边框线设为透明（无边框）
        if (m_areaPlot->GetPen()) {
            m_areaPlot->GetPen()->SetColorF(
                color.redF(), color.greenF(), color.blueF(), 0.0);
        }

        // 图例标签
        if (!name.isEmpty()) {
            m_areaPlot->SetLabel(name.toUtf8().constData());
        }
    }
}

vtkHistogram::~vtkHistogram()
{
    if (m_chart && m_areaPlot) {
        // 从图表中移除本面积图
        vtkIdType idx = m_chart->GetPlotIndex(m_areaPlot);
        if (idx >= 0) {
            m_chart->RemovePlot(idx);
        }
    }
}

// ==================== 基类接口实现 ====================

void vtkHistogram::setVisible(bool visible)
{
    m_visible = visible;
    if (m_areaPlot) {
        m_areaPlot->SetVisible(visible);
    }
    render();
}

void vtkHistogram::addToView(vtkContextView *view)
{
    if (!view) return;
    m_view = view;
}

void vtkHistogram::removeFromView(vtkContextView *view)
{
    Q_UNUSED(view);
    m_view = nullptr;
}

void vtkHistogram::render()
{
    if (m_view && m_view->GetRenderWindow()) {
        m_view->GetRenderWindow()->Render();
    }
}

// ==================== 属性操作 ====================

void vtkHistogram::setColor(const QColor &color)
{
    m_color = color;
    if (m_areaPlot && m_areaPlot->GetBrush()) {
        m_areaPlot->GetBrush()->SetColorF(
            color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
    render();
}

void vtkHistogram::updateData(const QVector<double> &data, int numBins)
{
    m_numBins = numBins;
    // 重建表列大小
    if (m_table) {
        vtkFloatArray *binCenterArr = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("bin_center"));
        vtkFloatArray *baselineArr = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("baseline"));
        vtkFloatArray *freqArr = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("frequency"));
        if (binCenterArr) binCenterArr->SetNumberOfValues(numBins);
        if (baselineArr) {
            baselineArr->SetNumberOfValues(numBins);
            baselineArr->Fill(0.0);
        }
        if (freqArr) freqArr->SetNumberOfValues(numBins);
    }
    computeHistogram(data, numBins);
    render();
}

void vtkHistogram::setTitle(const QString &title)
{
    if (m_chart) {
        m_chart->SetTitle(title.toUtf8().constData());
        applyChineseFont(m_chart->GetTitleProperties());
    }
}

void vtkHistogram::setXAxisTitle(const QString &title)
{
    if (m_chart) {
        vtkAxis *axis = m_chart->GetAxis(vtkAxis::BOTTOM);
        if (axis) {
            axis->SetTitle(title.toUtf8().constData());
            applyChineseFont(axis->GetTitleProperties());
        }
    }
}

void vtkHistogram::setYAxisTitle(const QString &title)
{
    if (m_chart) {
        vtkAxis *axis = m_chart->GetAxis(vtkAxis::LEFT);
        if (axis) {
            axis->SetTitle(title.toUtf8().constData());
            applyChineseFont(axis->GetTitleProperties());
        }
    }
}

double vtkHistogram::maxFrequency() const
{
    if (!m_table) return 0.0;
    vtkAbstractArray *freqCol = m_table->GetColumnByName("frequency");
    if (!freqCol || freqCol->GetNumberOfTuples() == 0) return 0.0;
    double maxVal = freqCol->GetVariantValue(0).ToDouble();
    for (vtkIdType i = 1; i < freqCol->GetNumberOfTuples(); ++i) {
        double v = freqCol->GetVariantValue(i).ToDouble();
        if (v > maxVal) maxVal = v;
    }
    return maxVal;
}

// ==================== 私有方法 ====================

void vtkHistogram::computeHistogram(const QVector<double> &data, int numBins)
{
    if (data.isEmpty() || numBins <= 0 || !m_table) return;

    double minVal = *std::min_element(data.begin(), data.end());
    double maxVal = *std::max_element(data.begin(), data.end());

    if (qFuzzyCompare(maxVal, minVal)) {
        minVal -= 1.0;
        maxVal += 1.0;
    }

    double binWidth = (maxVal - minVal) / numBins;

    QVector<int> frequencies(numBins, 0);
    for (double val : data) {
        int bin = static_cast<int>((val - minVal) / binWidth);
        if (bin >= numBins) bin = numBins - 1;
        if (bin < 0) bin = 0;
        frequencies[bin]++;
    }

    // 写入数据表
    vtkFloatArray *binCenterArr = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("bin_center"));
    vtkFloatArray *freqArr = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("frequency"));
    if (binCenterArr && freqArr) {
        for (int i = 0; i < numBins; ++i) {
            binCenterArr->SetValue(i, static_cast<float>(minVal + (i + 0.5) * binWidth));
            freqArr->SetValue(i, static_cast<float>(frequencies[i]));
        }
    }
}

void vtkHistogram::applyChineseFont(vtkTextProperty *prop)
{
    if (!prop) return;
    prop->SetFontFamily(VTK_FONT_FILE);
    prop->SetFontFile("C:/Windows/Fonts/msyh.ttc");
}
