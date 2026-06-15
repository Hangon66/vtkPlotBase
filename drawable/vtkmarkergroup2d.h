#ifndef VTKMARKERGROUP2D_H
#define VTKMARKERGROUP2D_H

#include <QString>
#include <QColor>
#include <QVector>
#include <QPointF>
#include <QUuid>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkRenderingCoreEnums.h>
#include <vtkTable.h>
#include <vtkPlotPoints.h>

class vtkChartHistogram2D;

/**
 * @brief 二维标记点样式枚举
 *
 * 定义二维图表中标记点的可选样式，
 * 与 VTK 9.6 的 vtkRenderingCoreEnums 中的标记样式一一对应。
 */
enum class Marker2DStyle {
    None    = VTK_MARKER_NONE,      ///< 无标记
    Cross   = VTK_MARKER_CROSS,     ///< 十字叉
    Plus    = VTK_MARKER_PLUS,      ///< 加号
    Square  = VTK_MARKER_SQUARE,    ///< 正方形
    Circle  = VTK_MARKER_CIRCLE,    ///< 圆形
    Diamond = VTK_MARKER_DIAMOND    ///< 菱形
};

/**
 * @brief vtkMarkerGroup2D 二维标记组类
 *
 * 管理一组具有相同颜色和样式的二维标记点。
 * 基于 vtkPlotPoints（散点图）实现，通过 vtkTable 存储坐标数据，
 * 添加到 vtkChartHistogram2D 中与热力图共享同一坐标系统。
 *
 * 每个标记组对应图表中的一个散点图（vtkPlotPoints），
 * 可独立控制可见性、颜色、样式、大小等属性。
 *
 * 注意：散点图（vtkPlotPoints）在首次有数据时才创建，
 * 遵循 VTK 的 "数据就绪 → AddPlot → SetInputData" 流程。
 */
class vtkMarkerGroup2D
{
public:
    /**
     * @brief 构造二维标记组
     *
     * @param name 标记组名称，用于图例显示。
     * @param color 标记点颜色。
     * @param style 标记点样式，默认为圆形。
     * @param size 标记点大小，默认为 12.0。
     */
    vtkMarkerGroup2D(const QString &name = "",
                     const QColor &color = Qt::white,
                     Marker2DStyle style = Marker2DStyle::Circle,
                     double size = 12.0);

    /**
     * @brief 析构函数
     *
     * 若标记组仍附着在图表上，将自动分离。
     */
    ~vtkMarkerGroup2D();

    // ===== 标识 =====

    /**
     * @brief 获取唯一标识符
     * @return QString 标识符
     */
    QString id() const { return m_id; }

    /**
     * @brief 获取标记组名称
     * @return QString 名称
     */
    QString name() const { return m_name; }

    /**
     * @brief 设置标记组名称
     * @param name 名称
     */
    void setName(const QString &name);

    // ===== 可见性 =====

    /**
     * @brief 设置可见性
     * @param visible 是否可见
     */
    void setVisible(bool visible);

    /**
     * @brief 获取可见性
     * @return bool 是否可见
     */
    bool visible() const { return m_visible; }

    // ===== 数据操作 =====

    /**
     * @brief 添加单个标记点
     * @param x X 坐标（数据坐标系）
     * @param y Y 坐标（数据坐标系）
     */
    void addPoint(double x, double y);

    /**
     * @brief 批量添加标记点
     * @param points 坐标点列表
     */
    void addPoints(const QVector<QPointF> &points);

    /**
     * @brief 替换所有标记点数据
     *
     * 清空现有点并设置新数据，触发图表更新。
     *
     * @param points 新的坐标点列表
     */
    void setPoints(const QVector<QPointF> &points);

    /**
     * @brief 清空所有标记点
     */
    void clearPoints();

    /**
     * @brief 获取所有标记点坐标
     * @return QVector<QPointF> 坐标点列表
     */
    QVector<QPointF> points() const;

    /**
     * @brief 获取标记点数量
     * @return int 点数
     */
    int pointCount() const;

    // ===== 外观设置 =====

    /**
     * @brief 设置标记点颜色
     * @param color 颜色
     */
    void setColor(const QColor &color);

    /**
     * @brief 获取标记点颜色
     * @return QColor 颜色
     */
    QColor color() const { return m_color; }

    /**
     * @brief 设置标记点样式
     * @param style 标记样式
     */
    void setMarkerStyle(Marker2DStyle style);

    /**
     * @brief 获取标记点样式
     * @return Marker2DStyle 标记样式
     */
    Marker2DStyle markerStyle() const { return m_style; }

    /**
     * @brief 设置标记点大小
     * @param size 大小值
     */
    void setMarkerSize(double size);

    /**
     * @brief 获取标记点大小
     * @return double 大小值
     */
    double markerSize() const { return m_size; }

    /**
     * @brief 设置标记点线宽
     * @param width 线宽
     */
    void setLineWidth(double width);

    /**
     * @brief 获取标记点线宽
     * @return double 线宽
     */
    double lineWidth() const { return m_lineWidth; }

    /**
     * @brief 设置图例标签
     * @param label 标签文本
     */
    void setLabel(const QString &label);

    /**
     * @brief 获取图例标签
     * @return QString 标签文本
     */
    QString label() const { return m_label; }

    // ===== 图表附着 =====

    /**
     * @brief 关联到图表
     *
     * 仅存储图表引用，散点图在首次有数据时才创建。
     *
     * @param chart 目标图表指针
     */
    void attachToChart(vtkChartHistogram2D *chart);

    /**
     * @brief 从图表分离标记组
     *
     * 从图表中移除散点图并断开关联。
     *
     * @param chart 目标图表指针
     */
    void detachFromChart(vtkChartHistogram2D *chart);

    /**
     * @brief 判断是否已附着到图表
     * @return bool 是否已附着
     */
    bool isAttached() const { return m_chart != nullptr; }

    /**
     * @brief 获取散点图对象
     * @return vtkPlotPoints* 散点图指针（可能为空）
     */
    vtkPlotPoints* plot() const { return m_plot; }

private:
    /**
     * @brief 在图表上创建散点图
     *
     * 按照参考示例流程：填充表数据 → AddPlot → SetInputData → 设置属性。
     * 仅在有数据且图表已关联时调用。
     */
    void createPlot();

    /**
     * @brief 重建 VTK 表数据
     *
     * 根据 m_points 列表更新 vtkTable 中的数据。
     */
    void rebuildTable();

    QString m_id;                  ///< 唯一标识符
    QString m_name;                ///< 标记组名称
    bool m_visible;                ///< 可见性

    QColor m_color;                ///< 标记点颜色
    Marker2DStyle m_style;         ///< 标记点样式
    double m_size;                 ///< 标记点大小
    double m_lineWidth;            ///< 标记点线宽
    QString m_label;               ///< 图例标签

    QVector<QPointF> m_points;     ///< 标记点坐标列表

    /**
     * @brief VTK 数据表（X、Y 两列）
     */
    vtkSmartPointer<vtkTable> m_table;

    /**
     * @brief 散点图对象（由 chart 拥有，不释放）
     */
    vtkPlotPoints *m_plot;

    /**
     * @brief 所属图表引用（不拥有）
     */
    vtkChartHistogram2D *m_chart;
};

#endif // VTKMARKERGROUP2D_H
