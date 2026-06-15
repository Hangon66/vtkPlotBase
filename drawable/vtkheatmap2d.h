#ifndef VTKHEATMAP2D_H
#define VTKHEATMAP2D_H

#include "vtkchartdrawable.h"
#include "vtkmarkergroup2d.h"
#include <QString>
#include <QColor>
#include <QUuid>
#include <QVector>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkChartHistogram2D.h>

class vtkContextView;
class vtkMarkerGroup2D;
class vtkTextProperty;

/**
 * @brief vtkHeatmap2D 二维热力图类
 *
 * 负责管理单个二维热力图的创建、显示和属性修改。
 * 基于 vtkChartHistogram2D 实现，使用 vtkImageData 作为数据源，
 * 通过 vtkColorTransferFunction 进行颜色映射。
 */
class vtkHeatmap2D : public vtkChartDrawable
{
public:
    /**
     * @brief 构造二维热力图
     * @param data 二维数据矩阵（按行优先排列，大小为 rows * cols）
     * @param rows 行数
     * @param cols 列数
     * @param colorBarTitle 颜色条标题
     */
    vtkHeatmap2D(const QVector<double> &data, int rows, int cols,
                 const QString &colorBarTitle = "");

    /**
     * @brief 析构函数
     */
    ~vtkHeatmap2D() override;

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
     * @brief 添加到上下文视图
     * @param view VTK 上下文视图
     */
    void addToView(vtkContextView *view) override;

    /**
     * @brief 从上下文视图移除
     * @param view VTK 上下文视图
     */
    void removeFromView(vtkContextView *view) override;

    /**
     * @brief 触发重渲染
     */
    void render() override;

    // ===== 二维热力图特有方法 =====

    /**
     * @brief 更新热力图数据
     * @param data 二维数据矩阵（按行优先排列，大小为 rows * cols）
     * @param rows 行数
     * @param cols 列数
     */
    void updateData(const QVector<double> &data, int rows, int cols);

    /**
     * @brief 设置数据原点坐标
     * @param x X 原点
     * @param y Y 原点
     */
    void setOrigin(double x, double y);

    /**
     * @brief 设置数据间距
     * @param dx X 方向间距
     * @param dy Y 方向间距
     */
    void setSpacing(double dx, double dy);

    /**
     * @brief 设置颜色映射范围
     * @param minVal 最小值
     * @param maxVal 最大值
     */
    void setValueRange(double minVal, double maxVal);

    /**
     * @brief 设置颜色条标题
     * @param title 标题
     */
    void setColorBarTitle(const QString &title);

    /**
     * @brief 设置图表标题
     * @param title 标题文本
     */
    void setChartTitle(const QString &title);

    /**
     * @brief 设置 X 轴标题
     * @param title X 轴标题文本
     */
    void setXAxisTitle(const QString &title);

    /**
     * @brief 设置 Y 轴标题
     * @param title Y 轴标题文本
     */
    void setYAxisTitle(const QString &title);

    // ===== 标记组操作 =====

    /**
     * @brief 添加标记组到热力图
     *
     * 创建标记组并附着到当前图表，标记组与热力图共享同一坐标系统。
     *
     * @param name 标记组名称，用于图例显示
     * @param color 标记点颜色
     * @param style 标记点样式，默认圆形
     * @param size 标记点大小，默认 12.0
     * @return vtkMarkerGroup2D* 标记组对象指针
     */
    vtkMarkerGroup2D* addMarkerGroup(const QString &name = "",
                                     const QColor &color = Qt::white,
                                     Marker2DStyle style = Marker2DStyle::Circle,
                                     double size = 12.0);

    /**
     * @brief 移除指定标记组
     * @param group 标记组对象
     */
    void removeMarkerGroup(vtkMarkerGroup2D *group);

    /**
     * @brief 清除所有标记组
     */
    void clearMarkerGroups();

    /**
     * @brief 获取所有标记组
     * @return QList<vtkMarkerGroup2D*> 标记组列表
     */
    QList<vtkMarkerGroup2D*> getMarkerGroups() const;

    /**
     * @brief 获取图表对象
     * @return vtkChartHistogram2D* 图表指针
     */
    vtkChartHistogram2D* chart() { return m_chart; }

private:
    /**
     * @brief 从向量数据创建 vtkImageData
     * @param data 二维数据矩阵
     * @param rows 行数
     * @param cols 列数
     */
    void createImageData(const QVector<double> &data, int rows, int cols);

    /**
     * @brief 创建默认颜色传输函数
     */
    void createDefaultTransferFunction();

    /**
     * @brief 为文本属性设置支持中文显示的字体
     *
     * 设置微软雅黑字体，确保中文字符正常渲染。
     *
     * @param prop 目标文本属性对象。
     */
    static void applyChineseFont(vtkTextProperty *prop);

    QString m_id;                           ///< 唯一标识符
    QString m_name;                         ///< 图表名称
    bool m_visible;                         ///< 可见性
    QString m_colorBarTitle;                ///< 颜色条标题

    /**
     * @brief 数据行数
     */
    int m_rows;

    /**
     * @brief 数据列数
     */
    int m_cols;

    // VTK 数据
    vtkSmartPointer<vtkImageData> m_imageData;              ///< 图像数据
    vtkSmartPointer<vtkColorTransferFunction> m_transferFunction; ///< 颜色传输函数
    vtkSmartPointer<vtkChartHistogram2D> m_chart;           ///< 二维直方图图表

    /**
     * @brief 标记组列表
     */
    QList<vtkMarkerGroup2D*> m_markerGroups;

    /**
     * @brief 上下文视图引用（不拥有）
     */
    vtkContextView *m_view;
};

#endif // VTKHEATMAP2D_H
