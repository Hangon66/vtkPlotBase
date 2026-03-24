#include "vtkplotbase.h"
#include "ui_vtkplotbase.h"

// VTK Qt 头文件
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>

#include <limits>
#include <algorithm>
#include <cstring>
#include <string>
#include <QDebug>

// ==================== 自定义交互器样式 ====================
// 处理键盘快捷键：R(重置视角)、X(侧视图)、Y(俯视图)、Z(前视图)

vtkStandardNewMacro(vtkPlotBaseInteractorStyle);

void vtkPlotBaseInteractorStyle::OnKeyPress()
{
    // 获取交互器
    vtkRenderWindowInteractor* interactor = this->Interactor;
    if (!interactor) return;
    
    const char* keySym = interactor->GetKeySym();
    if (!keySym) return;
    
    if (strcmp(keySym, "r") == 0 || strcmp(keySym, "R") == 0) {
        // R 键：重置到默认视角
        if (m_plotBase) {
            m_plotBase->resetView();
            qDebug() << "R key pressed: reset view";
        }
    } else if (strcmp(keySym, "x") == 0 || strcmp(keySym, "X") == 0) {
        // X 键：侧视图（从X轴方向看）
        if (m_plotBase) {
            m_plotBase->setViewSide();
            qDebug() << "X key pressed: side view";
        }
    } else if (strcmp(keySym, "y") == 0 || strcmp(keySym, "Y") == 0) {
        // Y 键：俯视图（从Y轴方向看）
        if (m_plotBase) {
            m_plotBase->setViewTop();
            qDebug() << "Y key pressed: top view";
        }
    } else if (strcmp(keySym, "z") == 0 || strcmp(keySym, "Z") == 0) {
        // Z 键：前视图（从Z轴方向看）
        if (m_plotBase) {
            m_plotBase->setViewFront();
            qDebug() << "Z key pressed: front view";
        }
    } else {
        // 其他按键传递给父类处理默认行为
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
    , m_autoScaleMargin(0.1)  // 10% 边距
    , m_legendVisible(true)
    , m_legendPosition(LegendPosition::TopRight)
{
    ui->setupUi(this);
    setupVTK();
}

vtkPlotBase::~vtkPlotBase()
{
    delete ui;
}

// ==================== 私有方法 ====================

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
    m_renderer->SetBackground(0.1, 0.1, 0.15);  // 深蓝灰背景
    renderWindow->AddRenderer(m_renderer);

    // 创建坐标轴
    createAxes();
    
    // 创建图例
    createLegend();

    // 设置相机
    m_renderer->ResetCamera();
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->Azimuth(30);
    camera->Elevation(30);
    
    // 保存默认相机参数
    saveDefaultCamera();

    // 设置自定义交互器样式（处理 R 键重置视角）
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
    
    qDebug() << "Default camera saved: pos =" 
             << m_defaultCameraPosition[0] << m_defaultCameraPosition[1] << m_defaultCameraPosition[2];
}

void vtkPlotBase::createAxes()
{
    // 创建 MATLAB 风格的三维立方体坐标轴
    m_cubeAxesActor = vtkSmartPointer<vtkCubeAxesActor>::New();

    // 设置相机（正确渲染所必需）
    m_cubeAxesActor->SetCamera(m_renderer->GetActiveCamera());

    // 设置坐标轴范围
    m_cubeAxesActor->SetXAxisRange(m_xMin, m_xMax);
    m_cubeAxesActor->SetYAxisRange(m_yMin, m_yMax);
    m_cubeAxesActor->SetZAxisRange(m_zMin, m_zMax);

    // 设置坐标轴标题
    m_cubeAxesActor->SetXTitle("X");
    m_cubeAxesActor->SetYTitle("Y");
    m_cubeAxesActor->SetZTitle("Z");

    // 设置坐标轴线宽
    m_cubeAxesActor->GetXAxesLinesProperty()->SetLineWidth(1.0);
    m_cubeAxesActor->GetYAxesLinesProperty()->SetLineWidth(1.0);
    m_cubeAxesActor->GetZAxesLinesProperty()->SetLineWidth(1.0);

    // 设置标签屏幕尺寸
    m_cubeAxesActor->SetScreenSize(8.0);
    m_cubeAxesActor->SetLabelOffset(10.0);

    // 显示坐标轴
    m_cubeAxesActor->SetVisibility(1);

    // 设置飞行模式（外边缘）
    m_cubeAxesActor->SetFlyModeToOuterEdges();

    // 启用网格线
    m_cubeAxesActor->DrawXGridlinesOn();
    m_cubeAxesActor->DrawYGridlinesOn();
    m_cubeAxesActor->DrawZGridlinesOn();

    // 禁用内部网格线
    m_cubeAxesActor->SetDrawXInnerGridlines(false);
    m_cubeAxesActor->SetDrawYInnerGridlines(false);
    m_cubeAxesActor->SetDrawZInnerGridlines(false);

    // 设置网格线颜色（深色背景用浅色）
    m_cubeAxesActor->GetXAxesGridlinesProperty()->SetColor(0.3, 0.3, 0.3);
    m_cubeAxesActor->GetYAxesGridlinesProperty()->SetColor(0.3, 0.3, 0.3);
    m_cubeAxesActor->GetZAxesGridlinesProperty()->SetColor(0.3, 0.3, 0.3);

    // 设置网格线位置（最近 - 跟随相机）
    m_cubeAxesActor->SetGridLineLocation(2);  // VTK_GRID_LINES_CLOSEST = 1

    // 禁用次要刻度
    m_cubeAxesActor->XAxisMinorTickVisibilityOff();
    m_cubeAxesActor->YAxisMinorTickVisibilityOff();
    m_cubeAxesActor->ZAxisMinorTickVisibilityOff();

    // 设置标签缩放（显示完整数字）
    m_cubeAxesActor->SetLabelScaling(false, 0, 0, 0);

    // 设置刻度位置（外部）
    m_cubeAxesActor->SetTickLocationToOutside();

    // 添加到渲染器
    m_renderer->AddActor(m_cubeAxesActor);
}

void vtkPlotBase::createLegend()
{
    // 创建图例框演员（MATLAB风格，用于3D场景）
    m_legendActor = vtkSmartPointer<vtkLegendBoxActor>::New();
    m_legendActor->SetNumberOfEntries(0);
    
    // 启用背景
    m_legendActor->UseBackgroundOn();
    m_legendActor->SetBackgroundColor(0.2, 0.2, 0.25);
    
    // 使用坐标系设置位置（较小尺寸）
    m_legendActor->GetPositionCoordinate()->SetValue(0.85, 0.85);
    m_legendActor->GetPosition2Coordinate()->SetValue(0.12, 0.12);
    
    // 设置文本属性
    m_legendActor->GetEntryTextProperty()->SetColor(1, 1, 1);
    m_legendActor->GetEntryTextProperty()->SetFontSize(8);
    
    m_legendActor->SetVisibility(m_legendVisible ? 1 : 0);
    
    m_renderer->AddActor(m_legendActor);
}

void vtkPlotBase::updateLegend()
{
    int entryCount = 0;
    
    // Count visible items with names
    for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            entryCount++;
        }
    }
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            entryCount++;
        }
    }
    
    if (entryCount == 0) {
        m_legendActor->SetNumberOfEntries(0);
        render();
        return;
    }
    
    // Set number of entries
    m_legendActor->SetNumberOfEntries(entryCount);
    
    int entry = 0;
    
    // Create a simple line symbol for curves
    // Symbol: 0-0.5, Gap: 0.5-1.0 (30% spacing)
    vtkSmartPointer<vtkPoints> linePts = vtkSmartPointer<vtkPoints>::New();
    linePts->InsertNextPoint(0, 0, 0);
    linePts->InsertNextPoint(0.5, 0, 0);
    linePts->InsertNextPoint(1.0, 0, 0);  // Right extent for spacing
    
    vtkSmartPointer<vtkPolyLine> lineCell = vtkSmartPointer<vtkPolyLine>::New();
    lineCell->GetPointIds()->SetNumberOfIds(2);
    lineCell->GetPointIds()->SetId(0, 0);
    lineCell->GetPointIds()->SetId(1, 1);
    
    vtkSmartPointer<vtkCellArray> lineCells = vtkSmartPointer<vtkCellArray>::New();
    lineCells->InsertNextCell(lineCell);
    
    vtkSmartPointer<vtkPolyData> lineSymbol = vtkSmartPointer<vtkPolyData>::New();
    lineSymbol->SetPoints(linePts);
    lineSymbol->SetLines(lineCells);
    
    // Add curves to legend
    for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            double color[3];
            it->actor->GetProperty()->GetColor(color);
            m_legendActor->SetEntry(entry++, lineSymbol, it->name.toUtf8().constData(), color);
        }
    }
    
    // Create symbols for markers
    // Symbol: 0-0.5, Gap: 0.5-1.0 (30% spacing)
    
    // Filled circle symbol for filled markers
    vtkSmartPointer<vtkRegularPolygonSource> filledCircleSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    filledCircleSource->SetNumberOfSides(16);
    filledCircleSource->SetRadius(0.25);
    filledCircleSource->SetCenter(0.25, 0, 0);
    filledCircleSource->Update();
    
    // Add right extent point for spacing
    vtkSmartPointer<vtkPolyData> filledCircleSymbol = vtkSmartPointer<vtkPolyData>::New();
    filledCircleSymbol->DeepCopy(filledCircleSource->GetOutput());
    filledCircleSymbol->GetPoints()->InsertNextPoint(1.0, 0, 0);
    
    // Hollow ring symbol for hollow markers
    vtkSmartPointer<vtkDiskSource> hollowRingSource = vtkSmartPointer<vtkDiskSource>::New();
    hollowRingSource->SetInnerRadius(0.15);
    hollowRingSource->SetOuterRadius(0.25);
    hollowRingSource->SetCircumferentialResolution(16);
    hollowRingSource->Update();
    
    // Transform to center at (0.25, 0, 0)
    vtkSmartPointer<vtkTransform> ringTransform = vtkSmartPointer<vtkTransform>::New();
    ringTransform->Translate(0.25, 0, 0);
    vtkSmartPointer<vtkTransformPolyDataFilter> ringTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    ringTransformFilter->SetInputConnection(hollowRingSource->GetOutputPort());
    ringTransformFilter->SetTransform(ringTransform);
    ringTransformFilter->Update();
    
    // Add right extent point for spacing
    vtkSmartPointer<vtkPolyData> hollowRingSymbol = vtkSmartPointer<vtkPolyData>::New();
    hollowRingSymbol->DeepCopy(ringTransformFilter->GetOutput());
    hollowRingSymbol->GetPoints()->InsertNextPoint(1.0, 0, 0);
    
    // Add markers to legend
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            double color[3];
            it->follower->GetProperty()->GetColor(color);
            
            // Use different symbol based on marker type
            if (it->filled) {
                m_legendActor->SetEntry(entry++, filledCircleSymbol, it->name.toUtf8().constData(), color);
            } else {
                m_legendActor->SetEntry(entry++, hollowRingSymbol, it->name.toUtf8().constData(), color);
            }
        }
    }
    
    // Update position based on setting
    updateLegendPosition();
    
    render();
}

