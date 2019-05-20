TEMPLATE = app
CONFIG += c++17
CONFIG -= qt

DEFINES +=

HEADERS += \
    saver.h

SOURCES += \
    main.cpp \
    saver.cpp

LIBS += -L$$OUT_PWD/../ekbase/ -lekbase
INCLUDEPATH += $$PWD/../ekbase
DEPENDPATH += $$PWD/../ekbase

LIBS += -L/home/user/cgate/lib/ -lcgate -lP2Sys -lP2ReplClient -lP2SysExt -lP2Tbl -lP2DB
