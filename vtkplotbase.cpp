#include "vtkplotbase.h"
#include "ui_vtkplotbase.h"

// VTK Qt includes
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>
#include <vtkMath.h>

vtkPlotBase::vtkPlotBase(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::vtkPlotBase)
    , m_vtkWidget(nullptr)
{
    ui->setupUi(this);
    setupVTK();
}

vtkPlotBase::~vtkPlotBase()
{
    delete ui;
}

void vtkPlotBase::setupVTK()
{
    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Create VTK Qt widget
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(m_vtkWidget);

    // Get the render window
    vtkSmartPointer<vtkRenderWindow> renderWindow = m_vtkWidget->renderWindow();

    // Create renderer
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.1, 0.2, 0.4);  // Dark blue background
    renderWindow->AddRenderer(m_renderer);

    // Create axes, curve and key points
    createAxes();
    createCurve();
    createKeyPoints();

    // Setup camera
    m_renderer->ResetCamera();
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->Azimuth(30);
    camera->Elevation(30);

    // Set interactor style
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    renderWindow->GetInteractor()->SetInteractorStyle(style);
}

void vtkPlotBase::createAxes()
{
    // Create MATLAB-style 3D cube axes
    m_cubeAxesActor = vtkSmartPointer<vtkCubeAxesActor>::New();

    // Set camera for the axes (required for proper rendering)
    m_cubeAxesActor->SetCamera(m_renderer->GetActiveCamera());

    // Set axis ranges
    m_cubeAxesActor->SetXAxisRange(-1.0, 2.0);
    m_cubeAxesActor->SetYAxisRange(-1.0, 2.0);
    m_cubeAxesActor->SetZAxisRange(-1.0, 3.0);

    // Set axis titles
    m_cubeAxesActor->SetXTitle("X Axis");
    m_cubeAxesActor->SetYTitle("Y Axis");
    m_cubeAxesActor->SetZTitle("Z Axis");

    // Set axis line width
    m_cubeAxesActor->GetXAxesLinesProperty()->SetLineWidth(1.0);
    m_cubeAxesActor->GetYAxesLinesProperty()->SetLineWidth(1.0);
    m_cubeAxesActor->GetZAxesLinesProperty()->SetLineWidth(1.0);

    // Set screen size for labels
    m_cubeAxesActor->SetScreenSize(8.0);
    m_cubeAxesActor->SetLabelOffset(10.0);

    // Show axes
    m_cubeAxesActor->SetVisibility(1);

    // Set fly mode (outer edges)
    m_cubeAxesActor->SetFlyModeToOuterEdges();

    // Enable grid lines
    m_cubeAxesActor->DrawXGridlinesOn();
    m_cubeAxesActor->DrawYGridlinesOn();
    m_cubeAxesActor->DrawZGridlinesOn();

    // Disable inner grid lines
    m_cubeAxesActor->SetDrawXInnerGridlines(false);
    m_cubeAxesActor->SetDrawYInnerGridlines(false);
    m_cubeAxesActor->SetDrawZInnerGridlines(false);

    // Set grid line colors (gray)
    m_cubeAxesActor->GetXAxesGridlinesProperty()->SetColor(0.5, 0.5, 0.5);
    m_cubeAxesActor->GetYAxesGridlinesProperty()->SetColor(0.5, 0.5, 0.5);
    m_cubeAxesActor->GetZAxesGridlinesProperty()->SetColor(0.5, 0.5, 0.5);

    // Set grid line location (all grid lines)
    m_cubeAxesActor->SetGridLineLocation(0);  // VTK_GRID_LINES_ALL = 0

    // Disable minor ticks
    m_cubeAxesActor->XAxisMinorTickVisibilityOff();
    m_cubeAxesActor->YAxisMinorTickVisibilityOff();
    m_cubeAxesActor->ZAxisMinorTickVisibilityOff();

    // Set label scaling (show full numbers)
    m_cubeAxesActor->SetLabelScaling(false, 0, 0, 0);

    // Set tick location (outside)
    m_cubeAxesActor->SetTickLocationToOutside();

    // Add to renderer
    m_renderer->AddActor(m_cubeAxesActor);
}

void vtkPlotBase::createCurve()
{
    // Create a 3D helix curve (like MATLAB plot3 style)
    const int numPoints = 100;
    
    // Create points for the curve
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    
    // Generate helix curve: x = cos(t), y = sin(t), z = t
    // t ranges from 0 to 2*pi, scaled to fit in the coordinate system
    double t_start = 0.0;
    double t_end = 2.0 * vtkMath::Pi();
    double t_step = (t_end - t_start) / (numPoints - 1);
    
    for (int i = 0; i < numPoints; ++i)
    {
        double t = t_start + i * t_step;
        double x = 0.5 * cos(t);  // Scale to fit in axes
        double y = 0.5 * sin(t);
        double z = t / vtkMath::Pi();  // z goes from 0 to 2
        points->InsertNextPoint(x, y, z);
    }
    
    // Create a polyline
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(numPoints);
    for (int i = 0; i < numPoints; ++i)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }
    
    // Create a cell array to store the polyline
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);
    
    // Create polydata
    m_curveData = vtkSmartPointer<vtkPolyData>::New();
    m_curveData->SetPoints(points);
    m_curveData->SetLines(cells);
    
    // Create mapper
    m_lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_lineMapper->SetInputData(m_curveData);
    
    // Create actor
    m_lineActor = vtkSmartPointer<vtkActor>::New();
    m_lineActor->SetMapper(m_lineMapper);
    m_lineActor->GetProperty()->SetColor(0.0, 0.5, 1.0);  // Blue color (MATLAB default blue)
    m_lineActor->GetProperty()->SetLineWidth(2.0);
    
    // Add to renderer
    m_renderer->AddActor(m_lineActor);
}

void vtkPlotBase::createKeyPoints()
{
    // Create a disk source (hollow circle ring) - like MATLAB 'o' marker
    m_diskSource = vtkSmartPointer<vtkDiskSource>::New();
    m_diskSource->SetInnerRadius(0.02);   // Inner radius (hole)
    m_diskSource->SetOuterRadius(0.025);  // Outer radius (edge)
    m_diskSource->SetCircumferentialResolution(64);  // Smooth circle
    m_diskSource->Update();
    
    // Create mapper
    m_keyPointsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_keyPointsMapper->SetInputConnection(m_diskSource->GetOutputPort());
    
    // Create start marker (red circle) at (0.5, 0, 0)
    m_startMarker = vtkSmartPointer<vtkFollower>::New();
    m_startMarker->SetMapper(m_keyPointsMapper);
    m_startMarker->SetPosition(0.5, 0.0, 0.0);
    m_startMarker->SetCamera(m_renderer->GetActiveCamera());
    m_startMarker->GetProperty()->SetColor(1.0, 0.0, 0.0);  // Red
    m_startMarker->GetProperty()->SetLineWidth(1.5);
    
    // Create end marker (green circle) at (0.5, 0, 2)
    m_endMarker = vtkSmartPointer<vtkFollower>::New();
    m_endMarker->SetMapper(m_keyPointsMapper);
    m_endMarker->SetPosition(0.5, 0.0, 2.0);
    m_endMarker->SetCamera(m_renderer->GetActiveCamera());
    m_endMarker->GetProperty()->SetColor(0.0, 0.8, 0.0);  // Green
    m_endMarker->GetProperty()->SetLineWidth(1.0);
    
    // Add to renderer
    m_renderer->AddActor(m_startMarker);
    m_renderer->AddActor(m_endMarker);
}
