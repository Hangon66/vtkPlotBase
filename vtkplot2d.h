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

// 前向声明 drawable 类
class vtkHeatmap2D;
class vtkMarkerGroup2D;

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
    QTimer *m_syncTimer;                                        ///\< 位置同步定时器
    vtkSmartPointer<vtkContextView> m_contextView;               ///< 上下文视图

    // 对象集合
    QList<vtkHeatmap2D*> m_heatmap2Ds;                           ///< 二维热力图列表

    // 初始化方法
    void setupVTK();                                             ///< 初始化 VTK
    void syncWindow();                                           ///< 同步顶层窗口位置与尺寸

    /**
     * @brief 渲染场景
     */
    void render();
};

#endif // VKTPLOT2D_H
