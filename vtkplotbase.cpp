#include "vtkplotbase.h"
#include "ui_vtkplotbase.h"

// 新的类头文件
#include "drawable/vtkcurve.h"
#include "drawable/vtkmarker.h"
#include "drawable/vtksurface.h"
#include "drawable/vtkheatmap.h"

// VTK Qt 头文件
#include <QVTKOpenGLNativeWidget.h>
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

#include <limits>
#include <algorithm>
#include <cstring>
#include <string>
#include <QDebug>
#include <QtGlobal>
#include <QShowEvent>

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
            qDebug() << "R key pressed: reset view";
        }
    } else if (strcmp(keySym, "x") == 0 || strcmp(keySym, "X") == 0) {
        if (m_plotBase) {
            m_plotBase->setViewSide();
            qDebug() << "X key pressed: side view";
        }
    } else if (strcmp(keySym, "y") == 0 || strcmp(keySym, "Y") == 0) {
        if (m_plotBase) {
            m_plotBase->setViewTop();
            qDebug() << "Y key pressed: top view";
        }
    } else if (strcmp(keySym, "z") == 0 || strcmp(keySym, "Z") == 0) {
        if (m_plotBase) {
            m_plotBase->setViewFront();
            qDebug() << "Z key pressed: front view";
        }
    } else {
        this->vtkInteractorStyleTrackballCamera::OnKeyPress();
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
    , m_legendVisible(true)
    , m_legendPosition(LegendPosition::TopRight)
    , m_titleVisible(false)
    , m_autoColorIndex(0)
{
    ui->setupUi(this);
    setupVTK();
}

vtkPlotBase::~vtkPlotBase()
{
    // 清除所有对象
    clearAll();
    delete ui;
}

// 窗口显示事件
void vtkPlotBase::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    
    // 首次显示时，根据实际数据重置相机
    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        m_renderer->ResetCamera();
        saveDefaultCamera();
    }
    
    updateAllScreenMarkerScales();
    render();
}

// ==================== 初始化方法 ====================

void vtkPlotBase::setupVTK()
{
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建 VTK Qt 控件
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(m_vtkWidget);

    // 获取渲染窗口
    vtkSmartPointer<vtkRenderWindow> renderWindow = m_vtkWidget->renderWindow();

    // 创建渲染器
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.1, 0.1, 0.15);
    renderWindow->AddRenderer(m_renderer);

    // 创建坐标轴
    createAxes();
    
    // 创建图例
    createLegend();
    
    // 创建颜色条
    createScalarBar();

    // 创建标题
    createTitle();

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
    if (m_autoScaleMode == AutoScaleMode::None) return;
    
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
    vtkCurve *curve = new vtkCurve(points, color, lineWidth);
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
    vtkSurface *surface = new vtkSurface(points, nx, ny, color, opacity);
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
    vtkHeatmap *heatmap = new vtkHeatmap(points, nx, ny, colorBarTitle);
    heatmap->setScalarBarActor(m_scalarBarActor);  // m_scalarBarActor 是 vtkSmartPointer
    heatmap->addToRenderer(m_renderer);
    m_heatmapSurfaces.append(heatmap);
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
