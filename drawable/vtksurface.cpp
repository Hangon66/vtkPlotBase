#include "vtksurface.h"

// Qt 头文件
#include <QVector3D>
#include <QColor>
#include <QUuid>

// VTK 头文件
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>

// vtkSurface 构造函数
vtkSurface::vtkSurface(const QVector<QVector3D> &points, int nx, int ny,
                       const QColor &color, double opacity)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name(QString())
    , m_color(color)
    , m_opacity(opacity)
    , m_visible(true)
    , m_renderer(nullptr)
{
    // 参数校验
    if (points.size() < 4 || nx < 2 || ny < 2) {
        return;
    }
    if (points.size() != nx * ny) {
        return;
    }
    
    // 创建四边形网格
    createQuadMesh(points, nx, ny);
    
    // 创建映射器
    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->SetInputData(m_polyData);
    
    // 创建演员
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    m_actor->GetProperty()->SetOpacity(opacity);
    m_actor->GetProperty()->SetInterpolationToPhong();
}

// vtkSurface 析构函数
vtkSurface::~vtkSurface()
{
    if (m_renderer) {
        removeFromRenderer(m_renderer);
    }
}

// 设置可见性
void vtkSurface::setVisible(bool visible)
{
    m_visible = visible;
    m_actor->SetVisibility(visible ? 1 : 0);
}

// 设置颜色
void vtkSurface::setColor(const QColor &color)
{
    m_color = color;
    m_actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
}

// 添加到渲染器
void vtkSurface::addToRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        m_renderer = renderer;
        renderer->AddActor(m_actor);
    }
}

// 从渲染器移除
void vtkSurface::removeFromRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        renderer->RemoveActor(m_actor);
        if (m_renderer == renderer) {
            m_renderer = nullptr;
        }
    }
}

// 触发重渲染
void vtkSurface::render()
{
    if (m_renderer) {
        m_renderer->GetRenderWindow()->Render();
    }
}

// 设置不透明度
void vtkSurface::setOpacity(double opacity)
{
    m_opacity = opacity;
    m_actor->GetProperty()->SetOpacity(opacity);
}

// 从网格点创建VTK多边形数据
void vtkSurface::createQuadMesh(const QVector<QVector3D> &points, int nx, int ny)
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
            
            // 计算当前网格左上角的点索引
            int idx = j * nx + i;
            
            // 设置四边形四个顶点的索引（逆时针顺序）
            //    idx+nx  --- idx+nx+1
            //       |          |
            //      idx  --- idx+1
            quad->GetPointIds()->SetId(0, idx);           // 左下角
            quad->GetPointIds()->SetId(1, idx + 1);       // 右下角
            quad->GetPointIds()->SetId(2, idx + nx + 1);  // 右上角
            quad->GetPointIds()->SetId(3, idx + nx);      // 左上角
            
            polys->InsertNextCell(quad);
        }
    }
    
    // 创建多边形数据
    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(pts);
    m_polyData->SetPolys(polys);
}
