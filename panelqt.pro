QT       += bluetooth core gui widgets x11extras
QT += KWindowSystem

#LIBS += -lX11


CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    clock.cpp \
    main.cpp \
    panelqt.cpp \
    pinbutton.cpp \
    taskbar.cpp \
    taskbutton.cpp

HEADERS += \
    clock.h \
    panelqt.h \
    pinbutton.h \
    taskbar.h \
    taskbutton.h

#FORMS += \
#    panelqt.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



