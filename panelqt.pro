QT  += core gui widgets x11extras dbus
QT  += KWindowSystem

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    appmenu.cpp \
    clock.cpp \
    sn/dbustypes.cpp \
    main.cpp \
    panelqt.cpp \
    pinbutton.cpp \
    snbutton.cpp \
    snframe.cpp \
    sn/sniasync.cpp \
    sn/statusnotifieriteminterface.cpp \
    sn/statusnotifierwatcher.cpp \
    taskbar.cpp \
    taskbutton.cpp \
    taskgroup.cpp

HEADERS += \
    appmenu.h \
    clock.h \
    sn/dbustypes.h \
    extras.h \
    panelqt.h \
    pinbutton.h \
    snbutton.h \
    snframe.h \
    sn/sniasync.h \
    sn/statusnotifieriteminterface.h \
    sn/statusnotifierwatcher.h \
    taskbar.h \
    taskbutton.h \
    taskgroup.h \
    toolbuttontextstyle.hpp

#LIBS += -L/usr/lib -ldbusmenu-qt5
unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += dbusmenu-qt5

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    statusnotifier-extra/org.kde.StatusNotifierItem.xml

RESOURCES += \
    panelqt.qrc


