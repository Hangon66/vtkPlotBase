QT       += core gui opengl openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# VTK Configuration
VTK_DIR = $$PWD/3rdparty/VTK-9.6

INCLUDEPATH += $$VTK_DIR/include/vtk-9.6

LIBS += -L$$VTK_DIR/lib
LIBS += -lvtkCommonCore-9.6
LIBS += -lvtkCommonDataModel-9.6
LIBS += -lvtkCommonExecutionModel-9.6
LIBS += -lvtkCommonMath-9.6
LIBS += -lvtkCommonMisc-9.6
LIBS += -lvtkCommonTransforms-9.6
LIBS += -lvtkRenderingCore-9.6
LIBS += -lvtkRenderingOpenGL2-9.6
LIBS += -lvtkRenderingAnnotation-9.6
LIBS += -lvtkRenderingFreeType-9.6
LIBS += -lvtkInteractionStyle-9.6
LIBS += -lvtkFiltersCore-9.6
LIBS += -lvtkFiltersSources-9.6
LIBS += -lvtkFiltersGeneral-9.6
LIBS += -lvtkGUISupportQt-9.6
LIBS += -lvtksys-9.6

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    vtkplotbase.cpp \
    drawable/vtkcurve.cpp \
    drawable/vtkmarker.cpp \
    drawable/vtksurface.cpp \
    drawable/vtkheatmap.cpp

HEADERS += \
    vtkplotbase.h \
    drawable/vtkdrawable.h \
    drawable/vtkcurve.h \
    drawable/vtkmarker.h \
    drawable/vtksurface.h \
    drawable/vtkheatmap.h

FORMS += \
    vtkplotbase.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
