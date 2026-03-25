#ifndef VTKDRAWABLE_H
#define VTKDRAWABLE_H

#include <QString>
#include <QColor>
#include <QVector3D>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

class vtkRenderer;

/**
 * @brief vtkDrawable 抽象基类
 * 
 * 定义所有可绘制对象的通用接口。
 * 曲线、曲面、标记点等类都应继承此类。
 */
class vtkDrawable
{
public:
    /**
     * @brief 析构函数
     */
    virtual ~vtkDrawable() = default;
    
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
     * @brief 设置颜色
     * @param color 颜色
     */
    virtual void setColor(const QColor &color) = 0;
    
    /**
     * @brief 获取颜色
     * @return QColor 颜色
     */
    virtual QColor color() const = 0;
    
    /**
     * @brief 添加到渲染器
     * @param renderer VTK渲染器
     */
    virtual void addToRenderer(vtkRenderer *renderer) = 0;
    
    /**
     * @brief 从渲染器移除
     * @param renderer VTK渲染器
     */
    virtual void removeFromRenderer(vtkRenderer *renderer) = 0;
    
    /**
     * @brief 触发重渲染
     */
    virtual void render() = 0;
};

#endif // VTKDRAWABLE_H
