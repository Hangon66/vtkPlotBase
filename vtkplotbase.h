#ifndef VTKPLOTBASE_H
#define VTKPLOTBASE_H

#include <QWidget>
#include <QVBoxLayout>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkCubeAxesActor.h>
#include <vtkRenderer.h>
#include <vtkLineSource.h>
#include <vtkSphereSource.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkDiskSource.h>
#include <vtkFollower.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>

// Forward declarations for VTK Qt widget
class QVTKOpenGLNativeWidget;

class vtkRenderWindow;

class vtkRenderWindowInteractor;

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkPlotBase;
}
QT_END_NAMESPACE

class vtkPlotBase : public QWidget
{
    Q_OBJECT

public:
    vtkPlotBase(QWidget *parent = nullptr);
    ~vtkPlotBase();

private:
    Ui::vtkPlotBase *ui;

    // VTK components
    QVTKOpenGLNativeWidget *m_vtkWidget;
    vtkSmartPointer<vtkRenderer> m_renderer;

    // Cube Axes (MATLAB style)
    vtkSmartPointer<vtkCubeAxesActor> m_cubeAxesActor;

    // Curve
    vtkSmartPointer<vtkPolyData> m_curveData;
    vtkSmartPointer<vtkPolyDataMapper> m_lineMapper;
    vtkSmartPointer<vtkActor> m_lineActor;

    // Key points (markers at start and end - MATLAB style circle)
    vtkSmartPointer<vtkDiskSource> m_diskSource;
    vtkSmartPointer<vtkPolyDataMapper> m_keyPointsMapper;
    vtkSmartPointer<vtkFollower> m_startMarker;
    vtkSmartPointer<vtkFollower> m_endMarker;

    void setupVTK();
    void createAxes();
    void createCurve();
    void createKeyPoints();
};
#endif // VTKPLOTBASE_H
