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
#include <QtGlobal>

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

// 保存默认相机参数（用于重置视角）
void vtkPlotBase::saveDefaultCamera()
{
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->GetPosition(m_defaultCameraPosition);      // 相机位置
    camera->GetFocalPoint(m_defaultCameraFocalPoint);  // 焦点
    camera->GetViewUp(m_defaultCameraViewUp);          // 上方向
    m_defaultCameraDistance = camera->GetDistance();   // 距离
    
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
    
    // 统计有名称且可见的条目数量
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
    for (auto it = m_surfaces.begin(); it != m_surfaces.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            entryCount++;
        }
    }
    
    // 如果没有条目，清空图例并返回
    if (entryCount == 0) {
        m_legendActor->SetNumberOfEntries(0);
        render();
        return;
    }
    
    // 设置条目数量
    m_legendActor->SetNumberOfEntries(entryCount);
    
    int entry = 0;
    
    // 创建曲线符号（线条）
    // 符号范围：0-0.5，间距：0.5-1.0（右侧留50%空白）
    vtkSmartPointer<vtkPoints> linePts = vtkSmartPointer<vtkPoints>::New();
    linePts->InsertNextPoint(0, 0, 0);
    linePts->InsertNextPoint(0.5, 0, 0);
    linePts->InsertNextPoint(1.0, 0, 0);  // 右侧扩展点，用于创建间距
    
    vtkSmartPointer<vtkPolyLine> lineCell = vtkSmartPointer<vtkPolyLine>::New();
    lineCell->GetPointIds()->SetNumberOfIds(2);
    lineCell->GetPointIds()->SetId(0, 0);
    lineCell->GetPointIds()->SetId(1, 1);
    
    vtkSmartPointer<vtkCellArray> lineCells = vtkSmartPointer<vtkCellArray>::New();
    lineCells->InsertNextCell(lineCell);
    
    vtkSmartPointer<vtkPolyData> lineSymbol = vtkSmartPointer<vtkPolyData>::New();
    lineSymbol->SetPoints(linePts);
    lineSymbol->SetLines(lineCells);
    
    // 将曲线添加到图例
    for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            double color[3];
            it->actor->GetProperty()->GetColor(color);
            m_legendActor->SetEntry(entry++, lineSymbol, it->name.toUtf8().constData(), color);
        }
    }
    
    // 创建标记符号
    // 符号范围：0-0.5，间距：0.5-1.0（右侧留50%空白）
    
    // 填充圆符号（用于填充标记）
    vtkSmartPointer<vtkRegularPolygonSource> filledCircleSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    filledCircleSource->SetNumberOfSides(16);
    filledCircleSource->SetRadius(0.25);
    filledCircleSource->SetCenter(0.25, 0, 0);
    filledCircleSource->Update();
    
    // 添加右侧扩展点用于创建间距
    vtkSmartPointer<vtkPolyData> filledCircleSymbol = vtkSmartPointer<vtkPolyData>::New();
    filledCircleSymbol->DeepCopy(filledCircleSource->GetOutput());
    filledCircleSymbol->GetPoints()->InsertNextPoint(1.0, 0, 0);
    
    // 空心环符号（用于空心标记）
    vtkSmartPointer<vtkDiskSource> hollowRingSource = vtkSmartPointer<vtkDiskSource>::New();
    hollowRingSource->SetInnerRadius(0.15);
    hollowRingSource->SetOuterRadius(0.25);
    hollowRingSource->SetCircumferentialResolution(16);
    hollowRingSource->Update();
    
    // 变换：将圆心移动到 (0.25, 0, 0)
    vtkSmartPointer<vtkTransform> ringTransform = vtkSmartPointer<vtkTransform>::New();
    ringTransform->Translate(0.25, 0, 0);
    vtkSmartPointer<vtkTransformPolyDataFilter> ringTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    ringTransformFilter->SetInputConnection(hollowRingSource->GetOutputPort());
    ringTransformFilter->SetTransform(ringTransform);
    ringTransformFilter->Update();
    
    // 添加右侧扩展点用于创建间距
    vtkSmartPointer<vtkPolyData> hollowRingSymbol = vtkSmartPointer<vtkPolyData>::New();
    hollowRingSymbol->DeepCopy(ringTransformFilter->GetOutput());
    hollowRingSymbol->GetPoints()->InsertNextPoint(1.0, 0, 0);
    
    // 将标记添加到图例
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            double color[3];
            it->follower->GetProperty()->GetColor(color);
            
            // 根据标记类型使用不同符号
            if (it->filled) {
                m_legendActor->SetEntry(entry++, filledCircleSymbol, it->name.toUtf8().constData(), color);
            } else {
                m_legendActor->SetEntry(entry++, hollowRingSymbol, it->name.toUtf8().constData(), color);
            }
        }
    }
    
    // 创建曲面符号（填充方块）
    vtkSmartPointer<vtkRegularPolygonSource> squareSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    squareSource->SetNumberOfSides(4);  // 正方形
    squareSource->SetRadius(0.25);
    squareSource->SetCenter(0.25, 0, 0);
    squareSource->Update();
    
    // 添加右侧扩展点用于创建间距
    vtkSmartPointer<vtkPolyData> squareSymbol = vtkSmartPointer<vtkPolyData>::New();
    squareSymbol->DeepCopy(squareSource->GetOutput());
    squareSymbol->GetPoints()->InsertNextPoint(1.0, 0, 0);
    
    // 将曲面添加到图例
    for (auto it = m_surfaces.begin(); it != m_surfaces.end(); ++it) {
        if (it->visible && !it->name.isEmpty()) {
            double color[3];
            it->actor->GetProperty()->GetColor(color);
            m_legendActor->SetEntry(entry++, squareSymbol, it->name.toUtf8().constData(), color);
        }
    }
    
    // 根据设置更新图例位置
    updateLegendPosition();
    
    // 根据图例条目数量动态调整图例框大小
    // 每个条目高度约0.035，最小宽度0.10，最大宽度0.18
    double legendWidth = qBound(0.10, 0.12 + entryCount * 0.005, 0.18);
    double legendHeight = qBound(0.04, 0.03 + entryCount * 0.035, 0.25);
    m_legendActor->GetPosition2Coordinate()->SetValue(legendWidth, legendHeight);
    
    // 根据图例框高度重新调整位置（使顶部/底部对齐）
    updateLegendPositionForSize(legendWidth, legendHeight);
    
    render();
}

