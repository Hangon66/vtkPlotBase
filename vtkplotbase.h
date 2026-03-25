#ifndef VTKPLOTBASE_H
#define VTKPLOTBASE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QUuid>
#include <QVector>
#include <QVector3D>
#include <QColor>

// VTK 头文件
#include <vtkSmartPointer.h>
#include <vtkCubeAxesActor.h>
#include <vtkRenderer.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkQuad.h>
#include <vtkDiskSource.h>
#include <vtkRegularPolygonSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkFollower.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLegendBoxActor.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkContourFilter.h>
#include <vtkStripper.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>

// VTK Qt 控件前向声明
class QVTKOpenGLNativeWidget;
class vtkRenderWindow;
class vtkRenderWindowInteractor;

// 曲线数据结构
struct CurveData
{
    QString id;                                          // 唯一标识符
    QString name;                                         // 图例名称
    vtkSmartPointer<vtkPolyData> polyData;                // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> mapper;            // 映射器
    vtkSmartPointer<vtkActor> actor;                      // 演员
    bool visible;                                         // 可见性
};

// 标记点大小模式
enum class MarkerSizeMode {
    Absolute,    // 绝对半径模式（固定世界坐标值）
    Relative,    // 相对半径模式（占X轴范围的比例）
    Screen       // 屏幕固定大小模式（像素）
};

// 标记点数据结构
struct MarkerData
{
    QString id;                                           // 唯一标识符
    QString name;                                         // 图例名称
    vtkSmartPointer<vtkPolyData> polyData;                // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> mapper;            // 映射器
    vtkSmartPointer<vtkFollower> follower;                // 跟随者（始终面向相机）
    bool filled;                                          // true=填充圆, false=空心环
    bool visible;                                         // 可见性
    double radius;                                        // 半径（世界坐标）
    double lineWidth;                                     // 线宽
    double relativeRadius;                                // 相对半径（占X轴范围的比例）
    double screenSize;                                    // 屏幕大小（像素）
    MarkerSizeMode sizeMode;                              // 大小模式
};

// 曲面数据结构
struct SurfaceData
{
    QString id;                                           // 唯一标识符
    QString name;                                         // 图例名称
    vtkSmartPointer<vtkPolyData> polyData;                // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> mapper;            // 映射器
    vtkSmartPointer<vtkActor> actor;                      // 演员
    bool visible;                                         // 可见性
    double opacity;                                       // 不透明度 (0.0 - 1.0)
};

// 热力图曲面数据结构
struct HeatmapSurfaceData
{
    QString id;                                           // 唯一标识符
    QString name;                                         // 图例名称
    vtkSmartPointer<vtkPolyData> polyData;                // 多边形数据
    vtkSmartPointer<vtkPolyDataMapper> mapper;            // 映射器
    vtkSmartPointer<vtkActor> actor;                      // 演员
    vtkSmartPointer<vtkLookupTable> lookupTable;          // 颜色查找表
    bool visible;                                         // 可见性
    double opacity;                                       // 不透明度 (0.0 - 1.0)
    double zMin;                                          // Z值最小值
    double zMax;                                          // Z值最大值
    
    // 等高线投影相关
    vtkSmartPointer<vtkPolyData> contourPolyData;         // 等高线数据
    vtkSmartPointer<vtkPolyDataMapper> contourMapper;     // 等高线映射器
    vtkSmartPointer<vtkActor> contourActor;               // 等高线演员
    bool contourVisible;                                  // 等高线可见性
    int contourCount;                                     // 等高线数量
    double contourBaseY;                                  // 等高线投影基准Y值
};

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkPlotBase;
}
QT_END_NAMESPACE

// 自适应缩放模式
enum class AutoScaleMode {
    None,         // 无自动缩放（固定坐标轴范围）
    Independent,  // 各轴独立缩放
    EqualRatio    // 等比例缩放（1:1:1）
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

private:
    class vtkPlotBase* m_plotBase = nullptr;
};

