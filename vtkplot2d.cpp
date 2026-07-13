#include "vtkplot2d.h"
#include "ui_vtkplot2d.h"

// drawable 类
#include "drawable/vtkheatmap2d.h"
#include "drawable/vtkhistogram.h"
#include "drawable/vtkmarkergroup2d.h"
#include "drawable/vtkscatterseries.h"
#include "drawable/vtklineseries.h"

// VTK Qt 头文件
#include <QVTKOpenGLNativeWidget.h>
#include <QSurfaceFormat>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkContextView.h>
#include <vtkContextInteractorStyle.h>
#include <vtkChartHistogram2D.h>
#include <vtkAxis.h>
#include <vtkColorLegend.h>
#include <vtkBrush.h>
#include <vtkNamedColors.h>
#include <vtkTextProperty.h>
#include <vtkChartXY.h>
#include <vtkChart.h>
#include <vtkChartLegend.h>
#include <vtkPlotArea.h>
#include <vtkPlotLine.h>
#include <vtkPlotPoints.h>
#include <vtkPen.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>

#include <QShowEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QScrollArea>

// ==================== 构造函数与析构函数 ====================

vtkPlot2D::vtkPlot2D(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::vtkPlot2D)
    , m_vtkWidget(nullptr)
    , m_interactionEnabled(false)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);  // 确保可接收键盘事件
    setupVTK();
}

vtkPlot2D::~vtkPlot2D()
{
    if (m_syncTimer)
        m_syncTimer->stop();
    clearAll();
    // overlay 无父对象，需显式删除
    delete m_overlayWindow;
    delete ui;
}

// ==================== 事件处理 ====================

void vtkPlot2D::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    static bool filterInstalled = false;
    if (!filterInstalled) {
        QWidget *p = parentWidget();
        while (p) {
            if (QScrollArea *sa = qobject_cast<QScrollArea*>(p)) {
                sa->installEventFilter(this);
                sa->viewport()->installEventFilter(this);
                filterInstalled = true;
                break;
            }
            p = p->parentWidget();
        }
    }

    render();
}

void vtkPlot2D::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    syncWindow();
}

void vtkPlot2D::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    syncWindow();
}

bool vtkPlot2D::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Scroll:
    case QEvent::Wheel:
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Paint:
        syncWindow();
        break;
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

void vtkPlot2D::wheelEvent(QWheelEvent *event)
{
    // 接受事件，阻止向上传播到 QScrollArea
    event->accept();
}

void vtkPlot2D::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_R) {
        // 重置所有图表坐标轴范围，恢复默认视图
        for (auto heatmap : m_heatmap2Ds) {
            if (heatmap->chart()) {
                heatmap->chart()->RecalculateBounds();
            }
        }
        if (m_histogramChart) {
            m_histogramChart->RecalculateBounds();
        }
        if (m_scatterChart) {
            m_scatterChart->RecalculateBounds();
        }
        render();
        return;
    }
    QWidget::keyPressEvent(event);
}

bool vtkPlot2D::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::ActivationChange:
        render();
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

// ==================== 初始化方法 ====================

void vtkPlot2D::setupVTK()
{
    // 创建 VTK 控件
    m_vtkWidget = new QVTKOpenGLNativeWidget();
    m_vtkWidget->setFormat(QVTKOpenGLNativeWidget::defaultFormat());

    // 创建独立顶层窗口承载 VTK 控件，避免 QOpenGLWidget 强制父窗口 RHI 切换为 OpenGL
    m_overlayWindow = new QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint);
    // 不使用布局管理器，避免 overlay resize 时连锁触发 VTK 控件 resize
    m_vtkWidget->setParent(m_overlayWindow);

    // 创建布局（自身无 VTK 控件，仅占位）
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 位置同步定时器
    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(16); // 16ms ≈ 60fps
    connect(m_syncTimer, &QTimer::timeout, this, &vtkPlot2D::syncWindow);
    m_syncTimer->start();

    // 创建上下文视图
    m_contextView = vtkSmartPointer<vtkContextView>::New();
    m_contextView->SetRenderWindow(m_vtkWidget->renderWindow());

    // 设置默认背景
    m_contextView->GetRenderer()->SetBackground(0.2, 0.2, 0.2);

    // 默认禁用交互（不可平移/缩放）
    setInteractionEnabled(false);
}

