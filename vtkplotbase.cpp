#include "vtkplotbase.h"
#include "ui_vtkplotbase.h"

// 新的类头文件
#include "drawable/vtkcurve.h"
#include "drawable/vtkmarker.h"
#include "drawable/vtksurface.h"
#include "drawable/vtkheatmap.h"

// VTK Qt 头文件
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkCommand.h>
#include <vtkPolyLine.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkDiskSource.h>
#include <vtkRegularPolygonSource.h>
#include <vtkLegendBoxActor.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkTextActor.h>
#include <vtkPointPicker.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <limits>
#include <algorithm>
#include <cstring>
#include <string>
#include <QDebug>
#include <QtGlobal>
#include <QShowEvent>
#include <QWheelEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QScrollArea>

// ==================== 自定义交互器样式 ====================

vtkStandardNewMacro(vtkPlotBaseInteractorStyle);

void vtkPlotBaseInteractorStyle::OnKeyPress()
{
    vtkRenderWindowInteractor* interactor = this->Interactor;
    if (!interactor) return;
    
    const char* keySym = interactor->GetKeySym();
    if (!keySym) return;
    
    if (strcmp(keySym, "r") == 0 || strcmp(keySym, "R") == 0) {
        if (m_plotBase) {
            m_plotBase->resetView();
            // qDebug() << "R key pressed: reset view";
        }
    } else if (strcmp(keySym, "x") == 0 || strcmp(keySym, "X") == 0) {
        if (m_plotBase) {
            m_plotBase->setViewSide();
            // qDebug() << "X key pressed: side view";
        }
    } else if (strcmp(keySym, "y") == 0 || strcmp(keySym, "Y") == 0) {
        if (m_plotBase) {
            m_plotBase->setViewTop();
            // qDebug() << "Y key pressed: top view";
        }
    } else if (strcmp(keySym, "z") == 0 || strcmp(keySym, "Z") == 0) {
        if (m_plotBase) {
            m_plotBase->setViewFront();
            // qDebug() << "Z key pressed: front view";
        }
    } else {
        this->vtkInteractorStyleTrackballCamera::OnKeyPress();
    }
}

void vtkPlotBaseInteractorStyle::OnMouseMove()
{
    // 先调用基类处理正常的鼠标交互（旋转、缩放等）
    vtkInteractorStyleTrackballCamera::OnMouseMove();

    // 悬浮数据显示
    if (m_plotBase && m_plotBase->isHoverDisplayEnabled() && m_plotBase->m_hoverTextActor && m_plotBase->m_pointPicker) {
        vtkRenderWindowInteractor* interactor = this->Interactor;
        if (!interactor) return;

        int* eventPos = interactor->GetEventPosition();
        int* winSize = interactor->GetSize();
        if (!winSize || winSize[0] <= 0 || winSize[1] <= 0) return;

        // 使用点拾取器拾取鼠标位置下方的最近数据点
        m_plotBase->m_pointPicker->Pick(eventPos[0], eventPos[1], 0, m_plotBase->renderer());

        vtkIdType pointId = m_plotBase->m_pointPicker->GetPointId();
        if (pointId >= 0) {
            vtkDataSet* dataSet = m_plotBase->m_pointPicker->GetDataSet();
            if (!dataSet) {
                m_plotBase->m_hoverTextActor->SetVisibility(0);
                m_plotBase->render();
                return;
            }

            // 获取拾取点的世界坐标
            double pickedPos[3];
            dataSet->GetPoint(pointId, pickedPos);

            // 构建显示文本
            QString text = QString("X: %1  Y: %2  Z: %3")
                .arg(pickedPos[0], 0, 'g', 6)
                .arg(pickedPos[1], 0, 'g', 6)
                .arg(pickedPos[2], 0, 'g', 6);

            // 如果点数据中包含标量值（如热力图的颜色映射值），追加显示
            vtkDataArray* scalars = dataSet->GetPointData()->GetScalars();
            if (scalars) {
                double scalarVal = scalars->GetTuple1(pointId);
                text += QString("\nValue: %1").arg(scalarVal, 0, 'g', 6);
            }

            m_plotBase->m_hoverTextActor->SetInput(text.toUtf8().constData());

            // 将鼠标显示坐标转换为归一化视口坐标，并偏移显示文本位置
            double viewX = static_cast<double>(eventPos[0]) / winSize[0];
            double viewY = static_cast<double>(eventPos[1]) / winSize[1];
            double textX = viewX + 0.02;
            double textY = viewY + 0.02;
            // 防止文本超出视口边界
            if (textX > 0.80) textX = viewX - 0.20;
            if (textY > 0.93) textY = viewY - 0.08;

            m_plotBase->m_hoverTextActor->GetPositionCoordinate()->SetValue(textX, textY);
            m_plotBase->m_hoverTextActor->SetVisibility(1);
        } else {
            // 未拾取到任何点，隐藏文本
            m_plotBase->m_hoverTextActor->SetVisibility(0);
        }
        m_plotBase->render();
    }
}

// ==================== 构造函数与析构函数 ====================

vtkPlotBase::vtkPlotBase(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::vtkPlotBase)
    , m_vtkWidget(nullptr)
    , m_xMin(-1.0), m_xMax(1.0)
    , m_yMin(-1.0), m_yMax(1.0)
    , m_zMin(-1.0), m_zMax(1.0)
    , m_autoScaleMode(AutoScaleMode::EqualRatio)
    , m_autoScaleMargin(0.1)
    , m_labelXMin(0.0), m_labelXMax(1.0)
    , m_labelYMin(0.0), m_labelYMax(1.0)
    , m_labelZMin(0.0), m_labelZMax(1.0)
    , m_labelRangesValid(false)
    , m_legendVisible(true)
    , m_legendPosition(LegendPosition::TopRight)
    , m_titleVisible(false)
    , m_cameraInitialized(false)
    , m_autoColorIndex(0)
    , m_hoverDisplayEnabled(false)
    , m_hoverTolerance(0.02)
{
    ui->setupUi(this);
    setupVTK();
}

vtkPlotBase::~vtkPlotBase()
{
    // 停止同步定时器，避免在析构过程中访问已释放对象
    if (m_syncTimer)
        m_syncTimer->stop();
    // 清除所有对象
    clearAll();
    // overlay 无父对象，需显式删除
    delete m_overlayWindow;
    delete ui;
}

// 窗口显示事件
void vtkPlotBase::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 首次显示时安装事件过滤器到父级 QScrollArea，捕获滚动事件即时同步
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

    updateAllScreenMarkerScales();
    render();
}