// 更新图例位置（根据当前位置设置）
void vtkPlotBase::updateLegendPosition()
{
    // 获取当前图例框大小
    double *pos2 = m_legendActor->GetPosition2Coordinate()->GetValue();
    double legendWidth = pos2[0];
    double legendHeight = pos2[1];
    updateLegendPositionForSize(legendWidth, legendHeight);
}

// 根据图例框大小更新位置（使顶部/底部贴边）
void vtkPlotBase::updateLegendPositionForSize(double legendWidth, double legendHeight)
{
    double margin = 0.02;  // 边距2%
    
    switch (m_legendPosition) {
        case LegendPosition::TopLeft:
            // 左上角：X靠左，Y使顶部贴边
            m_legendActor->GetPositionCoordinate()->SetValue(margin, 1.0 - legendHeight - margin);
            break;
        case LegendPosition::TopRight:
            // 右上角：X使右侧贴边，Y使顶部贴边
            m_legendActor->GetPositionCoordinate()->SetValue(1.0 - legendWidth - margin, 1.0 - legendHeight - margin);
            break;
        case LegendPosition::BottomLeft:
            // 左下角：X靠左，Y靠底部
            m_legendActor->GetPositionCoordinate()->SetValue(margin, margin);
            break;
        case LegendPosition::BottomRight:
            // 右下角：X使右侧贴边，Y靠底部
            m_legendActor->GetPositionCoordinate()->SetValue(1.0 - legendWidth - margin, margin);
            break;
    }
}

// 生成唯一ID（UUID格式）
QString vtkPlotBase::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

// 更新坐标轴边界（将当前范围应用到坐标轴）
void vtkPlotBase::updateAxesBounds()
{
    qDebug() << "updateAxesBounds: Setting bounds to"
             << "X[" << m_xMin << "," << m_xMax << "]"
             << "Y[" << m_yMin << "," << m_yMax << "]"
             << "Z[" << m_zMin << "," << m_zMax << "]";
    
    // 设置立方体边界（三维空间中的物理范围）
    m_cubeAxesActor->SetBounds(m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax);
    
    // 设置坐标轴标签范围
    m_cubeAxesActor->SetXAxisRange(m_xMin, m_xMax);
    m_cubeAxesActor->SetYAxisRange(m_yMin, m_yMax);
    m_cubeAxesActor->SetZAxisRange(m_zMin, m_zMax);
    
    m_renderer->ResetCamera();
    render();
}