void vtkPlotBase::updateLegendPosition()
{
    switch (m_legendPosition) {
        case LegendPosition::TopLeft:
            m_legendActor->GetPositionCoordinate()->SetValue(0.03, 0.85);
            break;
        case LegendPosition::TopRight:
            m_legendActor->GetPositionCoordinate()->SetValue(0.85, 0.85);
            break;
        case LegendPosition::BottomLeft:
            m_legendActor->GetPositionCoordinate()->SetValue(0.03, 0.03);
            break;
        case LegendPosition::BottomRight:
            m_legendActor->GetPositionCoordinate()->SetValue(0.85, 0.03);
            break;
    }
}

QString vtkPlotBase::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void vtkPlotBase::updateAxesBounds()
{
    qDebug() << "updateAxesBounds: Setting bounds to"
             << "X[" << m_xMin << "," << m_xMax << "]"
             << "Y[" << m_yMin << "," << m_yMax << "]"
             << "Z[" << m_zMin << "," << m_zMax << "]";
    
    // Set the cube bounds (physical extent in 3D space)
    m_cubeAxesActor->SetBounds(m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax);
    
    // Set the axis ranges (label values)
    m_cubeAxesActor->SetXAxisRange(m_xMin, m_xMax);
    m_cubeAxesActor->SetYAxisRange(m_yMin, m_yMax);
    m_cubeAxesActor->SetZAxisRange(m_zMin, m_zMax);
    
    m_renderer->ResetCamera();
    render();
}

