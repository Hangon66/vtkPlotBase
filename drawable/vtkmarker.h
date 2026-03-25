#ifndef VTKMARKER_H
#define VTKMARKER_H

#include "vtkdrawable.h"
#include <QVector3D>
#include <QColor>
#include <QUuid>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkFollower.h>
#include <vtkDiskSource.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkMath.h>

/**
 * @brief 标记点大小模式
 */
enum class MarkerSizeMode {
    Absolute,    // 绝对半径模式（固定世界坐标值）
    Relative,    // 相对半径模式（占X轴范围的比例）
    Screen       // 屏幕固定大小模式（像素）
};

/**
 * @brief vtkMarker 标记点类
 * 
 * 负责管理单个3D标记点（空心环或填充圆）的创建、显示和属性修改。
 * 支持三种大小模式：绝对半径、相对半径、屏幕固定大小。
 */
class vtkMarker : public vtkDrawable
{
public:
    /**
     * @brief 构造空心环标记
     * @param position 位置
     * @param color 颜色
     * @param screenSize 屏幕大小（像素）
     * @param lineWidth 线宽
     */
    vtkMarker(const QVector3D &position, const QColor &color, double screenSize, double lineWidth);
    
    /**
     * @brief 析构函数
     */
    ~vtkMarker() override;
    
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
    
    // ===== 标记特有方法 =====
    /**
     * @brief 设置位置
     * @param position 位置
     */
    void setPosition(const QVector3D &position);
    /**
     * @brief 获取位置
     * @return QVector3D 位置
     */
    QVector3D position() const;
    /**
     * @brief 设置线宽
     * @param width 线宽
     */
    void setLineWidth(double width);
    /**
     * @brief 获取线宽
     * @return double 线宽
     */
    double lineWidth() const { return m_lineWidth; }
    
    // ===== 大小模式控制 =====
    /**
     * @brief 设置绝对半径
     * @param radius 半径
     */
    void setRadius(double radius);
    /**
     * @brief 设置相对半径
     * @param xMin X轴最小值
     * @param xMax X轴最大值
     * @param ratio 相对比例
     */
    void setRelativeRadius(double xMin, double xMax, double ratio);
    /**
     * @brief 设置屏幕大小
     * @param screenSize 屏幕大小（像素）
     * @param windowHeight 窗口高度（像素）
     */
    void setScreenSize(double screenSize, int windowHeight);
    /**
     * @brief 更新屏幕大小（基于相机距离）
     * @param windowHeight 窗口高度（像素）
     */
    void updateScreenSize(int windowHeight);
    
    // ===== 查询 =====
    /**
     * @brief 是否填充
     * @return bool 是否填充
     */
    bool isFilled() const { return m_filled; }
    /**
     * @brief 获取大小模式
     * @return MarkerSizeMode 大小模式
     */
    MarkerSizeMode sizeMode() const { return m_sizeMode; }
    /**
     * @brief 获取当前半径
     * @return double 当前半径
     */
    double radius() const { return m_radius; }
    
    /**
     * @brief 获取VTK跟随者
     * @return vtkFollower* 跟随者指针
     */
    vtkFollower* follower() { return m_follower; }
    
    /**
     * @brief 获取跟随者（只读）
     * @return const vtkFollower* 跟随者指针
     */
    const vtkFollower* follower() const { return m_follower; }
    
    /**
     * @brief 设置是否填充模式
     * @param filled true=填充圆, false=空心环
     */
    void setFilled(bool filled);
    
    /**
     * @brief 设置相机（用于屏幕固定大小计算）
     * @param camera VTK相机
     */
    void setCamera(vtkCamera *camera);
    
    /**
     * @brief 设置坐标轴范围（用于相对半径模式）
     * @param xMin X轴最小值
     * @param xMax X轴最大值
     */
    void setAxisRange(double xMin, double xMax);

private:
    /**
     * @brief 重新创建空心环几何
     * @param radius 半径
     */
    void recreateHollowGeometry(double radius);
    
    /**
     * @brief 重新创建填充圆几何
     * @param radius 半径
     */
    void recreateFilledGeometry(double radius);
    
    /**
     * @brief 计算屏幕固定半径
     * @param windowHeight 窗口高度
     * @return double 计算后的世界坐标半径
     */
    double computeScreenRadius(int windowHeight);
    
    QString m_id;                              // 唯一标识符
    QString m_name;                            // 标记名称
    QColor m_color;                            // 颜色
    bool m_filled;                             // true=填充圆, false=空心环
    double m_lineWidth;                         // 线宽
    bool m_visible;                            // 可见性
    
    // 大小相关
    double m_radius;                            // 当前半径（世界坐标）
    double m_relativeRadius;                    // 相对半径（占X轴范围的比例）
    double m_screenSize;                       // 屏幕大小（像素）
    MarkerSizeMode m_sizeMode;                 // 大小模式
    
    // 坐标轴范围（用于相对半径）
    double m_xMin;
    double m_xMax;
    
    // VTK 数据
    vtkSmartPointer<vtkPolyData> m_polyData;   // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> m_mapper;  // 映射器
    vtkSmartPointer<vtkFollower> m_follower;  // 跟随者（始终面向相机）
    
    // 渲染器引用
    vtkRenderer *m_renderer;
    vtkCamera *m_camera;
};

#endif // VTKMARKER_H
