QT += testlib sql network core

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += tst_funcforserver_test.cpp \
           ../../server/Server/functions_to_server.cpp \
           ../../server/Server/mytcpserver.cpp \
           ../../server/Server/dbclient.cpp

HEADERS += ../../server/Server/functions_to_server.h \
           ../../server/Server/mytcpserver.h \
           ../../server/Server/dbclient.h

INCLUDEPATH += ../../server/Server
