#ifndef VTKLINESERIES_H
#define VTKLINESERIES_H

#include "vtkchartdrawable.h"
#include <QString>
#include <QColor>
#include <QPointF>
#include <QUuid>
#include <QVector>

class vtkContextView;
class vtkChartXY;
class vtkTable;
class vtkPlotLine;

/**
 * @brief vtkLineSeries 二维线条序列
 *
 * 封装 vtkTable(X/Y) + vtkPlotLine，管理单条线条。
 * 添加到共享的 vtkChartXY 中，可与散点序列叠加显示。
 */
class vtkLineSeries : public vtkChartDrawable
{
public:
    /**
     * @brief 构造线条序列
     *
     * @param chart 共享的 vtkChartXY（由 vtkPlot2D 管理）
     * @param points 坐标点列表（按顺序连线）
     * @param color 线条颜色
     * @param width 线条宽度
     * @param name 序列名称（用于图例显示）
     */
    vtkLineSeries(vtkChartXY *chart, const QVector<QPointF> &points,
                  const QColor &color, double width,
                  const QString &name);

    ~vtkLineSeries() override;

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

    // ===== 线条属性 =====

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
     * @brief 设置线条宽度
     * @param width 宽度
     */
    void setWidth(double width);

    /**
     * @brief 获取线条宽度
     * @return double
     */
    double width() const { return m_width; }

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
    QColor m_color;                     ///< 线条颜色
    double m_width;                     ///< 线条宽度
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
     * @brief 本序列的线条图（添加到 m_chart）
     */
    vtkPlotLine *m_plot;

    /**
     * @brief 上下文视图引用（用于触发重渲染）
     */
    vtkContextView *m_view;
};

#endif // VTKLINESERIES_H