// 计算数据边界（遍历所有曲线和标记）
void vtkPlotBase::computeDataBounds(double &xMin, double &xMax, double &yMin, double &yMax, double &zMin, double &zMax)
{
    // 初始化为无效值
    xMin = std::numeric_limits<double>::max();
    xMax = std::numeric_limits<double>::lowest();
    yMin = std::numeric_limits<double>::max();
    yMax = std::numeric_limits<double>::lowest();
    zMin = std::numeric_limits<double>::max();
    zMax = std::numeric_limits<double>::lowest();
    
    bool hasData = false;
    
    qDebug() << "computeDataBounds: curves count =" << m_curves.size() << ", markers count =" << m_markers.size();
    
    // 从曲线计算边界
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
    
    // 从标记计算边界
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
    
    // 从曲面计算边界
    for (auto it = m_surfaces.begin(); it != m_surfaces.end(); ++it) {
        if (!it->visible) continue;
        
        vtkPolyData *polyData = it->polyData;
        vtkPoints *pts = polyData->GetPoints();
        if (!pts || pts->GetNumberOfPoints() == 0) continue;
        
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
    
    // 从热力图曲面计算边界
    for (auto it = m_heatmapSurfaces.begin(); it != m_heatmapSurfaces.end(); ++it) {
        if (!it->visible) continue;
        
        vtkPolyData *polyData = it->polyData;
        vtkPoints *pts = polyData->GetPoints();
        if (!pts || pts->GetNumberOfPoints() == 0) continue;
        
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
    
    // 如果没有数据，设置默认范围
    if (!hasData) {
        xMin = -1.0; xMax = 1.0;
        yMin = -1.0; yMax = 1.0;
        zMin = -1.0; zMax = 1.0;
    }
}

// 自动缩放坐标轴（根据数据范围）
void vtkPlotBase::autoScaleIfNeeded()
{
    qDebug() << "autoScaleIfNeeded called, mode =" << static_cast<int>(m_autoScaleMode);
    if (m_autoScaleMode == AutoScaleMode::None) return;
    
    double xMin, xMax, yMin, yMax, zMin, zMax;
    computeDataBounds(xMin, xMax, yMin, yMax, zMin, zMax);
    
    qDebug() << "Data bounds:" << "X[" << xMin << "," << xMax << "]"
             << "Y[" << yMin << "," << yMax << "]"
             << "Z[" << zMin << "," << zMax << "]";
    
    // 添加边距
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    double zRange = zMax - zMin;
    
    // 处理零范围情况
    if (xRange < 1e-10) { xMin -= 1.0; xMax += 1.0; xRange = 2.0; }
    if (yRange < 1e-10) { yMin -= 1.0; yMax += 1.0; yRange = 2.0; }
    if (zRange < 1e-10) { zMin -= 1.0; zMax += 1.0; zRange = 2.0; }
    
    if (m_autoScaleMode == AutoScaleMode::EqualRatio) {
        // 等比例模式：所有坐标轴使用相同比例
        // 找到最大范围并应用到所有轴
        double maxRange = std::max({xRange, yRange, zRange});
        double margin = maxRange * m_autoScaleMargin;
        
        // 计算每个轴的中心
        double xCenter = (xMin + xMax) / 2.0;
        double yCenter = (yMin + yMax) / 2.0;
        double zCenter = (zMin + zMax) / 2.0;
        
        // 应用相等范围并添加边距
        double halfRange = maxRange / 2.0 + margin;
        m_xMin = xCenter - halfRange;
        m_xMax = xCenter + halfRange;
        m_yMin = yCenter - halfRange;
        m_yMax = yCenter + halfRange;
        m_zMin = zCenter - halfRange;
        m_zMax = zCenter + halfRange;
        
        qDebug() << "EqualRatio mode: maxRange =" << maxRange << "halfRange =" << halfRange;
    } else {
        // 独立模式：每个坐标轴独立缩放
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

// 渲染场景
void vtkPlotBase::render()
{
    if (m_vtkWidget && m_vtkWidget->renderWindow()) {
        m_vtkWidget->renderWindow()->Render();
    }
}

// ==================== 坐标系操作 ====================

// 设置坐标轴范围
void vtkPlotBase::setAxisRange(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax)
{
    m_xMin = xMin; m_xMax = xMax;
    m_yMin = yMin; m_yMax = yMax;
    m_zMin = zMin; m_zMax = zMax;
    updateAxesBounds();
}

// 设置坐标轴标题
void vtkPlotBase::setAxisTitles(const QString &xTitle, const QString &yTitle, const QString &zTitle)
{
    m_cubeAxesActor->SetXTitle(xTitle.toUtf8().constData());
    m_cubeAxesActor->SetYTitle(yTitle.toUtf8().constData());
    m_cubeAxesActor->SetZTitle(zTitle.toUtf8().constData());
    render();
}

// 设置网格线可见性
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

// 设置背景颜色
void vtkPlotBase::setBackground(const QColor &color)
{
    m_renderer->SetBackground(color.redF(), color.greenF(), color.blueF());
    render();
}

// 设置自动缩放模式
void vtkPlotBase::setAutoScaleMode(AutoScaleMode mode)
{
    m_autoScaleMode = mode;
    autoScaleIfNeeded();
}

// 获取自动缩放模式
AutoScaleMode vtkPlotBase::autoScaleMode() const
{
    return m_autoScaleMode;
}

// 自动适应数据范围
void vtkPlotBase::autoFit()
{
    autoScaleIfNeeded();
}

// ==================== 图例操作 ====================

// 设置图例可见性
void vtkPlotBase::setLegendVisible(bool visible)
{
    m_legendVisible = visible;
    m_legendActor->SetVisibility(visible ? 1 : 0);
    render();
}

// 设置图例位置
void vtkPlotBase::setLegendPosition(LegendPosition pos)
{
    m_legendPosition = pos;
    updateLegendPosition();
    render();
}

// 设置曲线名称（用于图例显示）
void vtkPlotBase::setCurveName(const QString &curveId, const QString &name)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].name = name;
        updateLegend();
    }
}

// 设置标记名称（用于图例显示）
void vtkPlotBase::setMarkerName(const QString &markerId, const QString &name)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].name = name;
        updateLegend();
    }
}

// 重置视角（恢复默认相机位置）
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

// 重置坐标轴范围（自动适应数据）
void vtkPlotBase::resetAxisRange()
{
    qDebug() << "resetAxisRange: triggering auto scale";
    autoScaleIfNeeded();
}

// 前视图（从Z轴方向看）
void vtkPlotBase::setViewFront()
{
    qDebug() << "setViewFront: front view (Z-axis)";
    vtkCamera *camera = m_renderer->GetActiveCamera();
    
    // 计算焦点（数据边界中心）
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    
    // 保持当前距离
    double distance = camera->GetDistance();
    
    // 设置相机位置（从Z轴方向）
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0], focalPoint[1], focalPoint[2] + distance);
    camera->SetViewUp(0, 1, 0);  // Y轴向上
    
    render();
}

// 俯视图（从Y轴方向看）
void vtkPlotBase::setViewTop()
{
    qDebug() << "setViewTop: top view (Y-axis)";
    vtkCamera *camera = m_renderer->GetActiveCamera();
    
    // 计算焦点（数据边界中心）
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    
    // 保持当前距离
    double distance = camera->GetDistance();
    
    // 设置相机位置（从Y轴方向）
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0], focalPoint[1] + distance, focalPoint[2]);
    camera->SetViewUp(0, 0, -1);  // Z轴向下
    
    render();
}

// 侧视图（从X轴方向看）
void vtkPlotBase::setViewSide()
{
    qDebug() << "setViewSide: side view (X-axis)";
    vtkCamera *camera = m_renderer->GetActiveCamera();
    
    // 计算焦点（数据边界中心）
    double focalPoint[3] = {
        (m_xMin + m_xMax) / 2.0,
        (m_yMin + m_yMax) / 2.0,
        (m_zMin + m_zMax) / 2.0
    };
    
    // 保持当前距离
    double distance = camera->GetDistance();
    
    // 设置相机位置（从X轴方向）
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0] + distance, focalPoint[1], focalPoint[2]);
    camera->SetViewUp(0, 0, 1);  // Z轴向上
    
    render();
}

// 重置相机
void vtkPlotBase::resetCamera()
{
    m_renderer->ResetCamera();
    render();
}

// ==================== 曲线操作 ====================

// 添加曲线（QVector3D点集）
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

// 添加曲线（分离的X/Y/Z数组）
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

// 设置曲线可见性
void vtkPlotBase::setCurveVisible(const QString &curveId, bool visible)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].visible = visible;
        m_curves[curveId].actor->SetVisibility(visible ? 1 : 0);
        updateLegend();
    }
}

// 设置曲线颜色
void vtkPlotBase::setCurveColor(const QString &curveId, const QColor &color)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        render();
    }
}

