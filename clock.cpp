#include "clock.h"
#include "panelqt.h"

Clock::~Clock(){}

extern QHash<QString, QFrame::Shape> Shapes;
extern QHash<QString, QFrame::Shadow> Shadows;

Clock::Clock(PanelQt * panel):
    QFrame(panel),
    mPanelName(panel->mPanelName),
    mSettings(new QSettings("PanelQt", mPanelName + "/clock")),
    mPanel(panel),

    mMenu(new QMenu(this)),
    mTimer(new QTimer(this)),
    mDateLabel(new QLabel(this)),
    mTimeLabel(new QLabel(this))
{
    setAttribute(Qt::WA_NoMousePropagation);
    mUsePanelFont = mSettings->value("usePanelFont", true).toBool();
    mFont = QFont();
    mFont.fromString(mSettings->value("font", QFont().toString()).toString());
    mDateFormat = mSettings->value("dateFormat","ddd, dd MMM yy").toString();
    mTimeFormat = mSettings->value("timeFormat", "HH:mm:ss").toString();
    mDirection = static_cast<QBoxLayout::Direction>(mSettings->value("direction", QBoxLayout::TopToBottom).toInt());
    mMargin = mSettings->value("margin", 2).toInt();

    mShape = mSettings->value("shape", "StyledPanel").toString();
    mShadow = mSettings->value("shadow", "Raised").toString();
    mLineWidth = mSettings->value("lineWidth", 1).toInt();
    mMidLineWidth = mSettings->value("midLineWidth", 1).toInt();

    //qDebug() << mFont.family() << mFont.pixelSize() << mFont.pointSize() << mFont.weight();


    mLayout = new QBoxLayout(mDirection, this);
    mLayout->setMargin(mMargin);
//    setFrameShape(Shape::StyledPanel);
//    setFrameShadow(Shadow::Sunken);

    changeFrame();

    mDateLabel->setAlignment(Qt::AlignCenter);
    mTimeLabel->setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setLayout(mLayout);

    mLayout->addWidget(mDateLabel);
    mLayout->addWidget(mTimeLabel);
    updateClock();
    connect(mTimer, &QTimer::timeout, this, &Clock::updateClock);
    startTimer();
    if(mUsePanelFont){
        setFont(mPanel->font());
    }
    else {
        setFont(mFont);
    }
}
void Clock::changeShape(QString s){
    mShape = s;
    mSettings->setValue("shape", s);
    changeFrame();
}
void Clock::changeShadow(QString s){
    mShadow = s;
    mSettings->setValue("shadow", s);
    changeFrame();
}
void Clock::changeLineWidth(int w){
    mLineWidth = w;
    mSettings->setValue("lineWidth", w);
    changeFrame();
}
void Clock::changeMidLineWidth(int w){
    mMidLineWidth = w;
    mSettings->setValue("midLineWidth", w);
    changeFrame();
}
void Clock::changeFrame(){
    setFrameShape(Shapes[mShape]);
    setFrameShadow(Shadows[mShadow]);
    setLineWidth(mLineWidth);
    setMidLineWidth(mMidLineWidth);
};
void Clock::showDialog(){
    QString dateTip =
        "DATE FORMAT STRING\n"\
        "d	The day as a number without a leading zero (1 to 31)\n"\
        "dd	The day as a number with a leading zero (01 to 31)\n"\
        "ddd	The abbreviated localized day name (e.g. 'Mon' to 'Sun'). Uses the system locale to localize the name, i.e. QLocale::system().\n"\
        "dddd	The long localized day name (e.g. 'Monday' to 'Sunday'). Uses the system locale to localize the name, i.e. QLocale::system().\n"\
        "M	The month as a number without a leading zero (1 to 12)\n"\
        "MM	The month as a number with a leading zero (01 to 12)\n"\
        "MMM	The abbreviated localized month name (e.g. 'Jan' to 'Dec'). Uses the system locale to localize the name, i.e. QLocale::system().\n"\
        "MMMM	The long localized month name (e.g. 'January' to 'December'). Uses the system locale to localize the name, i.e. QLocale::system().\n"\
        "yy	The year as a two digit number (00 to 99)\n"\
        "yyyy	The year as a four digit number. If the year is negative, a minus sign is prepended, making five characters.";

    QString timeTip =
        "TIME FORMAT STRING\n"\
        "h	The hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)\n"\
        "hh	The hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)\n"\
        "H	The hour without a leading zero (0 to 23, even with AM/PM display)\n"\
        "HH	The hour with a leading zero (00 to 23, even with AM/PM display)\n"\
        "m	The minute without a leading zero (0 to 59)\n"\
        "mm	The minute with a leading zero (00 to 59)\n"\
        "s	The whole second, without any leading zero (0 to 59)\n"\
        "ss	The whole second, with a leading zero where applicable (00 to 59)\n"\
        "z	The fractional part of the second, to go after a decimal point, without trailing zeroes (0 to 999). Thus \"s.z\" reports the seconds to full available (millisecond) precision without trailing zeroes.\n"\
        "zzz	The fractional part of the second, to millisecond precision, including trailing zeroes where applicable (000 to 999).\n"\
        "AP or A	Use AM/PM display. A/AP will be replaced by an upper-case version of either QLocale::amText() or QLocale::pmText().\n"\
        "ap or a	Use am/pm display. a/ap will be replaced by a lower-case version of either QLocale::amText() or QLocale::pmText().\n"\
        "t	The timezone (for example \"CEST\")";

    auto dialog = new QDialog(this, Qt::Popup);
    //dialog->setAttribute( Qt::WA_X11NetWmWindowTypePopupMenu);
    auto form = new QFormLayout(dialog);

    auto fontCheck = new QCheckBox(QString("%1 %2, %3")
                                   .arg(mPanel->font().family())
                                   .arg(mPanel->font().styleName())
                                   .arg(QString::number(qMax(mPanel->font().pointSize(), mPanel->font().pixelSize()))), dialog);
    auto fontButton = new QPushButton(QString("%1 %2, %3")
                                      .arg(mFont.family())
                                      .arg(mFont.styleName())
                                      .arg(QString::number(qMax(mFont.pointSize(), mFont.pixelSize())))
                                      , dialog);
    auto dateLabel = new QLabel("Date Format (hover for tooltip):", dialog);
    auto dateEdit = new QLineEdit(mDateFormat, dialog);
    auto timeLabel = new QLabel("Time Format (hover for tooltip):", dialog);
    auto timeEdit = new QLineEdit(mTimeFormat, dialog);
    auto directionCombo = new QComboBox(dialog);
    auto marginSpin = new QSpinBox(dialog);

    auto frameGroup = new QGroupBox("Frame Config", dialog);
    auto frameForm =new QFormLayout(dialog);
    frameGroup->setLayout(frameForm);

    auto frameShapeCombo = new QComboBox(dialog);
    auto frameShadowCombo = new QComboBox(dialog);
    auto frameLineSpin = new QSpinBox(dialog);
    auto frameMidLineSpin = new QSpinBox(dialog);

    frameShapeCombo->addItems({"NoFrame", "Box", "Panel", "StyledPanel", "HLine", "VLine", "WinPanel"});
    frameShapeCombo->setCurrentText(mShape);
    frameShadowCombo->addItems({"Plain", "Raised", "Sunken"});
    frameShadowCombo->setCurrentText(mShadow);
    frameLineSpin->setRange(0,10);
    frameLineSpin->setValue(mLineWidth);
    frameMidLineSpin->setRange(0,10);
    frameMidLineSpin->setValue(mMidLineWidth);

    connect(frameShapeCombo,    &QComboBox::currentTextChanged,                 this, &Clock::changeShape);
    connect(frameShadowCombo,   &QComboBox::currentTextChanged,                 this, &Clock::changeShadow);
    connect(frameLineSpin,      QOverload<int>::of(&QSpinBox::valueChanged),    this, &Clock::changeLineWidth);
    connect(frameMidLineSpin,   QOverload<int>::of(&QSpinBox::valueChanged),    this, &Clock::changeMidLineWidth);

    frameForm->addRow("Shape:", frameShapeCombo);
    frameForm->addRow("Shadow:", frameShadowCombo);
    frameForm->addRow("Line Width:", frameLineSpin);
    frameForm->addRow("Mid Line Width:", frameMidLineSpin);

    fontCheck->setChecked(mUsePanelFont);
    fontButton->setDisabled(mUsePanelFont);
    fontButton->setMinimumWidth(200);
    dateLabel->setToolTip(dateTip);
    dateEdit->setToolTip(dateTip);
    timeLabel->setToolTip(timeTip);
    timeEdit->setToolTip(timeTip);

//    QBoxLayout::LeftToRight	0	Horizontal from left to right.
//    QBoxLayout::RightToLeft	1	Horizontal from right to left.
//    QBoxLayout::TopToBottom	2	Vertical from top to bottom.
//    QBoxLayout::BottomToTop	3	Vertical from bottom to top.
    directionCombo->addItems({"LeftToRight", "RightToLeft", "TopToBottom", "BottomToTop"});
    directionCombo->setCurrentIndex(mDirection);
    marginSpin->setRange(0, 10);
    marginSpin->setValue(mMargin);
    connect(fontCheck, &QCheckBox::toggled, this, [=] (bool checked){
        fontButton->setDisabled(checked);
        setUsePanelFont(checked);
    });
    connect(fontButton, &QPushButton::clicked, this, [=]{
        auto fontDialog = new QFontDialog(dialog);
        fontDialog->setWindowFlags(Qt::Popup);
        fontDialog->setCurrentFont(mFont);
        connect(fontDialog, &QFontDialog::fontSelected, this, [=] (QFont font) {
           fontButton->setText(QString("%1 %2, %3")
                               .arg(font.family())
                               .arg(font.styleName())
                               .arg(QString::number(qMax(font.pointSize(), font.pixelSize()))));
           setClockFont(font);
        });
        fontDialog->show();
    });
    connect(dateEdit, &QLineEdit::textChanged, this, &Clock::setDateFormat);
    connect(timeEdit, &QLineEdit::textChanged, this, &Clock::setTimeFormat);
    connect(directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Clock::setClockDirection);
    connect(marginSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &Clock::setClockMargin);

//    form->addRow("Date Format:", dateEdit);
//    form->addRow("Time Format:", timeEdit);
    dialog->setWindowTitle("Clock Settings");
    form->addRow("Use panel font:", fontCheck);
    form->addRow("Select font:", fontButton);
    form->addRow(dateLabel, dateEdit);
    form->addRow(timeLabel, timeEdit);
    form->addRow("Direction:", directionCombo);
    form->addRow("Margin:", marginSpin);
    form->addRow(frameGroup);


    //auto center = mapFromParent(geometry().center());
    auto center = geometry().center();
    qDebug() << center;
    auto dialogGeo = mPanel->calculateMenuPosition(center, dialog->sizeHint(), 4, false);
    qDebug() << "menuGeo" << dialogGeo;
    dialog->setGeometry(dialogGeo);
    //connect(fontButton, QPushButton::textC)
    dialog->show();
}
void Clock::setUsePanelFont(bool usePanelFont){
    mUsePanelFont = usePanelFont;
    mSettings->setValue("usePanelFont", usePanelFont);
    if(usePanelFont){
        setFont(mPanel->font());
    }
    else {
        setFont(mFont);
    }
}
void Clock::setClockFont(QFont font){
    mFont = font;
    setFont(mFont);
    mSettings->setValue("font", mFont.toString());
}
void Clock::setDateFormat(QString newDateFormat){
    mDateFormat = newDateFormat;
    mDateLabel->setText(QDate::currentDate().toString(mDateFormat));
    saveSettings();
}
void Clock::setTimeFormat(QString newTimeFormat){
    mTimeFormat = newTimeFormat;
    mTimeLabel->setText(QTime::currentTime().toString(mTimeFormat));
    saveSettings();
}
void Clock::setClockDirection(int newDirection){
    mDirection = static_cast<QBoxLayout::Direction>(newDirection);
    mLayout->setDirection(mDirection);
    saveSettings();
}
void Clock::setClockMargin(int newMargin){
    mMargin = newMargin;
    mLayout->setMargin(mMargin);
    saveSettings();
}
void Clock::saveSettings(){
    mSettings->setValue("dateFormat", mDateFormat);
    mSettings->setValue("timeFormat", mTimeFormat);
    mSettings->setValue("direction", mDirection);
    mSettings->setValue("margin", mMargin);
}
void Clock::updateClock(){
    mDateLabel->setText(QDate::currentDate().toString(mDateFormat));
    mTimeLabel->setText(QTime::currentTime().toString(mTimeFormat));
}
void Clock::mousePressEvent(QMouseEvent *event){
    Q_UNUSED(event);
    showDialog();
//    qDebug() << "menu requested";
//    mMenu->clear();
//    QAction * a;
//    QBoxLayout::Direction dir = mLayout->direction();
//    if( dir != QBoxLayout::LeftToRight){
//        a = mMenu->addAction("Left To Right");
//        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::LeftToRight);});
//    }
//    if( dir != QBoxLayout::RightToLeft){
//        a = mMenu->addAction("Right To Left");
//        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::RightToLeft);});
//    }
//    if( dir != QBoxLayout::TopToBottom){
//        a = mMenu->addAction("Top To Bottom");
//        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::TopToBottom);});
//    }
//    if( dir != QBoxLayout::BottomToTop){
//        a = mMenu->addAction("Bottom To Top");
//        connect(a, &QAction::triggered, this, [=]{mLayout->setDirection(QBoxLayout::BottomToTop);});
//    }
//    auto menuGeo = mPanel->calculateMenuPosition(event->globalPos(), mMenu->sizeHint());
//    qDebug() << "menuGeo" << menuGeo;
//    mMenu->setGeometry(menuGeo);
//    mMenu->show();
}

void Clock::startTimer(){
    mTimer->setInterval(1000);
    mTimer->start();
}