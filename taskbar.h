#ifndef TASKBAR_H
#define TASKBAR_H

#include <QtWidgets>
#include <QX11Info>
#include <KWindowSystem>
#include <NETWM>

//#include <X11/Xlib.h>

#include "taskbutton.h"
#include "pinbutton.h"

class PanelQt;

class TaskBar : public QFrame
{

    Q_OBJECT
public:
    TaskBar(PanelQt * panel = nullptr);
    ~TaskBar();

private:
    QMap<WId, TaskButton*> mWinList;
    QMap<QString, PinButton*> mPinList;
    xcb_connection_t * conn;
    QBoxLayout * mLayout;
    PanelQt * mPanel;
    bool mShowAllScreens;
    QActionGroup * mActionGroup;
    int mMaxBtnWidth;
    bool acceptWindow(WId id, bool changed = false);
    void addPin(QString className);
    void addWindow(WId id);
    void closeRequested(WId id);
    void recalculateButtons();
private slots:
    void activeWindowChanged(WId id);
    void windowRemoved(WId id);
    void windowAdded(WId id);
    void windowChanged(WId id, NET::Properties properties, NET::Properties2 properties2);
signals:
    void buttonWidthChanged(int width);
};

#endif // TASKBAR_H