// ==================== 顶层窗口同步 ====================

void vtkPlot2D::syncWindow()
{
    if (!m_overlayWindow) return;

    // 检查顶层窗口是否最小化
    QWidget *topLevel = window();
    if (topLevel && topLevel->isMinimized()) {
        m_overlayWindow->hide();
        return;
    }

    if (!isVisible() || !m_vtkWidget || size().isEmpty()) {
        m_overlayWindow->hide();
        return;
    }

    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    QSize widgetSize = size();

    // 仅在控件尺寸真正变化时才调整 VTK 控件
    if (widgetSize != m_lastVtkSize) {
        m_vtkWidget->setGeometry(0, 0, widgetSize.width(), widgetSize.height());
        m_lastVtkSize = widgetSize;
    }

    // ---- 前置检查：QScrollArea 可见性 ----
    // 在 show/raise 之前判断是否在 viewport 可视区域内，
    // 避免 overlay 反复 show→raise→hide 窃取窗口激活状态
    QScrollArea *scrollArea = nullptr;
    QWidget *p = parentWidget();
    while (p) {
        scrollArea = qobject_cast<QScrollArea*>(p);
        if (scrollArea) break;
        p = p->parentWidget();
    }

    QRect targetRect;  // overlay 最终应放置的屏幕矩形

    if (scrollArea && scrollArea->viewport()) {
        QWidget *viewport = scrollArea->viewport();
        QPoint vpGlobal = viewport->mapToGlobal(QPoint(0, 0));
        QRect vpRect(vpGlobal, viewport->size());
        QRect widgetRect(globalPos, widgetSize);
        QRect visible = widgetRect.intersected(vpRect);

        if (visible.isEmpty()) {
            // 控件完全在 viewport 可视区域之外，保持 overlay 隐藏，不执行 show/raise
            m_overlayWindow->hide();
            return;
        }
        targetRect = visible;
    } else {
        targetRect = QRect(globalPos, widgetSize);
    }

    // ---- 通过可见性检查后，才 show/raise overlay ----
    if (!m_overlayWindow->isVisible())
        m_overlayWindow->show();

    // 确保独立窗口在主窗口之上（处理 z-order 变化，点击主窗口时 overlay 不会被遮挡）
    if (topLevel && topLevel->isActiveWindow()) {
        m_overlayWindow->raise();
    }

    // 设置 overlay 位置与尺寸
    m_overlayWindow->move(targetRect.topLeft());
    m_overlayWindow->resize(targetRect.size());
}

// ==================== 图表标题操作 ====================

void vtkPlot2D::setTitle(const QString &title)
{
    for (auto heatmap : m_heatmap2Ds) {
        heatmap->setChartTitle(title);
    }
}

void vtkPlot2D::setBackground(const QColor &color)
{
    if (m_contextView && m_contextView->GetRenderer()) {
        m_contextView->GetRenderer()->SetBackground(
            color.redF(), color.greenF(), color.blueF());
        render();
    }
}

void vtkPlot2D::setInteractionEnabled(bool enabled)
{
    m_interactionEnabled = enabled;
    if (!m_contextView) return;

    auto *interactor = m_contextView->GetInteractor();
    if (!interactor) return;

    if (enabled) {
        // 恢复默认上下文交互样式（平移、缩放）
        vtkNew<vtkContextInteractorStyle> style;
        interactor->SetInteractorStyle(style);
    } else {
        // 设置为空样式，禁用所有鼠标交互
        interactor->SetInteractorStyle(nullptr);
    }
}

// ==================== 坐标轴操作 ====================

void vtkPlot2D::setXAxisTitle(const QString &title)
{
    for (auto heatmap : m_heatmap2Ds) {
        heatmap->setXAxisTitle(title);
    }
}

void vtkPlot2D::setYAxisTitle(const QString &title)
{
    for (auto heatmap : m_heatmap2Ds) {
        heatmap->setYAxisTitle(title);
    }
}

// ==================== 二维热力图操作 ====================

vtkHeatmap2D* vtkPlot2D::addHeatmap2D(const QVector<double> &data, int rows, int cols,
                                       const QString &colorBarTitle)
{
    vtkHeatmap2D *heatmap = new vtkHeatmap2D(data, rows, cols, colorBarTitle);
    heatmap->addToView(m_contextView);
    m_heatmap2Ds.append(heatmap);
    render();
    return heatmap;
}

