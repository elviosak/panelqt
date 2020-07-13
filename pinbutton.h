#ifndef PINNEDBUTTON_H
#define PINNEDBUTTON_H

#include <QtWidgets>

class PanelQt;
class TaskBar;


class PinButton : public QToolButton
{
    Q_OBJECT

public:
    PinButton(QIcon icon, QString className, PanelQt * panel);

    ~PinButton();

    QString mClass;
    //bool mPinned;
    //void setTitle(QString title);
    //void setActionCheck(bool check);
    //void buttonWidthChanged(int w);
private:
    QIcon mIcon;
    QString mName;
    QString mExec;

    PanelQt * mPanel;
    TaskBar * mTaskbar;
    QMenu * mMenu;
    QAction * mAction;
    void updateMenu();
    void requestClose();
    void actionClicked(bool checked);

    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void triggered();
    void closeRequested(WId id);
};

#endif // PINNEDBUTTON_H
