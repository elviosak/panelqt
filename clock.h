#ifndef CLOCK_H
#define CLOCK_H

#include <QtWidgets>

class PanelQt;

class Clock : public QFrame
{

    Q_OBJECT
public:
    Clock(PanelQt * panel);
    ~Clock();

    void updateFont();
private:
    QString mPanelName;
    QSettings * mSettings;
    PanelQt * mPanel;
    QBoxLayout * mLayout;
    QMenu * mMenu;
    QTimer * mTimer;

    bool mUsePanelFont;
    QFont mFont;
    QLabel * mDateLabel;
    QLabel * mTimeLabel;
    QString mDateFormat;
    QString mTimeFormat;
    int mMargin;
    QBoxLayout::Direction mDirection;

    QString mShape;
    QString mShadow;
    int mLineWidth;
    int mMidLineWidth;

    void changeShape(QString s);
    void changeShadow(QString s);
    void changeLineWidth(int w);
    void changeMidLineWidth(int w);
    void changeFrame();


    void showDialog();
    void setUsePanelFont(bool usePanelFont);
    void setClockFont(QFont font);
    void setDateFormat(QString newDateFormat);
    void setTimeFormat(QString newTimeFormat);
    void setClockMargin(int newMargin);
    void setClockDirection(int newDirection);
    void saveSettings();


    void updateClock();
    void startTimer();
    void mousePressEvent(QMouseEvent * event);
    //void contextMenuEvent(QContextMenuEvent *event);
};

#endif // CLOCK_H