// 设置曲线线宽
void vtkPlotBase::setCurveLineWidth(const QString &curveId, double lineWidth)
{
    if (m_curves.contains(curveId)) {
        m_curves[curveId].actor->GetProperty()->SetLineWidth(lineWidth);
        render();
    }
}

// 更新曲线数据
void vtkPlotBase::updateCurveData(const QString &curveId, const QVector<QVector3D> &points)
{
    if (!m_curves.contains(curveId) || points.size() < 2) return;

    CurveData &curve = m_curves[curveId];

    // 创建新点集
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

    curve.polyData->SetPoints(pts);
    curve.polyData->SetLines(cells);
    curve.polyData->Modified();
    autoScaleIfNeeded();
    render();
}

// 移除曲线
void vtkPlotBase::removeCurve(const QString &curveId)
{
    if (m_curves.contains(curveId)) {
        m_renderer->RemoveActor(m_curves[curveId].actor);
        m_curves.remove(curveId);
        autoScaleIfNeeded();
        updateLegend();
    }
}

// 清除所有曲线
void vtkPlotBase::clearAllCurves()
{
    for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
        m_renderer->RemoveActor(it->actor);
    }
    m_curves.clear();
    autoScaleIfNeeded();
    updateLegend();
}

// 获取所有曲线ID
QStringList vtkPlotBase::getCurveIds() const
{
    return m_curves.keys();
}

// ==================== 空心环标记操作 ====================

// 添加空心环标记
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

// 设置标记可见性
void vtkPlotBase::setMarkerVisible(const QString &markerId, bool visible)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].visible = visible;
        m_markers[markerId].follower->SetVisibility(visible ? 1 : 0);
        autoScaleIfNeeded();
        updateLegend();
    }
}

// 设置标记颜色
void vtkPlotBase::setMarkerColor(const QString &markerId, const QColor &color)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].follower->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        render();
    }
}

// 设置标记半径
void vtkPlotBase::setMarkerRadius(const QString &markerId, double radius)
{
    if (!m_markers.contains(markerId)) return;

    MarkerData &marker = m_markers[markerId];
    marker.radius = radius;

    // 使用新半径重新创建圆盘源
    vtkSmartPointer<vtkDiskSource> diskSource = vtkSmartPointer<vtkDiskSource>::New();
    diskSource->SetInnerRadius(radius * 0.6);
    diskSource->SetOuterRadius(radius);
    diskSource->SetCircumferentialResolution(64);
    diskSource->Update();

    marker.polyData = diskSource->GetOutput();
    marker.mapper->SetInputConnection(diskSource->GetOutputPort());
    render();
}

// 设置标记线宽
void vtkPlotBase::setMarkerLineWidth(const QString &markerId, double lineWidth)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].lineWidth = lineWidth;
        m_markers[markerId].follower->GetProperty()->SetLineWidth(lineWidth);
        render();
    }
}

// 更新标记位置
void vtkPlotBase::updateMarkerPosition(const QString &markerId, const QVector3D &position)
{
    if (m_markers.contains(markerId)) {
        m_markers[markerId].follower->SetPosition(position.x(), position.y(), position.z());
        autoScaleIfNeeded();
        render();
    }
}

// 移除标记
void vtkPlotBase::removeMarker(const QString &markerId)
{
    if (m_markers.contains(markerId)) {
        m_renderer->RemoveActor(m_markers[markerId].follower);
        m_markers.remove(markerId);
        autoScaleIfNeeded();
        updateLegend();
    }
}

// 清除所有标记
void vtkPlotBase::clearAllMarkers()
{
    for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
        m_renderer->RemoveActor(it->follower);
    }
    m_markers.clear();
    autoScaleIfNeeded();
    updateLegend();
}

// 获取所有标记ID
QStringList vtkPlotBase::getMarkerIds() const
{
    return m_markers.keys();
}

// ==================== 填充圆标记操作 ====================

// 添加填充圆标记
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

// 设置填充标记可见性（委托给通用标记函数）
void vtkPlotBase::setFilledMarkerVisible(const QString &markerId, bool visible)
{
    setMarkerVisible(markerId, visible);
}

// 设置填充标记颜色（委托给通用标记函数）
void vtkPlotBase::setFilledMarkerColor(const QString &markerId, const QColor &color)
{
    setMarkerColor(markerId, color);
}

// 设置填充标记半径
void vtkPlotBase::setFilledMarkerRadius(const QString &markerId, double radius)
{
    if (!m_markers.contains(markerId)) return;

    MarkerData &marker = m_markers[markerId];
    if (!marker.filled) return;  // 仅用于填充标记

    marker.radius = radius;

    // 使用新半径重新创建多边形源
    vtkSmartPointer<vtkRegularPolygonSource> polygonSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    polygonSource->SetNumberOfSides(64);
    polygonSource->SetRadius(radius);
    polygonSource->SetCenter(0, 0, 0);
    polygonSource->Update();

    marker.polyData = polygonSource->GetOutput();
    marker.mapper->SetInputConnection(polygonSource->GetOutputPort());
    render();
}

// 更新填充标记位置（委托给通用标记函数）
void vtkPlotBase::updateFilledMarkerPosition(const QString &markerId, const QVector3D &position)
{
    updateMarkerPosition(markerId, position);
}

// 移除填充标记（委托给通用标记函数）
void vtkPlotBase::removeFilledMarker(const QString &markerId)
{
    removeMarker(markerId);
}

// 清除所有填充标记
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
    clearAllSurfaces();
    clearAllHeatmapSurfaces();
}

// ==================== 曲面操作 ====================

