#ifndef TASKGROUP_H
#define TASKGROUP_H

#include <QtWidgets>
#include <QX11Info>
#include <KWindowSystem>
#include <NETWM>

//#include <X11/Xlib.h>


class PanelQt;
class TaskBar;
class TaskButton;
class PinButton;

class TaskGroup
        : public QFrame
        //: public QToolBar
{

    Q_OBJECT
public:
    TaskGroup(QString className, QIcon icon, TaskBar* taskbar, PanelQt * panel, bool pinned = false, QString cmd = "");
    ~TaskGroup();

    QString mClassName;
    QIcon mIcon;
    TaskBar * mTaskBar;
    PanelQt * mPanel;
    bool mPinned;
    QString mCmd;
    int mWindowCount;
    QMap<WId, TaskButton*> mWinList;
    QBoxLayout * mLayout;
    QActionGroup * mActionGroup;
    PinButton * mPinButton;
    int mButtonWidth;
    int mGroupWidth;
    QPoint dragStartPosition;

    QString mShape;
    QString mShadow;
    int mLineWidth;
    int mMidLineWidth;

    void changeShape(QString s);
    void changeShadow(QString s);
    void changeLineWidth(int w);
    void changeMidLineWidth(int w);
    void changeFrame();

    void addPin();
    void removePin();
    void startPin();
    void setCmd(QString cmd);
    void raiseWindow(WId id);
    void addWindow(WId id, QString title, QIcon icon);
    void changeIcon(WId id, QIcon icon);
    void removeWindow(WId id);
    void setTitle(WId id, QString title);
    void setActive(WId id);
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void changeButtonWidth(int w);

signals:
    void buttonsChanged();
    void dragEntered(QObject * enteredObj, QObject * draggedObj);
    void buttonAdded(WId id);
    void buttonRemoved(WId id);
    void removeGroup(QString className);
};

#endif // TASKGROUP_H
