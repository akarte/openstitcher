TEMPLATE = app

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenStitcher

SOURCES += \
    stitching.cpp \
    mainwindow.cpp \
    innerwindow.cpp \
    imagelist.cpp \
    panzoomview.cpp \
    droptoolbar.cpp \
    config.cpp \
    decimalslider.cpp \
    camui.cpp \
    customgraphicspixmapitem.cpp \
    trackingui.cpp \
    trackingconfig.cpp \
    featuredetector.cpp \
    trackingstitching.cpp \
    faststitcher.cpp \
    stitchdata.cpp \
    stitchinfo.cpp \
    stitchimageitem.cpp

RESOURCES += qml.qrc
QMAKE_CXXFLAGS += -fopenmp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += C:\opencv-build\install\include
#   For 32bits executable:
# LIBS += -L"C:/opencv-buildx32/install/x64/mingw/bin"
#   For 64bits executable:
# LIBS += -L"C:/opencv-build/install/x64/mingw/bin"
LIBS += -L"C:/opencv-buildx32/install/x64/mingw/bin"
LIBS += \
    -lopencv_imgproc249 \
    -lopencv_calib3d249 \
    -lopencv_nonfree249 \
    -lopencv_flann249 \
    -lopencv_highgui249 \
    -lopencv_features2d249 \
    -lopencv_stitching249 \
    -lopencv_core249 \
    -lopencv_video249

LIBS += -fopenmp

OTHER_FILES +=

HEADERS += \
    mainwindow.h \
    innerwindow.h \
    imagelist.h \
    stitching.h \
    panzoomview.h \
    droptoolbar.h \
    config.h \
    decimalslider.h \
    camui.h \
    customgraphicspixmapitem.h \
    trackingui.h \
    trackingconfig.h \
    trackingstitching.h \
    faststitcher.h \
    stitchdata.h \
    stitchinfo.h \
    stitchimageitem.h

FORMS += \
    mainwindow.ui \
    innerwindow.ui \
    config.ui \
    camui.ui \
    trackingui.ui \
    trackingconfig.ui

DISTFILES += \
    process.txt
