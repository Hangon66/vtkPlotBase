#ifndef VTKPLOTBASE_H
#define VTKPLOTBASE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QMap>
#include <QVector3D>
#include <QColor>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkCubeAxesActor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkCallbackCommand.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLegendBoxActor.h>
#include <vtkTextProperty.h>
#include <vtkPointPicker.h>

// 前向声明
class QVTKOpenGLNativeWidget;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkTextActor;
class vtkPointPicker;

// 前向声明新的类
class vtkDrawable;
class vtkCurve;
class vtkMarker;
class vtkSurface;
class vtkHeatmap;

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkPlotBase;
}
QT_END_NAMESPACE

// 自适应缩放模式
enum class AutoScaleMode {
    None,         // 无自动缩放（固定坐标轴范围）
    Independent,  // 坐标轴适应曲面
    EqualRatio,   // 等比例缩放（1:1:1）
    StretchFill   // 拉伸填满立方体（归一化几何体，坐标轴标签显示原始范围）
};

// 图例位置
enum class LegendPosition {
    TopLeft,      // 左上角
    TopRight,     // 右上角
    BottomLeft,   // 左下角
    BottomRight   // 右下角
};

// 自定义交互器样式，处理 R 键重置视角
class vtkPlotBaseInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static vtkPlotBaseInteractorStyle* New();
    vtkTypeMacro(vtkPlotBaseInteractorStyle, vtkInteractorStyleTrackballCamera);
    
    void SetPlotBase(class vtkPlotBase* plot) { m_plotBase = plot; }
    
    void OnKeyPress() override;
    void OnMouseMove() override;

private:
    class vtkPlotBase* m_plotBase = nullptr;
};

/**
 * @brief vtkPlotBase 场景管理器类
 * 
 * 负责管理3D场景的坐标轴、相机、图例、颜色条等场景元素，
 * 以及协调所有可绘制对象（曲线、标记、曲面、热力图）的渲染。
 * 
 * 各类型对象的创建和管理已委托给专门的类：
 * - vtkCurve: 曲线
 * - vtkMarker: 标记点
 * - vtkSurface: 曲面
 * - vtkHeatmap: 热力图
 */
