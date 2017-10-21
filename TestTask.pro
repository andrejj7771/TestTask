TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp
QMAKE_CFLAGS += -pthread
LIBS += -lpthread -lglut -lGLU
INCLUDEPATH += -L/usr/lib/
