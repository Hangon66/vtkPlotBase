#include "vtkheatmap.h"
#include <limits>

// Qt 头文件
#include <QVector3D>
#include <QColor>
#include <QUuid>
#include <QString>

// VTK 头文件
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkPolyLine.h>
#include <vtkProperty.h>
#include <vtkPointData.h>

// vtkHeatmap 构造函数
vtkHeatmap::vtkHeatmap(const QVector<QVector3D> &points, int nx, int ny,
                       const QString &colorBarTitle)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name(QString())
    , m_opacity(1.0)
    , m_visible(true)
    , m_contourVisible(true)
    , m_contourCount(5)
    , m_contourBaseY(0.0)
    , m_zMin(0.0)
    , m_zMax(1.0)
    , m_renderer(nullptr)
    , m_scalarBarActor(nullptr)
    , m_colorBarTitle(colorBarTitle)
{
    // 参数校验
    if (points.size() < 4 || nx < 2 || ny < 2) {
        return;
    }
    if (points.size() != nx * ny) {
        return;
    }
    
    // 计算高度范围
    double heightMin = std::numeric_limits<double>::max();
    double heightMax = std::numeric_limits<double>::lowest();
    for (const auto &pt : points) {
        heightMin = std::min(heightMin, static_cast<double>(pt.y()));
        heightMax = std::max(heightMax, static_cast<double>(pt.y()));
    }
    m_zMin = heightMin;
    m_zMax = heightMax;
    
    // 创建四边形网格
    createQuadMesh(points, nx, ny);
    
    // 创建颜色查找表
    m_lookupTable = createRainbowLookupTable(heightMin, heightMax);
    
    // 创建映射器
    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->SetInputData(m_polyData);
    m_mapper->SetScalarModeToUsePointData();
    m_mapper->SetScalarRange(heightMin, heightMax);
    m_mapper->SetLookupTable(m_lookupTable);
    m_mapper->UseLookupTableScalarRangeOn();
    
    // 创建演员
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetInterpolationToPhong();
    
    // 创建等高线投影
    createContourProjection(heightMin, heightMax);
}

// vtkHeatmap 析构函数
vtkHeatmap::~vtkHeatmap()
{
    if (m_renderer) {
        removeFromRenderer(m_renderer);
    }
}

// 设置可见性
void vtkHeatmap::setVisible(bool visible)
{
    m_visible = visible;
    m_actor->SetVisibility(visible ? 1 : 0);
}

// 设置颜色（热力图颜色由标量值决定，此函数仅用于兼容）
void vtkHeatmap::setColor(const QColor &color)
{
    (void)color;  // 忽略颜色参数
}

// 添加到渲染器
void vtkHeatmap::addToRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        m_renderer = renderer;
        renderer->AddActor(m_actor);
        if (m_contourActor) {
            renderer->AddActor(m_contourActor);
        }
    }
}

// 从渲染器移除
void vtkHeatmap::removeFromRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        renderer->RemoveActor(m_actor);
        if (m_contourActor) {
            renderer->RemoveActor(m_contourActor);
        }
        if (m_renderer == renderer) {
            m_renderer = nullptr;
        }
    }
}

// 触发重渲染
void vtkHeatmap::render()
{
    if (m_renderer) {
        m_renderer->GetRenderWindow()->Render();
    }
}

// 设置不透明度
void vtkHeatmap::setOpacity(double opacity)
{
    m_opacity = opacity;
    m_actor->GetProperty()->SetOpacity(opacity);
}

// 设置等高线可见性
void vtkHeatmap::setContourVisible(bool visible)
{
    m_contourVisible = visible;
    if (m_contourActor) {
        m_contourActor->SetVisibility(visible ? 1 : 0);
    }
}

// 设置等高线数量
void vtkHeatmap::setContourCount(int count)
{
    if (count < 1) return;
    m_contourCount = count;
    updateContour();
}

// 设置等高线投影基准Y值
void vtkHeatmap::setContourBaseY(double y)
{
    m_contourBaseY = y;
    updateContour();
}

// 设置Z值范围
void vtkHeatmap::setZRange(double zMin, double zMax)
{
    m_zMin = zMin;
    m_zMax = zMax;
    updateContour();
}

// 设置颜色条标题
void vtkHeatmap::setColorBarTitle(const QString &title)
{
    m_colorBarTitle = title;
    if (m_scalarBarActor) {
        m_scalarBarActor->SetTitle(title.isEmpty() ? nullptr : title.toUtf8().constData());
    }
}

// 设置全局颜色条
void vtkHeatmap::setScalarBarActor(vtkSmartPointer<vtkScalarBarActor> &scalarBar)
{
    m_scalarBarActor = scalarBar.Get();
    if (scalarBar && m_lookupTable) {
        scalarBar->SetLookupTable(m_lookupTable);
        // 设置颜色条标题
        if (!m_colorBarTitle.isEmpty()) {
            scalarBar->SetTitle(m_colorBarTitle.toUtf8().constData());
        }
    }
}