void vtkPlotBase::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    syncWindow();
}

void vtkPlotBase::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    syncWindow();
}

bool vtkPlotBase::eventFilter(QObject *watched, QEvent *event)
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

// ==================== 顶层窗口同步 ====================

void vtkPlotBase::syncWindow()
{
    if (!m_overlayWindow) return;

    // 检查顶层窗口是否最小化
    QWidget *topLevel = window();
    if (topLevel && topLevel->isMinimized()) {
        m_overlayWindow->hide();
        return;
    }

    // 不可见时隐藏 overlay
    if (!isVisible() || !m_vtkWidget || size().isEmpty()) {
        m_overlayWindow->hide();
        return;
    }

    // 获取自身在屏幕上的全局位置
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    QSize widgetSize = size();

    // 仅在控件尺寸真正变化时才调整 VTK 控件（避免滚动时反复重建 OpenGL 帧缓冲区）
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

// 重写滚轮事件：阻止事件传播到父级滚动区域，同时让 VTK 正常处理缩放
void vtkPlotBase::wheelEvent(QWheelEvent *event)
{
    // 接受事件，阻止向上传播到 QScrollArea
    event->accept();
}

// ==================== 初始化方法 ====================

void vtkPlotBase::setupVTK()
{
    // 创建 VTK 控件
    m_vtkWidget = new QVTKOpenGLNativeWidget();
    m_vtkWidget->setFormat(QVTKOpenGLNativeWidget::defaultFormat());

    // 创建渲染窗口并设置给控件（必须使用 vtkGenericOpenGLRenderWindow）
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
        vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_vtkWidget->setRenderWindow(renderWindow);

    // 创建独立顶层窗口承载 VTK 控件，避免 QOpenGLWidget 强制父窗口 RHI 切换为 OpenGL
    // Qt::Window 使其成为独立顶层窗口，不参与主窗口的 widget 合成
    // Qt::FramelessWindowHint 去掉标题栏，视觉上与嵌入一致
    m_overlayWindow = new QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint);
    // 不使用布局管理器，避免 overlay resize 时连锁触发 VTK 控件 resize
    m_vtkWidget->setParent(m_overlayWindow);

    // 创建布局（自身无 VTK 控件，仅占位）
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 位置同步定时器：跟踪父窗口移动、QStackedWidget 页面切换、ScrollArea 滚动等
    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(16); // 16ms ≈ 60fps，高同步率
    connect(m_syncTimer, &QTimer::timeout, this, &vtkPlotBase::syncWindow);
    m_syncTimer->start();

    // 创建渲染器
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.1, 0.1, 0.15);
    m_vtkWidget->renderWindow()->AddRenderer(m_renderer);

    // 创建坐标轴
    createAxes();
    
    // 创建图例
    createLegend();
    
    // 创建颜色条
    createScalarBar();

    // 创建标题
    createTitle();

    // 创建悬浮显示组件
    createHoverDisplay();

    // 设置相机
    m_renderer->ResetCamera();
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->Azimuth(30);
    camera->Elevation(30);
    
    // 保存默认相机参数
    saveDefaultCamera();

    // 设置相机回调
    setupCameraCallback();

    // 设置自定义交互器样式
    vtkSmartPointer<vtkPlotBaseInteractorStyle> style =
        vtkSmartPointer<vtkPlotBaseInteractorStyle>::New();
    style->SetPlotBase(this);
    renderWindow->GetInteractor()->SetInteractorStyle(style);

    // 按需渲染：交互时限制30fps降低GPU负载，空闲时不渲染
    renderWindow->GetInteractor()->SetDesiredUpdateRate(30); // 交互时上限30fps
    renderWindow->GetInteractor()->SetStillUpdateRate(0);    // 静止时不渲染
}

void vtkPlotBase::saveDefaultCamera()
{
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->GetPosition(m_defaultCameraPosition);
    camera->GetFocalPoint(m_defaultCameraFocalPoint);
    camera->GetViewUp(m_defaultCameraViewUp);
    m_defaultCameraDistance = camera->GetDistance();
}

void vtkPlotBase::createAxes()
{
    m_cubeAxesActor = vtkSmartPointer<vtkCubeAxesActor>::New();
    m_cubeAxesActor->SetCamera(m_renderer->GetActiveCamera());
    m_cubeAxesActor->SetXAxisRange(m_xMin, m_xMax);
    m_cubeAxesActor->SetYAxisRange(m_yMin, m_yMax);
    m_cubeAxesActor->SetZAxisRange(m_zMin, m_zMax);
    m_cubeAxesActor->SetXTitle("X");
    m_cubeAxesActor->SetYTitle("Y");
    m_cubeAxesActor->SetZTitle("Z");
    m_cubeAxesActor->GetXAxesLinesProperty()->SetLineWidth(1.0);
    m_cubeAxesActor->GetYAxesLinesProperty()->SetLineWidth(1.0);
    m_cubeAxesActor->GetZAxesLinesProperty()->SetLineWidth(1.0);
    m_cubeAxesActor->SetScreenSize(8.0);
    m_cubeAxesActor->SetLabelOffset(10.0);
    m_cubeAxesActor->SetVisibility(1);
    m_cubeAxesActor->SetFlyModeToOuterEdges();
    m_cubeAxesActor->DrawXGridlinesOn();
    m_cubeAxesActor->DrawYGridlinesOn();
    m_cubeAxesActor->DrawZGridlinesOn();
    m_cubeAxesActor->SetDrawXInnerGridlines(false);
    m_cubeAxesActor->SetDrawYInnerGridlines(false);
    m_cubeAxesActor->SetDrawZInnerGridlines(false);
    m_cubeAxesActor->GetXAxesGridlinesProperty()->SetColor(0.3, 0.3, 0.3);
    m_cubeAxesActor->GetYAxesGridlinesProperty()->SetColor(0.3, 0.3, 0.3);
    m_cubeAxesActor->GetZAxesGridlinesProperty()->SetColor(0.3, 0.3, 0.3);
    m_cubeAxesActor->SetGridLineLocation(2);
    m_cubeAxesActor->XAxisMinorTickVisibilityOff();
    m_cubeAxesActor->YAxisMinorTickVisibilityOff();
    m_cubeAxesActor->ZAxisMinorTickVisibilityOff();
    m_cubeAxesActor->SetLabelScaling(false, 0, 0, 0);
    m_cubeAxesActor->SetTickLocationToOutside();
    m_renderer->AddActor(m_cubeAxesActor);
}

