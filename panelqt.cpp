#include "panelqt.h"
#include "appmenu.h"
#include "taskbar.h"
#include "snframe.h"
#include "clock.h"

extern QHash<QString, QFrame::Shape> Shapes;
extern QHash<QString, QFrame::Shadow> Shadows;

PanelQt::PanelQt(QString panelName, QWidget *parent):
    QFrame(parent),
    mSettings(new QSettings("PanelQt", panelName + "/panel")),
    mPanelName(panelName),
    //mBackgroundColor(QColor(QApplication::palette())),
    mLayout(new QHBoxLayout(this))
{
    mBackgroundColor = palette().color(QPalette::Window);
    mOpacity = mSettings->value("opacity", 180).toInt();
    mSystemFont = QFont();
    mUseSystemFont = mSettings->value("useSystemFont", true).toBool();
    mFont = QFont();
    mFont.fromString(mSettings->value("font", QFont().toString()).toString());
    mScreen = mSettings->value("screen", 0).toInt(); //QX11Info::appScreen();
    mPosition = mSettings->value("position", "Bottom").toString(); // Top or Bottom
    mHeight = mSettings->value("height", 32).toInt();

    mShape = mSettings->value("shape", "StyledPanel").toString();
    mShadow = mSettings->value("shadow", "Raised").toString();
    mLineWidth = mSettings->value("lineWidth", 1).toInt();
    mMidLineWidth = mSettings->value("midLineWidth", 1).toInt();

    //mPosition = "Top";
    if (mScreen > QApplication::screens().count() - 1)
        mScreen = QApplication::screens().count() - 1;


    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

//    setFrameShape(Shape::StyledPanel);
//    setFrameShadow(Shadow::Raised);
//    setMidLineWidth(4);

    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    setLayout(mLayout);
    setFocusPolicy(Qt::NoFocus);
    this->setContentsMargins(0,0,0,0);
    //setFrameShape(QFrame::NoFrame);
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus;
    setWindowFlags(flags);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
//    setAttribute(Qt::WA_AlwaysShowToolTips);

    //changeFrame();

    if(mUseSystemFont){
        setFont(mSystemFont);
    }
    else {
        setFont(mFont);
    }
    init();
}

void PanelQt::changeShape(QString s){
    mShape = s;
    mSettings->setValue("shape", s);
    changeFrame();
}
void PanelQt::changeShadow(QString s){
    mShadow = s;
    mSettings->setValue("shadow", s);
    changeFrame();
}
void PanelQt::changeLineWidth(int w){
    mLineWidth = w;
    mSettings->setValue("lineWidth", w);
    changeFrame();
}
void PanelQt::changeMidLineWidth(int w){
    mMidLineWidth = w;
    mSettings->setValue("midLineWidth", w);
    changeFrame();
}
void PanelQt::changeFrame(){
    setFrameShape(Shapes[mShape]);
    setFrameShadow(Shadows[mShadow]);
    setLineWidth(mLineWidth);
    setMidLineWidth(mMidLineWidth);
};

void PanelQt::setPanelOpacity(int o){
    mOpacity = o;
    mSettings->setValue("opacity", o);
    repaint();
}

void PanelQt::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    QColor color(mBackgroundColor);
    color.setAlpha(mOpacity);
    painter.fillRect(event->rect(), color);
}

void PanelQt::setUseSystemFont(bool useSystemFont){
    mUseSystemFont = useSystemFont;
    mSettings->setValue("useSystemFont", useSystemFont);
    if(useSystemFont){
        setFont(mSystemFont);
    }
    else {
        setFont(mFont);
    }
}
void PanelQt::setPanelFont(QFont font){
    mFont = font;
    setFont(mFont);
    mSettings->setValue("font", mFont.toString());
}

