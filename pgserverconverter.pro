QT += core sql
QT -= gui

CONFIG += c++11

TARGET = pgserverconverter
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
           converter.cpp

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += converter.h