void vtkPlotBase::computeDataBounds(double &xMin, double &xMax, double &yMin, double &yMax, double &zMin, double &zMax)
{
    // Initialize with invalid values
    xMin = std::numeric_limits<double>::max();
    xMax = std::numeric_limits<double>::lowest();
    yMin = std::numeric_limits<double>::max();
    yMax = std::numeric_limits<double>::lowest();
    zMin = std::numeric_limits<double>::max();
    zMax = std::numeric_limits<double>::lowest();
    
    bool hasData = false;
    
    qDebug() << "computeDataBounds: curves count =" << m_curves.size() << ", markers count =" << m_markers.size();
    
    // Compute bounds from curves
    for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
        qDebug() << "  Checking curve" << it.key() << "visible =" << it->visible;
        if (!it->visible) continue;
        
        vtkPolyData *polyData = it->polyData;
        vtkPoints *pts = polyData->GetPoints();
        if (!pts || pts->GetNumberOfPoints() == 0) continue;
        
        qDebug() << "    Points count:" << pts->GetNumberOfPoints();
        hasData = true;
        for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i) {
            double pt[3];
            pts->GetPoint(i, pt);
            xMin = std::min(xMin, pt[0]);
            xMax = std::max(xMax, pt[0]);
            yMin = std::min(yMin, pt[1]);
            yMax = std::max(yMax, pt[1]);
            zMin = std::min(zMin, pt[2]);
            zMax = std::max(zMax, pt[2]);
        }
    }
    
    // Compute bounds from markers
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        if (!it->visible) continue;
        
        hasData = true;
        double pos[3];
        it->follower->GetPosition(pos);
        double r = it->radius;
        
        xMin = std::min(xMin, pos[0] - r);
        xMax = std::max(xMax, pos[0] + r);
        yMin = std::min(yMin, pos[1] - r);
        yMax = std::max(yMax, pos[1] + r);
        zMin = std::min(zMin, pos[2] - r);
        zMax = std::max(zMax, pos[2] + r);
    }
    
    // If no data, set default range
    if (!hasData) {
        xMin = -1.0; xMax = 1.0;
        yMin = -1.0; yMax = 1.0;
        zMin = -1.0; zMax = 1.0;
    }
}

