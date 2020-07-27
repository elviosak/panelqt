#ifndef TASKBAR_H
#define TASKBAR_H

#include <QtWidgets>
#include <QX11Info>
#include <KWindowSystem>
#include <NETWM>

//#include <X11/Xlib.h>


class PanelQt;
class TaskButton;
class PinButton;
class TaskGroup;
class ElideLabel;


class TaskBar : public QFrame
{

    Q_OBJECT
public:
    TaskBar(PanelQt * panel = nullptr);
    ~TaskBar();

    QString mPanelName;
    QSettings * mSettings;
    QSettings * mGroupSettings;
    QMap<WId, TaskGroup*> mIdList;
    QMap<QString, TaskGroup*> mClassList;
    xcb_connection_t * conn;
    QBoxLayout * mLayout;
    PanelQt * mPanel;
    QActionGroup * mActionGroup;

    bool mShowAllScreens;
    int mMaxBtnWidth;
    QStringList mPinnedList;
    int mIconHeight;
    int mPinIconHeight;
    int mPinBtnWidth;
    bool mButtonAutoRaise;
    bool mPinAutoRaise;
    ElideLabel * mElideLabel;
    ElideLabel * mElideLabelUnderline;
    bool mButtonUnderline;
    QString mShape;
    QString mShadow;
    int mLineWidth;
    int mMidLineWidth;

    QString mGroupShape;
    QString mGroupShadow;
    int mGroupLineWidth;
    int mGroupMidLineWidth;

    void changeShape(QString s);
    void changeShadow(QString s);
    void changeLineWidth(int w);
    void changeMidLineWidth(int w);
    void changeFrame();

    void groupChangeShape(QString s);
    void groupChangeShadow(QString s);
    void groupChangeLineWidth(int w);
    void groupChangeMidLineWidth(int w);

    void setButtonUnderline(bool u);
    void setButtonAutoRaise(bool autoRaise);
    void setPinAutoRaise(bool autoRaise);
    void setShowAllScreens(bool show);
    void setMaxBtnWidth(int w);
    void setIconHeight(int h);
    void setPinIconHeight(int h);
    void setPinBtnWidth(int w);

    void resetTaskBar();

    void loadPinned();
    void savePinned(QString className, QIcon icon, QString cmd);
    void savePinList();
    void removePin(QString className);

    bool acceptWindow(WId id, bool changed = false);
    void addGroup(QString className, QIcon icon, bool pinned = false, QString cmd = QString());
    void addWindow(WId id);
    void closeRequested(WId id);
    void buttonRemoved(WId id);
    void recalculateButtonsDelay();
    void recalculateButtons();
    void removeGroup(QString className);
    void swapGroups(QObject * sender, QObject * draggedObj);
private:
    void activeWindowChanged(WId id);
    void windowRemoved(WId id);
    void windowAdded(WId id);
    void windowChanged(WId id, NET::Properties properties, NET::Properties2 properties2);
signals:
    void groupShapeChanged(QString s);
    void groupShadowChanged(QString s);
    void groupLineWidthChanged(int w);
    void groupMidLineWidthChanged(int w);

    void buttonAutoRaiseChanged(bool autoRaise);
    void pinAutoRaiseChanged(bool autoRaise);
    void buttonWidthChanged(int width);
    void pinIconHeightChanged(int h);
    void pinBtnWidthChanged(int w);
    void iconHeightChanged(int h);
};

#endif // TASKBAR_H