// 更新等高线
void vtkHeatmap::updateContour()
{
    if (!m_renderer) return;
    
    // 移除旧的等高线
    if (m_contourActor) {
        m_renderer->RemoveActor(m_contourActor);
    }
    
    // 重新创建等高线
    createContourProjection(m_zMin, m_zMax);
    
    // 添加新的等高线
    if (m_contourActor) {
        m_renderer->AddActor(m_contourActor);
    }
}

// 创建彩虹颜色查找表
vtkSmartPointer<vtkLookupTable> vtkHeatmap::createRainbowLookupTable(double zMin, double zMax)
{
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetRange(zMin, zMax);
    lookupTable->SetNumberOfTableValues(256);
    
    // 彩虹色：蓝 -> 青 -> 绿 -> 黄 -> 红
    for (int i = 0; i < 256; ++i) {
        double t = i / 255.0;
        double r, g, b;
        
        if (t < 0.25) {
            r = 0.0;
            g = t * 4.0;
            b = 1.0;
        } else if (t < 0.5) {
            r = 0.0;
            g = 1.0;
            b = 1.0 - (t - 0.25) * 4.0;
        } else if (t < 0.75) {
            r = (t - 0.5) * 4.0;
            g = 1.0;
            b = 0.0;
        } else {
            r = 1.0;
            g = 1.0 - (t - 0.75) * 4.0;
            b = 0.0;
        }
        
        lookupTable->SetTableValue(i, r, g, b, 1.0);
    }
    
    lookupTable->Build();
    return lookupTable;
}

// 从网格点创建VTK多边形数据
void vtkHeatmap::createQuadMesh(const QVector<QVector3D> &points, int nx, int ny)
{
    // 创建点集
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    for (const auto &pt : points) {
        pts->InsertNextPoint(pt.x(), pt.y(), pt.z());
    }
    
    // 创建四边形网格
    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    for (int j = 0; j < ny - 1; ++j) {
        for (int i = 0; i < nx - 1; ++i) {
            vtkSmartPointer<vtkQuad> quad = vtkSmartPointer<vtkQuad>::New();
            int idx = j * nx + i;
            
            quad->GetPointIds()->SetId(0, idx);
            quad->GetPointIds()->SetId(1, idx + 1);
            quad->GetPointIds()->SetId(2, idx + nx + 1);
            quad->GetPointIds()->SetId(3, idx + nx);
            
            polys->InsertNextCell(quad);
        }
    }
    
    // 创建多边形数据
    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(pts);
    m_polyData->SetPolys(polys);
    
    // 添加标量数据（高度值）
    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetNumberOfComponents(1);
    scalars->SetName("Height");
    for (const auto &pt : points) {
        scalars->InsertNextValue(pt.y());
    }
    m_polyData->GetPointData()->SetScalars(scalars);
}

// 创建等高线投影
void vtkHeatmap::createContourProjection(double heightMin, double heightMax)
{
    if (!m_polyData) return;
    
    // 创建等高线过滤器
    vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
    contourFilter->SetInputData(m_polyData);
    contourFilter->ComputeNormalsOff();
    
    // 设置等高线值
    double range = heightMax - heightMin;
    double step = range / (m_contourCount + 1);
    
    for (int i = 1; i <= m_contourCount; ++i) {
        contourFilter->SetValue(i - 1, heightMin + i * step);
    }
    contourFilter->Update();
    
    // 准备投影数据
    vtkSmartPointer<vtkPolyData> projectedContour = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> projectedPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkFloatArray> projectedScalars = vtkSmartPointer<vtkFloatArray>::New();
    projectedScalars->SetNumberOfComponents(1);
    projectedScalars->SetName("Height");
    
    vtkPolyData* contourOutput = contourFilter->GetOutput();
    vtkPoints* contourPoints = contourOutput->GetPoints();
    
    // 投影等高线点
    for (vtkIdType i = 0; i < contourPoints->GetNumberOfPoints(); ++i) {
        double pt[3];
        contourPoints->GetPoint(i, pt);
        projectedPoints->InsertNextPoint(pt[0], m_contourBaseY, pt[2]);
        projectedScalars->InsertNextValue(pt[1]);
    }
    
    projectedContour->SetPoints(projectedPoints);
    projectedContour->SetLines(contourOutput->GetLines());
    projectedContour->GetPointData()->SetScalars(projectedScalars);
    
    // 创建等高线映射器
    m_contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_contourMapper->SetInputData(projectedContour);
    m_contourMapper->SetScalarModeToUsePointData();
    m_contourMapper->SetScalarRange(heightMin, heightMax);
    m_contourMapper->SetLookupTable(m_lookupTable);
    m_contourMapper->UseLookupTableScalarRangeOn();
    m_contourMapper->ScalarVisibilityOn();
    
    // 创建等高线演员
    m_contourActor = vtkSmartPointer<vtkActor>::New();
    m_contourActor->SetMapper(m_contourMapper);
    m_contourActor->GetProperty()->SetLineWidth(1.5);
    m_contourActor->SetVisibility(m_contourVisible ? 1 : 0);
}
