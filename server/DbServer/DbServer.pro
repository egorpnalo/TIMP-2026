QT -= gui
QT += core network sql

CONFIG += c++17 console

CONFIG -= app_bundle

SOURCES += main_db.cpp \
    dbtcpserver.cpp \
    databasemanager.cpp

HEADERS += dbtcpserver.h \
    databasemanager.h \

