TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += poco1.7.4/include

SOURCES += main.cpp \
    DBConfiguration.cpp


LIBS += -L./poco1.7.4/lib/win32_x86
HEADERS += \
    DBConfiguration.h
