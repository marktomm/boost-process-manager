#-------------------------------------------------
#
# Project created by QtCreator 2014-01-03T23:01:06
#
#-------------------------------------------------



QT       += core

QT       -= gui

TARGET = ProcessManager
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS +=-lboost_system \
        -lboost_filesystem \
        -lboost_date_time \
        -lboost_thread

SOURCES += main.cpp \
    ProcessManager.cpp

QMAKE_CXXFLAGS += -std=c++0x

HEADERS += \
    ProcessManager.h
