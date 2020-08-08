#include "panelqt.h"
#include "appmenu.h"
#include "taskbar.h"
#include "volumeframe.h"
#include "snframe.h"
#include "clock.h"

extern QHash<QString, QFrame::Shape> Shapes;
extern QHash<QString, QFrame::Shadow> Shadows;

PanelQt::PanelQt(QString panelName, QWidget *parent):
    QFrame(parent),
    mSettings(new QSettings("PanelQt", panelName + "/panel")),
    mPanelName(panelName),
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
    mWidthPercentage = mSettings->value("widthPercentage", 70).toInt();
    mAlignment = mSettings->value("alignment", "Center").toString(); // Left, Center or Right

    mShape = mSettings->value("shape", "NoFrame").toString();
    mShadow = mSettings->value("shadow", "Plain").toString();
    mLineWidth = mSettings->value("lineWidth", 0).toInt();
    mMidLineWidth = mSettings->value("midLineWidth", 0).toInt();

    mAllPlugins = QStringList({"AppMenu", "TaskBar", "Status Notifier", "Volume", "Clock"});
    mPlugins = mSettings->value("plugins", mAllPlugins).toStringList();
    //mPosition = "Top";
    if (mScreen > QApplication::screens().count() - 1)
        mScreen = QApplication::screens().count() - 1;


    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    setLayout(mLayout);
    setFocusPolicy(Qt::NoFocus);
    this->setContentsMargins(0,0,0,0);
    //setFrameShape(QFrame::NoFrame);
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus;
    setWindowFlags(flags);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);

    if(mUseSystemFont){
        setFont(mSystemFont);
    }
    else {
        setFont(mFont);
    }
    init();
    connect(KWindowSystem::self(), &KWindowSystem::strutChanged, this, &PanelQt::updatePanelGeometry);
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
    useSystemFont ? setFont(mSystemFont) : setFont(mFont);
    if(mClock) mClock->updateFont();
}
void PanelQt::setPanelFont(QFont font){
    mFont = font;
    setFont(mFont);
    mSettings->setValue("font", mFont.toString());
    if(mClock) mClock->updateFont();
}

void PanelQt::mousePressEvent(QMouseEvent *event){
    showDialog(event->globalPos());
}
void PanelQt::setPanelHeight(int h){
    mHeight = h;
    mSettings->setValue("height", h);
    updatePanelGeometryAndStrut();
}
void PanelQt::setPanelScreen(int screen){
    mScreen = screen;
    mSettings->setValue("screen", screen);
    updatePanelGeometryAndStrut();
    if(!mTaskBar->mShowAllScreens)
        mTaskBar->resetTaskBar();
}
void PanelQt::setPanelAlignment(QString a){
    mAlignment = a;
    mSettings->setValue("alignment", a);
    updatePanelGeometryAndStrut();
}
void PanelQt::setWidthPercentage(int w){
    mWidthPercentage = w;
    mSettings->setValue("widthPercentage", w);
    updatePanelGeometryAndStrut();
}
void PanelQt::setPanelPosition(QString p){
    mPosition = p;
    mSettings->setValue("position", p);
    updatePanelGeometryAndStrut();
}
void PanelQt::updatePanelGeometryAndStrut(){
    calculateGeometry();
    setGeometry(mGeometry);
    setFixedSize(mGeometry.size());
    updateStrut();
}
void PanelQt::updatePanelGeometry(){
    calculateGeometry();
    setGeometry(mGeometry);
    setFixedSize(mGeometry.size());
}
void PanelQt::init(){
    if(mPlugins.contains("AppMenu")){
        mAppMenu = new AppMenu(this);
        mLayout->addWidget(mAppMenu);
    }
    if(mPlugins.contains("TaskBar")){
        mTaskBar = new TaskBar(this);
        mLayout->addWidget(mTaskBar);
    }

    if(mPlugins.contains("Status Notifier")){
        mSNFrame = new SNFrame(this);
        mLayout->addWidget(mSNFrame);
    }
    if(mPlugins.contains("Volume")){
        mVolumeFrame = new VolumeFrame(this);
        mLayout->addWidget(mVolumeFrame);
    }
    if(mPlugins.contains("Clock")){
        mClock = new Clock(this);
        mLayout->addWidget(mClock);
    }
    mLayout->setSpacing(2);
    updatePanelGeometryAndStrut();
}

