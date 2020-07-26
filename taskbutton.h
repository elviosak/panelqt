#ifndef TASKBUTTON_H
#define TASKBUTTON_H

#include <QtWidgets>

class PanelQt;
class TaskBar;
class TaskGroup;

enum TilePosition{
    Left,
    Center,
    Right
};

class TaskButton : public QToolButton
{
    Q_OBJECT
    Q_ENUM(TilePosition)
public:
    TaskButton(WId id, QIcon icon, QString title, QString className, QActionGroup * actionGroup, TaskGroup * group, TaskBar * taskbar, PanelQt * panel);
    ~TaskButton();

    QString mClass;
    bool mPinned;
    void setTitle(QString title);
    void setActionCheck(bool check);
    void buttonWidthChanged(int w);


    WId mId;
    QIcon mIcon;
    QString mTitle;

    TaskGroup * mGroup;
    TaskBar * mTaskBar;
    PanelQt * mPanel;

    QMenu * mMenu;
    QAction * mAction;

    void updateAutoRaise(bool autoRaise);
    void updateIconSize(int height);
    void changeIcon(QIcon icon);

    void dragEnterEvent(QDragEnterEvent *e) override;
    void tileWindow(TilePosition pos, int perc);
    void moveApplicationToPrevNextMonitor(bool next);
private:
    void updateMenu();
    void requestClose();
    void actionClicked(bool checked);
    void contextMenuEvent(QContextMenuEvent *event) override;


signals:
    void triggered();
    void closeRequested(WId id);
};

#endif // TASKBUTTON_H