// 添加曲面（QVector3D网格点）
QString vtkPlotBase::addSurface(const QVector<QVector3D> &points, int nx, int ny,
                                const QColor &color, double opacity)
{
    // 参数校验：至少需要4个点构成一个四边形，网格维度至少2x2
    if (points.size() < 4 || nx < 2 || ny < 2) return QString();
    // 验证点数与网格维度是否匹配
    if (points.size() != nx * ny) return QString();

    // 生成唯一标识符
    QString id = generateId();
    
    // 初始化曲面数据结构
    SurfaceData surface;
    surface.id = id;
    surface.visible = true;      // 默认可见
    surface.opacity = opacity;   // 不透明度（0.0透明 - 1.0不透明）

    // ==================== 创建点集 ====================
    // 将QVector3D点集转换为VTK点集格式
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    for (const auto &pt : points) {
        pts->InsertNextPoint(pt.x(), pt.y(), pt.z());
    }

    // ==================== 创建多边形网格 ====================
    // 使用四边形（vtkQuad）单元构建网格
    // 网格拓扑：nx列 x ny行的点阵，生成 (nx-1) x (ny-1) 个四边形单元
    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    for (int j = 0; j < ny - 1; ++j) {        // 遍历行（Y方向）
        for (int i = 0; i < nx - 1; ++i) {    // 遍历列（X方向）
            // 创建四边形单元
            vtkSmartPointer<vtkQuad> quad = vtkSmartPointer<vtkQuad>::New();
            
            // 计算当前网格左上角的点索引
            // 点的排列顺序：按行优先，即 [0,1,2,...,nx-1] 为第一行，[nx,nx+1,...] 为第二行
            int idx = j * nx + i;
            
            // 设置四边形四个顶点的索引（逆时针顺序）
            //    idx+nx  --- idx+nx+1
            //       |          |
            //      idx  --- idx+1
            quad->GetPointIds()->SetId(0, idx);           // 左下角
            quad->GetPointIds()->SetId(1, idx + 1);       // 右下角
            quad->GetPointIds()->SetId(2, idx + nx + 1);  // 右上角
            quad->GetPointIds()->SetId(3, idx + nx);      // 左上角
            
            // 将四边形单元添加到单元数组
            polys->InsertNextCell(quad);
        }
    }

    // ==================== 创建多边形数据 ====================
    // vtkPolyData是VTK中表示多边形数据的核心类
    surface.polyData = vtkSmartPointer<vtkPolyData>::New();
    surface.polyData->SetPoints(pts);    // 设置顶点坐标
    surface.polyData->SetPolys(polys);   // 设置多边形单元

    // ==================== 创建映射器 ====================
    // 映射器负责将数据转换为图形基元
    surface.mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    surface.mapper->SetInputData(surface.polyData);

    // ==================== 创建演员 ====================
    // 演员代表场景中可渲染的对象
    surface.actor = vtkSmartPointer<vtkActor>::New();
    surface.actor->SetMapper(surface.mapper);
    
    // 设置外观属性
    surface.actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());  // 颜色
    surface.actor->GetProperty()->SetOpacity(opacity);                                    // 不透明度
    surface.actor->GetProperty()->SetInterpolationToPhong();  // Phong着色（平滑过渡）

    // ==================== 添加到场景 ====================
    m_renderer->AddActor(surface.actor);  // 添加演员到渲染器
    m_surfaces[id] = surface;              // 保存到曲面映射表
    
    autoScaleIfNeeded();  // 自动调整坐标轴范围
    updateLegend();       // 更新图例

    return id;
}

// 添加曲面（分离的X/Y/Z数组）
QString vtkPlotBase::addSurface(const QVector<double> &x, const QVector<double> &y, const QVector<double> &z,
                                int nx, int ny, const QColor &color, double opacity)
{
    if (x.size() != y.size() || x.size() != z.size() || x.size() < 4) return QString();
    if (x.size() != nx * ny) return QString();

    QVector<QVector3D> points;
    for (int i = 0; i < x.size(); ++i) {
        points.append(QVector3D(x[i], y[i], z[i]));
    }
    return addSurface(points, nx, ny, color, opacity);
}

// 设置曲面可见性
void vtkPlotBase::setSurfaceVisible(const QString &surfaceId, bool visible)
{
    if (m_surfaces.contains(surfaceId)) {
        m_surfaces[surfaceId].visible = visible;
        m_surfaces[surfaceId].actor->SetVisibility(visible ? 1 : 0);
        updateLegend();
    }
}

// 设置曲面颜色
void vtkPlotBase::setSurfaceColor(const QString &surfaceId, const QColor &color)
{
    if (m_surfaces.contains(surfaceId)) {
        m_surfaces[surfaceId].actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        render();
    }
}

// 设置曲面不透明度
void vtkPlotBase::setSurfaceOpacity(const QString &surfaceId, double opacity)
{
    if (m_surfaces.contains(surfaceId)) {
        m_surfaces[surfaceId].opacity = opacity;
        m_surfaces[surfaceId].actor->GetProperty()->SetOpacity(opacity);
        render();
    }
}

// 设置曲面图例名称
void vtkPlotBase::setSurfaceName(const QString &surfaceId, const QString &name)
{
    if (m_surfaces.contains(surfaceId)) {
        m_surfaces[surfaceId].name = name;
        updateLegend();
    }
}

// 移除曲面
void vtkPlotBase::removeSurface(const QString &surfaceId)
{
    if (m_surfaces.contains(surfaceId)) {
        m_renderer->RemoveActor(m_surfaces[surfaceId].actor);
        m_surfaces.remove(surfaceId);
        autoScaleIfNeeded();
        updateLegend();
    }
}

// 清除所有曲面
void vtkPlotBase::clearAllSurfaces()
{
    for (auto it = m_surfaces.begin(); it != m_surfaces.end(); ++it) {
        m_renderer->RemoveActor(it->actor);
    }
    m_surfaces.clear();
    autoScaleIfNeeded();
    updateLegend();
}

// 获取所有曲面ID
QStringList vtkPlotBase::getSurfaceIds() const
{
    return m_surfaces.keys();
}

// ==================== 热力图曲面操作 ====================

