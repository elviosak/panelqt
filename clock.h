#ifndef CLOCK_H
#define CLOCK_H

#include <QtWidgets>

class PanelQt;

class Clock : public QFrame
{

    Q_OBJECT
public:
    Clock(PanelQt * panel, QWidget * parent = nullptr);
    ~Clock();
private:
    PanelQt * mPanel;
    QBoxLayout * mLayout;
    QMenu * mMenu;
    QTimer * mTimer;
    QLabel * mDateLabel;
    QLabel * mTimeLabel;
    QString mDateFormat;
    QString mTimeFormat;
    void setClock();
    void startTimer();
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // CLOCK_H
