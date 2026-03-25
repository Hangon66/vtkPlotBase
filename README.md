# vtkPlotBase - Qt + VTK 三维图表控件

基于 Qt 6.7.3 和 VTK 9.6 的三维可视化图表控件，提供 MATLAB 风格的三维坐标系和丰富的图表绑定功能。

## 功能特性

### 三维坐标系
- MATLAB 风格的三维坐标轴（vtkCubeAxesActor）
- 自适应数据边界缩放
- 等比例缩放模式
- 可配置的坐标轴标题

### 曲线
- 添加 3D 曲线（支持自动颜色）
- 自定义颜色、线宽
- 图例名称设置

### 标记点
- 空心环标记（Hollow Marker）
- 填充圆标记（Filled Marker）
- 自定义颜色、半径、线宽
- 始终面向相机

### 曲面
- 单色曲面可视化
- 自定义颜色和不透明度
- Phong 着色

### 热力图曲面
- 高度映射颜色（彩虹渐变）
- 等高线投影（投影到坐标系底部）
- 颜色条（Scalar Bar）
- 可配置等高线数量

### 图例
- 自动生成图例
- 可配置位置（左上角/右上角）
- 符号与文字间距优化

### 交互
- 鼠标旋转、平移、缩放
- 键盘视角切换（1-6 键）
- R 键重置视角

## 依赖

- Qt 6.7.3
- VTK 9.6
- MSVC 2019 64bit

## 快速开始

### 基本使用

```cpp
#include "vtkplotbase.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    vtkPlotBase w;
    w.setWindowTitle("3D Plot");
    w.resize(800, 600);
    w.setAxisTitles("X", "Y", "Z");
    
    // 添加曲线（自动颜色）
    QVector<QVector3D> points;
    for (int i = 0; i <= 100; ++i) {
        double t = i * 0.1;
        points.append(QVector3D(t, sin(t), 0));
    }
    QString curveId = w.addCurve(points);  // 自动选择颜色
    w.setCurveName(curveId, "正弦曲线");
    
    w.show();
    return a.exec();
}
```

### 热力图曲面

```cpp
// 创建 Sinc 函数曲面
const int nx = 60, nz = 60;
QVector<QVector3D> points;

for (int j = 0; j < nz; ++j) {
    for (int i = 0; i < nx; ++i) {
        double x = (i - nx/2.0) * 0.15;
        double z = (j - nz/2.0) * 0.15;
        double r = sqrt(x*x + z*z);
        double y = (r < 0.01) ? 1.0 : sin(r) / r;
        points.append(QVector3D(x, y, z));
    }
}

QString surfaceId = w.addHeatmapSurface(points, nx, nz, "Height");
w.setHeatmapContourCount(surfaceId, 8);  // 设置等高线数量
```

### 自动颜色

添加曲线、标记、曲面时不指定颜色，系统自动选择不重复的颜色：

```cpp
// 自动颜色（推荐）
w.addCurve(points1);              // 自动选择颜色1
w.addCurve(points2);              // 自动选择颜色2（不同颜色）
w.addHollowMarker(position);      // 自动选择颜色3

// 指定颜色
w.addCurve(points, Qt::red, 2.0);
```

预定义颜色列表：蓝、橙、绿、红、紫、棕、粉、灰、黄绿、青

## API 参考

### 曲线操作

| 方法 | 说明 |
|------|------|
| `addCurve(points, color, lineWidth)` | 添加曲线（指定颜色） |
| `addCurve(points, lineWidth)` | 添加曲线（自动颜色） |
| `setCurveColor(id, color)` | 设置曲线颜色 |
| `setCurveLineWidth(id, width)` | 设置曲线线宽 |
| `setCurveName(id, name)` | 设置图例名称 |
| `setCurveVisible(id, visible)` | 设置可见性 |
| `removeCurve(id)` | 移除曲线 |

### 标记操作

| 方法 | 说明 |
|------|------|
| `addHollowMarker(position, color, radius, lineWidth)` | 添加空心标记（指定颜色） |
| `addHollowMarker(position, radius, lineWidth)` | 添加空心标记（自动颜色） |
| `addFilledMarker(position, color, radius)` | 添加填充标记（指定颜色） |
| `addFilledMarker(position, radius)` | 添加填充标记（自动颜色） |
| `setMarkerName(id, name)` | 设置图例名称 |

### 曲面操作

| 方法 | 说明 |
|------|------|
| `addSurface(points, nx, ny, color, opacity)` | 添加曲面（指定颜色） |
| `addSurface(points, nx, ny, opacity)` | 添加曲面（自动颜色） |
| `setSurfaceOpacity(id, opacity)` | 设置不透明度 |

### 热力图操作

| 方法 | 说明 |
|------|------|
| `addHeatmapSurface(points, nx, ny, title)` | 添加热力图曲面 |
| `setHeatmapContourVisible(id, visible)` | 设置等高线可见性 |
| `setHeatmapContourCount(id, count)` | 设置等高线数量 |
| `setHeatmapColorBarVisible(visible)` | 设置颜色条可见性 |
| `setHeatmapColorBarTitle(title)` | 设置颜色条标题 |

### 图例操作

| 方法 | 说明 |
|------|------|
| `setLegendVisible(visible)` | 显示/隐藏图例 |
| `setLegendPosition(position)` | 设置图例位置 |

### 坐标系操作

| 方法 | 说明 |
|------|------|
| `setAxisTitles(x, y, z)` | 设置坐标轴标题 |
| `setAxisRange(xMin, xMax, yMin, yMax, zMin, zMax)` | 设置坐标轴范围 |
| `setAutoScaleMode(mode)` | 设置自适应缩放模式 |
| `resetView()` | 重置视角 |
| `resetAxisRange()` | 重置坐标轴范围 |

## 示例文件

| 文件 | 说明 |
|------|------|
| `example/example_curve.cpp` | 曲线示例：螺旋线、正弦、余弦、抛物线 |
| `example/example_hollow_marker.cpp` | 空心标记示例：关键点标记、立方体顶点 |
| `example/example_filled_marker.cpp` | 填充标记示例：散点分布、球面点 |
| `example/example_surface.cpp` | 曲面示例：抛物面、马鞍面、波浪面 |
| `example/example_heatmap.cpp` | 热力图示例：Sinc 函数、等高线投影 |

## 键盘快捷键

| 按键 | 功能 |
|------|------|
| `1` | 前视图（+Y 方向） |
| `2` | 后视图（-Y 方向） |
| `3` | 左视图（-X 方向） |
| `4` | 右视图（+X 方向） |
| `5` | 俯视图（+Z 方向） |
| `6` | 仰视图（-Z 方向） |
| `R` | 重置视角 |

## 项目结构

```
VTKtest/
├── 3rdparty/VTK-9.6/          # VTK 库
├── example/                    # 示例代码
│   ├── example_curve.cpp
│   ├── example_hollow_marker.cpp
│   ├── example_filled_marker.cpp
│   ├── example_surface.cpp
│   └── example_heatmap.cpp
├── vtkplotbase.h               # 控件头文件
├── vtkplotbase.cpp             # 控件实现
├── vtkplotbase.ui              # UI 文件
├── main.cpp                    # 主程序
└── VTKtest.pro                 # Qt 项目文件
```

## 许可证

MIT License