void vtkPlotBase::createLegend()
{
    m_legendActor = vtkSmartPointer<vtkLegendBoxActor>::New();
    m_legendActor->SetNumberOfEntries(0);
    m_legendActor->UseBackgroundOn();
    m_legendActor->SetBackgroundColor(0.2, 0.2, 0.25);
    m_legendActor->GetPositionCoordinate()->SetValue(0.85, 0.85);
    m_legendActor->GetPosition2Coordinate()->SetValue(0.12, 0.12);
    m_legendActor->GetEntryTextProperty()->SetColor(1, 1, 1);
    m_legendActor->GetEntryTextProperty()->SetFontSize(6);
    m_legendActor->SetVisibility(m_legendVisible ? 1 : 0);
    m_renderer->AddActor(m_legendActor);
}

void vtkPlotBase::createScalarBar()
{
    m_scalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
    m_scalarBarActor->SetNumberOfLabels(5);
    m_scalarBarActor->SetOrientationToVertical();
    m_scalarBarActor->SetWidth(0.08);
    m_scalarBarActor->SetHeight(0.45);
    m_scalarBarActor->SetPosition(0.90, 0.30);
    m_scalarBarActor->GetTitleTextProperty()->SetColor(1, 1, 1);
    m_scalarBarActor->GetTitleTextProperty()->SetFontSize(12);
    m_scalarBarActor->GetLabelTextProperty()->SetColor(1, 1, 1);
    m_scalarBarActor->GetLabelTextProperty()->SetFontSize(10);
    m_scalarBarActor->SetTextPositionToSucceedScalarBar();
    m_scalarBarActor->SetVerticalTitleSeparation(10);
    m_scalarBarActor->SetTextPad(2);
    m_scalarBarActor->SetVisibility(0);  // 默认隐藏
    m_renderer->AddViewProp(m_scalarBarActor);
}

void vtkPlotBase::createTitle()
{
    m_titleActor = vtkSmartPointer<vtkTextActor>::New();
    m_titleActor->SetInput("");
    
    // 设置文本属性
    vtkTextProperty* textProp = m_titleActor->GetTextProperty();
    textProp->SetFontFamily(VTK_FONT_FILE);
    textProp->SetFontFile("C:/Windows/Fonts/msyh.ttc");  // 微软雅黑
    textProp->SetFontSize(18);
    textProp->SetColor(1, 1, 1);  // 白色
    textProp->SetJustificationToCentered();        // 水平居中
    textProp->SetVerticalJustificationToTop();     // 垂直顶部对齐
    textProp->SetOpacity(1.0);
    
    // 设置位置（使用视口坐标）
    // vtkTextActor 使用 SetPosition 设置的是显示坐标（像素）
    // 需要使用 SetPositionCoordinate 来设置视口坐标
    m_titleActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    m_titleActor->GetPositionCoordinate()->SetValue(0.5, 0.96);
    
    m_titleActor->SetVisibility(0);  // 默认隐藏
    m_renderer->AddViewProp(m_titleActor);
}

void vtkPlotBase::createHoverDisplay()
{
    // 创建悬浮信息文本演员
    m_hoverTextActor = vtkSmartPointer<vtkTextActor>::New();
    m_hoverTextActor->SetInput("");

    vtkTextProperty* textProp = m_hoverTextActor->GetTextProperty();
    textProp->SetFontSize(12);
    textProp->SetColor(1.0, 1.0, 1.0);
    textProp->SetBackgroundColor(0.05, 0.05, 0.1);
    textProp->SetBackgroundOpacity(0.75);
    textProp->SetOpacity(1.0);

    m_hoverTextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    m_hoverTextActor->GetPositionCoordinate()->SetValue(0.0, 0.0);
    m_hoverTextActor->SetVisibility(0);
    m_renderer->AddViewProp(m_hoverTextActor);

    // 创建点拾取器
    m_pointPicker = vtkSmartPointer<vtkPointPicker>::New();
    m_pointPicker->SetTolerance(m_hoverTolerance);
}

void vtkPlotBase::setTitle(const QString &title)
{
    m_titleActor->SetInput(title.toUtf8().constData());
    m_titleActor->SetVisibility(1);
    m_titleVisible = true;
    render();
}

void vtkPlotBase::setTitleVisible(bool visible)
{
    m_titleVisible = visible;
    m_titleActor->SetVisibility(visible ? 1 : 0);
    render();
}

void vtkPlotBase::setTitleColor(const QColor &color)
{
    m_titleActor->GetTextProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    render();
}

void vtkPlotBase::setTitleFontSize(int size)
{
    m_titleActor->GetTextProperty()->SetFontSize(size);
    render();
}

// ==================== 悬浮数据显示 ====================

void vtkPlotBase::setHoverDisplayEnabled(bool enabled)
{
    m_hoverDisplayEnabled = enabled;
    if (m_hoverTextActor) {
        if (!enabled) {
            m_hoverTextActor->SetVisibility(0);
        }
    }
    render();
}

bool vtkPlotBase::isHoverDisplayEnabled() const
{
    return m_hoverDisplayEnabled;
}

void vtkPlotBase::setHoverTolerance(double tolerance)
{
    m_hoverTolerance = tolerance;
    if (m_pointPicker) {
        m_pointPicker->SetTolerance(tolerance);
    }
}

