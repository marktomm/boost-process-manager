#-------------------------------------------------
#
# Project created by QtCreator 2014-01-03T23:01:06
#
#-------------------------------------------------



QT       += core

QT       -= gui

TARGET = ProcessManager
CONFIG   += console
CONFIG   += debug
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += PM_DEBUG

LIBS +=-lboost_system \
        -lboost_filesystem \
        -lboost_date_time \
        -lboost_thread

SOURCES += main.cpp \
    LinuxProcessManager.cpp \
    ProcessManagerException.cpp

QMAKE_CXXFLAGS += -ggdb

HEADERS += \
    LinuxProcessManager.h \
    ProcessManagerException.h \
    ProcessManagerBase.h