class vtkPlotBase : public QWidget
{
    Q_OBJECT
    friend class vtkPlotBaseInteractorStyle;

public:
    explicit vtkPlotBase(QWidget *parent = nullptr);
    ~vtkPlotBase();

protected:
    void showEvent(QShowEvent *event) override;    // 窗口显示事件，用于更新屏幕固定大小标记点

public:
    // ===== 坐标系操作 =====
    void setAxisRange(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);  // 设置坐标轴范围
    void setAxisTitles(const QString &xTitle, const QString &yTitle, const QString &zTitle);          // 设置坐标轴标题
    void setGridVisible(bool visible);                                                               // 设置网格可见性
    void setBackground(const QColor &color);                                                          // 设置背景颜色
    void setAutoScaleMode(AutoScaleMode mode);                                                        // 设置自适应缩放模式（默认：Independent）
    AutoScaleMode autoScaleMode() const;                                                             // 获取当前自适应缩放模式
    void autoFit();                                                                                  // 自适应所有数据
    void resetView();                                                                                // 重置相机到默认视角
    void resetAxisRange();                                                                           // 重置坐标轴范围（触发自适应）
    void setViewFront();                                                                             // 设置前视图（Z轴方向）
    void setViewTop();                                                                               // 设置俯视图（Y轴方向）
    void setViewSide();                                                                              // 设置侧视图（X轴方向）
    void resetCamera();                                                                              // 重置相机（自动适配所有数据）

    // ===== 图例操作 =====
    void setLegendVisible(bool visible);                                                              // 显示/隐藏图例
    void setLegendPosition(LegendPosition pos);                                                       // 设置图例位置
    void setCurveName(const QString &curveId, const QString &name);                                   // 设置曲线图例名称
    void setMarkerName(const QString &markerId, const QString &name);                                 // 设置标记点图例名称

    // ===== 曲线操作 =====
    QString addCurve(const QVector<QVector3D> &points,                                                // 添加曲线（指定颜色）
                     const QColor &color,
                     double lineWidth);
    QString addCurve(const QVector<QVector3D> &points, double lineWidth = 2.0);                          // 添加曲线（自动颜色）
    void setCurveVisible(const QString &curveId, bool visible);                                       // 设置曲线可见性
    void setCurveColor(const QString &curveId, const QColor &color);                                  // 设置曲线颜色
    void setCurveWidth(const QString &curveId, double width);                                        // 设置曲线线宽
    void updateCurveData(const QString &curveId, const QVector<QVector3D> &points);                   // 更新曲线数据
    void removeCurve(const QString &curveId);                                                         // 移除曲线
    void clearAllCurves();                                                                            // 清除所有曲线
    QStringList getCurveIds() const;                                                                  // 获取所有曲线ID

    // ===== 空心环标记操作 =====
    QString addHollowMarker(const QVector3D &position,                                                // 添加空心环标记（屏幕固定大小）
                            const QColor &color,
                            double screenSize = 10.0,
                            double lineWidth = 2.0);
    QString addHollowMarker(const QVector3D &position);                                               // 添加空心环标记（自动颜色，默认屏幕大小10像素）
    QString addScreenHollowMarker(const QVector3D &position,                                          // 添加屏幕固定大小空心环标记
                                  const QColor &color,
                                  double screenSize = 10.0,
                                  double lineWidth = 2.0);
    QString addScreenHollowMarker(const QVector3D &position);                                         // 添加屏幕固定大小空心环标记（自动颜色，默认10像素）
    void setMarkerVisible(const QString &markerId, bool visible);                                     // 设置标记可见性
    void setMarkerColor(const QString &markerId, const QColor &color);                                // 设置标记颜色
    void setMarkerRadius(const QString &markerId, double radius);                                     // 设置标记半径（绝对值）
    void setMarkerRelativeRadius(const QString &markerId, double ratio);                              // 设置标记相对半径（占X轴范围的比例，如0.02表示2%）
    void setMarkerScreenSize(const QString &markerId, double screenSize);                             // 设置标记屏幕大小（像素）
    void setMarkerLineWidth(const QString &markerId, double lineWidth);                               // 设置标记线宽
    void updateMarkerPosition(const QString &markerId, const QVector3D &position);                   // 更新标记位置
    void removeMarker(const QString &markerId);                                                       // 移除标记
    void clearAllMarkers();                                                                           // 清除所有空心环标记
    QStringList getMarkerIds() const;                                                                 // 获取所有标记ID

