TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    image.cpp
QMAKE_CFLAGS += -pthread -lpthread
LIBS += -pthread
HEADERS += \
    image.h
