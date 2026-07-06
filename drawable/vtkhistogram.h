#ifndef VTKHISTOGRAM_H
#define VTKHISTOGRAM_H

#include "vtkchartdrawable.h"
#include <QString>
#include <QColor>
#include <QUuid>
#include <QVector>

class vtkContextView;
class vtkChartXY;
class vtkTable;
class vtkPlotArea;
class vtkTextProperty;

/**
 * @brief vtkHistogram 概率分布直方图（填充面积图）
 *
 * 使用 vtkChartXY + vtkPlotArea 实现半透明填充面积显示。
 * 每个 vtkHistogram 实例拥有自己的 vtkTable（bin_center、baseline、frequency）
 * 和 vtkPlotArea，添加到共享的 vtkChartXY 中。多组数据自然重叠显示。
 */
class vtkHistogram : public vtkChartDrawable
{
public:
    /**
     * @brief 构造直方图面积图
     *
     * 对输入数据进行等宽分箱统计，创建 vtkPlotArea 添加到共享图表。
     *
     * @param chart 共享的 vtkChartXY（由 vtkPlot2D 管理）
     * @param data 原始标量数据
     * @param numBins 分箱数量
     * @param color 填充颜色（含 alpha 通道控制透明度）
     * @param name 序列名称（用于图例显示）
     */
    vtkHistogram(vtkChartXY *chart, const QVector<double> &data, int numBins,
                 const QColor &color, const QString &name);

    ~vtkHistogram() override;

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

    // ===== 直方图属性 =====

    /**
     * @brief 获取分箱数量
     * @return int 分箱数
     */
    int numBins() const { return m_numBins; }

    /**
     * @brief 获取柱状图颜色
     * @return QColor
     */
    QColor color() const { return m_color; }

    /**
     * @brief 设置颜色（含 alpha）
     * @param color 颜色
     */
    void setColor(const QColor &color);

    /**
     * @brief 更新数据（重新分箱并更新面积图）
     * @param data 原始标量数据
     * @param numBins 分箱数量
     */
    void updateData(const QVector<double> &data, int numBins);

    /**
     * @brief 设置标题（影响共享 chart）
     * @param title 标题文本
     */
    void setTitle(const QString &title);

    void setXAxisTitle(const QString &title);
    void setYAxisTitle(const QString &title);

    vtkChartXY* chart() { return m_chart; }

    /**
     * @brief 获取本序列的最大频率值
     * @return double 最大频率
     */
    double maxFrequency() const;

private:
    /**
     * @brief 对数据进行分箱统计并写入 vtkTable
     * @param data 原始标量数据
     * @param numBins 分箱数量
     */
    void computeHistogram(const QVector<double> &data, int numBins);

    /**
     * @brief 设置中文字体
     * @param prop VTK 文本属性
     */
    static void applyChineseFont(vtkTextProperty *prop);

    QString m_id;                       ///< 唯一标识符
    QString m_name;                     ///< 序列名称
    bool m_visible;                     ///< 可见性
    int m_numBins;                      ///< 分箱数量
    QColor m_color;                     ///< 填充颜色（含 alpha）

    /**
     * @brief 共享图表（由 vtkPlot2D 管理生命周期）
     */
    vtkChartXY *m_chart;

    /**
     * @brief 本序列的数据表（bin_center、baseline、frequency 三列）
     */
    vtkSmartPointer<vtkTable> m_table;

    /**
     * @brief 本序列的面积图（添加到 m_chart）
     */
    vtkPlotArea *m_areaPlot;

    /**
     * @brief 上下文视图引用（用于触发重渲染）
     */
    vtkContextView *m_view;
};

#endif // VTKHISTOGRAM_H