    // ===== 填充圆标记操作 =====
    QString addFilledMarker(const QVector3D &position,                                                // 添加填充圆标记（屏幕固定大小）
                            const QColor &color,
                            double screenSize = 10.0);
    QString addFilledMarker(const QVector3D &position);                                               // 添加填充圆标记（自动颜色，默认屏幕大小10像素）
    QString addScreenFilledMarker(const QVector3D &position,                                          // 添加屏幕固定大小填充圆标记
                                  const QColor &color,
                                  double screenSize = 10.0);
    QString addScreenFilledMarker(const QVector3D &position);                                         // 添加屏幕固定大小填充圆标记（自动颜色，默认10像素）
    void setFilledMarkerVisible(const QString &markerId, bool visible);                               // 设置标记可见性
    void setFilledMarkerColor(const QString &markerId, const QColor &color);                          // 设置标记颜色
    void setFilledMarkerRadius(const QString &markerId, double radius);                               // 设置标记半径（绝对值）
    void setFilledMarkerRelativeRadius(const QString &markerId, double ratio);                        // 设置标记相对半径（占X轴范围的比例，如0.02表示2%）
    void setFilledMarkerScreenSize(const QString &markerId, double screenSize);                       // 设置标记屏幕大小（像素）
    void updateFilledMarkerPosition(const QString &markerId, const QVector3D &position);             // 更新标记位置
    void removeFilledMarker(const QString &markerId);                                                 // 移除标记
    void clearAllFilledMarkers();                                                                     // 清除所有填充圆标记
    QStringList getFilledMarkerIds() const;                                                           // 获取所有填充标记ID

    // ===== 曲面操作 =====
    QString addSurface(const QVector<QVector3D> &points, int nx, int ny,                              // 添加曲面（网格点）
                       const QColor &color,
                       double opacity);
    QString addSurface(const QVector<QVector3D> &points, int nx, int ny, double opacity = 0.7);          // 添加曲面（自动颜色）
    void setSurfaceVisible(const QString &surfaceId, bool visible);                                    // 设置曲面可见性
    void setSurfaceColor(const QString &surfaceId, const QColor &color);                               // 设置曲面颜色
    void setSurfaceOpacity(const QString &surfaceId, double opacity);                                  // 设置曲面不透明度
    void setSurfaceName(const QString &surfaceId, const QString &name);                                // 设置曲面图例名称
    void removeSurface(const QString &surfaceId);                                                      // 移除曲面
    void clearAllSurfaces();                                                                           // 清除所有曲面
    QStringList getSurfaceIds() const;                                                                 // 获取所有曲面ID

    // ===== 热力图曲面操作 =====
    QString addHeatmapSurface(const QVector<QVector3D> &points, int nx, int ny,                        // 添加热力图曲面
                              const QString &colorBarTitle = "Z Value");
    void setHeatmapSurfaceVisible(const QString &surfaceId, bool visible);                             // 设置热力图曲面可见性
    void setHeatmapSurfaceOpacity(const QString &surfaceId, double opacity);                           // 设置热力图曲面不透明度
    void setHeatmapSurfaceName(const QString &surfaceId, const QString &name);                         // 设置热力图曲面图例名称
    void setHeatmapColorBarVisible(bool visible);                                                      // 设置颜色条可见性
    void setHeatmapColorBarTitle(const QString &title);                                                // 设置颜色条标题
    void setHeatmapContourVisible(const QString &surfaceId, bool visible);                             // 设置等高线可见性
    void setHeatmapContourCount(const QString &surfaceId, int count);                                  // 设置等高线数量
    void removeHeatmapSurface(const QString &surfaceId);                                               // 移除热力图曲面
    void clearAllHeatmapSurfaces();                                                                    // 清除所有热力图曲面
    QStringList getHeatmapSurfaceIds() const;                                                          // 获取所有热力图曲面ID