void vtkPlotBase::updateLegend()
{
    int entryCount = 0;
    
    // 统计可见条目数量
    for (auto curve : m_curves) {
        if (curve->visible() && !curve->name().isEmpty()) {
            entryCount++;
        }
    }
    for (auto marker : m_markers) {
        if (marker->visible() && !marker->name().isEmpty()) {
            entryCount++;
        }
    }
    for (auto surface : m_surfaces) {
        if (surface->visible() && !surface->name().isEmpty()) {
            entryCount++;
        }
    }
    
    if (entryCount == 0) {
        m_legendActor->SetNumberOfEntries(0);
        render();
        return;
    }
    
    m_legendActor->SetNumberOfEntries(entryCount);
    
    int entry = 0;
    
    // 创建曲线符号
    vtkSmartPointer<vtkPoints> linePts = vtkSmartPointer<vtkPoints>::New();
    linePts->InsertNextPoint(0, 0, 0);
    linePts->InsertNextPoint(2.0, 0, 0);
    linePts->InsertNextPoint(2.3, 0, 0);
    
    vtkSmartPointer<vtkPolyLine> lineCell = vtkSmartPointer<vtkPolyLine>::New();
    lineCell->GetPointIds()->SetNumberOfIds(2);
    lineCell->GetPointIds()->SetId(0, 0);
    lineCell->GetPointIds()->SetId(1, 1);
    
    vtkSmartPointer<vtkCellArray> lineCells = vtkSmartPointer<vtkCellArray>::New();
    lineCells->InsertNextCell(lineCell);
    
    vtkSmartPointer<vtkPolyData> lineSymbol = vtkSmartPointer<vtkPolyData>::New();
    lineSymbol->SetPoints(linePts);
    lineSymbol->SetLines(lineCells);
    
    // 添加曲线到图例
    for (auto curve : m_curves) {
        if (curve->visible() && !curve->name().isEmpty()) {
            double color[3];
            curve->actor()->GetProperty()->GetColor(color);
            m_legendActor->SetEntry(entry++, lineSymbol, curve->name().toUtf8().constData(), color);
        }
    }
    
    // 创建填充圆符号
    vtkSmartPointer<vtkRegularPolygonSource> filledCircleSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    filledCircleSource->SetNumberOfSides(16);
    filledCircleSource->SetRadius(0.25);
    filledCircleSource->SetCenter(0.25, 0, 0);
    filledCircleSource->Update();
    
    vtkSmartPointer<vtkPolyData> filledCircleSymbol = vtkSmartPointer<vtkPolyData>::New();
    filledCircleSymbol->DeepCopy(filledCircleSource->GetOutput());
    filledCircleSymbol->GetPoints()->InsertNextPoint(0.8, 0, 0);
    
    // 创建空心环符号
    vtkSmartPointer<vtkDiskSource> hollowRingSource = vtkSmartPointer<vtkDiskSource>::New();
    hollowRingSource->SetInnerRadius(0.15);
    hollowRingSource->SetOuterRadius(0.25);
    hollowRingSource->SetCircumferentialResolution(16);
    hollowRingSource->Update();
    
    vtkSmartPointer<vtkTransform> ringTransform = vtkSmartPointer<vtkTransform>::New();
    ringTransform->Translate(0.25, 0, 0);
    vtkSmartPointer<vtkTransformPolyDataFilter> ringTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    ringTransformFilter->SetInputConnection(hollowRingSource->GetOutputPort());
    ringTransformFilter->SetTransform(ringTransform);
    ringTransformFilter->Update();
    
    vtkSmartPointer<vtkPolyData> hollowRingSymbol = vtkSmartPointer<vtkPolyData>::New();
    hollowRingSymbol->DeepCopy(ringTransformFilter->GetOutput());
    hollowRingSymbol->GetPoints()->InsertNextPoint(0.8, 0, 0);
    
    // 添加标记到图例
    for (auto marker : m_markers) {
        if (marker->visible() && !marker->name().isEmpty()) {
            double color[3];
            marker->follower()->GetProperty()->GetColor(color);
            
            if (marker->isFilled()) {
                m_legendActor->SetEntry(entry++, filledCircleSymbol, marker->name().toUtf8().constData(), color);
            } else {
                m_legendActor->SetEntry(entry++, hollowRingSymbol, marker->name().toUtf8().constData(), color);
            }
        }
    }
    
    // 创建曲面符号
    vtkSmartPointer<vtkRegularPolygonSource> squareSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    squareSource->SetNumberOfSides(4);
    squareSource->SetRadius(0.25);
    squareSource->SetCenter(0.25, 0, 0);
    squareSource->Update();
    
    vtkSmartPointer<vtkPolyData> squareSymbol = vtkSmartPointer<vtkPolyData>::New();
    squareSymbol->DeepCopy(squareSource->GetOutput());
    squareSymbol->GetPoints()->InsertNextPoint(0.8, 0, 0);
    
    // 添加曲面到图例
    for (auto surface : m_surfaces) {
        if (surface->visible() && !surface->name().isEmpty()) {
            double color[3];
            surface->actor()->GetProperty()->GetColor(color);
            m_legendActor->SetEntry(entry++, squareSymbol, surface->name().toUtf8().constData(), color);
        }
    }
    
    // 动态调整图例高度
    double legendWidth = 0.12;
    double legendHeight = qBound(0.04, 0.03 + entryCount * 0.035, 0.30);
    m_legendActor->GetPosition2Coordinate()->SetValue(legendWidth, legendHeight);
    
    double posX = 1.0 - legendWidth;
    double posY = 1.0 - legendHeight;
    m_legendActor->GetPositionCoordinate()->SetValue(posX, posY);
    
    render();
}

void vtkPlotBase::updateLegendPosition()
{
    double *pos2 = m_legendActor->GetPosition2Coordinate()->GetValue();
    double legendWidth = pos2[0];
    double legendHeight = pos2[1];
    
    double margin = 0.0;
    
    switch (m_legendPosition) {
        case LegendPosition::TopLeft:
            m_legendActor->GetPositionCoordinate()->SetValue(margin, 1.0 - legendHeight - margin);
            break;
        case LegendPosition::TopRight:
            m_legendActor->GetPositionCoordinate()->SetValue(1.0 - legendWidth - margin, 1.0 - legendHeight - margin);
            break;
        case LegendPosition::BottomLeft:
            m_legendActor->GetPositionCoordinate()->SetValue(margin, margin);
            break;
        case LegendPosition::BottomRight:
            m_legendActor->GetPositionCoordinate()->SetValue(1.0 - legendWidth - margin, margin);
            break;
    }
}

QColor vtkPlotBase::getNextAutoColor()
{
    static const QColor colorPalette[] = {
        QColor(31, 119, 180),
        QColor(255, 127, 14),
        QColor(44, 160, 44),
        QColor(214, 39, 40),
        QColor(148, 103, 189),
        QColor(140, 86, 75),
        QColor(227, 119, 194),
        QColor(127, 127, 127),
        QColor(188, 189, 34),
        QColor(23, 190, 187),
    };
    static const int paletteSize = sizeof(colorPalette) / sizeof(colorPalette[0]);
    
    QColor color = colorPalette[m_autoColorIndex % paletteSize];
    m_autoColorIndex++;
    return color;
}

