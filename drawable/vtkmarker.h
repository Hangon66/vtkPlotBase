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
    
    // 标记特有方法
    void setPosition(const QVector3D &position);  // 设置位置
    QVector3D position() const;                   // 获取位置
    
    void setLineWidth(double width);              // 设置线宽
    double lineWidth() const { return m_lineWidth; }
    
    // 大小模式控制
    void setRadius(double radius);                                    // 设置绝对半径
    void setRelativeRadius(double xMin, double xMax, double ratio);  // 设置相对半径
    void setScreenSize(double screenSize, int windowHeight);          // 设置屏幕大小
    void updateScreenSize(int windowHeight);                          // 更新屏幕大小（基于相机距离）
    
    // 查询
    bool isFilled() const { return m_filled; }                        // 是否填充
    MarkerSizeMode sizeMode() const { return m_sizeMode; }            // 获取大小模式
    double radius() const { return m_radius; }                        // 获取当前半径
    
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
