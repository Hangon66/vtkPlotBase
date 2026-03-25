#include "vtkmarker.h"

// Qt 头文件
#include <QVector3D>
#include <QColor>
#include <QUuid>

// VTK 头文件
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>

// vtkMarker 构造函数（空心环）
vtkMarker::vtkMarker(const QVector3D &position, const QColor &color, 
                     double screenSize, double lineWidth)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name(QString())
    , m_color(color)
    , m_filled(false)
    , m_lineWidth(lineWidth)
    , m_visible(true)
    , m_radius(1.0)
    , m_relativeRadius(0.0)
    , m_screenSize(screenSize)
    , m_sizeMode(MarkerSizeMode::Screen)
    , m_xMin(-1.0)
    , m_xMax(1.0)
    , m_renderer(nullptr)
    , m_camera(nullptr)
{
    // 创建圆盘源（空心环），使用单位半径
    vtkSmartPointer<vtkDiskSource> diskSource = vtkSmartPointer<vtkDiskSource>::New();
    diskSource->SetInnerRadius(0.6);   // 内半径（孔）
    diskSource->SetOuterRadius(1.0);   // 外半径（边）
    diskSource->SetCircumferentialResolution(64);
    diskSource->Update();

    m_polyData = diskSource->GetOutput();

    // 创建映射器
    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->SetInputConnection(diskSource->GetOutputPort());

    // 创建跟随者
    m_follower = vtkSmartPointer<vtkFollower>::New();
    m_follower->SetMapper(m_mapper);
    m_follower->SetPosition(position.x(), position.y(), position.z());
    m_follower->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    m_follower->GetProperty()->SetLineWidth(lineWidth);
    
    // 设置默认缩放
    m_follower->SetScale(1.0, 1.0, 1.0);
}

// vtkMarker 析构函数
vtkMarker::~vtkMarker()
{
    if (m_renderer) {
        removeFromRenderer(m_renderer);
    }
}

// 设置可见性
void vtkMarker::setVisible(bool visible)
{
    m_visible = visible;
    m_follower->SetVisibility(visible ? 1 : 0);
}

// 设置颜色
void vtkMarker::setColor(const QColor &color)
{
    m_color = color;
    m_follower->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
}

// 添加到渲染器
void vtkMarker::addToRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        m_renderer = renderer;
        if (m_camera) {
            m_follower->SetCamera(m_camera);
        } else {
            m_follower->SetCamera(renderer->GetActiveCamera());
            m_camera = renderer->GetActiveCamera();
        }
        renderer->AddActor(m_follower);
    }
}

// 从渲染器移除
void vtkMarker::removeFromRenderer(vtkRenderer *renderer)
{
    if (renderer) {
        renderer->RemoveActor(m_follower);
        if (m_renderer == renderer) {
            m_renderer = nullptr;
        }
    }
}

// 触发重渲染
void vtkMarker::render()
{
    if (m_renderer) {
        m_renderer->GetRenderWindow()->Render();
    }
}

// 设置位置
void vtkMarker::setPosition(const QVector3D &position)
{
    m_follower->SetPosition(position.x(), position.y(), position.z());
}

// 获取位置
QVector3D vtkMarker::position() const
{
    double pos[3];
    m_follower->GetPosition(pos);
    return QVector3D(pos[0], pos[1], pos[2]);
}

// 设置线宽
void vtkMarker::setLineWidth(double width)
{
    m_lineWidth = width;
    m_follower->GetProperty()->SetLineWidth(width);
}

// 设置相机
void vtkMarker::setCamera(vtkCamera *camera)
{
    m_camera = camera;
    m_follower->SetCamera(camera);
}

// 设置坐标轴范围
void vtkMarker::setAxisRange(double xMin, double xMax)
{
    m_xMin = xMin;
    m_xMax = xMax;
    
    // 如果是相对半径模式，重新计算
    if (m_sizeMode == MarkerSizeMode::Relative) {
        setRelativeRadius(m_xMin, m_xMax, m_relativeRadius);
    }
}