void vtkPlotBase::autoScaleIfNeeded()
{
    qDebug() << "autoScaleIfNeeded called, mode =" << static_cast<int>(m_autoScaleMode);
    if (m_autoScaleMode == AutoScaleMode::None) return;
    
    double xMin, xMax, yMin, yMax, zMin, zMax;
    computeDataBounds(xMin, xMax, yMin, yMax, zMin, zMax);
    
    qDebug() << "Data bounds:" << "X[" << xMin << "," << xMax << "]"
             << "Y[" << yMin << "," << yMax << "]"
             << "Z[" << zMin << "," << zMax << "]";
    
    // Add margin
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    double zRange = zMax - zMin;
    
    // Handle zero range case
    if (xRange < 1e-10) { xMin -= 1.0; xMax += 1.0; xRange = 2.0; }
    if (yRange < 1e-10) { yMin -= 1.0; yMax += 1.0; yRange = 2.0; }
    if (zRange < 1e-10) { zMin -= 1.0; zMax += 1.0; zRange = 2.0; }
    
    if (m_autoScaleMode == AutoScaleMode::EqualRatio) {
        // Equal ratio mode: all axes have the same scale
        // Find the maximum range and apply it to all axes
        double maxRange = std::max({xRange, yRange, zRange});
        double margin = maxRange * m_autoScaleMargin;
        
        // Calculate center of each axis
        double xCenter = (xMin + xMax) / 2.0;
        double yCenter = (yMin + yMax) / 2.0;
        double zCenter = (zMin + zMax) / 2.0;
        
        // Apply equal range with margin
        double halfRange = maxRange / 2.0 + margin;
        m_xMin = xCenter - halfRange;
        m_xMax = xCenter + halfRange;
        m_yMin = yCenter - halfRange;
        m_yMax = yCenter + halfRange;
        m_zMin = zCenter - halfRange;
        m_zMax = zCenter + halfRange;
        
        qDebug() << "EqualRatio mode: maxRange =" << maxRange << "halfRange =" << halfRange;
    } else {
        // Independent mode: each axis scales independently
        m_xMin = xMin - xRange * m_autoScaleMargin;
        m_xMax = xMax + xRange * m_autoScaleMargin;
        m_yMin = yMin - yRange * m_autoScaleMargin;
        m_yMax = yMax + yRange * m_autoScaleMargin;
        m_zMin = zMin - zRange * m_autoScaleMargin;
        m_zMax = zMax + zRange * m_autoScaleMargin;
    }
    
    qDebug() << "Axis range after margin:" << "X[" << m_xMin << "," << m_xMax << "]"
             << "Y[" << m_yMin << "," << m_yMax << "]"
             << "Z[" << m_zMin << "," << m_zMax << "]";
    
    updateAxesBounds();
}

