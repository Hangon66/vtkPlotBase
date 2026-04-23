#ifndef VTKCHARTDRAWABLE_H
#define VTKCHARTDRAWABLE_H

#include <QString>
#include <QColor>
#include <vtkSmartPointer.h>

class vtkContextView;
class vtkChart;

/**
 * @brief vtkChartDrawable 抽象基类
 *
 * 定义所有二维图表可绘制对象的通用接口。
 * 与 vtkDrawable（三维）不同，此类基于 vtkContextView + vtkChart 体系。
 * 二维热力图等图表类型应继承此类。
 */
class vtkChartDrawable
{
public:
    /**
     * @brief 析构函数
     */
    virtual ~vtkChartDrawable() = default;

    /**
     * @brief 获取对象的唯一标识符
     * @return QString 唯一ID
     */
    virtual QString id() const = 0;

    /**
     * @brief 获取对象名称（用于图例显示）
     * @return QString 名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 设置对象名称
     * @param name 新名称
     */
    virtual void setName(const QString &name) = 0;

    /**
     * @brief 设置可见性
     * @param visible 是否可见
     */
    virtual void setVisible(bool visible) = 0;

    /**
     * @brief 获取可见性
     * @return bool 可见性
     */
    virtual bool visible() const = 0;

    /**
     * @brief 添加到上下文视图
     * @param view VTK 上下文视图
     */
    virtual void addToView(vtkContextView *view) = 0;

    /**
     * @brief 从上下文视图移除
     * @param view VTK 上下文视图
     */
    virtual void removeFromView(vtkContextView *view) = 0;

    /**
     * @brief 触发重渲染
     */
    virtual void render() = 0;
};

#endif // VTKCHARTDRAWABLE_H