// 设置绝对半径
void vtkMarker::setRadius(double radius)
{
    m_radius = radius;
    m_relativeRadius = 0.0;
    m_screenSize = 0.0;
    m_sizeMode = MarkerSizeMode::Absolute;

    if (m_filled) {
        recreateFilledGeometry(radius);
    } else {
        recreateHollowGeometry(radius);
    }
}

// 设置相对半径
void vtkMarker::setRelativeRadius(double xMin, double xMax, double ratio)
{
    m_xMin = xMin;
    m_xMax = xMax;
    m_relativeRadius = ratio;
    m_sizeMode = MarkerSizeMode::Relative;

    double xRange = m_xMax - m_xMin;
    if (xRange < 1e-10) xRange = 1.0;
    
    double actualRadius = xRange * ratio;
    m_radius = actualRadius;

    if (m_filled) {
        recreateFilledGeometry(actualRadius);
    } else {
        recreateHollowGeometry(actualRadius);
    }
}

// 设置屏幕大小
void vtkMarker::setScreenSize(double screenSize, int windowHeight)
{
    m_screenSize = screenSize;
    m_sizeMode = MarkerSizeMode::Screen;
    
    double actualRadius = computeScreenRadius(windowHeight);
    m_radius = actualRadius;
    m_follower->SetScale(actualRadius, actualRadius, actualRadius);
}

// 更新屏幕大小
void vtkMarker::updateScreenSize(int windowHeight)
{
    if (m_sizeMode == MarkerSizeMode::Screen) {
        double actualRadius = computeScreenRadius(windowHeight);
        m_radius = actualRadius;
        m_follower->SetScale(actualRadius, actualRadius, actualRadius);
    }
}

// 设置是否填充模式
void vtkMarker::setFilled(bool filled)
{
    if (m_filled != filled) {
        m_filled = filled;
        if (filled) {
            recreateFilledGeometry(m_radius);
        } else {
            recreateHollowGeometry(m_radius);
        }
    }
}

// 计算屏幕固定半径
double vtkMarker::computeScreenRadius(int windowHeight)
{
    if (!m_camera || windowHeight <= 0) {
        return 1.0;
    }

    double pos[3];
    m_follower->GetPosition(pos);

    // 计算标记点到相机的距离
    double cameraPos[3];
    m_camera->GetPosition(cameraPos);
    double dx = pos[0] - cameraPos[0];
    double dy = pos[1] - cameraPos[1];
    double dz = pos[2] - cameraPos[2];
    double distance = sqrt(dx*dx + dy*dy + dz*dz);

    if (distance < 1e-10) distance = 1.0;

    // 计算相机视角
    double angle = m_camera->GetViewAngle() * vtkMath::Pi() / 180.0;

    // 计算世界坐标单位/像素
    double worldPerPixel = 2.0 * distance * tan(angle / 2.0) / windowHeight;

    // 计算目标半径
    return m_screenSize * worldPerPixel / 2.0;
}

// 重新创建空心环几何
void vtkMarker::recreateHollowGeometry(double radius)
{
    vtkSmartPointer<vtkDiskSource> diskSource = vtkSmartPointer<vtkDiskSource>::New();
    diskSource->SetInnerRadius(radius * 0.6);
    diskSource->SetOuterRadius(radius);
    diskSource->SetCircumferentialResolution(64);
    diskSource->Update();

    m_polyData = diskSource->GetOutput();
    m_mapper->SetInputConnection(diskSource->GetOutputPort());
}

// 重新创建填充圆几何
void vtkMarker::recreateFilledGeometry(double radius)
{
    vtkSmartPointer<vtkRegularPolygonSource> polygonSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    polygonSource->SetNumberOfSides(64);
    polygonSource->SetRadius(radius);
    polygonSource->SetCenter(0, 0, 0);
    polygonSource->Update();

    m_polyData = polygonSource->GetOutput();
    m_mapper->SetInputConnection(polygonSource->GetOutputPort());
}
