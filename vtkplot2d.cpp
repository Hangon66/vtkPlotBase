#include "vtkplot2d.h"
#include "ui_vtkplot2d.h"

// drawable 类
#include "drawable/vtkheatmap2d.h"

// VTK Qt 头文件
#include <QVTKOpenGLNativeWidget.h>
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

// ==================== 构造函数与析构函数 ====================

vtkPlot2D::vtkPlot2D(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::vtkPlot2D)
    , m_vtkWidget(nullptr)
{
    ui->setupUi(this);
    setupVTK();
}

vtkPlot2D::~vtkPlot2D()
{
    clearAll();
    delete ui;
}

// ==================== 事件处理 ====================

void vtkPlot2D::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    render();
}

void vtkPlot2D::wheelEvent(QWheelEvent *event)
{
    // 接受事件，阻止向上传播到 QScrollArea
    event->accept();
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
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建 VTK Qt 控件
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(m_vtkWidget);

    // 创建上下文视图
    m_contextView = vtkSmartPointer<vtkContextView>::New();
    // 将 vtkWidget 的渲染窗口设置给上下文视图，避免创建多余窗口
    m_contextView->SetRenderWindow(m_vtkWidget->renderWindow());

    // 设置默认背景
    m_contextView->GetRenderer()->SetBackground(0.2, 0.2, 0.2);
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
