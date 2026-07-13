#ifndef VKTPLOT2D_H
#define VKTPLOT2D_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QMap>
#include <QColor>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkContextView.h>

// 前向声明
class QVTKOpenGLNativeWidget;
class vtkGenericOpenGLRenderWindow;
class vtkChartHistogram2D;
class vtkColorTransferFunction;
class vtkChartXY;
class vtkTextProperty;

// 前向声明 drawable 类
class vtkHeatmap2D;
class vtkHistogram;
class vtkMarkerGroup2D;
class vtkScatterSeries;
class vtkLineSeries;

// 标记样式枚举定义在 vtkmarkergroup2d.h 中
#include "drawable/vtkmarkergroup2d.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkPlot2D;
}
QT_END_NAMESPACE

/**
 * @brief vtkPlot2D 二维图表控件类
 *
 * 负责管理二维图表场景，基于 VTK 的 ContextView 体系，
 * 支持二维热力图等图表类型的创建、显示和交互。
 * 与 vtkPlotBase（三维）平行，采用相同的外部接口风格。
 */
class vtkPlot2D : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit vtkPlot2D(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~vtkPlot2D();

protected:
    /**
     * @brief 窗口显示事件
     * @param event 显示事件对象
     */
    void showEvent(QShowEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief 重写滚轮事件处理。
     *
     * 根据焦点状态决定是否拦截滚轮事件：
     * - 有焦点：阻止事件传播，进行缩放
     * - 无焦点：不拦截，让事件传播到父级滚动区域
     *
     * @param event 滚轮事件对象。
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief 键盘事件处理
     *
     * 按 R 键重置所有图表的坐标轴范围，恢复默认视图。
     *
     * @param event 键盘事件对象
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 通用事件处理
     * @param event 事件对象
     * @return 事件是否被处理
     */
    bool event(QEvent *event) override;

public:
    // ===== 图表标题操作 =====

    /**
     * @brief 设置图表标题文本
     * @param title 标题文本
     */
    void setTitle(const QString &title);

    /**
     * @brief 设置背景颜色
     * @param color 背景颜色
     */
    void setBackground(const QColor &color);

    // ===== 交互控制 =====

    /**
     * @brief 设置是否允许鼠标交互（拖拽平移、滚轮缩放）
     *
     * 默认禁用交互，视图固定不可移动。启用后可进行平移和缩放操作。
     *
     * @param enabled true=启用交互，false=禁用交互
     */
    void setInteractionEnabled(bool enabled);

    /**
     * @brief 查询当前交互状态
     * @return bool 是否允许交互
     */
    bool isInteractionEnabled() const { return m_interactionEnabled; }

    // ===== 坐标轴操作 =====

    /**
     * @brief 设置 X 轴标题
     * @param title X 轴标题
     */
    void setXAxisTitle(const QString &title);

    /**
     * @brief 设置 Y 轴标题
     * @param title Y 轴标题
     */
    void setYAxisTitle(const QString &title);

    // ===== 二维热力图操作 =====

    /**
     * @brief 添加二维热力图
     * @param data 二维数据矩阵（按行优先排列，大小为 rows * cols）
     * @param rows 行数
     * @param cols 列数
     * @param colorBarTitle 颜色条标题
     * @return vtkHeatmap2D* 热力图对象指针
     */
    vtkHeatmap2D* addHeatmap2D(const QVector<double> &data, int rows, int cols,
                               const QString &colorBarTitle = "");

    /**
     * @brief 设置二维热力图可见性
     * @param heatmap 热力图对象
     * @param visible 是否可见
     */
    void setHeatmap2DVisible(vtkHeatmap2D *heatmap, bool visible);

    /**
     * @brief 更新二维热力图数据
     * @param heatmap 热力图对象
     * @param data 二维数据矩阵
     * @param rows 行数
     * @param cols 列数
     */
    void updateHeatmap2DData(vtkHeatmap2D *heatmap, const QVector<double> &data,
                             int rows, int cols);

    /**
     * @brief 移除二维热力图
     * @param heatmap 热力图对象
     */
    void removeHeatmap2D(vtkHeatmap2D *heatmap);

    /**
     * @brief 清除所有二维热力图
     */
    void clearAllHeatmap2D();

    /**
     * @brief 获取所有二维热力图
     * @return QList<vtkHeatmap2D*> 热力图列表
     */
    QList<vtkHeatmap2D*> getHeatmap2Ds() const;

    /**
     * @brief 设置二维热力图的离散颜色映射表
     *
     * 将连续渐变色表替换为离散颜色列表，适用于二值图（如二维码）或分类数据。
     * 颜色按值域等分映射：第 i 个颜色对应值域 [i/N, (i+1)/N)。
     *
     * 典型用法（二维码）：
     * @code
     * plot.setHeatmap2DDiscreteColorMap(heatmap, {Qt::white, Qt::black});
     * @endcode
     *
     * @param heatmap 热力图对象
     * @param colors 离散颜色列表，按值域从低到高排列，至少 2 个
     */
    void setHeatmap2DDiscreteColorMap(vtkHeatmap2D *heatmap, const QVector<QColor> &colors);

    /**
     * @brief 设置二维热力图颜色条的可见性
     * @param heatmap 热力图对象
     * @param visible true 显示颜色条，false 隐藏颜色条
     */
    void setHeatmap2DColorBarVisible(vtkHeatmap2D *heatmap, bool visible);

    // ===== 标记组操作 =====

    /**
     * @brief 向指定热力图添加标记组
     *
     * @param heatmap 目标热力图
     * @param name 标记组名称
     * @param color 标记点颜色
     * @param style 标记点样式
     * @param size 标记点大小
     * @return vtkMarkerGroup2D* 标记组指针
     */
    vtkMarkerGroup2D* addMarkerGroup(vtkHeatmap2D *heatmap,
                                     const QString &name = "",
                                     const QColor &color = Qt::white,
                                     Marker2DStyle style = Marker2DStyle::Circle,
                                     double size = 12.0);

    /**
     * @brief 向第一个热力图添加标记组（便捷方法）
     *
     * 当仅有一个热力图时可直接使用此方法，无需指定目标热力图。
     *
     * @param name 标记组名称
     * @param color 标记点颜色
     * @param style 标记点样式
     * @param size 标记点大小
     * @return vtkMarkerGroup2D* 标记组指针，无热力图时返回 nullptr
     */
    vtkMarkerGroup2D* addMarkerGroup(const QString &name = "",
                                     const QColor &color = Qt::white,
                                     Marker2DStyle style = Marker2DStyle::Circle,
                                     double size = 12.0);

    /**
     * @brief 从指定热力图移除标记组
     * @param heatmap 热力图
     * @param group 标记组
     */
    void removeMarkerGroup(vtkHeatmap2D *heatmap, vtkMarkerGroup2D *group);

    /**
     * @brief 清除指定热力图的所有标记组
     * @param heatmap 热力图
     */
    void clearMarkerGroups(vtkHeatmap2D *heatmap);

    // ===== 概率分布直方图操作 =====

    /**
     * @brief 添加概率分布直方图
     *
     * 对输入的标量数据进行等宽分箱，统计频次并以柱状图展示。
     *
     * @param data 原始标量数据
     * @param numBins 分箱数量，默认 50
     * @param color 柱状图颜色，默认青色
     * @param name 图表名称（用于图例显示），默认为空
     * @return vtkHistogram* 直方图对象指针
     */
    vtkHistogram* addHistogram(const QVector<double> &data, int numBins = 50,
                               const QColor &color = Qt::cyan,
                               const QString &name = "");

    /**
     * @brief 设置直方图可见性
     * @param histogram 直方图对象
     * @param visible 是否可见
     */
    void setHistogramVisible(vtkHistogram *histogram, bool visible);

    /**
     * @brief 移除直方图
     * @param histogram 直方图对象
     */
    void removeHistogram(vtkHistogram *histogram);

    /**
     * @brief 清除所有直方图
     */
    void clearAllHistograms();

    /**
     * @brief 获取所有直方图
     * @return QList<vtkHistogram*> 直方图列表
     */
    QList<vtkHistogram*> getHistograms() const;

    /**
     * @brief 在直方图表中添加垂直参考线
     *
     * 在指定的 x 坐标处绘制一条垂直虚线，用于标记参考位置。
     *
     * @param xValue x 坐标值
     * @param color 线条颜色（默认红色）
     * @param width 线条宽度（默认 2.0）
     * @param label 图例名称（为空时不在图例中显示）
     */
    void addHistogramRefLine(double xValue,
                              const QColor &color = QColor(255, 0, 0),
                              double width = 2.0,
                              const QString &label = "");

    // ===== 散点图操作 =====

    /**
     * @brief 添加散点序列
     *
     * 在共享的散点图表中添加一组散点。
     * 首次调用时自动创建共享 vtkChartXY 并配置坐标轴样式。
     *
     * @param points 坐标点列表
     * @param color 标记点颜色（默认青色）
     * @param style 标记点样式（默认圆形）
     * @param size 标记点大小（默认 10.0）
     * @param name 序列名称（用于图例显示）
     * @return vtkScatterSeries* 散点序列指针
     */
    vtkScatterSeries* addScatterSeries(const QVector<QPointF> &points,
                                       const QColor &color = Qt::cyan,
                                       Marker2DStyle style = Marker2DStyle::Circle,
                                       double size = 10.0,
                                       const QString &name = "");

    /**
     * @brief 添加线条序列
     *
     * 在共享的散点图表中添加一条折线。
     * 可与散点序列叠加显示。
     *
     * @param points 坐标点列表（按顺序连线）
     * @param color 线条颜色（默认白色）
     * @param width 线条宽度（默认 2.0）
     * @param name 序列名称（用于图例显示）
     * @return vtkLineSeries* 线条序列指针
     */
    vtkLineSeries* addLineSeries(const QVector<QPointF> &points,
                                  const QColor &color = Qt::white,
                                  double width = 2.0,
                                  const QString &name = "");

    /**
     * @brief 清除所有散点序列
     */
    void clearAllScatterSeries();

    /**
     * @brief 清除所有线条序列
     */
    void clearAllLineSeries();

    // ===== 清除所有 =====

    /**
     * @brief 清除所有图表对象
     */
    void clearAll();

    // ===== 内部访问方法 =====

    /**
     * @brief 获取上下文视图
     * @return vtkContextView* 上下文视图指针
     */
    vtkContextView* contextView() const { return m_contextView; }

private:
    Ui::vtkPlot2D *ui;

    // VTK 组件
    QVTKOpenGLNativeWidget *m_vtkWidget;                        ///\< VTK Qt 控件
    QWidget *m_overlayWindow;                                   ///\< VTK 独立顶层窗口（不参与主窗口 RHI 合成）
    QTimer *m_syncTimer;                                        ///< 位置同步定时器
    QSize m_lastVtkSize;                                        ///< 上一次 VTK 控件尺寸（去重 resize）
    vtkSmartPointer<vtkContextView> m_contextView;               ///< 上下文视图
    bool m_interactionEnabled;                                   ///< 是否允许鼠标交互（默认 false）

    // 对象集合
    QList<vtkHeatmap2D*> m_heatmap2Ds;                           ///< 二维热力图列表
    QList<vtkHistogram*> m_histograms;                            ///< 概率分布直方图序列列表
    vtkSmartPointer<vtkChartXY> m_histogramChart;                   ///< 共享图表（vtkChartXY，包含所有面积图）

    QList<vtkScatterSeries*> m_scatterSeries;                       ///< 散点序列列表
    QList<vtkLineSeries*> m_lineSeries;                             ///< 线条序列列表
    vtkSmartPointer<vtkChartXY> m_scatterChart;                     ///< 散点图共享图表

    // 初始化方法
    void setupVTK();                                             ///< 初始化 VTK
    void syncWindow();                                           ///< 同步顶层窗口位置与尺寸

    /**
     * @brief 渲染场景
     */
    void render();
};

#endif // VKTPLOT2D_H
