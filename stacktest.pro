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
CONFIG += debug
QMAKE_CXXFLAGS += -fomit-frame-pointer

TEMPLATE = app


SOURCES += main.cpp \
    stackcontext.cpp \
    stackcontext_x86_64.s

HEADERS += \
    stackcontext.h