void vtkPlot2D::setHeatmap2DVisible(vtkHeatmap2D *heatmap, bool visible)
{
    if (heatmap) {
        heatmap->setVisible(visible);
    }
}

void vtkPlot2D::updateHeatmap2DData(vtkHeatmap2D *heatmap, const QVector<double> &data,
                                    int rows, int cols)
{
    if (heatmap) {
        heatmap->updateData(data, rows, cols);
    }
}

void vtkPlot2D::removeHeatmap2D(vtkHeatmap2D *heatmap)
{
    if (heatmap) {
        heatmap->removeFromView(m_contextView);
        m_heatmap2Ds.removeAll(heatmap);
        delete heatmap;
        render();
    }
}

void vtkPlot2D::clearAllHeatmap2D()
{
    for (auto heatmap : m_heatmap2Ds) {
        heatmap->removeFromView(m_contextView);
        delete heatmap;
    }
    m_heatmap2Ds.clear();
    render();
}

void vtkPlot2D::setHeatmap2DDiscreteColorMap(vtkHeatmap2D *heatmap, const QVector<QColor> &colors)
{
    if (heatmap)
        heatmap->setDiscreteColorMap(colors);
}

void vtkPlot2D::setHeatmap2DColorBarVisible(vtkHeatmap2D *heatmap, bool visible)
{
    if (heatmap)
        heatmap->setColorBarVisible(visible);
}

QList<vtkHeatmap2D*> vtkPlot2D::getHeatmap2Ds() const
{
    return m_heatmap2Ds;
}

// ==================== 标记组操作 ====================

vtkMarkerGroup2D* vtkPlot2D::addMarkerGroup(vtkHeatmap2D *heatmap,
                                             const QString &name,
                                             const QColor &color,
                                             Marker2DStyle style,
                                             double size)
{
    if (heatmap) {
        return heatmap->addMarkerGroup(name, color, style, size);
    }
    return nullptr;
}

vtkMarkerGroup2D* vtkPlot2D::addMarkerGroup(const QString &name,
                                             const QColor &color,
                                             Marker2DStyle style,
                                             double size)
{
    if (!m_heatmap2Ds.isEmpty()) {
        return m_heatmap2Ds.first()->addMarkerGroup(name, color, style, size);
    }
    return nullptr;
}

void vtkPlot2D::removeMarkerGroup(vtkHeatmap2D *heatmap, vtkMarkerGroup2D *group)
{
    if (heatmap && group) {
        heatmap->removeMarkerGroup(group);
    }
}

void vtkPlot2D::clearMarkerGroups(vtkHeatmap2D *heatmap)
{
    if (heatmap) {
        heatmap->clearMarkerGroups();
    }
}

// ==================== 清除所有 ====================

void vtkPlot2D::clearAll()
{
    clearAllHeatmap2D();
    clearAllHistograms();
    clearAllScatterSeries();
    clearAllLineSeries();
}

// ==================== 概率分布直方图操作 ====================

