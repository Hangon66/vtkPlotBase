#include "vtkplotbase.h"

#include <QApplication>

// VTK module initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    vtkPlotBase w;
    w.show();
    return a.exec();
}
