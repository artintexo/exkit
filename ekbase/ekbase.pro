TEMPLATE = lib
CONFIG += c++17 object_parallel_to_source
CONFIG -= qt

DEFINES +=

HEADERS += \
    base/context.h \
    plaza/plaza.h \
    simulator/simulator.h \
    base/strategy.h \
    base/base.h \
    base/eventmanager.h \
    base/logmanager.h \
    base/holder.h \
    base/ordermanager.h \
    base/sessionmanager.h \
    base/securitymanager.h \
    base/datetime.h \
    base/timemanager.h \
    plaza/envplaza.h \
    plaza/dataplaza.h \
    plaza/execplaza.h \
    simulator/datasim.h \
    simulator/execsim.h \
    plaza/callback.h \
    plaza/description.h \
    plaza/utils.h \
    plaza/connection.h \
    plaza/listener.h \
    plaza/futinforepl.h \
    plaza/sessiondesc.h \
    plaza/securitydesc.h \
    base/subsmanager.h \
    plaza/dealsrepl.h \
    plaza/heartbeatdesc.h \
    plaza/tradedesc.h \
    plaza/aggrdesc.h \
    plaza/futaggrrepl.h \
    base/datawriter.h \
    base/datareader.h \
    base/order.h \
    base/positionmanager.h \
    base/statmanager.h \
    plaza/publisher.h \
    plaza/futaddorderdesc.h \
    plaza/futdelorderdesc.h \
    plaza/futdeluserordersdesc.h \
    plaza/pubrepl.h \
    plaza/msg99desc.h \
    plaza/msg100desc.h \
    plaza/msg101desc.h \
    plaza/msg102desc.h \
    plaza/msg103desc.h \
    plaza/futtraderepl.h \
    plaza/execdesc.h

SOURCES += \
    base/context.cpp \
    plaza/plaza.cpp \
    simulator/simulator.cpp \
    base/strategy.cpp \
    base/base.cpp \
    base/eventmanager.cpp \
    base/logmanager.cpp \
    base/holder.cpp \
    base/ordermanager.cpp \
    base/sessionmanager.cpp \
    base/securitymanager.cpp \
    base/datetime.cpp \
    base/timemanager.cpp \
    plaza/envplaza.cpp \
    plaza/dataplaza.cpp \
    plaza/execplaza.cpp \
    simulator/datasim.cpp \
    simulator/execsim.cpp \
    plaza/callback.cpp \
    plaza/description.cpp \
    plaza/utils.cpp \
    plaza/connection.cpp \
    plaza/listener.cpp \
    plaza/futinforepl.cpp \
    plaza/sessiondesc.cpp \
    plaza/securitydesc.cpp \
    base/subsmanager.cpp \
    plaza/dealsrepl.cpp \
    plaza/heartbeatdesc.cpp \
    plaza/tradedesc.cpp \
    plaza/aggrdesc.cpp \
    plaza/futaggrrepl.cpp \
    base/datawriter.cpp \
    base/datareader.cpp \
    base/order.cpp \
    base/positionmanager.cpp \
    base/statmanager.cpp \
    plaza/publisher.cpp \
    plaza/futaddorderdesc.cpp \
    plaza/futdelorderdesc.cpp \
    plaza/futdeluserordersdesc.cpp \
    plaza/pubrepl.cpp \
    plaza/msg99desc.cpp \
    plaza/msg100desc.cpp \
    plaza/msg101desc.cpp \
    plaza/msg102desc.cpp \
    plaza/msg103desc.cpp \
    plaza/futtraderepl.cpp \
    plaza/execdesc.cpp

LIBS += -L/home/user/cgate/lib/ -lcgate -lP2Sys -lP2ReplClient -lP2SysExt -lP2Tbl -lP2DB
INCLUDEPATH += /home/user/cgate/include
DEPENDPATH += /home/user/cgate/include

LIBS += -lstdc++fs
