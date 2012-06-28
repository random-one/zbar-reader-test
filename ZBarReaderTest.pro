QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32:INCLUDEPATH *= $(ZBAR_ROOT)/include
win32:INCLUDEPATH *= $(ZBAR_ROOT)/include/zbar

win32:LIBS *= -L$(ZBAR_ROOT)/bin/ -llibzbar-0

unix:LIBS *= -lzbar
unix:LIBS *= -lzbarqt

TARGET = ZBarReaderTest
TEMPLATE = app

SOURCES += main.cpp\
           ZBarReaderTest.cpp

HEADERS += \
           ZBarReaderTest.h

FORMS   += \
           ZBarReaderTest.ui
