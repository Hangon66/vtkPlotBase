#include "vtkcurve.h"

// Qt 头文件
#include <QVector3D>
#include <QColor>
#include <QUuid>

// VTK 头文件
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>

// vtkCurve 构造函数
vtkCurve::vtkCurve(const QVector<QVector3D> &points, const QColor &color, double lineWidth)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name(QString())
    , m_color(color)
    , m_lineWidth(lineWidth)
    , m_visible(true)
    , m_renderer(nullptr)
{
    // 参数校验
    if (points.size() < 2) {
        return;
    }
    
    // 创建VTK多段线数据
    createPolyLine(points);
    
    // 创建映射器
    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->SetInputData(m_polyData);
    
    // 创建演员
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    m_actor->GetProperty()->SetLineWidth(lineWidth);
}

// vtkCurve 析构函数
vtkCurve::~vtkCurve()
{
    if (m_renderer) {
        removeFromRenderer(m_renderer);
    }
}

// 设置可见性
void vtkCurve::setVisible(bool visible)
{
    m_visible = visible;
    m_actor->SetVisibility(visible ? 1 : 0);
}

// 设置颜色
void vtkCurve::setColor(const QColor &color)
{
    m_color = color;
    m_actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
}

// 添加到渲染器
void vtkCurve::addToRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        m_renderer = renderer;
        renderer->AddActor(m_actor);
    }
}

// 从渲染器移除
void vtkCurve::removeFromRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        renderer->RemoveActor(m_actor);
        if (m_renderer == renderer) {
            m_renderer = nullptr;
        }
    }
}

// 触发重渲染
void vtkCurve::render()
{
    if (m_renderer) {
        m_renderer->GetRenderWindow()->Render();
    }
}

// 设置线宽
void vtkCurve::setLineWidth(double width)
{
    m_lineWidth = width;
    m_actor->GetProperty()->SetLineWidth(width);
}

// 更新曲线数据
void vtkCurve::updateData(const QVector<QVector3D> &points)
{
    if (points.size() < 2) return;
    
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
    
    // 更新多边形数据
    m_polyData->SetPoints(pts);
    m_polyData->SetLines(cells);
    m_polyData->Modified();
}

// 获取点数据
QVector<QVector3D> vtkCurve::points() const
{
    QVector<QVector3D> result;
    if (!m_polyData || !m_polyData->GetPoints()) {
        return result;
    }
    
    vtkPoints *pts = m_polyData->GetPoints();
    for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i) {
        double pt[3];
        pts->GetPoint(i, pt);
        result.append(QVector3D(pt[0], pt[1], pt[2]));
    }
    return result;
}

// 从点集创建VTK多段线数据
void vtkCurve::createPolyLine(const QVector<QVector3D> &points)
{
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
    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(pts);
    m_polyData->SetLines(cells);
}
