TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lgdi32
LIBS += -lcomdlg32

SOURCES += main.cpp \
    cabstractobjscene.cpp \
    ccamera.cpp \
    coordsystem.cpp \
    matrix.cpp \
    cobjscene.cpp

HEADERS += \
    cabstractobjscene.h \
    ccamera.h \
    coordsystem.h \
    matrix.h \
    cobjscene.h

RESOURCES +=

