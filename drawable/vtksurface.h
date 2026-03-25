#ifndef VTKSURFACE_H
#define VTKSURFACE_H

#include "vtkdrawable.h"
#include <QVector3D>
#include <QColor>
#include <QUuid>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkPoints.h>
#include <vtkQuad.h>
#include <vtkCellArray.h>

class vtkRenderer;

/**
 * @brief vtkSurface 曲面类
 * 
 * 负责管理单个3D曲面（四边形网格）的创建、显示和属性修改。
 * 曲面由M×N的网格点组成，形成(M-1)×(N-1)个四边形单元。
 */
class vtkSurface : public vtkDrawable
{
public:
    /**
     * @brief 构造曲面
     * @param points 网格点集（按行优先排列）
     * @param nx X方向点数
     * @param ny Y方向点数
     * @param color 颜色
     * @param opacity 不透明度 (0.0 - 1.0)
     */
    vtkSurface(const QVector<QVector3D> &points, int nx, int ny, 
               const QColor &color, double opacity);
    
    /**
     * @brief 析构函数
     */
    ~vtkSurface() override;
    
    // vtkDrawable 接口实现
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    void setName(const QString &name) override { m_name = name; }
    void setVisible(bool visible) override;
    bool visible() const override { return m_visible; }
    void setColor(const QColor &color) override;
    QColor color() const override { return m_color; }
    void addToRenderer(vtkRenderer *renderer) override;
    void removeFromRenderer(vtkRenderer *renderer) override;
    void render() override;
    
    // 曲面特有方法
    void setOpacity(double opacity);           // 设置不透明度
    double opacity() const { return m_opacity; }  // 获取不透明度
    
    /**
     * @brief 获取VTK演员
     * @return vtkActor* 演员指针
     */
    vtkActor* actor() { return m_actor; }
    
    /**
     * @brief 获取演员（只读）
     * @return const vtkActor* 演员指针
     */
    const vtkActor* actor() const { return m_actor; }
    
    /**
     * @brief 获取多边形数据
     * @return vtkPolyData* 多边形数据指针
     */
    vtkPolyData* polyData() { return m_polyData; }
    
    /**
     * @brief 获取多边形数据（只读）
     * @return const vtkPolyData* 多边形数据指针
     */
    const vtkPolyData* polyData() const { return m_polyData; }

private:
    /**
     * @brief 从网格点创建VTK多边形数据
     * @param points 网格点集
     * @param nx X方向点数
     * @param ny Y方向点数
     */
    void createQuadMesh(const QVector<QVector3D> &points, int nx, int ny);
    
    QString m_id;                              // 唯一标识符
    QString m_name;                            // 曲面名称
    QColor m_color;                            // 颜色
    double m_opacity;                         // 不透明度
    bool m_visible;                            // 可见性
    
    // VTK 数据
    vtkSmartPointer<vtkPolyData> m_polyData;   // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> m_mapper;  // 映射器
    vtkSmartPointer<vtkActor> m_actor;         // 演员
    
    // 渲染器引用
    vtkRenderer *m_renderer;
};

#endif // VTKSURFACE_H
