/**
 * @file main.cpp
 * @brief 主程序 - 二维热力图标记点示例
 */

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

// 封装类
#include "vtkplot2d.h"
#include "drawable/vtkheatmap2d.h"
#include "drawable/vtkmarkergroup2d.h"

#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChartHistogram2D.h>
#include <vtkColorLegend.h>
#include <vtkColorTransferFunction.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlotPoints.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <vtkTextProperty.h>

#include <vector>
#include <utility>
#include <cmath>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    return app.exec();
}