void PanelQt::addRemovePlugin(QString plugin, bool add){
    int index = 0;
    for (auto p : mAllPlugins) {
        if(p==plugin)
            break;
        if(mPlugins.contains(p))
            ++index;
    }
    qDebug() << "plugin" << add << plugin << index;

    if(plugin == "AppMenu"){
        if(add){
            mAppMenu = new AppMenu(this);
            mLayout->insertWidget(index, mAppMenu);
            mPlugins.insert(index, plugin);
        }else {
            mLayout->takeAt(index)->widget()->deleteLater();
            //mAppMenu->deleteLater();
            mAppMenu = nullptr;
            mPlugins.removeAt(index);
        }
    }
    if(plugin == "TaskBar"){
        if(add){
            mTaskBar = new TaskBar(this);
            mLayout->insertWidget(index, mTaskBar);
            mPlugins.insert(index, plugin);
        }else {
            mLayout->takeAt(index)->widget()->deleteLater();
            //mTaskBar->deleteLater();
            mTaskBar = nullptr;
            mPlugins.removeAt(index);
        }
    }

    if(plugin == "Status Notifier"){
        if(add){
            mSNFrame = new SNFrame(this);
            mLayout->insertWidget(index, mSNFrame);
            mPlugins.insert(index, plugin);
            updatePanelGeometryAndStrut();
        }else {
            mLayout->takeAt(index)->widget()->deleteLater();
            //mSNFrame->deleteLater();
            mSNFrame = nullptr;
            mPlugins.removeAt(index);
        }

    }
    if(plugin == "Volume"){
        if(add){
            mVolumeFrame = new VolumeFrame(this);
            mLayout->insertWidget(index, mVolumeFrame);
            mPlugins.insert(index, plugin);
        }else {
            mLayout->takeAt(index)->widget()->deleteLater();
            mVolumeFrame = nullptr;
            mPlugins.removeAt(index);
        }
    }
    if(plugin == "Clock"){
        if(add){
            mClock = new Clock(this);
            mLayout->insertWidget(index, mClock);
            mPlugins.insert(index, plugin);
        }else {
            mLayout->takeAt(index)->widget()->deleteLater();
            mClock = nullptr;
            mPlugins.removeAt(index);
        }
    }
    mSettings->setValue("plugins", mPlugins);
}
QRect PanelQt::calculateMenuPosition(QPoint pos, QSize size, int gap, bool isGlobal, float hScale, float vScale, QString anchor){
    QSize scaledSize;
    scaledSize.setWidth(size.width() * hScale);
    scaledSize.setHeight(size.height() * vScale);
    auto screenGeo = QApplication::screens()[mScreen]->geometry();

    screenGeo.setLeft(mGeometry.left());
    screenGeo.setRight(mGeometry.right());
    //qDebug() << "screenGeo" << screenGeo << "mgeo" << mGeometry;
    auto menuGeo = isGlobal ? QRect(pos, scaledSize) : QRect(mapToGlobal(pos), scaledSize);
    //qDebug() << "calculateMenuPosition" << pos << mapToGlobal(pos) << screenGeo << size << scaledSize << menuGeo;

    if (anchor == "Center"){
        menuGeo.moveLeft(menuGeo.left() - scaledSize.width()/2); //center menu
    }else if (anchor == "Right"){
        menuGeo.moveLeft(menuGeo.left() - scaledSize.width()); //center menu
    }

    if(mPosition == "Bottom")
        screenGeo.moveBottom(screenGeo.bottom() - mHeight);
    if(mPosition == "Top")
        screenGeo.moveTop(screenGeo.top() + mHeight);

    //qDebug() << "moveTop" << menuGeo;

    if (menuGeo.top() < screenGeo.top() + gap){
        menuGeo.moveTop(screenGeo.top() + gap);
    }
    if (menuGeo.bottom() > screenGeo.bottom() - gap){
        menuGeo.moveBottom(screenGeo.bottom() - gap);
    }
//  no horizontal gap
//    if (menuGeo.left() < screenGeo.left() + gap){
//        menuGeo.moveLeft(screenGeo.left() + gap);
//    }
//    if (menuGeo.right() > screenGeo.right() - gap){
//        menuGeo.moveRight(screenGeo.right() - gap);
//    }
    if (menuGeo.left() < screenGeo.left()){
        menuGeo.moveLeft(screenGeo.left());
    }
    if (menuGeo.right() > screenGeo.right()){
        menuGeo.moveRight(screenGeo.right());
    }

    qDebug() << "calculateMenuPosition" << screenGeo.left() << screenGeo.right() << menuGeo.left() << menuGeo.right();
    return menuGeo;
}
void PanelQt::calculateGeometry(){
    QRect scrGeo = QApplication::screens()[mScreen]->geometry();

    if (mPosition == "Bottom"){
        mGeometry = QRect(QPoint(scrGeo.left(), scrGeo.bottom() - mHeight), QPoint(scrGeo.right(), scrGeo.bottom()));
    }
    else if(mPosition == "Top") {
        mGeometry = QRect(QPoint(scrGeo.left(), scrGeo.top()), QPoint(scrGeo.right(), scrGeo.top() + mHeight));
    }
    int w = mGeometry.width() * mWidthPercentage / 100;
    int left = mGeometry.left();
    if(mAlignment == "Center"){
        left += (mGeometry.width() - w) / 2;
    }else if (mAlignment == "Right") {
        left += mGeometry.width() - w;
    }
    mGeometry.setWidth(w);
    mGeometry.moveLeft(left);
    qDebug() << "calculateGeometry" << mGeometry << mGeometry.bottom() << scrGeo.bottom();
}