vtkHistogram* vtkPlot2D::addHistogram(const QVector<double> &data, int numBins,
                                       const QColor &color, const QString &name)
{
    // 首次调用时创建共享图表并配置样式
    if (!m_histogramChart) {
        m_histogramChart = vtkSmartPointer<vtkChartXY>::New();
        m_contextView->GetScene()->AddItem(m_histogramChart);

        // 配置坐标轴字体（白色 + 中文字体）
        for (int i = 0; i < 4; ++i) {
            vtkAxis *axis = m_histogramChart->GetAxis(i);
            if (axis) {
                axis->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
                axis->GetTitleProperties()->SetFontSize(14);
                axis->GetTitleProperties()->SetBold(1);
                vtkTextProperty *tp = axis->GetTitleProperties();
                tp->SetFontFamily(VTK_FONT_FILE);
                tp->SetFontFile("C:/Windows/Fonts/msyh.ttc");
                axis->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
                axis->GetLabelProperties()->SetFontSize(12);
                vtkTextProperty *lp = axis->GetLabelProperties();
                lp->SetFontFamily(VTK_FONT_FILE);
                lp->SetFontFile("C:/Windows/Fonts/msyh.ttc");
            }
        }

        // 默认轴标题
        m_histogramChart->GetAxis(vtkAxis::BOTTOM)->SetTitle("Value");
        m_histogramChart->GetAxis(vtkAxis::LEFT)->SetTitle("Frequency");

        // 默认图表标题
        m_histogramChart->SetTitle("Histogram");
        m_histogramChart->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
        m_histogramChart->GetTitleProperties()->SetFontSize(18);
        m_histogramChart->GetTitleProperties()->SetBold(1);
        vtkTextProperty *ttp = m_histogramChart->GetTitleProperties();
        ttp->SetFontFamily(VTK_FONT_FILE);
        ttp->SetFontFile("C:/Windows/Fonts/msyh.ttc");

        // 显示图例
        m_histogramChart->SetShowLegend(true);
        if (m_histogramChart->GetLegend()) {
            vtkTextProperty *legendProp = m_histogramChart->GetLegend()->GetLabelProperties();
            legendProp->SetFontFamily(VTK_FONT_FILE);
            legendProp->SetFontFile("C:/Windows/Fonts/msyh.ttc");
        }
    }

    // 创建直方图面积图（添加到共享图表）
    vtkHistogram *histogram = new vtkHistogram(m_histogramChart, data, numBins, color, name);
    histogram->addToView(m_contextView);
    m_histograms.append(histogram);
    render();
    return histogram;
}

void vtkPlot2D::setHistogramVisible(vtkHistogram *histogram, bool visible)
{
    if (histogram) {
        histogram->setVisible(visible);
    }
}

void vtkPlot2D::removeHistogram(vtkHistogram *histogram)
{
    if (histogram) {
        delete histogram;  // 析构函数会从图表移除面积图
        m_histograms.removeAll(histogram);
        render();
    }
}

void vtkPlot2D::clearAllHistograms()
{
    for (auto histogram : m_histograms) {
        delete histogram;
    }
    m_histograms.clear();

    // 从场景移除共享图表并释放
    if (m_histogramChart) {
        m_contextView->GetScene()->RemoveItem(m_histogramChart);
        m_histogramChart = nullptr;
    }
    render();
}

QList<vtkHistogram*> vtkPlot2D::getHistograms() const
{
    return m_histograms;
}

void vtkPlot2D::addHistogramRefLine(double xValue, const QColor &color, double width,
                                     const QString &label)
{
    if (!m_histogramChart || m_histograms.isEmpty()) return;

    // 从所有直方图数据中获取最大频率值
    double maxY = 0.0;
    for (auto *h : m_histograms) {
        double mf = h->maxFrequency();
        if (mf > maxY) maxY = mf;
    }
    if (maxY <= 0.0) maxY = 1.0;

    // 创建参考线数据表（两个点构成垂直线）
    vtkSmartPointer<vtkTable> refTable = vtkSmartPointer<vtkTable>::New();
    vtkSmartPointer<vtkFloatArray> xArr = vtkSmartPointer<vtkFloatArray>::New();
    xArr->SetName("x");
    xArr->SetNumberOfValues(2);
    xArr->SetValue(0, static_cast<float>(xValue));
    xArr->SetValue(1, static_cast<float>(xValue));
    refTable->AddColumn(xArr);

    vtkSmartPointer<vtkFloatArray> yArr = vtkSmartPointer<vtkFloatArray>::New();
    yArr->SetName("y");
    yArr->SetNumberOfValues(2);
    yArr->SetValue(0, 0.0f);
    yArr->SetValue(1, static_cast<float>(maxY));
    refTable->AddColumn(yArr);

    // 添加线型图
    vtkPlot *plot = m_histogramChart->AddPlot(vtkChart::LINE);
    if (plot) {
        plot->SetInputData(refTable, 0, 1);
        if (plot->GetPen()) {
            plot->GetPen()->SetColorF(color.redF(), color.greenF(), color.blueF(), 1.0);
            plot->GetPen()->SetWidth(width);
            plot->GetPen()->SetLineType(vtkPen::DASH_LINE);
        }
        plot->SetSelectable(false);
        // 设置图例标签，为空时不在图例中显示
        if (!label.isEmpty()) {
            plot->SetLabel(label.toUtf8().constData());
        } else {
            plot->SetLegendVisibility(false);
        }
    }
    render();
}

// ==================== 渲染 ====================