void PanelQt::mousePressEvent(QMouseEvent *event){
    showDialog(event->globalPos());
}
void PanelQt::setPanelHeight(int h){
    mHeight = h;
    mSettings->setValue("height", h);
    updatePanelGeometry();
}
void PanelQt::setPanelScreen(int screen){
    mScreen = screen;
    mSettings->setValue("screen", screen);
    updatePanelGeometry();
    if(!mTaskBar->mShowAllScreens)
        mTaskBar->resetTaskBar();
}
void PanelQt::setPanelPosition(QString p){
    mPosition = p;
    mSettings->setValue("position", p);
    updatePanelGeometry();
}
void PanelQt::updatePanelGeometry(){
    calculateGeometry();
    setGeometry(mGeometry);
    setFixedSize(mGeometry.size());
    updateStrut();
}
void PanelQt::init(){
    mAppMenu = new AppMenu(this);
    mTaskBar = new TaskBar(this);
    mSNFrame = new SNFrame(this);
    mClock = new Clock(this);


//    auto spacer = new QFrame();
//    spacer->setFrameShape(QFrame::Shape::NoFrame);
//    spacer->setBaseSize(mGeometry.width(), 1);
//    spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    mLayout->addWidget(mAppMenu);
    mLayout->addWidget(mTaskBar);
    //mLayout->addWidget(spacer);
    mLayout->addSpacing(4);

    mLayout->addWidget(mSNFrame);
    mLayout->addWidget(mClock);

    updatePanelGeometry();
}
QRect PanelQt::calculateMenuPosition(QPoint pos, QSize size, int gap, bool isGlobal, float hScale, float vScale){
    QSize scaledSize;
    scaledSize.setWidth(size.width() * hScale);
    scaledSize.setHeight(size.height() * vScale);
    auto screenGeo = QApplication::screens()[mScreen]->geometry();
    auto menuGeo = isGlobal ? QRect(pos, scaledSize) : QRect(mapToGlobal(pos), scaledSize);
    //qDebug() << "calculateMenuPosition" << pos << mapToGlobal(pos) << isGlobal << menuGeo;

    menuGeo.moveLeft(menuGeo.left() - scaledSize.width()/2); //center menu

    if(mPosition == "Bottom")
        screenGeo.moveBottom(screenGeo.bottom() - mHeight);
    if(mPosition == "Top")
        screenGeo.moveTop(screenGeo.top() + mHeight);

    //qDebug() << "moveTop" << menuGeo;

    if (menuGeo.top() < screenGeo.top() + gap){
        menuGeo.moveTop(screenGeo.top() + gap);
        //qDebug() << "moveTop" << menuGeo;
    }
    if (menuGeo.bottom() > screenGeo.bottom() - gap){
        menuGeo.moveBottom(screenGeo.bottom() - gap);
        //qDebug() << "moveBottom" << menuGeo;
    }
    if (menuGeo.left() < screenGeo.left() + gap){
        menuGeo.moveLeft(screenGeo.left() + gap);
        //qDebug() << "moveLeft" << menuGeo;
    }
    if (menuGeo.right() > screenGeo.right() - gap){
        menuGeo.moveRight(screenGeo.right() - gap);
        //qDebug() << "moveRight" << menuGeo;
    }

    //qDebug() << "calculateMenuPosition" << screenGeo << menuGeo;
    return menuGeo;
}
void PanelQt::calculateGeometry(){
    QRect scr_geo = QApplication::screens()[mScreen]->geometry();
    if (mPosition == "Bottom"){
        mGeometry = QRect(scr_geo.bottomLeft() - QPoint(0, mHeight), scr_geo.bottomRight());
    }
    else if(mPosition == "Top") {
        mGeometry = QRect(scr_geo.topLeft(), scr_geo.topRight() + QPoint(0, mHeight));
    }
}