    // ===== 清除所有 =====
    void clearAll();                                                                                  // 清除所有曲线、标记和曲面

private:
    Ui::vtkPlotBase *ui;

    // VTK 组件
    QVTKOpenGLNativeWidget *m_vtkWidget;                    // VTK Qt 控件
    vtkSmartPointer<vtkRenderer> m_renderer;                // 渲染器
    vtkSmartPointer<vtkCubeAxesActor> m_cubeAxesActor;      // 三维坐标轴演员
    vtkSmartPointer<vtkLegendBoxActor> m_legendActor;       // 图例演员
    vtkSmartPointer<vtkCallbackCommand> m_cameraCallback;   // 相机回调命令（用于屏幕固定大小标记）

    // 数据存储
    QMap<QString, CurveData> m_curves;                      // 曲线集合<UUID, CurveData>
    QMap<QString, MarkerData> m_markers;                    // 标记点集合<UUID, MarkerData>
    QMap<QString, SurfaceData> m_surfaces;                  // 曲面集合<UUID, SurfaceData>
    QMap<QString, HeatmapSurfaceData> m_heatmapSurfaces;    // 热力图曲面集合<UUID, HeatmapSurfaceData>
    
    // 颜色条
    vtkSmartPointer<vtkScalarBarActor> m_scalarBarActor;    // 颜色条演员

    // 坐标轴范围
    double m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax;
    
    // 自适应缩放
    AutoScaleMode m_autoScaleMode;                          // 自适应缩放模式
    double m_autoScaleMargin;                               // 边距百分比 (0.0 - 1.0)
    
    // 默认相机参数
    double m_defaultCameraPosition[3];                      // 默认相机位置
    double m_defaultCameraFocalPoint[3];                    // 默认焦点位置
    double m_defaultCameraViewUp[3];                        // 默认上方向
    double m_defaultCameraDistance;                         // 默认距离
    
    // 图例
    bool m_legendVisible;                                   // 图例可见性
    LegendPosition m_legendPosition;                        // 图例位置
    
    // 自动颜色
    int m_autoColorIndex;                                   // 自动颜色索引
    QColor getNextAutoColor();                               // 获取下一个自动颜色

    void setupVTK();                                        // 初始化 VTK
    void createAxes();                                      // 创建坐标轴
    void createLegend();                                    // 创建图例
    void saveDefaultCamera();                               // 保存默认相机参数
    void updateLegend();                                    // 更新图例
    void updateLegendPosition();                            // 更新图例位置
    void updateLegendPositionForSize(double legendWidth, double legendHeight);  // 根据大小更新图例位置
    QString generateId();                                   // 生成唯一ID
    void updateAxesBounds();                                // 更新坐标轴边界
    void computeDataBounds(double &xMin, double &xMax, double &yMin, double &yMax, double &zMin, double &zMax);  // 计算数据边界
    void autoScaleIfNeeded();                               // 按需自适应缩放
    void updateScalarBar(double zMin, double zMax, const QString &title);  // 更新颜色条
    void createContourProjection(HeatmapSurfaceData &surface, double heightMin, double heightMax);  // 创建等高线投影
    void updateAllMarkerScales();                           // 更新所有标记点的缩放（基于相对半径）
    void updateMarkerScale(MarkerData &marker);             // 更新单个标记点的缩放
    void updateAllScreenMarkerScales();                     // 更新所有屏幕固定大小标记点的缩放
    void updateScreenMarkerScale(MarkerData &marker);       // 更新单个屏幕固定大小标记点的缩放
    void setupCameraCallback();                             // 设置相机回调（用于屏幕固定大小标记）
    static void cameraCallback(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);  // 相机回调静态函数
    void render();                                          // 渲染场景
};
#endif // VTKPLOTBASE_H
