#ifndef PINNEDBUTTON_H
#define PINNEDBUTTON_H

#include <QtWidgets>

class PanelQt;
class TaskBar;
class TaskGroup;

class PinButton : public QFrame
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

    QLabel * mBtnIcon;
    QPalette::ColorRole mPaletteColor;
    bool mHover;

    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;

    void handleClick(bool press); // press or release

    void setActive(bool active);
    void changeIcon(QIcon i);
    void updatePinWidth(int w);
    void updateIconSize(int h);
private:
    void actionClicked();
    void showMenu();

signals:

};

#endif // PINNEDBUTTON_H