class vtkPlotBase : public QWidget
{
    Q_OBJECT
    friend class vtkPlotBaseInteractorStyle;

public:
    explicit vtkPlotBase(QWidget *parent = nullptr);
    ~vtkPlotBase();

protected:
    void showEvent(QShowEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief 重写滚轮事件处理。
     *
     * 用于阻止滚轮事件传播到父级滚动区域（如 QScrollArea），
     * 同时让 VTK 控件正常处理滚轮缩放。
     *
     * @param event 滚轮事件对象。
     */
    void wheelEvent(QWheelEvent *event) override;

public:
    // ===== 坐标系操作 =====
    void setAxisRange(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);  // 设置坐标轴范围
    void setAxisTitles(const QString &xTitle, const QString &yTitle, const QString &zTitle);          // 设置坐标轴标题

    /**
     * @brief 仅设置坐标轴刻度标签范围，不影响物理边界。
     *
     * 当几何体已归一化到等比空间时，使用此方法设置原始数据范围作为刻度标签，
     * 使坐标轴显示真实数值而物理坐标系保持 1:1:1 等比。
     *
     * @param xMin X 轴刻度最小值。
     * @param xMax X 轴刻度最大值。
     * @param yMin Y 轴刻度最小值。
     * @param yMax Y 轴刻度最大值。
     * @param zMin Z 轴刻度最小值。
     * @param zMax Z 轴刻度最大值。
     */
    void setAxisLabelRange(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);

    void setGridVisible(bool visible);                                                               // 设置网格可见性
    void setBackground(const QColor &color);                                                          // 设置背景颜色
    /**
     * @brief 设置自适应缩放模式。
     *
     * 必须在添加数据（addSurface/addCurve/addHeatmapSurface 等）之前调用。
     * StretchFill 模式会在数据添加时将几何体坐标归一化到 [0,1]，
     * 若在添加数据后切换模式，已有几何体不会被重新归一化，导致显示异常。
     *
     * @param mode 缩放模式（None/Independent/EqualRatio/StretchFill）。
     */
    void setAutoScaleMode(AutoScaleMode mode);
    AutoScaleMode autoScaleMode() const;                                                             // 获取当前自适应缩放模式
    void autoFit();                                                                                  // 自适应所有数据
    void resetView();                                                                                // 重置相机到默认视角
    void resetAxisRange();                                                                           // 重置坐标轴范围
    void setViewFront();                                                                             // 设置前视图（Z轴方向）
    void setViewTop();                                                                               // 设置俯视图（Y轴方向）
    void setViewSide();                                                                              // 设置侧视图（X轴方向）
    void resetCamera();                                                                              // 重置相机

    // ===== 图例操作 =====
    void setLegendVisible(bool visible);                                                              // 显示/隐藏图例
    void setLegendPosition(LegendPosition pos);                                                       // 设置图例位置

    // ===== 标题操作 =====
    /**
     * @brief 设置窗口标题文本。
     *
     * 在窗口顶部居中显示标题文本，设置后自动显示。
     *
     * @param title 标题文本内容。
     */
    void setTitle(const QString &title);

    /**
     * @brief 设置标题可见性。
     *
     * @param visible true 显示标题，false 隐藏标题。
     */
    void setTitleVisible(bool visible);

    /**
     * @brief 设置标题颜色。
     *
     * @param color 标题文本颜色。
     */
    void setTitleColor(const QColor &color);

    /**
     * @brief 设置标题字号。
     *
     * @param size 字号大小（默认 18）。
     */
    void setTitleFontSize(int size);

    // ===== 悬浮数据显示 =====
    /**
     * @brief 设置鼠标悬浮数据显示的启用状态。
     *
     * 启用后，鼠标在场景内移动时会自动拾取最近的数据点，
     * 并在鼠标附近显示坐标及标量值信息。
     *
     * @param enabled true 启用悬浮显示，false 禁用。
     */
    void setHoverDisplayEnabled(bool enabled);

    /**
     * @brief 获取鼠标悬浮数据显示的启用状态。
     * @return bool true 表示已启用，false 表示已禁用。
     */
    bool isHoverDisplayEnabled() const;

    /**
     * @brief 设置悬浮拾取的容差值。
     *
     * 容差越大，鼠标离数据点越远也能被拾取到；容差越小则要求越精确。
     * 默认值为 0.02。
     *
     * @param tolerance 拾取容差值（世界坐标单位）。
     */
    void setHoverTolerance(double tolerance);

    // ===== 曲线操作（委托给 vtkCurve）=====
    vtkCurve* addCurve(const QVector<QVector3D> &points, const QColor &color, double lineWidth);    // 添加曲线
    vtkCurve* addCurve(const QVector<QVector3D> &points, double lineWidth = 2.0);                    // 添加曲线（自动颜色）
    void setCurveVisible(vtkCurve* curve, bool visible);                                             // 设置曲线可见性
    void setCurveColor(vtkCurve* curve, const QColor &color);                                       // 设置曲线颜色
    void setCurveWidth(vtkCurve* curve, double width);                                              // 设置曲线线宽
    void updateCurveData(vtkCurve* curve, const QVector<QVector3D> &points);                       // 更新曲线数据
    void removeCurve(vtkCurve* curve);                                                               // 移除曲线
    void clearAllCurves();                                                                            // 清除所有曲线
    QList<vtkCurve*> getCurves() const;                                                              // 获取所有曲线

    // ===== 标记点操作（委托给 vtkMarker）=====
    vtkMarker* addHollowMarker(const QVector3D &position, const QColor &color,                       // 添加空心环标记
                               double screenSize = 10.0, double lineWidth = 2.0);
    vtkMarker* addHollowMarker(const QVector3D &position);                                          // 添加空心环（自动颜色）
    vtkMarker* addFilledMarker(const QVector3D &position, const QColor &color,                     // 添加填充圆标记
                               double screenSize = 10.0);
    vtkMarker* addFilledMarker(const QVector3D &position);                                         // 添加填充圆（自动颜色）
    void setMarkerVisible(vtkMarker* marker, bool visible);                                          // 设置标记可见性
    void setMarkerColor(vtkMarker* marker, const QColor &color);                                    // 设置标记颜色
    void setMarkerRadius(vtkMarker* marker, double radius);                                         // 设置标记半径
    void setMarkerRelativeRadius(vtkMarker* marker, double ratio);                                  // 设置相对半径
    void setMarkerScreenSize(vtkMarker* marker, double screenSize);                                 // 设置屏幕大小
    void updateMarkerPosition(vtkMarker* marker, const QVector3D &position);                        // 更新标记位置
    void removeMarker(vtkMarker* marker);                                                            // 移除标记
    void clearAllMarkers();                                                                           // 清除所有标记
    QList<vtkMarker*> getMarkers() const;                                                            // 获取所有标记

    // ===== 曲面操作（委托给 vtkSurface）=====
    vtkSurface* addSurface(const QVector<QVector3D> &points, int nx, int ny,                        // 添加曲面
                           const QColor &color, double opacity);
    vtkSurface* addSurface(const QVector<QVector3D> &points, int nx, int ny, double opacity = 0.7); // 添加曲面（自动颜色）
    void setSurfaceVisible(vtkSurface* surface, bool visible);                                        // 设置曲面可见性
    void setSurfaceColor(vtkSurface* surface, const QColor &color);                                   // 设置曲面颜色
    void setSurfaceOpacity(vtkSurface* surface, double opacity);                                       // 设置曲面不透明度
    void removeSurface(vtkSurface* surface);                                                          // 移除曲面
    void clearAllSurfaces();                                                                           // 清除所有曲面
    QList<vtkSurface*> getSurfaces() const;                                                           // 获取所有曲面

    // ===== 热力图操作（委托给 vtkHeatmap）=====
    vtkHeatmap* addHeatmapSurface(const QVector<QVector3D> &points, int nx, int ny,                 // 添加热力图
                                  const QString &colorBarTitle = "");
    void setHeatmapSurfaceVisible(vtkHeatmap* heatmap, bool visible);                                // 设置热力图可见性
    void setHeatmapSurfaceOpacity(vtkHeatmap* heatmap, double opacity);                              // 设置不透明度
    void setHeatmapColorBarVisible(bool visible);                                                    // 设置颜色条可见性
    void setHeatmapColorBarTitle(const QString &title);                                              // 设置颜色条标题
    void setHeatmapContourVisible(vtkHeatmap* heatmap, bool visible);                               // 设置等高线可见性
    void setHeatmapContourCount(vtkHeatmap* heatmap, int count);                                    // 设置等高线数量
    void removeHeatmapSurface(vtkHeatmap* heatmap);                                                  // 移除热力图
    void clearAllHeatmapSurfaces();                                                                  // 清除所有热力图
    QList<vtkHeatmap*> getHeatmapSurfaces() const;                                                   // 获取所有热力图

    // ===== 清除所有 =====
    void clearAll();                                                                                  // 清除所有对象

    // ===== 内部访问方法（供 vtkMarker 等调用）=====
    vtkRenderer* renderer() const { return m_renderer; }                                            // 获取渲染器
    vtkScalarBarActor* scalarBarActor() const { return m_scalarBarActor; }                         // 获取颜色条
    double xMin() const { return m_xMin; }
    double xMax() const { return m_xMax; }
    double yMin() const { return m_yMin; }
    double yMax() const { return m_yMax; }
    double zMin() const { return m_zMin; }
    double zMax() const { return m_zMax; }

private:
    Ui::vtkPlotBase *ui;

    // VTK 组件
    QVTKOpenGLNativeWidget *m_vtkWidget;                    // VTK Qt 控件
    QWidget *m_overlayWindow;                               // VTK 独立顶层窗口（不参与主窗口 RHI 合成）
    QTimer *m_syncTimer;                                    // 位置同步定时器
    QSize m_lastVtkSize;                                    // 上一次 VTK 控件尺寸（去重 resize）
    vtkSmartPointer<vtkRenderer> m_renderer;                // 渲染器
    vtkSmartPointer<vtkCubeAxesActor> m_cubeAxesActor;      // 三维坐标轴演员
    vtkSmartPointer<vtkLegendBoxActor> m_legendActor;       // 图例演员
    vtkSmartPointer<vtkScalarBarActor> m_scalarBarActor;    // 颜色条演员
    vtkSmartPointer<vtkCallbackCommand> m_cameraCallback;   // 相机回调命令

    // 对象集合
    QList<vtkCurve*> m_curves;                             // 曲线列表
    QList<vtkMarker*> m_markers;                           // 标记点列表
    QList<vtkSurface*> m_surfaces;                          // 曲面列表
    QList<vtkHeatmap*> m_heatmapSurfaces;                   // 热力图列表

    // 坐标轴范围
    double m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax;
    
    // 自适应缩放
    AutoScaleMode m_autoScaleMode;
    double m_autoScaleMargin;                               // 边距百分比

    /**
     * @brief StretchFill 模式下的原始坐标轴标签范围。
     *
     * 几何体归一化到 [0,1] 空间后，坐标轴标签需显示原始数据范围，
     * 这组变量存储原始数据边界，在 updateAxesBounds() 中覆盖刻度标签。
     */
    double m_labelXMin, m_labelXMax;
    double m_labelYMin, m_labelYMax;
    double m_labelZMin, m_labelZMax;

    /**
     * @brief 标签范围是否已初始化标志。
     *
     * 首次添加 StretchFill 模式数据时设为 true。
     */
    bool m_labelRangesValid;
    
    // 默认相机参数
    double m_defaultCameraPosition[3];
    double m_defaultCameraFocalPoint[3];
    double m_defaultCameraViewUp[3];
    double m_defaultCameraDistance;
    
    // 图例
    bool m_legendVisible;
    LegendPosition m_legendPosition;

    // 标题
    /**
     * @brief 标题文本演员，用于在窗口顶部显示标题。
     */
    vtkSmartPointer<vtkTextActor> m_titleActor;

    /**
     * @brief 标题可见性标志。
     */
    bool m_titleVisible;

    /**
     * @brief 相机是否已初始化标志。
     *
     * 首次添加数据后会自动初始化相机位置。
     */
    bool m_cameraInitialized;
    
    // 自动颜色
    int m_autoColorIndex;
    QColor getNextAutoColor();

    // 悬浮数据显示
    /**
     * @brief 悬浮信息文本演员，在鼠标附近显示拾取到的数据。
     */
    vtkSmartPointer<vtkTextActor> m_hoverTextActor;

    /**
     * @brief 点拾取器，用于在鼠标移动时拾取最近的数据点。
     */
    vtkSmartPointer<vtkPointPicker> m_pointPicker;

    /**
     * @brief 悬浮显示是否启用的标志位。
     */
    bool m_hoverDisplayEnabled;

    /**
     * @brief 悬浮拾取容差值（世界坐标单位）。
     */
    double m_hoverTolerance;

    // 初始化和配置
    void setupVTK();                                        // 初始化 VTK
    void syncWindow();                                      // 同步顶层窗口位置与尺寸
    void createAxes();                                      // 创建坐标轴
    void createLegend();                                    // 创建图例
    void createScalarBar();                                 // 创建颜色条
    void createTitle();                                     // 创建标题
    void createHoverDisplay();                               // 创建悬浮显示组件
    void saveDefaultCamera();                               // 保存默认相机参数
    
    // 更新方法
    void updateLegend();                                    // 更新图例
    void updateLegendPosition();                            // 更新图例位置
    void updateAxesBounds();                                // 更新坐标轴边界
    void updateScalarBar();                                 // 更新颜色条
    void computeDataBounds(double &xMin, double &xMax, double &yMin, double &yMax, double &zMin, double &zMax);  // 计算数据边界
    void autoScaleIfNeeded();                               // 按需自适应缩放
    void updateAllMarkerScales();                           // 更新所有标记点缩放
    void updateAllScreenMarkerScales();                     // 更新屏幕固定大小标记

    // StretchFill 辅助方法

    /**
     * @brief 计算点集的三轴边界。
     *
     * @param points 输入点集。
     * @param xMin X 轴最小值输出。
     * @param xMax X 轴最大值输出。
     * @param yMin Y 轴最小值输出。
     * @param yMax Y 轴最大值输出。
     * @param zMin Z 轴最小值输出。
     * @param zMax Z 轴最大值输出。
     */
    void computePointsBounds(const QVector<QVector3D> &points,
                             double &xMin, double &xMax,
                             double &yMin, double &yMax,
                             double &zMin, double &zMax);

    /**
     * @brief 在 StretchFill 模式下归一化点集并更新标签范围。
     *
     * 将点集各轴映射到 [0,1]，并将原始数据范围取并集更新到标签范围变量。
     * 非 StretchFill 模式时不做任何操作。
     *
     * @param points 输入点集，将被就地归一化。
     * @return true 执行了归一化，false 未执行（非 StretchFill 模式）。
     */
    bool applyStretchFillIfNeeded(QVector<QVector3D> &points);
    
    // 相机回调
    void setupCameraCallback();
    static void cameraCallback(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);
    
    void render();                                          // 渲染场景
};

#endif // VTKPLOTBASE_H