void vtkPlotBase::render()
{
    if (m_vtkWidget && m_vtkWidget->renderWindow()) {
        m_vtkWidget->renderWindow()->Render();
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

// ==================== 图例操作 ====================

void vtkPlotBase::setLegendVisible(bool visible)
{
    m_legendVisible = visible;
    m_legendActor->SetVisibility(visible ? 1 : 0);
    render();
}

void vtkPlotBase::setLegendPosition(LegendPosition pos)
{
    m_legendPosition = pos;
    updateLegendPosition();
    render();
}

void vtkPlotBase::setCurveName(const QString &curveId, const QString &name)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].name = name;
        updateLegend();
    }
}

void vtkPlotBase::setMarkerName(const QString &markerId, const QString &name)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].name = name;
        updateLegend();
    }
}

void vtkPlotBase::resetView()
{
    qDebug() << "resetView: restoring default camera view";
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->SetPosition(m_defaultCameraPosition);
    camera->SetFocalPoint(m_defaultCameraFocalPoint);
    camera->SetViewUp(m_defaultCameraViewUp);
    camera->SetDistance(m_defaultCameraDistance);
    render();
}

void vtkPlotBase::resetAxisRange()
{
    qDebug() << "resetAxisRange: triggering auto scale";
    autoScaleIfNeeded();
}

void vtkPlotBase::setViewFront()
{
    // Front view: looking from Z-axis (camera at +Z)
    qDebug() << "setViewFront: front view (Z-axis)";
    vtkCamera *camera = m_renderer->GetActiveCamera();
    
    // Calculate focal point (center of the data bounds)
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    
    // Keep current distance
    double distance = camera->GetDistance();
    
    // Set camera position (from Z-axis)
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0], focalPoint[1], focalPoint[2] + distance);
    camera->SetViewUp(0, 1, 0);  // Y-axis up
    
    render();
}

