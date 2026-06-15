#[=======================================================================[.rst:
VTKChartConfig.cmake - VTK 图表库 CMake 配置文件
---------------------------------------------------

使用方法：
  1. 将本库目录添加到 CMAKE_PREFIX_PATH 或设置 VTKChart_DIR
  2. 在 CMakeLists.txt 中添加：

      find_package(VTKChart REQUIRED)
      target_link_libraries(your_target PRIVATE VTKChart::VTKChart)

依赖：Qt 6.x + VTK 9.6
#]=======================================================================]

cmake_minimum_required(VERSION 3.16)

# 获取本文件所在目录
get_filename_component(VTKCHART_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# ==================== 查找依赖 ====================

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

# VTK 9
if(NOT VTK_DIR)
    set(VTK_DIR "$ENV{VTK_DIR}")
endif()

if(NOT VTK_DIR)
    # 尝试默认路径
    if(EXISTS "${VTKCHART_CMAKE_DIR}/3rdparty/VTK-9.6")
        set(VTK_DIR "${VTKCHART_CMAKE_DIR}/3rdparty/VTK-9.6")
    endif()
endif()

# VTK 头文件和库文件路径
set(VTK_INCLUDE_DIR "${VTK_DIR}/include/vtk-9.6")
set(VTK_LIB_DIR "${VTK_DIR}/lib")
set(VTK_BIN_DIR "${VTK_DIR}/bin")

if(NOT EXISTS "${VTK_INCLUDE_DIR}")
    message(FATAL_ERROR "VTK 9.6 not found. Set VTK_DIR to VTK installation path.")
endif()

# ==================== VTK 库列表 ====================
set(VTK_LIBRARIES
    vtkCommonCore-9.6
    vtkCommonDataModel-9.6
    vtkCommonExecutionModel-9.6
    vtkCommonMath-9.6
    vtkCommonMisc-9.6
    vtkCommonTransforms-9.6
    vtkCommonColor-9.6
    vtkRenderingCore-9.6
    vtkRenderingOpenGL2-9.6
    vtkRenderingAnnotation-9.6
    vtkRenderingContext2D-9.6
    vtkRenderingContextOpenGL2-9.6
    vtkChartsCore-9.6
    vtkViewsContext2D-9.6
    vtkRenderingFreeType-9.6
    vtkInteractionStyle-9.6
    vtkFiltersCore-9.6
    vtkFiltersSources-9.6
    vtkFiltersGeneral-9.6
    vtkGUISupportQt-9.6
    vtksys-9.6
)

# ==================== 源文件列表 ====================
set(VTKCHART_SOURCES
    ${VTKCHART_CMAKE_DIR}/vtkplotbase.cpp
    ${VTKCHART_CMAKE_DIR}/vtkplot2d.cpp
    ${VTKCHART_CMAKE_DIR}/drawable/vtkcurve.cpp
    ${VTKCHART_CMAKE_DIR}/drawable/vtkmarker.cpp
    ${VTKCHART_CMAKE_DIR}/drawable/vtksurface.cpp
    ${VTKCHART_CMAKE_DIR}/drawable/vtkheatmap.cpp
    ${VTKCHART_CMAKE_DIR}/drawable/vtkheatmap2d.cpp
    ${VTKCHART_CMAKE_DIR}/drawable/vtkmarkergroup2d.cpp
)

set(VTKCHART_HEADERS
    ${VTKCHART_CMAKE_DIR}/vtkplotbase.h
    ${VTKCHART_CMAKE_DIR}/vtkplot2d.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtkdrawable.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtkchartdrawable.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtkcurve.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtkmarker.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtksurface.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtkheatmap.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtkheatmap2d.h
    ${VTKCHART_CMAKE_DIR}/drawable/vtkmarkergroup2d.h
)

set(VTKCHART_FORMS
    ${VTKCHART_CMAKE_DIR}/vtkplotbase.ui
    ${VTKCHART_CMAKE_DIR}/vtkplot2d.ui
)

# ==================== 创建库目标 ====================

# 自动处理 UI 文件
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 创建静态库（也可改为 SHARED）
add_library(VTKChart STATIC ${VTKCHART_SOURCES} ${VTKCHART_HEADERS} ${VTKCHART_FORMS})

# 设置 C++17 标准
target_compile_features(VTKChart PUBLIC cxx_std_17)

# 头文件包含路径
target_include_directories(VTKChart
    PUBLIC
        ${VTKCHART_CMAKE_DIR}
        ${VTKCHART_CMAKE_DIR}/drawable
        ${VTK_INCLUDE_DIR}
)

# 链接库
target_link_libraries(VTKChart
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
    PRIVATE
        ${VTK_LIBRARIES}
)

# 链接库搜索路径
target_link_directories(VTKChart PRIVATE ${VTK_LIB_DIR})

# 创建别名目标，便于使用 find_package 风格
add_library(VTKChart::VTKChart ALIAS VTKChart)

# ==================== Windows DLL 复制 ====================
if(WIN32)
    # 构建后复制 VTK DLL 到输出目录
    add_custom_command(TARGET VTKChart POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VTK_BIN_DIR}"
            "$<TARGET_FILE_DIR:VTKChart>"
        COMMENT "Copying VTK DLLs to output directory"
    )
endif()

# ==================== 导出配置 ====================
set(VTKChart_FOUND TRUE)
set(VTKChart_INCLUDE_DIRS
    ${VTKCHART_CMAKE_DIR}
    ${VTKCHART_CMAKE_DIR}/drawable
    ${VTK_INCLUDE_DIR}
)
set(VTKChart_LIBRARIES VTKChart::VTKChart)

message(STATUS "VTKChart found at: ${VTKCHART_CMAKE_DIR}")
message(STATUS "  VTK include: ${VTK_INCLUDE_DIR}")
message(STATUS "  VTK lib: ${VTK_LIB_DIR}")
