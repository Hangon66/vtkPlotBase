#include "vtkplotbase.h"

#include <QApplication>
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <vtkMath.h>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    vtkPlotBase w;
    w.setWindowTitle("VTK 3D Plot Widget");
    w.resize(800, 600);
    
    // Set axis titles (axis range will auto-fit to data)
    w.setAxisTitles("X", "Y", "Z");
    
    // Create helix curve data
    QVector<QVector3D> helixPoints;
    const int numPoints = 100;
    for (int i = 0; i < numPoints; ++i) {
        double t = i * 2.0 * vtkMath::Pi() / (numPoints - 1);
        double x = 0.5 * cos(t);
        double y = 0.5 * sin(t);
        double z = t / vtkMath::Pi();
        helixPoints.append(QVector3D(x, y, z));
    }
    
    // Add curve (blue, line width 2.0) - axis will auto-fit
    QString curveId = w.addCurve(helixPoints, QColor(0, 120, 255), 2.0);
    w.setCurveName(curveId, "Helix");
    
    // Add hollow marker at start (red)
    QString marker1 = w.addHollowMarker(QVector3D(0.5, 0.0, 0.0), QColor(255, 0, 0), 0.05, 1.5);
    w.setMarkerName(marker1, "Start");
    
    // Add filled marker at end (green)
    QString marker2 = w.addFilledMarker(QVector3D(0.5, 0.0, 2.0), QColor(0, 200, 0), 0.05);
    w.setMarkerName(marker2, "End");
    
    w.show();
    return a.exec();
}