void vtkPlotBase::setViewTop()
{
    // Top view: looking from Y-axis (camera at +Y)
    qDebug() << "setViewTop: top view (Y-axis)";
    vtkCamera *camera = m_renderer->GetActiveCamera();
    
    // Calculate focal point (center of the data bounds)
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    
    // Keep current distance
    double distance = camera->GetDistance();
    
    // Set camera position (from Y-axis)
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0], focalPoint[1] + distance, focalPoint[2]);
    camera->SetViewUp(0, 0, -1);  // Z-axis down (towards negative Z)
    
    render();
}

void vtkPlotBase::setViewSide()
{
    // Side view: looking from X-axis (camera at +X)
    qDebug() << "setViewSide: side view (X-axis)";
    vtkCamera *camera = m_renderer->GetActiveCamera();
    
    // Calculate focal point (center of the data bounds)
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    
    // Keep current distance
    double distance = camera->GetDistance();
    
    // Set camera position (from X-axis)
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0] + distance, focalPoint[1], focalPoint[2]);
    camera->SetViewUp(0, 0, 1);  // Z-axis up
    
    render();
}

void vtkPlotBase::resetCamera()
{
    m_renderer->ResetCamera();
    render();
}

// ==================== 曲线操作 ====================

QString vtkPlotBase::addCurve(const QVector<QVector3D> &points, const QColor &color, double lineWidth)
{
    if (points.size() < 2) return QString();

    QString id = generateId();
    CurveData curve;
    curve.id = id;
    curve.visible = true;

    // 创建点集
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    for (const auto &pt : points) {
        pts->InsertNextPoint(pt.x(), pt.y(), pt.z());
    }

    // 创建多段线
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(points.size());
    for (int i = 0; i < points.size(); ++i) {
        polyLine->GetPointIds()->SetId(i, i);
    }

    // 创建单元数组
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);

    // 创建多边形数据
    curve.polyData = vtkSmartPointer<vtkPolyData>::New();
    curve.polyData->SetPoints(pts);
    curve.polyData->SetLines(cells);

    // 创建映射器
    curve.mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    curve.mapper->SetInputData(curve.polyData);

    // 创建演员
    curve.actor = vtkSmartPointer<vtkActor>::New();
    curve.actor->SetMapper(curve.mapper);
    curve.actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    curve.actor->GetProperty()->SetLineWidth(lineWidth);

    m_renderer->AddActor(curve.actor);
    m_curves[id] = curve;
    qDebug() << "addCurve: curve added, calling autoScaleIfNeeded";
    autoScaleIfNeeded();
    updateLegend();

    return id;
}

QString vtkPlotBase::addCurve(const QVector<double> &x, const QVector<double> &y, const QVector<double> &z,
                              const QColor &color, double lineWidth)
{
    if (x.size() != y.size() || x.size() != z.size() || x.size() < 2) return QString();

    QVector<QVector3D> points;
    for (int i = 0; i < x.size(); ++i) {
        points.append(QVector3D(x[i], y[i], z[i]));
    }
    return addCurve(points, color, lineWidth);
}

void vtkPlotBase::setCurveVisible(const QString &curveId, bool visible)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].visible = visible;
        m_curves[curveId].actor->SetVisibility(visible ? 1 : 0);
        updateLegend();
    }
}

void vtkPlotBase::setCurveColor(const QString &curveId, const QColor &color)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        render();
    }
}

void vtkPlotBase::setCurveLineWidth(const QString &curveId, double lineWidth)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].actor->GetProperty()->SetLineWidth(lineWidth);
        render();
    }
}