void vtkPlot2D::render()
{
    if (m_vtkWidget && m_vtkWidget->renderWindow()) {
        m_vtkWidget->renderWindow()->Render();
    }
}

// ==================== 散点图操作 ====================

vtkScatterSeries* vtkPlot2D::addScatterSeries(const QVector<QPointF> &points,
                                               const QColor &color,
                                               Marker2DStyle style,
                                               double size,
                                               const QString &name)
{
    // 首次调用时创建共享图表并配置样式
    if (!m_scatterChart) {
        m_scatterChart = vtkSmartPointer<vtkChartXY>::New();
        m_contextView->GetScene()->AddItem(m_scatterChart);

        // 配置坐标轴字体（白色 + 中文字体）
        for (int i = 0; i < 4; ++i) {
            vtkAxis *axis = m_scatterChart->GetAxis(i);
            if (axis) {
                axis->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
                axis->GetTitleProperties()->SetFontSize(14);
                axis->GetTitleProperties()->SetBold(1);
                vtkTextProperty *tp = axis->GetTitleProperties();
                tp->SetFontFamily(VTK_FONT_FILE);
                tp->SetFontFile("C:/Windows/Fonts/msyh.ttc");
                axis->GetLabelProperties()->SetColor(1.0, 1.0, 1.0);
                axis->GetLabelProperties()->SetFontSize(12);
                vtkTextProperty *lp = axis->GetLabelProperties();
                lp->SetFontFamily(VTK_FONT_FILE);
                lp->SetFontFile("C:/Windows/Fonts/msyh.ttc");
            }
        }

        // 默认轴标题
        m_scatterChart->GetAxis(vtkAxis::BOTTOM)->SetTitle("X");
        m_scatterChart->GetAxis(vtkAxis::LEFT)->SetTitle("Y");

        // 默认图表标题
        m_scatterChart->SetTitle("Scatter Plot");
        m_scatterChart->GetTitleProperties()->SetColor(1.0, 1.0, 1.0);
        m_scatterChart->GetTitleProperties()->SetFontSize(18);
        m_scatterChart->GetTitleProperties()->SetBold(1);
        vtkTextProperty *ttp = m_scatterChart->GetTitleProperties();
        ttp->SetFontFamily(VTK_FONT_FILE);
        ttp->SetFontFile("C:/Windows/Fonts/msyh.ttc");

        // 显示图例
        m_scatterChart->SetShowLegend(true);
        if (m_scatterChart->GetLegend()) {
            vtkTextProperty *legendProp = m_scatterChart->GetLegend()->GetLabelProperties();
            legendProp->SetFontFamily(VTK_FONT_FILE);
            legendProp->SetFontFile("C:/Windows/Fonts/msyh.ttc");
        }
    }

    vtkScatterSeries *series = new vtkScatterSeries(m_scatterChart, points, color, style, size, name);
    series->addToView(m_contextView);
    m_scatterSeries.append(series);
    render();
    return series;
}

vtkLineSeries* vtkPlot2D::addLineSeries(const QVector<QPointF> &points,
                                          const QColor &color,
                                          double width,
                                          const QString &name)
{
    // 确保共享图表已创建（复用散点图表创建逻辑）
    if (!m_scatterChart) {
        addScatterSeries({}, Qt::transparent, Marker2DStyle::None, 0.0, "");
    }

    vtkLineSeries *series = new vtkLineSeries(m_scatterChart, points, color, width, name);
    series->addToView(m_contextView);
    m_lineSeries.append(series);
    render();
    return series;
}

void vtkPlot2D::clearAllScatterSeries()
{
    for (auto *series : m_scatterSeries) {
        delete series;
    }
    m_scatterSeries.clear();

    // 若线条也清空，则移除共享图表
    if (m_lineSeries.isEmpty() && m_scatterChart) {
        m_contextView->GetScene()->RemoveItem(m_scatterChart);
        m_scatterChart = nullptr;
    }
    render();
}

void vtkPlot2D::clearAllLineSeries()
{
    for (auto *series : m_lineSeries) {
        delete series;
    }
    m_lineSeries.clear();

    // 若散点也清空，则移除共享图表
    if (m_scatterSeries.isEmpty() && m_scatterChart) {
        m_contextView->GetScene()->RemoveItem(m_scatterChart);
        m_scatterChart = nullptr;
    }
    render();
}