void PanelQt::updateStrut(){
//    auto geo = frameGeometry();
//    WId win = winId();
//    int top = mPosition == "Top" ? mHeight + 2 : 0;
//    int bottom = mPosition == "Bottom" ? mHeight + 2 : 0;
//    KWindowSystem::setStrut(win, 0, 0, top, bottom);

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
        bottom_width = virtGeo.bottom() - mGeometry.bottom() + mHeight + 2;
        bottom_start = mGeometry.left();
        bottom_end = mGeometry.right();
    }
    if(mPosition == "Top"){
        top_width = mGeometry.top() + mHeight + 2;
        top_start = mGeometry.left();
        top_end = mGeometry.right();
    }
    KWindowSystem::setExtendedStrut	(win, left_width, left_start, left_end, right_width, right_start, right_end, top_width, top_start, top_end, 	bottom_width, bottom_start, bottom_end);
}

void PanelQt::showDialog(QPoint pos){
    auto dialog = new QDialog(this, Qt::Popup);
    auto box = new QHBoxLayout(dialog);

    //Panel config
    auto panelGroup = new QGroupBox("Panel Settings:", dialog);
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
    auto alignmentCombo = new QComboBox(dialog);
    auto widthPercentageSpin = new QSpinBox(dialog);
    auto heightSpin = new QSpinBox(dialog);
    auto opacitySlider = new QSlider(Qt::Horizontal, dialog);

//    auto panelShapeCombo = new QComboBox(dialog);
//    auto panelShadowCombo = new QComboBox(dialog);
//    auto panelLineSpin = new QSpinBox(dialog);
//    auto panelMidLineSpin = new QSpinBox(dialog);


    fontCheck->setChecked(mUseSystemFont);
    fontButton->setDisabled(mUseSystemFont);
    fontButton->setMinimumWidth(200);
    screenSpin->setRange(0, QApplication::screens().count() - 1);
    screenSpin->setValue(mScreen);

    positionCombo->addItem("Top");
    positionCombo->addItem("Bottom");
    positionCombo->setCurrentText(mPosition);

    alignmentCombo->addItem("Left");
    alignmentCombo->addItem("Center");
    alignmentCombo->addItem("Right");
    alignmentCombo->setCurrentText(mAlignment);

    widthPercentageSpin->setRange(10, 100);
    widthPercentageSpin->setValue(mWidthPercentage);
    widthPercentageSpin->setSuffix("%");

    heightSpin->setRange(10, 100);
    heightSpin->setValue(mHeight);
    heightSpin->setSuffix("px");

    opacitySlider->setRange(0, 255);
    opacitySlider->setValue(mOpacity);
    opacitySlider->setTickPosition(QSlider::TicksAbove);

//    panelShapeCombo->addItems({"NoFrame", "Box", "Panel", "StyledPanel", "HLine", "VLine", "WinPanel"});
//    panelShapeCombo->setCurrentText(mShape);
//    panelShadowCombo->addItems({"Plain", "Raised", "Sunken"});
//    panelShadowCombo->setCurrentText(mShadow);
//    panelLineSpin->setRange(0,10);
//    panelLineSpin->setValue(mLineWidth);
//    panelMidLineSpin->setRange(0,10);
//    panelMidLineSpin->setValue(mMidLineWidth);

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
    connect(alignmentCombo, &QComboBox::currentTextChanged, this, &PanelQt::setPanelAlignment);
    connect(widthPercentageSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PanelQt::setWidthPercentage);
    connect(heightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PanelQt::setPanelHeight);
    connect(opacitySlider, QOverload<int>::of(&QSlider::valueChanged), this, &PanelQt::setPanelOpacity);
//    connect(panelShapeCombo,    &QComboBox::currentTextChanged,                 this, &PanelQt::changeShape);
//    connect(panelShadowCombo,   &QComboBox::currentTextChanged,                 this, &PanelQt::changeShadow);
//    connect(panelLineSpin,      QOverload<int>::of(&QSpinBox::valueChanged),    this, &PanelQt::changeLineWidth);
//    connect(panelMidLineSpin,   QOverload<int>::of(&QSpinBox::valueChanged),    this, &PanelQt::changeMidLineWidth);

    panelForm->addRow("Use system font:", fontCheck);
    panelForm->addRow("Select font:", fontButton);
    panelForm->addRow("Screen:", screenSpin);
    panelForm->addRow("Position:", positionCombo);
    panelForm->addRow("Alignment:", alignmentCombo);
    panelForm->addRow("Width:", widthPercentageSpin);
    panelForm->addRow("Height:", heightSpin);
    panelForm->addRow("Background opacity:", opacitySlider);
//    panelForm->addRow("Shape:", panelShapeCombo);
//    panelForm->addRow("Shadow:", panelShadowCombo);
//    panelForm->addRow("Line Width:", panelLineSpin);
//    panelForm->addRow("Mid Line Width:", panelMidLineSpin);

    panelGroup->setLayout(panelForm);
    box->addWidget(panelGroup);

    /////PLUGINS

    auto pluginsGroup = new QGroupBox("Plugins", panelGroup);
    auto pluginsForm = new QFormLayout(pluginsGroup);

    for(auto plugin: mAllPlugins){
        if(plugin != "TaskBar"){
            auto pluginCheck = new QCheckBox(pluginsGroup);
            pluginCheck->setChecked(mPlugins.contains(plugin));
            connect(pluginCheck, &QCheckBox::toggled, this, [=](bool check){
                addRemovePlugin(plugin, check);
            });
            pluginsForm->addRow(plugin, pluginCheck);
        }
    }
    panelForm->addRow(pluginsGroup);
    /////TASKBAR


    auto taskBarConfig =  mTaskBar->createTaskBarConfig(dialog);
    box->addWidget(taskBarConfig);

    //////LAYOUT







    //Position
    auto dialogGeo = calculateMenuPosition(pos, dialog->sizeHint(), 4, true);
    qDebug() << "geo" << dialogGeo;
    dialog->setGeometry(dialogGeo);
    dialog->show();
}
PanelQt::~PanelQt()
{
    //delete ui;
}