void vtkPlotBase::updateCurveData(const QString &curveId, const QVector<QVector3D> &points)
{
    if (!m_curves.contains(curveId) || points.size() < 2) return;

    CurveData &curve = m_curves[curveId];

    // Create new points
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    for (const auto &pt : points) {
        pts->InsertNextPoint(pt.x(), pt.y(), pt.z());
    }

    // Create polyline
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(points.size());
    for (int i = 0; i < points.size(); ++i) {
        polyLine->GetPointIds()->SetId(i, i);
    }

    // Create cell array
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);

    curve.polyData->SetPoints(pts);
    curve.polyData->SetLines(cells);
    curve.polyData->Modified();
    autoScaleIfNeeded();
    render();
}

void vtkPlotBase::removeCurve(const QString &curveId)
{
    if (m_curves.contains(curveId)) {
        m_renderer->RemoveActor(m_curves[curveId].actor);
        m_curves.remove(curveId);
        autoScaleIfNeeded();
        updateLegend();
    }
}

void vtkPlotBase::clearAllCurves()
{
    for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
        m_renderer->RemoveActor(it->actor);
    }
    m_curves.clear();
    autoScaleIfNeeded();
    updateLegend();
}

QStringList vtkPlotBase::getCurveIds() const
{
    return m_curves.keys();
}

// ==================== 空心环标记操作 ====================

QString vtkPlotBase::addHollowMarker(const QVector3D &position, const QColor &color, double radius, double lineWidth)
{
    QString id = generateId();
    MarkerData marker;
    marker.id = id;
    marker.filled = false;
    marker.visible = true;
    marker.radius = radius;
    marker.lineWidth = lineWidth;

    // 创建圆盘源（空心环）
    vtkSmartPointer<vtkDiskSource> diskSource = vtkSmartPointer<vtkDiskSource>::New();
    diskSource->SetInnerRadius(radius * 0.6);   // 内半径（孔）
    diskSource->SetOuterRadius(radius);          // 外半径（边）
    diskSource->SetCircumferentialResolution(64);
    diskSource->Update();

    marker.polyData = diskSource->GetOutput();

    // 创建映射器
    marker.mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    marker.mapper->SetInputConnection(diskSource->GetOutputPort());

    // 创建跟随者（始终面向相机）
    marker.follower = vtkSmartPointer<vtkFollower>::New();
    marker.follower->SetMapper(marker.mapper);
    marker.follower->SetPosition(position.x(), position.y(), position.z());
    marker.follower->SetCamera(m_renderer->GetActiveCamera());
    marker.follower->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    marker.follower->GetProperty()->SetLineWidth(lineWidth);

    m_renderer->AddActor(marker.follower);
    m_markers[id] = marker;
    autoScaleIfNeeded();
    updateLegend();

    return id;
}

void vtkPlotBase::setMarkerVisible(const QString &markerId, bool visible)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].visible = visible;
        m_markers[markerId].follower->SetVisibility(visible ? 1 : 0);
        autoScaleIfNeeded();
        updateLegend();
    }
}

void vtkPlotBase::setMarkerColor(const QString &markerId, const QColor &color)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].follower->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        render();
    }
}

void vtkPlotBase::setMarkerRadius(const QString &markerId, double radius)
{
    if (!m_markers.contains(markerId)) return;

    MarkerData &marker = m_markers[markerId];
    marker.radius = radius;

    // Recreate disk source with new radius
    vtkSmartPointer<vtkDiskSource> diskSource = vtkSmartPointer<vtkDiskSource>::New();
    diskSource->SetInnerRadius(radius * 0.6);
    diskSource->SetOuterRadius(radius);
    diskSource->SetCircumferentialResolution(64);
    diskSource->Update();

    marker.polyData = diskSource->GetOutput();
    marker.mapper->SetInputConnection(diskSource->GetOutputPort());
    render();
}

void vtkPlotBase::setMarkerLineWidth(const QString &markerId, double lineWidth)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].lineWidth = lineWidth;
        m_markers[markerId].follower->GetProperty()->SetLineWidth(lineWidth);
        render();
    }
}

