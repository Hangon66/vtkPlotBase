#ifndef VTKSCATTERSERIES_H
#define VTKSCATTERSERIES_H

#include "vtkchartdrawable.h"
#include "vtkmarkergroup2d.h"
#include <QString>
#include <QColor>
#include <QPointF>
#include <QUuid>
#include <QVector>

class vtkContextView;
class vtkChartXY;
class vtkTable;
class vtkPlotPoints;

/**
 * @brief vtkScatterSeries 二维散点序列
 *
 * 封装 vtkTable(X/Y) + vtkPlotPoints，管理单个散点数据序列。
 * 添加到共享的 vtkChartXY 中，可与线条序列叠加显示。
 */
class vtkScatterSeries : public vtkChartDrawable
{
public:
    /**
     * @brief 构造散点序列
     *
     * @param chart 共享的 vtkChartXY（由 vtkPlot2D 管理）
     * @param points 坐标点列表
     * @param color 标记点颜色
     * @param style 标记点样式
     * @param size 标记点大小
     * @param name 序列名称（用于图例显示）
     */
    vtkScatterSeries(vtkChartXY *chart, const QVector<QPointF> &points,
                     const QColor &color, Marker2DStyle style, double size,
                     const QString &name);

    ~vtkScatterSeries() override;

    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    void setName(const QString &name) override { m_name = name; }

    /**
     * @brief 设置可见性
     * @param visible 是否可见
     */
    void setVisible(bool visible) override;

    bool visible() const override { return m_visible; }

    /**
     * @brief 添加到上下文视图
     * @param view VTK 上下文视图
     */
    void addToView(vtkContextView *view) override;

    void removeFromView(vtkContextView *view) override;
    void render() override;

    // ===== 散点属性 =====

    /**
     * @brief 设置颜色
     * @param color 颜色
     */
    void setColor(const QColor &color);

    /**
     * @brief 获取颜色
     * @return QColor
     */
    QColor color() const { return m_color; }

    /**
     * @brief 设置标记点样式
     * @param style 标记样式
     */
    void setMarkerStyle(Marker2DStyle style);

    /**
     * @brief 获取标记点样式
     * @return Marker2DStyle
     */
    Marker2DStyle markerStyle() const { return m_style; }

    /**
     * @brief 设置标记点大小
     * @param size 大小值
     */
    void setMarkerSize(double size);

    /**
     * @brief 获取标记点大小
     * @return double
     */
    double markerSize() const { return m_size; }

    /**
     * @brief 替换所有数据点
     * @param points 新的坐标点列表
     */
    void setPoints(const QVector<QPointF> &points);

    /**
     * @brief 获取所有数据点
     * @return QVector<QPointF>
     */
    QVector<QPointF> points() const { return m_points; }

    /**
     * @brief 获取共享图表指针
     * @return vtkChartXY*
     */
    vtkChartXY* chart() { return m_chart; }

private:
    /**
     * @brief 将 m_points 写入 vtkTable
     */
    void rebuildTable();

    QString m_id;                       ///< 唯一标识符
    QString m_name;                     ///< 序列名称
    bool m_visible;                     ///< 可见性
    QColor m_color;                     ///< 标记点颜色
    Marker2DStyle m_style;              ///< 标记点样式
    double m_size;                      ///< 标记点大小
    QVector<QPointF> m_points;          ///< 坐标点列表

    /**
     * @brief 共享图表（由 vtkPlot2D 管理生命周期）
     */
    vtkChartXY *m_chart;

    /**
     * @brief 本序列的数据表（X、Y 两列）
     */
    vtkSmartPointer<vtkTable> m_table;

    /**
     * @brief 本序列的散点图（添加到 m_chart）
     */
    vtkPlotPoints *m_plot;

    /**
     * @brief 上下文视图引用（用于触发重渲染）
     */
    vtkContextView *m_view;
};

#endif // VTKSCATTERSERIES_H
