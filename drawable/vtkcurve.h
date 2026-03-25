#ifndef VTKCURVE_H
#define VTKCURVE_H

#include "vtkdrawable.h"
#include <QVector3D>
#include <QVector>
#include <QMap>
#include <QUuid>
#include <QColor>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>

/**
 * @brief vtkCurve 曲线类
 * 
 * 负责管理单个3D曲线对象的创建、显示和属性修改。
 * 曲线由一系列3D点组成，使用vtkPolyLine渲染。
 */
class vtkCurve : public vtkDrawable
{
public:
    /**
     * @brief 构造曲线
     * @param points 曲线点集
     * @param color 曲线颜色
     * @param lineWidth 线宽
     */
    vtkCurve(const QVector<QVector3D> &points, const QColor &color, double lineWidth);
    
    /**
     * @brief 析构函数
     */
    ~vtkCurve() override;
    
    // ===== vtkDrawable 接口实现 =====
    /**
     * @brief 获取唯一标识符
     * @return QString 标识符
     */
    QString id() const override { return m_id; }
    /**
     * @brief 获取名称
     * @return QString 名称
     */
    QString name() const override { return m_name; }
    /**
     * @brief 设置名称
     * @param name 名称
     */
    void setName(const QString &name) override { m_name = name; }
    /**
     * @brief 设置可见性
     * @param visible 是否可见
     */
    void setVisible(bool visible) override;
    /**
     * @brief 获取可见性
     * @return bool 是否可见
     */
    bool visible() const override { return m_visible; }
    /**
     * @brief 设置颜色
     * @param color 颜色
     */
    void setColor(const QColor &color) override;
    /**
     * @brief 获取颜色
     * @return QColor 颜色
     */
    QColor color() const override { return m_color; }
    /**
     * @brief 添加到渲染器
     * @param renderer 渲染器
     */
    void addToRenderer(vtkRenderer *renderer) override;
    /**
     * @brief 从渲染器移除
     * @param renderer 渲染器
     */
    void removeFromRenderer(vtkRenderer *renderer) override;
    /**
     * @brief 渲染
     */
    void render() override;
    
    // ===== 曲线特有方法 =====
    /**
     * @brief 设置线宽
     * @param width 线宽值
     */
    void setLineWidth(double width);      
    
    /**
     * @brief 获取线宽
     * @return double 当前线宽值
     */
    double lineWidth() const { return m_lineWidth; }  
    
    /**
     * @brief 更新曲线数据
     * @param points 新的点数据
     */
    void updateData(const QVector<QVector3D> &points);  
    
    /**
     * @brief 获取点数据
     * @return QVector<QVector3D> 点集
     */
    QVector<QVector3D> points() const;
    
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

private:
    /**
     * @brief 从点集创建VTK多段线数据
     * @param points 曲线点集
     */
    void createPolyLine(const QVector<QVector3D> &points);
    
    QString m_id;                              // 唯一标识符
    QString m_name;                            // 曲线名称
    QColor m_color;                            // 曲线颜色
    double m_lineWidth;                        // 线宽
    bool m_visible;                            // 可见性
    
    // VTK 数据
    vtkSmartPointer<vtkPolyData> m_polyData;   // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> m_mapper;  // 映射器
    vtkSmartPointer<vtkActor> m_actor;         // 演员
    
    // 回调渲染器引用（用于render函数）
    vtkRenderer *m_renderer;
};

#endif // VTKCURVE_H