void vtkPlotBase::updateMarkerPosition(const QString &markerId, const QVector3D &position)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].follower->SetPosition(position.x(), position.y(), position.z());
        autoScaleIfNeeded();
        render();
    }
}

void vtkPlotBase::removeMarker(const QString &markerId)
{
    if (m_markers.contains(markerId)) {
        m_renderer->RemoveActor(m_markers[markerId].follower);
        m_markers.remove(markerId);
        autoScaleIfNeeded();
        updateLegend();
    }
}

void vtkPlotBase::clearAllMarkers()
{
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        m_renderer->RemoveActor(it->follower);
    }
    m_markers.clear();
    autoScaleIfNeeded();
    updateLegend();
}

QStringList vtkPlotBase::getMarkerIds() const
{
    return m_markers.keys();
}

// ==================== 填充圆标记操作 ====================

QString vtkPlotBase::addFilledMarker(const QVector3D &position, const QColor &color, double radius)
{
    QString id = generateId();
    MarkerData marker;
    marker.id = id;
    marker.filled = true;
    marker.visible = true;
    marker.radius = radius;
    marker.lineWidth = 1.0;

    // 创建正多边形源（填充圆）
    vtkSmartPointer<vtkRegularPolygonSource> polygonSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    polygonSource->SetNumberOfSides(64);
    polygonSource->SetRadius(radius);
    polygonSource->SetCenter(0, 0, 0);
    polygonSource->Update();

    marker.polyData = polygonSource->GetOutput();

    // 创建映射器
    marker.mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    marker.mapper->SetInputConnection(polygonSource->GetOutputPort());

    // 创建跟随者（始终面向相机）
    marker.follower = vtkSmartPointer<vtkFollower>::New();
    marker.follower->SetMapper(marker.mapper);
    marker.follower->SetPosition(position.x(), position.y(), position.z());
    marker.follower->SetCamera(m_renderer->GetActiveCamera());
    marker.follower->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());

    m_renderer->AddActor(marker.follower);
    m_markers[id] = marker;
    autoScaleIfNeeded();
    updateLegend();

    return id;
}

void vtkPlotBase::setFilledMarkerVisible(const QString &markerId, bool visible)
{
    setMarkerVisible(markerId, visible);
}

void vtkPlotBase::setFilledMarkerColor(const QString &markerId, const QColor &color)
{
    setMarkerColor(markerId, color);
}

void vtkPlotBase::setFilledMarkerRadius(const QString &markerId, double radius)
{
    if (!m_markers.contains(markerId)) return;

    MarkerData &marker = m_markers[markerId];
    if (!marker.filled) return;  // Only for filled markers

    marker.radius = radius;

    // Recreate polygon source with new radius
    vtkSmartPointer<vtkRegularPolygonSource> polygonSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    polygonSource->SetNumberOfSides(64);
    polygonSource->SetRadius(radius);
    polygonSource->SetCenter(0, 0, 0);
    polygonSource->Update();

    marker.polyData = polygonSource->GetOutput();
    marker.mapper->SetInputConnection(polygonSource->GetOutputPort());
    render();
}

void vtkPlotBase::updateFilledMarkerPosition(const QString &markerId, const QVector3D &position)
{
    updateMarkerPosition(markerId, position);
}

void vtkPlotBase::removeFilledMarker(const QString &markerId)
{
    removeMarker(markerId);
}

void vtkPlotBase::clearAllFilledMarkers()
{
    QStringList idsToRemove;
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        if (it->filled) {
            idsToRemove.append(it.key());
        }
    }
    for (const QString &id : idsToRemove) {
        removeMarker(id);
    }
}

QStringList vtkPlotBase::getFilledMarkerIds() const
{
    QStringList ids;
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        if (it->filled) {
            ids.append(it.key());
        }
    }
    return ids;
}

// ==================== 清除所有 ====================

void vtkPlotBase::clearAll()
{
    clearAllCurves();
    clearAllMarkers();
}