void vtkPlotBase::updateAxesBounds()
{
    m_cubeAxesActor->SetBounds(m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax);
    m_cubeAxesActor->SetXAxisRange(m_xMin, m_xMax);
    m_cubeAxesActor->SetYAxisRange(m_yMin, m_yMax);
    m_cubeAxesActor->SetZAxisRange(m_zMin, m_zMax);

    // StretchFill 模式下，坐标轴刻度标签显示原始数据范围
    if (m_autoScaleMode == AutoScaleMode::StretchFill && m_labelRangesValid) {
        m_cubeAxesActor->SetXAxisRange(m_labelXMin, m_labelXMax);
        m_cubeAxesActor->SetYAxisRange(m_labelYMin, m_labelYMax);
        m_cubeAxesActor->SetZAxisRange(m_labelZMin, m_labelZMax);
    }

    updateAllMarkerScales();
    m_renderer->ResetCamera();
    render();
}

void vtkPlotBase::computeDataBounds(double &xMin, double &xMax, double &yMin, double &yMax, double &zMin, double &zMax)
{
    xMin = std::numeric_limits<double>::max();
    xMax = std::numeric_limits<double>::lowest();
    yMin = std::numeric_limits<double>::max();
    yMax = std::numeric_limits<double>::lowest();
    zMin = std::numeric_limits<double>::max();
    zMax = std::numeric_limits<double>::lowest();
    
    bool hasData = false;
    
    // 从曲线计算边界
    for (auto curve : m_curves) {
        if (!curve->visible()) continue;
        const auto points = curve->points();
        for (const auto &pt : points) {
            xMin = std::min(xMin, static_cast<double>(pt.x()));
            xMax = std::max(xMax, static_cast<double>(pt.x()));
            yMin = std::min(yMin, static_cast<double>(pt.y()));
            yMax = std::max(yMax, static_cast<double>(pt.y()));
            zMin = std::min(zMin, static_cast<double>(pt.z()));
            zMax = std::max(zMax, static_cast<double>(pt.z()));
            hasData = true;
        }
    }
    
    // 从标记计算边界
    for (auto marker : m_markers) {
        if (!marker->visible()) continue;
        auto pos = marker->position();
        double r = marker->radius();
        xMin = std::min(xMin, static_cast<double>(pos.x()) - r);
        xMax = std::max(xMax, static_cast<double>(pos.x()) + r);
        yMin = std::min(yMin, static_cast<double>(pos.y()) - r);
        yMax = std::max(yMax, static_cast<double>(pos.y()) + r);
        zMin = std::min(zMin, static_cast<double>(pos.z()) - r);
        zMax = std::max(zMax, static_cast<double>(pos.z()) + r);
        hasData = true;
    }
    
    // 从曲面计算边界
    for (auto surface : m_surfaces) {
        if (!surface->visible()) continue;
        auto polyData = surface->polyData();
        if (!polyData || !polyData->GetPoints()) continue;
        vtkPoints *pts = polyData->GetPoints();
        for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i) {
            double pt[3];
            pts->GetPoint(i, pt);
            xMin = std::min(xMin, pt[0]);
            xMax = std::max(xMax, pt[0]);
            yMin = std::min(yMin, pt[1]);
            yMax = std::max(yMax, pt[1]);
            zMin = std::min(zMin, pt[2]);
            zMax = std::max(zMax, pt[2]);
            hasData = true;
        }
    }
    
    // 从热力图计算边界
    for (auto heatmap : m_heatmapSurfaces) {
        if (!heatmap->visible()) continue;
        auto polyData = heatmap->polyData();
        if (!polyData || !polyData->GetPoints()) continue;
        vtkPoints *pts = polyData->GetPoints();
        for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i) {
            double pt[3];
            pts->GetPoint(i, pt);
            xMin = std::min(xMin, pt[0]);
            xMax = std::max(xMax, pt[0]);
            yMin = std::min(yMin, pt[1]);
            yMax = std::max(yMax, pt[1]);
            zMin = std::min(zMin, pt[2]);
            zMax = std::max(zMax, pt[2]);
            hasData = true;
        }
    }
    
    if (!hasData) {
        xMin = -1.0; xMax = 1.0;
        yMin = -1.0; yMax = 1.0;
        zMin = -1.0; zMax = 1.0;
    }
}

void vtkPlotBase::autoScaleIfNeeded()
{
    // 即使不需要自动缩放，也要初始化相机
    if (m_autoScaleMode == AutoScaleMode::None) {
        if (!m_cameraInitialized) {
            m_renderer->ResetCamera();
            saveDefaultCamera();
            m_cameraInitialized = true;
        }
        return;
    }
    
    double xMin, xMax, yMin, yMax, zMin, zMax;
    computeDataBounds(xMin, xMax, yMin, yMax, zMin, zMax);
    
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    double zRange = zMax - zMin;
    
    if (xRange < 1e-10) { xMin -= 1.0; xMax += 1.0; xRange = 2.0; }
    if (yRange < 1e-10) { yMin -= 1.0; yMax += 1.0; yRange = 2.0; }
    if (zRange < 1e-10) { zMin -= 1.0; zMax += 1.0; zRange = 2.0; }
    
    if (m_autoScaleMode == AutoScaleMode::EqualRatio) {
        double maxRange = std::max({xRange, yRange, zRange});
        double margin = maxRange * m_autoScaleMargin;
        
        double xCenter = (xMin + xMax) / 2.0;
        double yCenter = (yMin + yMax) / 2.0;
        double zCenter = (zMin + zMax) / 2.0;
        
        double halfRange = maxRange / 2.0 + margin;
        m_xMin = xCenter - halfRange;
        m_xMax = xCenter + halfRange;
        m_yMin = yCenter - halfRange;
        m_yMax = yCenter + halfRange;
        m_zMin = zCenter - halfRange;
        m_zMax = zCenter + halfRange;
    } else {
        m_xMin = xMin - xRange * m_autoScaleMargin;
        m_xMax = xMax + xRange * m_autoScaleMargin;
        m_yMin = yMin - yRange * m_autoScaleMargin;
        m_yMax = yMax + yRange * m_autoScaleMargin;
        m_zMin = zMin - zRange * m_autoScaleMargin;
        m_zMax = zMax + zRange * m_autoScaleMargin;
    }
    
    updateAxesBounds();
    
    // 首次添加数据后初始化相机位置
    if (!m_cameraInitialized) {
        m_renderer->ResetCamera();
        saveDefaultCamera();
        m_cameraInitialized = true;
    }
}

void vtkPlotBase::render()
{
    if (m_vtkWidget && m_vtkWidget->renderWindow()) {
        m_vtkWidget->renderWindow()->Render();
    }
}

void vtkPlotBase::updateAllMarkerScales()
{
    for (auto marker : m_markers) {
        if (marker->sizeMode() == MarkerSizeMode::Relative) {
            marker->setAxisRange(m_xMin, m_xMax);
        }
    }
}

