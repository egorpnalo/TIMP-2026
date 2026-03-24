QT -= gui
QT += core network sql

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += main_serv.cpp \
           functions_to_server.cpp \
           mytcpserver.cpp \

HEADERS += mytcpserver.h \
           functions_to_server.h \

