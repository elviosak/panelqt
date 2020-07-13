#ifndef TASKBUTTON_H
#define TASKBUTTON_H

#include <QtWidgets>

class PanelQt;
class TaskBar;

class LeftAlignedTextStyle : public QProxyStyle
{
    using QProxyStyle::QProxyStyle;
public:
    virtual void drawItemText(QPainter * painter, const QRect & rect, int flags
            , const QPalette & pal, bool enabled, const QString & text
            , QPalette::ColorRole textRole = QPalette::NoRole) const override;
};


class TaskButton : public QToolButton
{
    Q_OBJECT

public:
    TaskButton(QIcon icon, QString className, PanelQt * panel, bool pinned);
    TaskButton(WId id, QIcon icon, QString title, QString className, QActionGroup * actionGroup, TaskBar * taskbar, PanelQt * panel);
    ~TaskButton();

    QString mClass;
    bool mPinned;
    void setTitle(QString title);
    void setActionCheck(bool check);
    void buttonWidthChanged(int w);
private:
    WId mId;
    QIcon mIcon;
    QString mTitle;

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

#endif // TASKBUTTON_H