void vtkPlotBase::updateAllScreenMarkerScales()
{
    int* windowSize = m_renderer->GetRenderWindow()->GetSize();
    int winHeight = windowSize ? windowSize[1] : 600;
    
    for (auto marker : m_markers) {
        if (marker->sizeMode() == MarkerSizeMode::Screen) {
            marker->updateScreenSize(winHeight);
        }
    }
}

// ==================== StretchFill 辅助方法 ====================

void vtkPlotBase::computePointsBounds(const QVector<QVector3D> &points,
                                       double &xMin, double &xMax,
                                       double &yMin, double &yMax,
                                       double &zMin, double &zMax)
{
    xMin = std::numeric_limits<double>::max();
    xMax = std::numeric_limits<double>::lowest();
    yMin = std::numeric_limits<double>::max();
    yMax = std::numeric_limits<double>::lowest();
    zMin = std::numeric_limits<double>::max();
    zMax = std::numeric_limits<double>::lowest();

    for (const auto &pt : points) {
        xMin = std::min(xMin, static_cast<double>(pt.x()));
        xMax = std::max(xMax, static_cast<double>(pt.x()));
        yMin = std::min(yMin, static_cast<double>(pt.y()));
        yMax = std::max(yMax, static_cast<double>(pt.y()));
        zMin = std::min(zMin, static_cast<double>(pt.z()));
        zMax = std::max(zMax, static_cast<double>(pt.z()));
    }
}

bool vtkPlotBase::applyStretchFillIfNeeded(QVector<QVector3D> &points)
{
    if (m_autoScaleMode != AutoScaleMode::StretchFill) return false;

    // 计算原始数据边界
    double origXMin, origXMax, origYMin, origYMax, origZMin, origZMax;
    computePointsBounds(points, origXMin, origXMax, origYMin, origYMax, origZMin, origZMax);

    // 防止除零
    double xRange = (origXMax - origXMin) > 1e-10 ? (origXMax - origXMin) : 1.0;
    double yRange = (origYMax - origYMin) > 1e-10 ? (origYMax - origYMin) : 1.0;
    double zRange = (origZMax - origZMin) > 1e-10 ? (origZMax - origZMin) : 1.0;

    // 归一化到 [0,1] 空间
    for (auto &pt : points) {
        pt.setX(static_cast<float>((pt.x() - origXMin) / xRange));
        pt.setY(static_cast<float>((pt.y() - origYMin) / yRange));
        pt.setZ(static_cast<float>((pt.z() - origZMin) / zRange));
    }

    // 更新标签范围（取并集）
    if (!m_labelRangesValid) {
        m_labelXMin = origXMin; m_labelXMax = origXMax;
        m_labelYMin = origYMin; m_labelYMax = origYMax;
        m_labelZMin = origZMin; m_labelZMax = origZMax;
        m_labelRangesValid = true;
    } else {
        m_labelXMin = std::min(m_labelXMin, origXMin); m_labelXMax = std::max(m_labelXMax, origXMax);
        m_labelYMin = std::min(m_labelYMin, origYMin); m_labelYMax = std::max(m_labelYMax, origYMax);
        m_labelZMin = std::min(m_labelZMin, origZMin); m_labelZMax = std::max(m_labelZMax, origZMax);
    }

    return true;
}

void vtkPlotBase::setupCameraCallback()
{
    if (m_cameraCallback) return;

    m_cameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    m_cameraCallback->SetCallback(cameraCallback);
    m_cameraCallback->SetClientData(this);

    m_renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, m_cameraCallback);
}

void vtkPlotBase::cameraCallback(vtkObject* caller, unsigned long eventId, void* clientData, void* callData)
{
    (void)caller;
    (void)eventId;
    (void)callData;

    vtkPlotBase* self = static_cast<vtkPlotBase*>(clientData);
    if (self) {
        self->updateAllScreenMarkerScales();
        self->render();
    }
}

void vtkPlotBase::updateScalarBar()
{
    if (!m_heatmapSurfaces.isEmpty() && m_scalarBarActor) {
        auto heatmap = m_heatmapSurfaces.first();
        if (heatmap && heatmap->lookupTable()) {
            m_scalarBarActor->SetLookupTable(heatmap->lookupTable());
            m_scalarBarActor->SetVisibility(1);
        }
    }
}

// ==================== 坐标系操作 ====================

void vtkPlotBase::setAxisRange(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax)
{
    m_xMin = xMin; m_xMax = xMax;
    m_yMin = yMin; m_yMax = yMax;
    m_zMin = zMin; m_zMax = zMax;
    updateAxesBounds();
}

void vtkPlotBase::setAxisTitles(const QString &xTitle, const QString &yTitle, const QString &zTitle)
{
    m_cubeAxesActor->SetXTitle(xTitle.toUtf8().constData());
    m_cubeAxesActor->SetYTitle(yTitle.toUtf8().constData());
    m_cubeAxesActor->SetZTitle(zTitle.toUtf8().constData());
    render();
}

void vtkPlotBase::setAxisLabelRange(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax)
{
    m_cubeAxesActor->SetXAxisRange(xMin, xMax);
    m_cubeAxesActor->SetYAxisRange(yMin, yMax);
    m_cubeAxesActor->SetZAxisRange(zMin, zMax);
    render();
}

void vtkPlotBase::setGridVisible(bool visible)
{
    if (visible) {
        m_cubeAxesActor->DrawXGridlinesOn();
        m_cubeAxesActor->DrawYGridlinesOn();
        m_cubeAxesActor->DrawZGridlinesOn();
    } else {
        m_cubeAxesActor->DrawXGridlinesOff();
        m_cubeAxesActor->DrawYGridlinesOff();
        m_cubeAxesActor->DrawZGridlinesOff();
    }
    render();
}

void vtkPlotBase::setBackground(const QColor &color)
{
    m_renderer->SetBackground(color.redF(), color.greenF(), color.blueF());
    render();
}

// 注意：必须在添加数据之前调用。
// StretchFill 模式在 addSurface/addCurve 等内部将几何体坐标归一化到 [0,1]，
// 若已有数据再切换模式，已存储的几何体不会被重新归一化。
void vtkPlotBase::setAutoScaleMode(AutoScaleMode mode)
{
    m_autoScaleMode = mode;
    autoScaleIfNeeded();
}

AutoScaleMode vtkPlotBase::autoScaleMode() const
{
    return m_autoScaleMode;
}

