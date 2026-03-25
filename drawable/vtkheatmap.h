#ifndef VTKHEATMAP_H
#define VTKHEATMAP_H

#include "vtkdrawable.h"
#include <QVector3D>
#include <QColor>
#include <QUuid>
#include <QString>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkPoints.h>
#include <vtkQuad.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkContourFilter.h>
#include <vtkStripper.h>
#include <vtkPointData.h>

class vtkRenderer;

/**
 * @brief vtkHeatmap 热力图曲面类
 * 
 * 负责管理单个3D热力图曲面的创建、显示和属性修改。
 * 热力图根据Y值（高度）映射到彩虹颜色，并支持等高线投影。
 */
class vtkHeatmap : public vtkDrawable
{
public:
    /**
     * @brief 构造热力图曲面
     * @param points 网格点集（按行优先排列）
     * @param nx X方向点数
     * @param ny Y方向点数
     * @param colorBarTitle 颜色条标题
     */
    vtkHeatmap(const QVector<QVector3D> &points, int nx, int ny,
               const QString &colorBarTitle);
    
    /**
     * @brief 析构函数
     */
    ~vtkHeatmap() override;
    
    // vtkDrawable 接口实现
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    void setName(const QString &name) override { m_name = name; }
    void setVisible(bool visible) override;
    bool visible() const override { return m_visible; }
    void setColor(const QColor &color) override;  // 注意：热力图颜色由标量值决定，此函数仅用于兼容
    QColor color() const override { return QColor(); }
    void addToRenderer(vtkRenderer *renderer) override;
    void removeFromRenderer(vtkRenderer *renderer) override;
    void render() override;
    
    // 热力图特有方法
    void setOpacity(double opacity);                   // 设置不透明度
    double opacity() const { return m_opacity; }       // 获取不透明度
    
    void setContourVisible(bool visible);               // 设置等高线可见性
    bool isContourVisible() const { return m_contourVisible; }
    
    void setContourCount(int count);                   // 设置等高线数量
    int contourCount() const { return m_contourCount; }
    
    void setContourBaseY(double y);                    // 设置等高线投影基准Y值
    double contourBaseY() const { return m_contourBaseY; }
    
    void setZRange(double zMin, double zMax);          // 设置Z值范围
    double zMin() const { return m_zMin; }
    double zMax() const { return m_zMax; }
    
    void setColorBarTitle(const QString &title);        // 设置颜色条标题
    
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
     * @brief 获取等高线演员
     * @return vtkActor* 等高线演员指针
     */
    vtkActor* contourActor() { return m_contourActor; }
    
    /**
     * @brief 获取颜色查找表
     * @return vtkLookupTable* 颜色查找表指针
     */
    vtkLookupTable* lookupTable() { return m_lookupTable; }
    
    /**
     * @brief 获取多边形数据
     * @return vtkPolyData* 多边形数据指针
     */
    vtkPolyData* polyData() { return m_polyData; }
    
    /**
     * @brief 设置全局颜色条（用于多热力图共享）
     * @param scalarBar 颜色条演员（智能指针）
     */
    void setScalarBarActor(vtkSmartPointer<vtkScalarBarActor> &scalarBar);
    
    /**
     * @brief 更新等高线（当Z范围或数量改变时调用）
     */
    void updateContour();

private:
    /**
     * @brief 创建彩虹颜色查找表
     * @param zMin Z值最小值
     * @param zMax Z值最大值
     * @return vtkSmartPointer<vtkLookupTable> 颜色查找表
     */
    static vtkSmartPointer<vtkLookupTable> createRainbowLookupTable(double zMin, double zMax);
    
    /**
     * @brief 从网格点创建VTK多边形数据
     * @param points 网格点集
     * @param nx X方向点数
     * @param ny Y方向点数
     */
    void createQuadMesh(const QVector<QVector3D> &points, int nx, int ny);
    
    /**
     * @brief 创建等高线投影
     * @param heightMin 高度最小值
     * @param heightMax 高度最大值
     */
    void createContourProjection(double heightMin, double heightMax);
    
    QString m_id;                              // 唯一标识符
    QString m_name;                            // 曲面名称
    double m_opacity;                         // 不透明度
    bool m_visible;                            // 可见性
    bool m_contourVisible;                     // 等高线可见性
    int m_contourCount;                        // 等高线数量
    double m_contourBaseY;                    // 等高线投影基准Y值
    double m_zMin;                            // Z值最小值
    double m_zMax;                            // Z值最大值
    
    // VTK 数据
    vtkSmartPointer<vtkPolyData> m_polyData;   // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> m_mapper;  // 映射器
    vtkSmartPointer<vtkActor> m_actor;         // 演员
    
    // 等高线数据
    vtkSmartPointer<vtkPolyData> m_contourPolyData;  // 等高线数据
    vtkSmartPointer<vtkPolyDataMapper> m_contourMapper;  // 等高线映射器
    vtkSmartPointer<vtkActor> m_contourActor;  // 等高线演员
    
    vtkSmartPointer<vtkLookupTable> m_lookupTable;  // 颜色查找表
    
    // 渲染器引用
    vtkRenderer *m_renderer;
    vtkScalarBarActor *m_scalarBarActor;       // 共享颜色条
    QString m_colorBarTitle;                   // 颜色条标题
};

#endif // VTKHEATMAP_H
