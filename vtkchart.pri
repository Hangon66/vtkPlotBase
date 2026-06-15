#-------------------------------------------------
# vtkchart.pri - VTK 图表库 Qt 项目包含文件
#
# 使用方法：
#   1. 将本文件复制到项目根目录或 libs/ 目录
#   2. 在 .pro 文件中添加：include(vtkchart.pri)
#
# 依赖：Qt 6.x + VTK 9.6
#-------------------------------------------------

QT += core gui opengl openglwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# ==================== VTK 配置 ====================
# VTK 安装路径，可通过环境变量 VTK_DIR 覆盖
isEmpty(VTK_DIR) {
    VTK_DIR = $$(VTK_DIR)
}
isEmpty(VTK_DIR) {
    VTK_DIR = $$PWD/3rdparty/VTK-9.6
}

exists($$VTK_DIR/include/vtk-9.6) {
    INCLUDEPATH += $$VTK_DIR/include/vtk-9.6

    LIBS += -L$$VTK_DIR/lib \
            -lvtkCommonCore-9.6 \
            -lvtkCommonDataModel-9.6 \
            -lvtkCommonExecutionModel-9.6 \
            -lvtkCommonMath-9.6 \
            -lvtkCommonMisc-9.6 \
            -lvtkCommonTransforms-9.6 \
            -lvtkCommonColor-9.6 \
            -lvtkRenderingCore-9.6 \
            -lvtkRenderingOpenGL2-9.6 \
            -lvtkRenderingAnnotation-9.6 \
            -lvtkRenderingContext2D-9.6 \
            -lvtkRenderingContextOpenGL2-9.6 \
            -lvtkChartsCore-9.6 \
            -lvtkViewsContext2D-9.6 \
            -lvtkRenderingFreeType-9.6 \
            -lvtkInteractionStyle-9.6 \
            -lvtkFiltersCore-9.6 \
            -lvtkFiltersSources-9.6 \
            -lvtkFiltersGeneral-9.6 \
            -lvtkGUISupportQt-9.6 \
            -lvtksys-9.6

    # Windows: 自动复制 VTK DLL 到输出目录
    win32 {
        VTK_BIN_DIR = $$VTK_DIR/bin
        CONFIG(debug, debug|release) {
            BUILD_DIR = $$OUT_PWD/debug
        } else {
            BUILD_DIR = $$OUT_PWD/release
        }
        VTK_BIN_WIN = $$shell_path($$VTK_BIN_DIR)
        BUILD_WIN = $$shell_path($$BUILD_DIR)
        QMAKE_POST_LINK += cmd /c xcopy /Y /Q "$$VTK_BIN_WIN\*.dll" "$$BUILD_WIN" $$escape_expand(\n)
    }
} else {
    warning("VTK not found at $$VTK_DIR, please set VTK_DIR environment variable")
}

# ==================== 图表库源文件 ====================
VTKCHART_DIR = $$PWD

INCLUDEPATH += $$VTKCHART_DIR \
               $$VTKCHART_DIR/drawable

SOURCES += \
    $$VTKCHART_DIR/vtkplotbase.cpp \
    $$VTKCHART_DIR/vtkplot2d.cpp \
    $$VTKCHART_DIR/drawable/vtkcurve.cpp \
    $$VTKCHART_DIR/drawable/vtkmarker.cpp \
    $$VTKCHART_DIR/drawable/vtksurface.cpp \
    $$VTKCHART_DIR/drawable/vtkheatmap.cpp \
    $$VTKCHART_DIR/drawable/vtkheatmap2d.cpp \
    $$VTKCHART_DIR/drawable/vtkmarkergroup2d.cpp

HEADERS += \
    $$VTKCHART_DIR/vtkplotbase.h \
    $$VTKCHART_DIR/vtkplot2d.h \
    $$VTKCHART_DIR/drawable/vtkdrawable.h \
    $$VTKCHART_DIR/drawable/vtkchartdrawable.h \
    $$VTKCHART_DIR/drawable/vtkcurve.h \
    $$VTKCHART_DIR/drawable/vtkmarker.h \
    $$VTKCHART_DIR/drawable/vtksurface.h \
    $$VTKCHART_DIR/drawable/vtkheatmap.h \
    $$VTKCHART_DIR/drawable/vtkheatmap2d.h \
    $$VTKCHART_DIR/drawable/vtkmarkergroup2d.h

FORMS += \
    $$VTKCHART_DIR/vtkplotbase.ui \
    $$VTKCHART_DIR/vtkplot2d.ui