void PanelQt::updateStrut(){

//    auto geo = QApplication::screens()[mScreen] -> geometry();
//    WId win = winId();
//    KWindowSystem::setStrut(win, geo.left(), geo.right(), geo.bottom() + mHeight, geo.bottom());

    auto virtGeo = QApplication::screens()[mScreen]->virtualGeometry();
    WId win = winId();                //the id of the window
    int left_width = 0,     //width of the strut at the left edge
        left_start = 0,     //starting y coordinate of the strut at the left edge
        left_end = 0,       //ending y coordinate of the strut at the left edge
        right_width = 0,    //width of the strut at the right edge
        right_start = 0,    //starting y coordinate of the strut at the right edge
        right_end = 0,      //ending y coordinate of the strut at the right edge
        top_width = 0,      //width of the strut at the top edge
        top_start = 0,      //starting x coordinate of the strut at the top edge
        top_end = 0,        //ending x coordinate of the strut at the top edge
        bottom_width = 0,   //width of the strut at the bottom edge
        bottom_start = 0,   //starting x coordinate of the strut at the bottom edge
        bottom_end = 0;     //ending x coordinate of the strut at the bottom edge

    if(mPosition == "Bottom"){
        bottom_width = virtGeo.bottom() - mGeometry.bottom() + mHeight;
        bottom_start = mGeometry.left();
        bottom_end = mGeometry.right();
    }
    if(mPosition == "Top"){
        top_width = mGeometry.top() + mHeight;
        top_start = mGeometry.left();
        top_end = mGeometry.right();
    }
    KWindowSystem::setExtendedStrut	(win, left_width, left_start, left_end, right_width, right_start, right_end, top_width, top_start, top_end, 	bottom_width, bottom_start, bottom_end);
}
void PanelQt::showDialog(QPoint pos){
    auto dialog = new QDialog(this, Qt::Popup);
    auto box = new QHBoxLayout(dialog);

    //Panel config
    auto panelGroup = new QGroupBox("Panel Config", dialog);
    auto panelForm = new QFormLayout(panelGroup);

    auto fontCheck = new QCheckBox(QString("%1 %2, %3")
                                   .arg(mSystemFont.family())
                                   .arg(mSystemFont.styleName())
                                   .arg(QString::number(qMax(mSystemFont.pointSize(), mSystemFont.pixelSize()))), dialog);
    auto fontButton = new QPushButton(QString("%1 %2, %3")
                                      .arg(mFont.family())
                                      .arg(mFont.styleName())
                                      .arg(QString::number(qMax(mFont.pointSize(), mFont.pixelSize())))
                                      , dialog);
    auto screenSpin = new QSpinBox(dialog);
    auto positionCombo = new QComboBox(dialog);
    auto heightSpin = new QSpinBox(dialog);
    auto opacitySlider = new QSlider(Qt::Horizontal, dialog);

    auto panelShapeCombo = new QComboBox(dialog);
    auto panelShadowCombo = new QComboBox(dialog);
    auto panelLineSpin = new QSpinBox(dialog);
    auto panelMidLineSpin = new QSpinBox(dialog);


    fontCheck->setChecked(mUseSystemFont);
    fontButton->setDisabled(mUseSystemFont);
    fontButton->setMinimumWidth(200);
    screenSpin->setRange(0, QApplication::screens().count() - 1);
    screenSpin->setValue(mScreen);

    positionCombo->addItem("Top");
    positionCombo->addItem("Bottom");
    positionCombo->setCurrentText(mPosition);

    heightSpin->setRange(10, 100);
    heightSpin->setValue(mHeight);

    opacitySlider->setRange(0, 255);
    opacitySlider->setValue(mOpacity);
    opacitySlider->setTickPosition(QSlider::TicksAbove);

    panelShapeCombo->addItems({"NoFrame", "Box", "Panel", "StyledPanel", "HLine", "VLine", "WinPanel"});
    panelShapeCombo->setCurrentText(mShape);
    panelShadowCombo->addItems({"Plain", "Raised", "Sunken"});
    panelShadowCombo->setCurrentText(mTaskBar->mShadow);
    panelLineSpin->setRange(0,10);
    panelLineSpin->setValue(mTaskBar->mLineWidth);
    panelMidLineSpin->setRange(0,10);
    panelMidLineSpin->setValue(mTaskBar->mMidLineWidth);

    connect(fontCheck, &QCheckBox::toggled, this, [=] (bool checked){
        fontButton->setDisabled(checked);
        setUseSystemFont(checked);
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
           setPanelFont(font);
        });
        fontDialog->show();
    });
    connect(screenSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PanelQt::setPanelScreen);
    connect(positionCombo, &QComboBox::currentTextChanged, this, &PanelQt::setPanelPosition);
    connect(heightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PanelQt::setPanelHeight);
    connect(opacitySlider, QOverload<int>::of(&QSlider::valueChanged), this, &PanelQt::setPanelOpacity);
    connect(panelShapeCombo,    &QComboBox::currentTextChanged,                 this, &PanelQt::changeShape);
    connect(panelShadowCombo,   &QComboBox::currentTextChanged,                 this, &PanelQt::changeShadow);
    connect(panelLineSpin,      QOverload<int>::of(&QSpinBox::valueChanged),    this, &PanelQt::changeLineWidth);
    connect(panelMidLineSpin,   QOverload<int>::of(&QSpinBox::valueChanged),    this, &PanelQt::changeMidLineWidth);

    panelForm->addRow("Use system font:", fontCheck);
    panelForm->addRow("Select font:", fontButton);
    panelForm->addRow("Screen:", screenSpin);
    panelForm->addRow("Position:", positionCombo);
    panelForm->addRow("Height:", heightSpin);
    panelForm->addRow("Background opacity:", opacitySlider);
    panelForm->addRow("Shape:", panelShapeCombo);
    panelForm->addRow("Shadow:", panelShadowCombo);
    panelForm->addRow("Line Width:", panelLineSpin);
    panelForm->addRow("Mid Line Width:", panelMidLineSpin);


    //Taskbar config
    auto taskbarGroup = new QGroupBox("TaskBar Config", dialog);
    auto taskbarForm = new QFormLayout(taskbarGroup);

    auto showScreenCheck = new QCheckBox(taskbarGroup);
    auto barShapeCombo = new QComboBox(dialog);
    auto barShadowCombo = new QComboBox(dialog);
    auto barLineSpin = new QSpinBox(dialog);
    auto barMidLineSpin = new QSpinBox(dialog);

    showScreenCheck->setChecked(mTaskBar->mShowAllScreens);
    barShapeCombo->addItems({"NoFrame", "Box", "Panel", "StyledPanel", "HLine", "VLine", "WinPanel"});
    barShapeCombo->setCurrentText(mTaskBar->mShape);
    barShadowCombo->addItems({"Plain", "Raised", "Sunken"});
    barShadowCombo->setCurrentText(mTaskBar->mShadow);
    barLineSpin->setRange(0,10);
    barLineSpin->setValue(mTaskBar->mLineWidth);
    barMidLineSpin->setRange(0,10);
    barMidLineSpin->setValue(mTaskBar->mMidLineWidth);

    connect(showScreenCheck,    &QCheckBox::toggled,                            mTaskBar, &TaskBar::setShowAllScreens);
    connect(barShapeCombo,      &QComboBox::currentTextChanged,                 mTaskBar, &TaskBar::changeShape);
    connect(barShadowCombo,     &QComboBox::currentTextChanged,                 mTaskBar, &TaskBar::changeShadow);
    connect(barLineSpin,        QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::changeLineWidth);
    connect(barMidLineSpin,     QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::changeMidLineWidth);

    taskbarForm->addRow("Show from all screens:", showScreenCheck);
    taskbarForm->addRow("Shape:", barShapeCombo);
    taskbarForm->addRow("Shadow:", barShadowCombo);
    taskbarForm->addRow("Line Width:", barLineSpin);
    taskbarForm->addRow("Mid Line Width:", barMidLineSpin);

    //////GROUP

    auto groupGroup = new QGroupBox("Group Config", dialog);
    auto groupForm =new QFormLayout(groupGroup);
    groupGroup->setLayout(groupForm);

    auto groupShapeCombo = new QComboBox(dialog);
    auto groupShadowCombo = new QComboBox(dialog);
    auto groupLineSpin = new QSpinBox(dialog);
    auto groupMidLineSpin = new QSpinBox(dialog);

    groupShapeCombo->addItems({"NoFrame", "Box", "Panel", "StyledPanel", "HLine", "VLine", "WinPanel"});
    groupShapeCombo->setCurrentText(mTaskBar->mGroupShape);
    groupShadowCombo->addItems({"Plain", "Raised", "Sunken"});
    groupShadowCombo->setCurrentText(mTaskBar->mGroupShadow);
    groupLineSpin->setRange(0,10);
    groupLineSpin->setValue(mTaskBar->mGroupLineWidth);
    groupMidLineSpin->setRange(0,10);
    groupMidLineSpin->setValue(mTaskBar->mGroupMidLineWidth);

    connect(groupShapeCombo,    &QComboBox::currentTextChanged,                 mTaskBar, &TaskBar::groupChangeShape);
    connect(groupShadowCombo,   &QComboBox::currentTextChanged,                 mTaskBar, &TaskBar::groupChangeShadow);
    connect(groupLineSpin,      QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::groupChangeLineWidth);
    connect(groupMidLineSpin,   QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::groupChangeMidLineWidth);

    groupForm->addRow("Shape:", groupShapeCombo);
    groupForm->addRow("Shadow:", groupShadowCombo);
    groupForm->addRow("Line Width:", groupLineSpin);
    groupForm->addRow("Mid Line Width:", groupMidLineSpin);

    taskbarForm->addRow(groupGroup);

    //////BUTTONS
    auto buttonGroup = new QGroupBox("Button Config", dialog);
    auto buttonForm = new QFormLayout(buttonGroup);
    buttonGroup->setLayout(buttonForm);

    auto maxBtnWidthSpin = new QSpinBox(taskbarGroup);
    auto iconHeightSpin = new QSpinBox(taskbarGroup);
    auto buttonRaiseCheck = new QCheckBox(taskbarGroup);
    auto buttonUnderline = new QCheckBox(taskbarGroup);

    maxBtnWidthSpin->setRange(50, 500);
    maxBtnWidthSpin->setValue(mTaskBar->mMaxBtnWidth);
    maxBtnWidthSpin->setMinimumWidth(50);
    iconHeightSpin->setRange(12,128);
    iconHeightSpin->setValue(mTaskBar->mIconHeight);
    buttonRaiseCheck->setChecked(mTaskBar->mButtonAutoRaise);
    buttonUnderline->setChecked(mTaskBar->mButtonUnderline);

    buttonForm->addRow("Width:", maxBtnWidthSpin);
    buttonForm->addRow("Icon size", iconHeightSpin);
    buttonForm->addRow("Button auto raise:", buttonRaiseCheck);
    buttonForm->addRow("Underline active button:", buttonUnderline);

    connect(maxBtnWidthSpin,    QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::setMaxBtnWidth);
    connect(iconHeightSpin,     QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::setIconHeight);
    connect(buttonRaiseCheck,   &QCheckBox::toggled,                            mTaskBar, &TaskBar::setButtonAutoRaise);
    connect(buttonUnderline,    &QCheckBox::toggled,                            mTaskBar, &TaskBar::setButtonUnderline);

    taskbarForm->addRow(buttonGroup);

    ////////PINS
    auto pinGroup = new QGroupBox("Pin Config", dialog);
    auto pinForm = new QFormLayout(pinGroup);
    pinGroup->setLayout(pinForm);

    auto pinIconHeightSpin = new QSpinBox(taskbarGroup);
    auto pinBtnWidthSpin = new QSpinBox(taskbarGroup);
    auto pinRaiseCheck = new QCheckBox(taskbarGroup);

    pinIconHeightSpin->setRange(12,128);
    pinIconHeightSpin->setValue(mTaskBar->mPinIconHeight);
    pinBtnWidthSpin->setRange(12,500);
    pinBtnWidthSpin->setValue(mTaskBar->mPinBtnWidth);
    pinRaiseCheck->setChecked(mTaskBar->mPinAutoRaise);

    pinForm->addRow("Icon size:", pinIconHeightSpin);
    pinForm->addRow("Width:", pinBtnWidthSpin);
    pinForm->addRow("Auto raise:", pinRaiseCheck);

    connect(pinIconHeightSpin,  QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::setPinIconHeight);
    connect(pinBtnWidthSpin,    QOverload<int>::of(&QSpinBox::valueChanged),    mTaskBar, &TaskBar::setPinBtnWidth);
    connect(pinRaiseCheck,      &QCheckBox::toggled,                            mTaskBar, &TaskBar::setPinAutoRaise);

    taskbarForm->addRow(pinGroup);

    //////LAYOUT

    panelGroup->setLayout(panelForm);
    taskbarGroup->setLayout(taskbarForm);

    box->addWidget(panelGroup);
    box->addWidget(taskbarGroup);
    dialog->setLayout(box);
    //Position
    auto dialogGeo = calculateMenuPosition(pos, dialog->sizeHint(), 4, false);
    qDebug() << "geo" << dialogGeo;
    dialog->setGeometry(dialogGeo);
    dialog->show();
}
PanelQt::~PanelQt()
{
    //delete ui;
}