// 创建彩虹颜色查找表
vtkSmartPointer<vtkLookupTable> createRainbowLookupTable(double zMin, double zMax)
{
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetRange(zMin, zMax);
    lookupTable->SetNumberOfTableValues(256);
    
    // 彩虹色：蓝 -> 青 -> 绿 -> 黄 -> 红
    for (int i = 0; i < 256; ++i) {
        double t = i / 255.0;
        double r, g, b;
        
        if (t < 0.25) {
            // 蓝到青
            r = 0.0;
            g = t * 4.0;
            b = 1.0;
        } else if (t < 0.5) {
            // 青到绿
            r = 0.0;
            g = 1.0;
            b = 1.0 - (t - 0.25) * 4.0;
        } else if (t < 0.75) {
            // 绿到黄
            r = (t - 0.5) * 4.0;
            g = 1.0;
            b = 0.0;
        } else {
            // 黄到红
            r = 1.0;
            g = 1.0 - (t - 0.75) * 4.0;
            b = 0.0;
        }
        
        lookupTable->SetTableValue(i, r, g, b, 1.0);
    }
    
    lookupTable->Build();
    return lookupTable;
}

// 添加热力图曲面（QVector3D网格点）
// 根据Y值（高度）映射颜色，生成等高线投影，并显示颜色条
QString vtkPlotBase::addHeatmapSurface(const QVector<QVector3D> &points, int nx, int ny,
                                       const QString &colorBarTitle)
{
    // ==================== 参数校验 ====================
    // 至少需要4个点构成一个四边形，网格维度至少2x2
    if (points.size() < 4 || nx < 2 || ny < 2) return QString();
    // 验证点数与网格维度是否匹配
    if (points.size() != nx * ny) return QString();

    // ==================== 初始化曲面数据 ====================
    QString id = generateId();
    HeatmapSurfaceData surface;
    surface.id = id;
    surface.visible = true;           // 默认可见
    surface.opacity = 1.0;            // 完全不透明
    surface.contourVisible = true;    // 默认显示等高线
    surface.contourCount = 5;         // 默认5条等高线

    // ==================== 计算高度范围（Y值） ====================
    // 高度范围用于颜色映射和等高线生成
    double heightMin = std::numeric_limits<double>::max();
    double heightMax = std::numeric_limits<double>::lowest();
    for (const auto &pt : points) {
        heightMin = std::min(heightMin, static_cast<double>(pt.y()));
        heightMax = std::max(heightMax, static_cast<double>(pt.y()));
    }
    surface.zMin = heightMin;  // 保存高度范围（用于后续等高线重建）
    surface.zMax = heightMax;

    // ==================== 创建点集 ====================
    // 将QVector3D点集转换为VTK点集格式
    // 曲面在ZX平面展开，Y作为高度值
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    for (const auto &pt : points) {
        pts->InsertNextPoint(pt.x(), pt.y(), pt.z());
    }

    // ==================== 创建多边形网格 ====================
    // 使用四边形（vtkQuad）单元构建网格
    // 网格拓扑：nx列 x ny行的点阵，生成 (nx-1) x (ny-1) 个四边形单元
    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    for (int j = 0; j < ny - 1; ++j) {        // 遍历行
        for (int i = 0; i < nx - 1; ++i) {    // 遍历列
            // 创建四边形单元
            vtkSmartPointer<vtkQuad> quad = vtkSmartPointer<vtkQuad>::New();
            
            // 计算当前网格左下角的点索引（行优先排列）
            int idx = j * nx + i;
            
            // 设置四边形四个顶点的索引（逆时针顺序）
            quad->GetPointIds()->SetId(0, idx);           // 左下角
            quad->GetPointIds()->SetId(1, idx + 1);       // 右下角
            quad->GetPointIds()->SetId(2, idx + nx + 1);  // 右上角
            quad->GetPointIds()->SetId(3, idx + nx);      // 左上角
            polys->InsertNextCell(quad);
        }
    }

    // ==================== 创建多边形数据 ====================
    surface.polyData = vtkSmartPointer<vtkPolyData>::New();
    surface.polyData->SetPoints(pts);     // 设置顶点坐标
    surface.polyData->SetPolys(polys);    // 设置多边形单元
    
    // ==================== 添加标量数据（高度值） ====================
    // 标量数据用于颜色映射：Y值（高度）决定颜色
    // 每个点对应一个标量值，映射到彩虹色查找表
    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetNumberOfComponents(1);   // 单分量标量
    scalars->SetName("Height");          // 标量名称
    for (const auto &pt : points) {
        scalars->InsertNextValue(pt.y());  // Y值作为标量
    }
    surface.polyData->GetPointData()->SetScalars(scalars);

    // ==================== 创建颜色查找表 ====================
    // 彩虹色渐变：蓝 → 青 → 绿 → 黄 → 红
    // 高度值低显示蓝色，高度值高显示红色
    surface.lookupTable = createRainbowLookupTable(heightMin, heightMax);

    // ==================== 创建映射器 ====================
    // 映射器负责将数据转换为图形基元，并应用颜色映射
    surface.mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    surface.mapper->SetInputData(surface.polyData);
    surface.mapper->SetScalarModeToUsePointData();       // 使用点数据中的标量
    surface.mapper->SetScalarRange(heightMin, heightMax); // 设置标量范围
    surface.mapper->SetLookupTable(surface.lookupTable);  // 应用颜色查找表
    surface.mapper->UseLookupTableScalarRangeOn();        // 使用查找表的标量范围

    // ==================== 创建演员 ====================
    surface.actor = vtkSmartPointer<vtkActor>::New();
    surface.actor->SetMapper(surface.mapper);
    surface.actor->GetProperty()->SetInterpolationToPhong();  // Phong着色（平滑过渡）

    // ==================== 添加到场景 ====================
    m_renderer->AddActor(surface.actor);
    
    // 先添加曲面到map（让autoScaleIfNeeded能计算边界）
    m_heatmapSurfaces[id] = surface;
    
    // ==================== 自动调整坐标轴范围 ====================
    // 计算所有数据的边界，更新坐标轴显示范围
    autoScaleIfNeeded();
    
    // ==================== 创建等高线投影 ====================
    // 等高线投影到坐标系Y轴最小值平面（自适应）
    // 等高线颜色与曲面对应高度的颜色一致
    m_heatmapSurfaces[id].contourBaseY = m_yMin;  // 投影到坐标系Y轴最小值
    createContourProjection(m_heatmapSurfaces[id], heightMin, heightMax);
    
    // ==================== 更新颜色条 ====================
    // 颜色条显示在画面右侧，标题为colorBarTitle
    updateScalarBar(heightMin, heightMax, colorBarTitle);

    return id;
}

