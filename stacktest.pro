#-------------------------------------------------
#
# Project created by QtCreator 2011-07-03T10:03:05
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = stacktest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    stackcontext.cpp \
    stackcontext_x86_64.s \
    deferred.cpp \
    pendingcallback.cpp

HEADERS += \
    stackcontext.h \
    deferred.h \
    pendingcallback.h
