TEMPLATE = app
CONFIG += c++17 object_parallel_to_source
CONFIG -= qt

DEFINES +=

HEADERS += \
    example.h

SOURCES += \
    main.cpp \
    example.cpp

LIBS += -L$$OUT_PWD/../ekbase/ -lekbase
INCLUDEPATH += $$PWD/../ekbase
DEPENDPATH += $$PWD/../ekbase

LIBS += -L/home/user/cgate/lib/ -lcgate -lP2Sys -lP2ReplClient -lP2SysExt -lP2Tbl -lP2DB