void vtkPlotBase::autoFit()
{
    autoScaleIfNeeded();
}

void vtkPlotBase::resetView()
{
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->SetPosition(m_defaultCameraPosition);
    camera->SetFocalPoint(m_defaultCameraFocalPoint);
    camera->SetViewUp(m_defaultCameraViewUp);
    camera->SetDistance(m_defaultCameraDistance);
    render();
}

void vtkPlotBase::resetAxisRange()
{
    autoScaleIfNeeded();
}

void vtkPlotBase::setViewFront()
{
    vtkCamera *camera = m_renderer->GetActiveCamera();
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    double distance = camera->GetDistance();
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0], focalPoint[1], focalPoint[2] + distance);
    camera->SetViewUp(0, 1, 0);
    render();
}

void vtkPlotBase::setViewTop()
{
    vtkCamera *camera = m_renderer->GetActiveCamera();
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    double distance = camera->GetDistance();
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0], focalPoint[1] + distance, focalPoint[2]);
    camera->SetViewUp(0, 0, -1);
    render();
}

void vtkPlotBase::setViewSide()
{
    vtkCamera *camera = m_renderer->GetActiveCamera();
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    double distance = camera->GetDistance();
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0] + distance, focalPoint[1], focalPoint[2]);
    camera->SetViewUp(0, 0, 1);
    render();
}

void vtkPlotBase::resetCamera()
{
    m_renderer->ResetCamera();
    render();
}

// ==================== 图例操作 ====================

void vtkPlotBase::setLegendVisible(bool visible)
{
    m_legendVisible = visible;
    m_legendActor->SetVisibility(visible ? 1 : 0);
    updateLegend();  // 无论显示还是隐藏都更新图例内容
    render();
}

void vtkPlotBase::setLegendPosition(LegendPosition pos)
{
    m_legendPosition = pos;
    updateLegendPosition();
    render();
}

// ==================== 曲线操作 ====================

vtkCurve* vtkPlotBase::addCurve(const QVector<QVector3D> &points, const QColor &color, double lineWidth)
{
    QVector<QVector3D> effectivePoints = points;
    applyStretchFillIfNeeded(effectivePoints);

    vtkCurve *curve = new vtkCurve(effectivePoints, color, lineWidth);
    curve->addToRenderer(m_renderer);
    m_curves.append(curve);
    autoScaleIfNeeded();
    updateLegend();
    return curve;
}

vtkCurve* vtkPlotBase::addCurve(const QVector<QVector3D> &points, double lineWidth)
{
    return addCurve(points, getNextAutoColor(), lineWidth);
}

void vtkPlotBase::setCurveVisible(vtkCurve* curve, bool visible)
{
    if (curve) {
        curve->setVisible(visible);
        updateLegend();
    }
}

void vtkPlotBase::setCurveColor(vtkCurve* curve, const QColor &color)
{
    if (curve) {
        curve->setColor(color);
        updateLegend();
    }
}

void vtkPlotBase::setCurveWidth(vtkCurve* curve, double width)
{
    if (curve) {
        curve->setLineWidth(width);
    }
}

void vtkPlotBase::updateCurveData(vtkCurve* curve, const QVector<QVector3D> &points)
{
    if (curve) {
        curve->updateData(points);
        autoScaleIfNeeded();
    }
}

void vtkPlotBase::removeCurve(vtkCurve* curve)
{
    if (curve) {
        curve->removeFromRenderer(m_renderer);
        m_curves.removeAll(curve);
        delete curve;
        autoScaleIfNeeded();
        updateLegend();
    }
}

void vtkPlotBase::clearAllCurves()
{
    for (auto curve : m_curves) {
        curve->removeFromRenderer(m_renderer);
        delete curve;
    }
    m_curves.clear();
    autoScaleIfNeeded();
    updateLegend();
}

QList<vtkCurve*> vtkPlotBase::getCurves() const
{
    return m_curves;
}

// ==================== 标记点操作 ====================

vtkMarker* vtkPlotBase::addHollowMarker(const QVector3D &position, const QColor &color,
                                        double screenSize, double lineWidth)
{
    vtkMarker *marker = new vtkMarker(position, color, screenSize, lineWidth);
    marker->setCamera(m_renderer->GetActiveCamera());
    marker->setAxisRange(m_xMin, m_xMax);
    marker->addToRenderer(m_renderer);
    m_markers.append(marker);
    autoScaleIfNeeded();
    updateLegend();
    return marker;
}

vtkMarker* vtkPlotBase::addHollowMarker(const QVector3D &position)
{
    return addHollowMarker(position, getNextAutoColor(), 10.0, 2.0);
}

vtkMarker* vtkPlotBase::addFilledMarker(const QVector3D &position, const QColor &color,
                                         double screenSize)
{
    vtkMarker *marker = new vtkMarker(position, color, screenSize, 1.0);
    marker->setFilled(true);
    marker->setCamera(m_renderer->GetActiveCamera());
    marker->setAxisRange(m_xMin, m_xMax);
    marker->addToRenderer(m_renderer);
    m_markers.append(marker);
    autoScaleIfNeeded();
    updateLegend();
    return marker;
}

vtkMarker* vtkPlotBase::addFilledMarker(const QVector3D &position)
{
    return addFilledMarker(position, getNextAutoColor(), 10.0);
}

void vtkPlotBase::setMarkerVisible(vtkMarker* marker, bool visible)
{
    if (marker) {
        marker->setVisible(visible);
        updateLegend();
    }
}

void vtkPlotBase::setMarkerColor(vtkMarker* marker, const QColor &color)
{
    if (marker) {
        marker->setColor(color);
        updateLegend();
    }
}

void vtkPlotBase::setMarkerRadius(vtkMarker* marker, double radius)
{
    if (marker) {
        marker->setRadius(radius);
    }
}

void vtkPlotBase::setMarkerRelativeRadius(vtkMarker* marker, double ratio)
{
    if (marker) {
        marker->setRelativeRadius(m_xMin, m_xMax, ratio);
    }
}

void vtkPlotBase::setMarkerScreenSize(vtkMarker* marker, double screenSize)
{
    if (marker) {
        int* windowSize = m_renderer->GetRenderWindow()->GetSize();
        int winHeight = windowSize ? windowSize[1] : 600;
        marker->setScreenSize(screenSize, winHeight);
    }
}

void vtkPlotBase::updateMarkerPosition(vtkMarker* marker, const QVector3D &position)
{
    if (marker) {
        marker->setPosition(position);
        autoScaleIfNeeded();
    }
}

