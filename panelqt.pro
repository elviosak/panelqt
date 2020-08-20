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
    taskgroup.cpp \
    vol/audiodevice.cpp \
    vol/audioengine.cpp \
    vol/pulseaudioengine.cpp \
    volumebutton.cpp \
    volumeframe.cpp

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
    toolbuttontextstyle.hpp \
    vol/audiodevice.h \
    vol/audioengine.h \
    vol/pulseaudioengine.h \
    volumebutton.h \
    volumeframe.h


#LIBS += -lpulse-simple
LIBS += -lpulse

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += dbusmenu-qt5

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin

desktopfile.files = res/panelqt.desktop
desktopfile.path = /usr/share/applications/
iconfile.files = res/panelqt.svg
iconfile.path = /usr/share/icons/hicolor/scalable/apps/

target.path = /usr/bin

INSTALLS += target desktopfile iconfile
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

#DISTFILES += \
#    statusnotifier-extra/org.kde.StatusNotifierItem.xml

RESOURCES += \
    panelqt.qrc






unix:!macx: LIBS += -ldbusmenu-qt5
