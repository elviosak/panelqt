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

class TaskButton : public QFrame
{
    Q_OBJECT
    Q_ENUM(TilePosition)
public:
    TaskButton(WId id, QIcon icon, QString title, QString className, TaskGroup * group, TaskBar * taskbar, PanelQt * panel);
    ~TaskButton();

    QString mClass;
    bool mPinned;
    void setTitle(QString title);
    void setActionCheck(bool check);
    void buttonWidthChanged(int w);

    QLabel * mBtnIcon;
    QLabel * mBtnText;

    WId mId;
    QIcon mIcon;
    QString mTitle;

    TaskGroup * mGroup;
    TaskBar * mTaskBar;
    PanelQt * mPanel;
    QHBoxLayout * mLayout;

    bool mActive;
    QPalette::ColorRole mPaletteColor;

    void paintEvent(QPaintEvent *event) override;
    void activeWindowChanged(WId id);
    void setActive(bool active);

    void changeIconSize(int height);
    void changeIcon(QIcon i);
    void changeText(QString t);

//    void mousePressEvent(QMouseEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void tileWindow(TilePosition pos, int perc);
    void moveApplicationToPrevNextMonitor(bool next);
private:
    void requestClose();
    void actionClicked(bool checked);
    void showMenu();


signals:
    void triggered();
    void closeRequested(WId id);
};

#endif // TASKBUTTON_H
