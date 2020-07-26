#ifndef PINNEDBUTTON_H
#define PINNEDBUTTON_H

#include <QtWidgets>

class PanelQt;
class TaskBar;
class TaskGroup;

class PinButton : public QToolButton
{
    Q_OBJECT

public:
    PinButton(QIcon icon, QString className, TaskGroup * group, TaskBar * taskbar, PanelQt * panel);
    ~PinButton();

    QIcon mIcon;
    QString mClassName;
    TaskGroup * mGroup;
    TaskBar * mTaskBar;
    PanelQt * mPanel;

    QMenu * mMenu;
    QAction * mAction;

    void updateAutoRaise(bool autoRaise);
    void updatePinWidth(int w);
    void updateIconSize(int h);
private:
    void actionClicked();
    void contextMenuEvent(QContextMenuEvent *event);

signals:

};

#endif // PINNEDBUTTON_H
