#include "clock.h"
#include "panelqt.h"

Clock::~Clock(){}

Clock::Clock(PanelQt * panel, QWidget * parent):
    QFrame(parent),
    mPanel(panel),
    mLayout(new QBoxLayout(QBoxLayout::TopToBottom, this)),
    mMenu(new QMenu(this)),
    mTimer(new QTimer(this)),
    mDateLabel(new QLabel(this)),
    mTimeLabel(new QLabel(this))
{
    mLayout->setMargin(0);

    mDateLabel->setAlignment(Qt::AlignCenter);
    mTimeLabel->setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setLayout(mLayout);
//    DATE FORMAT STRING
//    d	The day as a number without a leading zero (1 to 31)
//    dd	The day as a number with a leading zero (01 to 31)
//    ddd	The abbreviated localized day name (e.g. 'Mon' to 'Sun'). Uses the system locale to localize the name, i.e. QLocale::system().
//    dddd	The long localized day name (e.g. 'Monday' to 'Sunday'). Uses the system locale to localize the name, i.e. QLocale::system().
//    M	The month as a number without a leading zero (1 to 12)
//    MM	The month as a number with a leading zero (01 to 12)
//    MMM	The abbreviated localized month name (e.g. 'Jan' to 'Dec'). Uses the system locale to localize the name, i.e. QLocale::system().
//    MMMM	The long localized month name (e.g. 'January' to 'December'). Uses the system locale to localize the name, i.e. QLocale::system().
//    yy	The year as a two digit number (00 to 99)
//    yyyy	The year as a four digit number. If the year is negative, a minus sign is prepended, making five characters.
    mDateFormat = "ddd, dd MMM yy";

//    TIME FORMAT STRING
//    h	The hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
//    hh	The hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
//    H	The hour without a leading zero (0 to 23, even with AM/PM display)
//    HH	The hour with a leading zero (00 to 23, even with AM/PM display)
//    m	The minute without a leading zero (0 to 59)
//    mm	The minute with a leading zero (00 to 59)
//    s	The whole second, without any leading zero (0 to 59)
//    ss	The whole second, with a leading zero where applicable (00 to 59)
//    z	The fractional part of the second, to go after a decimal point, without trailing zeroes (0 to 999). Thus "s.z" reports the seconds to full available (millisecond) precision without trailing zeroes.
//    zzz	The fractional part of the second, to millisecond precision, including trailing zeroes where applicable (000 to 999).
//    AP or A	Use AM/PM display. A/AP will be replaced by an upper-case version of either QLocale::amText() or QLocale::pmText().
//    ap or a	Use am/pm display. a/ap will be replaced by a lower-case version of either QLocale::amText() or QLocale::pmText().
//    t	The timezone (for example "CEST")
    mTimeFormat = "HH:mm:ss";

    mLayout->addWidget(mDateLabel);
    mLayout->addWidget(mTimeLabel);
    setClock();
    connect(mTimer, &QTimer::timeout, this, &Clock::setClock);
    startTimer();
}
void Clock::setClock(){
    mDateLabel->setText(QDate::currentDate().toString(mDateFormat));
    mTimeLabel->setText(QTime::currentTime().toString(mTimeFormat));
}
void Clock::contextMenuEvent(QContextMenuEvent *event){
    qDebug() << "menu requested";
    mMenu->clear();
    QAction * a;
    QBoxLayout::Direction dir = mLayout->direction();
    if( dir != QBoxLayout::LeftToRight){
        a = mMenu->addAction("Left To Right");
        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::LeftToRight);});
    }
    if( dir != QBoxLayout::RightToLeft){
        a = mMenu->addAction("Right To Left");
        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::RightToLeft);});
    }
    if( dir != QBoxLayout::TopToBottom){
        a = mMenu->addAction("Top To Bottom");
        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::TopToBottom);});
    }
    if( dir != QBoxLayout::BottomToTop){
        a = mMenu->addAction("Bottom To Top");
        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::BottomToTop);});
    }
    auto menuGeo = mPanel->calculateMenuPosition(event->globalPos(), mMenu->sizeHint());
    qDebug() << "menuGeo" << menuGeo;
    mMenu->setGeometry(menuGeo);
    mMenu->show();
}

void Clock::startTimer(){
    mTimer->setInterval(1000);
    mTimer->start();
}