void vtkPlotBase::removeMarker(vtkMarker* marker)
{
    if (marker) {
        marker->removeFromRenderer(m_renderer);
        m_markers.removeAll(marker);
        delete marker;
        autoScaleIfNeeded();
        updateLegend();
    }
}

void vtkPlotBase::clearAllMarkers()
{
    for (auto marker : m_markers) {
        marker->removeFromRenderer(m_renderer);
        delete marker;
    }
    m_markers.clear();
    autoScaleIfNeeded();
    updateLegend();
}

QList<vtkMarker*> vtkPlotBase::getMarkers() const
{
    return m_markers;
}

// ==================== 曲面操作 ====================

vtkSurface* vtkPlotBase::addSurface(const QVector<QVector3D> &points, int nx, int ny,
                                    const QColor &color, double opacity)
{
    QVector<QVector3D> effectivePoints = points;
    applyStretchFillIfNeeded(effectivePoints);

    vtkSurface *surface = new vtkSurface(effectivePoints, nx, ny, color, opacity);
    surface->addToRenderer(m_renderer);
    m_surfaces.append(surface);
    autoScaleIfNeeded();
    updateLegend();
    return surface;
}

vtkSurface* vtkPlotBase::addSurface(const QVector<QVector3D> &points, int nx, int ny, double opacity)
{
    return addSurface(points, nx, ny, getNextAutoColor(), opacity);
}

void vtkPlotBase::setSurfaceVisible(vtkSurface* surface, bool visible)
{
    if (surface) {
        surface->setVisible(visible);
        updateLegend();
    }
}

void vtkPlotBase::setSurfaceColor(vtkSurface* surface, const QColor &color)
{
    if (surface) {
        surface->setColor(color);
        updateLegend();
    }
}

void vtkPlotBase::setSurfaceOpacity(vtkSurface* surface, double opacity)
{
    if (surface) {
        surface->setOpacity(opacity);
    }
}

void vtkPlotBase::removeSurface(vtkSurface* surface)
{
    if (surface) {
        surface->removeFromRenderer(m_renderer);
        m_surfaces.removeAll(surface);
        delete surface;
        autoScaleIfNeeded();
        updateLegend();
    }
}

void vtkPlotBase::clearAllSurfaces()
{
    for (auto surface : m_surfaces) {
        surface->removeFromRenderer(m_renderer);
        delete surface;
    }
    m_surfaces.clear();
    autoScaleIfNeeded();
    updateLegend();
}

QList<vtkSurface*> vtkPlotBase::getSurfaces() const
{
    return m_surfaces;
}

// ==================== 热力图操作 ====================

vtkHeatmap* vtkPlotBase::addHeatmapSurface(const QVector<QVector3D> &points, int nx, int ny,
                                            const QString &colorBarTitle)
{
    QVector<QVector3D> effectivePoints = points;
    double origYMin = 0.0, origYMax = 1.0;

    if (m_autoScaleMode == AutoScaleMode::StretchFill) {
        // 在归一化前记录原始 Y 范围（标量范围）
        double origXMin, origXMax, origZMin, origZMax;
        computePointsBounds(points, origXMin, origXMax, origYMin, origYMax, origZMin, origZMax);
    }
    applyStretchFillIfNeeded(effectivePoints);

    vtkHeatmap *heatmap = new vtkHeatmap(effectivePoints, nx, ny, colorBarTitle);
    heatmap->setScalarBarActor(m_scalarBarActor);  // m_scalarBarActor 是 vtkSmartPointer
    heatmap->addToRenderer(m_renderer);
    m_heatmapSurfaces.append(heatmap);

    // StretchFill 模式下重映射标量值到原始范围
    if (m_autoScaleMode == AutoScaleMode::StretchFill) {
        heatmap->remapScalarRange(origYMin, origYMax);
    }

    autoScaleIfNeeded();
    heatmap->setContourBaseY(m_yMin);  // 投影到坐标系Y轴最小值（需在 autoScaleIfNeeded 后）
    updateScalarBar();
    return heatmap;
}

void vtkPlotBase::setHeatmapSurfaceVisible(vtkHeatmap* heatmap, bool visible)
{
    if (heatmap) {
        heatmap->setVisible(visible);
    }
}

void vtkPlotBase::setHeatmapSurfaceOpacity(vtkHeatmap* heatmap, double opacity)
{
    if (heatmap) {
        heatmap->setOpacity(opacity);
    }
}

void vtkPlotBase::setHeatmapColorBarVisible(bool visible)
{
    if (m_scalarBarActor) {
        m_scalarBarActor->SetVisibility(visible ? 1 : 0);
        render();
    }
}

void vtkPlotBase::setHeatmapColorBarTitle(const QString &title)
{
    if (m_scalarBarActor) {
        m_scalarBarActor->SetTitle(title.toUtf8().constData());
        render();
    }
}

void vtkPlotBase::setHeatmapContourVisible(vtkHeatmap* heatmap, bool visible)
{
    if (heatmap) {
        heatmap->setContourVisible(visible);
    }
}

void vtkPlotBase::setHeatmapContourCount(vtkHeatmap* heatmap, int count)
{
    if (heatmap) {
        heatmap->setContourCount(count);
    }
}

void vtkPlotBase::removeHeatmapSurface(vtkHeatmap* heatmap)
{
    if (heatmap) {
        heatmap->removeFromRenderer(m_renderer);
        m_heatmapSurfaces.removeAll(heatmap);
        delete heatmap;
        autoScaleIfNeeded();
        
        if (m_heatmapSurfaces.isEmpty() && m_scalarBarActor) {
            m_scalarBarActor->SetVisibility(0);
        }
        render();
    }
}

void vtkPlotBase::clearAllHeatmapSurfaces()
{
    for (auto heatmap : m_heatmapSurfaces) {
        heatmap->removeFromRenderer(m_renderer);
        delete heatmap;
    }
    m_heatmapSurfaces.clear();
    
    if (m_scalarBarActor) {
        m_scalarBarActor->SetVisibility(0);
    }
    autoScaleIfNeeded();
    render();
}

QList<vtkHeatmap*> vtkPlotBase::getHeatmapSurfaces() const
{
    return m_heatmapSurfaces;
}

// ==================== 清除所有 ====================

void vtkPlotBase::clearAll()
{
    clearAllCurves();
    clearAllMarkers();
    clearAllSurfaces();
    clearAllHeatmapSurfaces();
}
