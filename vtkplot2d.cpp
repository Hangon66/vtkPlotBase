#include "vtkplot2d.h"
#include "ui_vtkplot2d.h"

// drawable 类
#include "drawable/vtkheatmap2d.h"
#include "drawable/vtkmarkergroup2d.h"

// VTK Qt 头文件
#include <QVTKOpenGLNativeWidget.h>
#include <QSurfaceFormat>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkContextView.h>
#include <vtkChartHistogram2D.h>
#include <vtkAxis.h>
#include <vtkColorLegend.h>
#include <vtkBrush.h>
#include <vtkNamedColors.h>
#include <vtkTextProperty.h>

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

    if (!m_overlayWindow->isVisible())
        m_overlayWindow->show();

    // 确保独立窗口在主窗口之上（处理 z-order 变化，点击主窗口时 overlay 不会被遮挡）
    if (topLevel && topLevel->isActiveWindow()) {
        m_overlayWindow->raise();
    }

    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    QSize widgetSize = size();

    // 仅在控件尺寸真正变化时才调整 VTK 控件
    static QSize lastVtkSize;
    if (widgetSize != lastVtkSize) {
        m_vtkWidget->setGeometry(0, 0, widgetSize.width(), widgetSize.height());
        lastVtkSize = widgetSize;
    }

    // 处理 QScrollArea 场景：裁剪到可视区域
    QScrollArea *scrollArea = nullptr;
    QWidget *p = parentWidget();
    while (p) {
        scrollArea = qobject_cast<QScrollArea*>(p);
        if (scrollArea) break;
        p = p->parentWidget();
    }

    if (scrollArea && scrollArea->viewport()) {
        QWidget *viewport = scrollArea->viewport();
        QPoint vpGlobal = viewport->mapToGlobal(QPoint(0, 0));
        QRect vpRect(vpGlobal, viewport->size());
        QRect widgetRect(globalPos, widgetSize);
        QRect visible = widgetRect.intersected(vpRect);

        if (visible.isEmpty()) {
            m_overlayWindow->hide();
            return;
        }

        // overlay 调整到可见区域大小，VTK 控件保持完整尺寸
        // 无布局管理器，overlay resize 不会触发 VTK 控件 resize
        m_overlayWindow->move(visible.topLeft());
        m_overlayWindow->resize(visible.size());
    } else {
        m_overlayWindow->move(globalPos);
        m_overlayWindow->resize(widgetSize);
    }
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
}

// ==================== 渲染 ====================

void vtkPlot2D::render()
{
    if (m_vtkWidget && m_vtkWidget->renderWindow()) {
        m_vtkWidget->renderWindow()->Render();
    }
}