// 添加热力图曲面（分离的X/Y/Z数组）
QString vtkPlotBase::addHeatmapSurface(const QVector<double> &x, const QVector<double> &y, const QVector<double> &z,
                                       int nx, int ny, const QString &colorBarTitle)
{
    if (x.size() != y.size() || x.size() != z.size() || x.size() < 4) return QString();
    if (x.size() != nx * ny) return QString();

    QVector<QVector3D> points;
    for (int i = 0; i < x.size(); ++i) {
        points.append(QVector3D(x[i], y[i], z[i]));
    }
    return addHeatmapSurface(points, nx, ny, colorBarTitle);
}

// 更新颜色条
void vtkPlotBase::updateScalarBar(double zMin, double zMax, const QString &title)
{
    if (!m_scalarBarActor) {
        // 创建颜色条演员
        m_scalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
        
        // 设置标签数量（显示5个刻度值）
        m_scalarBarActor->SetNumberOfLabels(5);
        
        // 设置为垂直方向
        m_scalarBarActor->SetOrientationToVertical();
        
        // 设置尺寸和位置（右侧）
        m_scalarBarActor->SetWidth(0.08);    // 宽度占屏幕8%
        m_scalarBarActor->SetHeight(0.45);   // 高度占屏幕45%
        m_scalarBarActor->SetPosition(0.90, 0.30);  // 右侧，底部30%
        
        // 设置文本属性（白色文字）
        m_scalarBarActor->GetTitleTextProperty()->SetColor(1, 1, 1);  // 标题颜色
        m_scalarBarActor->GetTitleTextProperty()->SetFontSize(12);   // 标题字号
        m_scalarBarActor->GetLabelTextProperty()->SetColor(1, 1, 1);  // 标签颜色
        m_scalarBarActor->GetLabelTextProperty()->SetFontSize(10);    // 标签字号
        
        // 设置标题位置在颜色条右侧
        m_scalarBarActor->SetTextPositionToSucceedScalarBar();
        m_scalarBarActor->SetVerticalTitleSeparation(10);  // 标题与颜色条间距10像素
        m_scalarBarActor->SetTextPad(2);  // 文本框填充2像素
        
        // 添加到渲染器
        m_renderer->AddViewProp(m_scalarBarActor);
    }
    
    // 使用第一个热力图曲面的颜色表
    if (!m_heatmapSurfaces.isEmpty()) {
        m_scalarBarActor->SetLookupTable(m_heatmapSurfaces.first().lookupTable);
    }
    m_scalarBarActor->SetTitle(title.toUtf8().constData());
}

// 设置热力图曲面可见性
void vtkPlotBase::setHeatmapSurfaceVisible(const QString &surfaceId, bool visible)
{
    if (m_heatmapSurfaces.contains(surfaceId)) {
        m_heatmapSurfaces[surfaceId].visible = visible;
        m_heatmapSurfaces[surfaceId].actor->SetVisibility(visible ? 1 : 0);
        render();
    }
}

// 设置热力图曲面不透明度
void vtkPlotBase::setHeatmapSurfaceOpacity(const QString &surfaceId, double opacity)
{
    if (m_heatmapSurfaces.contains(surfaceId)) {
        m_heatmapSurfaces[surfaceId].opacity = opacity;
        m_heatmapSurfaces[surfaceId].actor->GetProperty()->SetOpacity(opacity);
        render();
    }
}

// 设置热力图曲面图例名称
void vtkPlotBase::setHeatmapSurfaceName(const QString &surfaceId, const QString &name)
{
    if (m_heatmapSurfaces.contains(surfaceId)) {
        m_heatmapSurfaces[surfaceId].name = name;
    }
}

// 设置颜色条可见性
void vtkPlotBase::setHeatmapColorBarVisible(bool visible)
{
    if (m_scalarBarActor) {
        m_scalarBarActor->SetVisibility(visible ? 1 : 0);
        render();
    }
}

// 设置颜色条标题
void vtkPlotBase::setHeatmapColorBarTitle(const QString &title)
{
    if (m_scalarBarActor) {
        m_scalarBarActor->SetTitle(title.toUtf8().constData());
        render();
    }
}

// 移除热力图曲面
void vtkPlotBase::removeHeatmapSurface(const QString &surfaceId)
{
    if (m_heatmapSurfaces.contains(surfaceId)) {
        m_renderer->RemoveActor(m_heatmapSurfaces[surfaceId].actor);
        // 移除等高线演员
        if (m_heatmapSurfaces[surfaceId].contourActor) {
            m_renderer->RemoveActor(m_heatmapSurfaces[surfaceId].contourActor);
        }
        m_heatmapSurfaces.remove(surfaceId);
        
        // 如果没有热力图曲面了，隐藏颜色条
        if (m_heatmapSurfaces.isEmpty() && m_scalarBarActor) {
            m_scalarBarActor->SetVisibility(0);
        }
        
        autoScaleIfNeeded();
        render();
    }
}

// 清除所有热力图曲面
void vtkPlotBase::clearAllHeatmapSurfaces()
{
    for (auto it = m_heatmapSurfaces.begin(); it != m_heatmapSurfaces.end(); ++it) {
        m_renderer->RemoveActor(it->actor);
        // 移除等高线演员
        if (it->contourActor) {
            m_renderer->RemoveActor(it->contourActor);
        }
    }
    m_heatmapSurfaces.clear();
    
    // 隐藏颜色条
    if (m_scalarBarActor) {
        m_scalarBarActor->SetVisibility(0);
    }
    
    autoScaleIfNeeded();
    render();
}

