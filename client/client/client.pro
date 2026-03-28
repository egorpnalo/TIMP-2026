QT += core gui network widgets

CONFIG += c++17

SOURCES += main.cpp \
           client.cpp \
           mainwindow.cpp \
           regandauth.cpp

HEADERS += client.h \
           mainwindow.h \
           regandauth.h

FORMS += mainwindow.ui \
         regandauth.ui