// 获取所有热力图曲面ID
QStringList vtkPlotBase::getHeatmapSurfaceIds() const
{
    return m_heatmapSurfaces.keys();
}

// 创建等高线投影（投影到坐标系Y轴最小值平面）
// 等高线颜色与曲面对应高度的颜色一致（使用相同的lookupTable）
void vtkPlotBase::createContourProjection(HeatmapSurfaceData &surface, double heightMin, double heightMax)
{
    // ==================== 创建等高线过滤器 ====================
    // vtkContourFilter根据标量值生成等值线/等值面
    // 输入曲面数据，输出等高线几何
    vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
    contourFilter->SetInputData(surface.polyData);   // 输入曲面多边形数据
    contourFilter->ComputeNormalsOff();              // 不计算法线（不需要）
    
    // ==================== 设置等高线值 ====================
    // 按指定数量均匀分布等高线
    // 等高线不包含边界值（heightMin和heightMax），只取中间值
    int numContours = surface.contourCount;          // 等高线数量
    double range = heightMax - heightMin;            // 高度范围
    double step = range / (numContours + 1);         // 步长（均匀分布）
    
    // 设置每条等高线的高度值
    // 例如：范围0-10，5条等高线，步长=10/6≈1.67
    // 等高线位置：1.67, 3.33, 5.0, 6.67, 8.33
    for (int i = 1; i <= numContours; ++i) {
        contourFilter->SetValue(i - 1, heightMin + i * step);
    }
    contourFilter->Update();  // 执行过滤器，生成等高线
    
    // ==================== 准备投影数据 ====================
    // 创建投影后的等高线数据结构
    vtkSmartPointer<vtkPolyData> projectedContour = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> projectedPoints = vtkSmartPointer<vtkPoints>::New();
    
    // 创建标量数组，保存原始高度值（用于着色）
    // 投影后点的Y坐标变为contourBaseY，需要单独保存原始高度用于颜色映射
    vtkSmartPointer<vtkFloatArray> projectedScalars = vtkSmartPointer<vtkFloatArray>::New();
    projectedScalars->SetNumberOfComponents(1);      // 单分量标量
    projectedScalars->SetName("Height");             // 标量名称
    
    // 获取等高线过滤器的输出
    vtkPolyData* contourOutput = contourFilter->GetOutput();
    vtkPoints* contourPoints = contourOutput->GetPoints();
    
    // ==================== 投影等高线点 ====================
    // 将等高线从曲面位置投影到坐标系Y轴最小值平面
    // 保留X和Z坐标，Y坐标设为contourBaseY（坐标系底部）
    for (vtkIdType i = 0; i < contourPoints->GetNumberOfPoints(); ++i) {
        double pt[3];
        contourPoints->GetPoint(i, pt);              // 获取原始点坐标
        
        // 投影到坐标系Y轴最小值平面
        // pt[0]: X坐标不变
        // pt[1]: 原始高度值 → 投影后变为contourBaseY
        // pt[2]: Z坐标不变
        projectedPoints->InsertNextPoint(pt[0], surface.contourBaseY, pt[2]);
        
        // 保存原始高度值（pt[1]）作为标量，用于颜色映射
        // 这样投影后的等高线仍显示原始高度对应的颜色
        projectedScalars->InsertNextValue(pt[1]);
    }
    
    // ==================== 构建投影等高线数据 ====================
    projectedContour->SetPoints(projectedPoints);           // 设置投影后的点
    projectedContour->SetLines(contourOutput->GetLines());   // 复制原始等高线的拓扑（线段连接）
    projectedContour->GetPointData()->SetScalars(projectedScalars);  // 设置标量数据
    
    // ==================== 创建等高线映射器 ====================
    // 使用与曲面相同的lookupTable，确保颜色一致
    surface.contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    surface.contourMapper->SetInputData(projectedContour);
    surface.contourMapper->SetScalarModeToUsePointData();          // 使用点数据中的标量
    surface.contourMapper->SetScalarRange(heightMin, heightMax);    // 设置标量范围
    surface.contourMapper->SetLookupTable(surface.lookupTable);     // 使用曲面的颜色查找表
    surface.contourMapper->UseLookupTableScalarRangeOn();           // 使用查找表的标量范围
    surface.contourMapper->ScalarVisibilityOn();                    // 启用标量着色
    
    // ==================== 创建等高线演员 ====================
    surface.contourActor = vtkSmartPointer<vtkActor>::New();
    surface.contourActor->SetMapper(surface.contourMapper);
    // 注意：不调用SetColor()，让mapper使用lookupTable着色
    // 如果调用SetColor()会覆盖标量着色效果
    surface.contourActor->GetProperty()->SetLineWidth(1.5);         // 线宽
    surface.contourActor->SetVisibility(surface.contourVisible ? 1 : 0);  // 设置可见性
    
    // ==================== 添加到场景 ====================
    m_renderer->AddActor(surface.contourActor);
}

// 设置等高线可见性
void vtkPlotBase::setHeatmapContourVisible(const QString &surfaceId, bool visible)
{
    if (m_heatmapSurfaces.contains(surfaceId)) {
        m_heatmapSurfaces[surfaceId].contourVisible = visible;
        if (m_heatmapSurfaces[surfaceId].contourActor) {
            m_heatmapSurfaces[surfaceId].contourActor->SetVisibility(visible ? 1 : 0);
        }
        render();
    }
}

// 设置等高线数量
void vtkPlotBase::setHeatmapContourCount(const QString &surfaceId, int count)
{
    if (!m_heatmapSurfaces.contains(surfaceId)) return;
    if (count < 1) return;
    
    HeatmapSurfaceData &surface = m_heatmapSurfaces[surfaceId];
    surface.contourCount = count;
    
    // 重新创建等高线
    if (surface.contourActor) {
        m_renderer->RemoveActor(surface.contourActor);
    }
    createContourProjection(surface, surface.zMin, surface.zMax);
    render();
}

