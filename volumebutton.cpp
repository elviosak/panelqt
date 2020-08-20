
#include "volumebutton.h"
#include "panelqt.h"
#include "volumeframe.h"

#include "vol/audiodevice.h"
#include "vol/pulseaudioengine.h"

extern QHash<QString, QFrame::Shape> Shapes;
extern QHash<QString, QFrame::Shadow> Shadows;

VolumeButton::VolumeButton(VolumeFrame * frame, PanelQt * panel)
    :QFrame(frame),
    mSettings(new QSettings("PanelQt", panel->mPanelName + "/volumebutton")),
    mPanel(panel),
    mFrame(frame)
{
    setAttribute(Qt::WA_NoMousePropagation);
    setMouseTracking(true);
    mPaletteColor = QPalette::Window;
    mIconSize = mSettings->value("iconSize", 20).toInt();
    mButtonWidth = mSettings->value("buttonWidth", 30).toInt();
    mMixerCommand = mSettings->value("mixerCommand", "pavucontrol-qt").toString();

    mShape = mSettings->value("shape", "Box").toString();
    mShadow = mSettings->value("shadow", "Raised").toString();
    mLineWidth = mSettings->value("lineWidth", 1).toInt();
    mMidLineWidth = mSettings->value("midLineWidth", 0).toInt();
    mSinkPriority = mSettings->value("sinkPriority", {}).toStringList();

    changeFrame();

    auto box = new QHBoxLayout(this);
    mBtnIcon = new QLabel(this);
    box->setMargin(0);
    box->setSpacing(0);
    box->setContentsMargins(2,0,2,0);

    mPaletteColor = QPalette::Window;
    mIconQSize = QSize(mIconSize, mIconSize);
    mBtnIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    box->addWidget(mBtnIcon,1, Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_AlwaysShowToolTips);

    mBar = new QProgressBar(this);
    mBar->setRange(0,100);
    mBar->setFormat(QString());
    mBar->setOrientation(Qt::Vertical);
    mBar->setMaximumWidth(6);
    mBar->setMaximumHeight(mIconSize);
    box->addWidget(mBar, 0, Qt::AlignCenter);

    setMinimumWidth(mButtonWidth);
    setMaximumWidth(mButtonWidth);

    mEngine = new PulseAudioEngine(this);


    connect(mEngine, &PulseAudioEngine::sinkListChanged, this, &VolumeButton::changeSinkList);
    changeSinkList();

    updateIcon();
}


void VolumeButton::changeShape(QString s){
    mShape = s;
    mSettings->setValue("shape", s);
    changeFrame();
}
void VolumeButton::changeShadow(QString s){
    mShadow = s;
    mSettings->setValue("shadow", s);
    changeFrame();
}
void VolumeButton::changeLineWidth(int w){
    mLineWidth = w;
    mSettings->setValue("lineWidth", w);
    changeFrame();
}
void VolumeButton::changeMidLineWidth(int w){
    mMidLineWidth = w;
    mSettings->setValue("midLineWidth", w);
    changeFrame();
}
void VolumeButton::changeFrame(){
    mFrame->setFrameShape(Shapes[mShape]);
    mFrame->setFrameShadow(Shadows[mShadow]);
    mFrame->setLineWidth(mLineWidth);
    mFrame->setMidLineWidth(mMidLineWidth);
};
void VolumeButton::changeMixerCommand(QString cmd){
    mSettings->setValue("mixerCommand", cmd);
}
void VolumeButton::launchMixer(){
    QString Exec = mMixerCommand;
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
    QStringList splitCmd = QProcess::splitCommand(Exec);
    QProcess::startDetached(splitCmd.takeFirst(), splitCmd);

#else
    QProcess::startDetached(Exec);
#endif
}
void VolumeButton::volumeChanged(int v){
    setToolTip(QString("%1 %").arg(v));
    mBar->setValue(v);
    mBar->repaint();
    updateIcon();
//    if(mHover){

//    }
}
QList<AudioDevice*> VolumeButton::sortSinks(QList<AudioDevice *> sinkList){
    QList<AudioDevice *> newList;
    for(auto sinkName : mSinkPriority){
        for(auto sink: sinkList){
            if (sink->name() == sinkName){
                newList.append(sink);
                break;
            }
        }
    }
    for (auto sink : sinkList){
        if(!newList.contains(sink)){
            newList.append(sink);
        }
    }
    return newList;
}
void VolumeButton::changeSinkPriority(){
    mSettings->setValue("sinkPriority", mSinkPriority);
}
void VolumeButton::changeSinkList(){
    //qDeleteAll(mSinkList);
    mSinkList = sortSinks(mEngine->sinks());
    if(mSinkList.count() > 0){
        changeSink(0);
    }else{
        mSink = nullptr;
    }
}
void VolumeButton::changeSink(int i){
    if(mSinkList.count() > i){
        if(nullptr != mSink){
            mSink = nullptr;
        }
        mSink = mSinkList[i];
        connect(mSink, &AudioDevice::volumeChanged, this, &VolumeButton::volumeChanged);
        volumeChanged(mSink->volume());
    }
}
void VolumeButton::paintEvent(QPaintEvent *event){
    if(mPaletteColor == QPalette::Window)
        return;

    QPainter painter(this);
    QColor color(palette().color(mPaletteColor));
    color.setAlpha(mPanel->mOpacity);
    painter.fillRect(event->rect(), color);
}
void VolumeButton::leaveEvent(QEvent *e){
    mHover = false;
    mPaletteColor = QPalette::Window;
    repaint();
    e->accept();
}
void VolumeButton::enterEvent(QEvent *e){
    mPaletteColor = QPalette::Light;
    mHover = true;

    repaint();
    e->accept();
}
void VolumeButton::mouseMoveEvent(QMouseEvent *e){
    Q_UNUSED(e);
}
void VolumeButton::mousePressEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
        handleClick(true);
    }
}
void VolumeButton::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
        handleClick(false);
        e->accept();
    }else if(e->button() == Qt::RightButton){
        e->accept();
        showConfig();
    }
}

void VolumeButton::handleClick(bool press){
    if(press){
        mPaletteColor = QPalette::Base;
    }else {
        if(mHover){
            mPaletteColor = QPalette::Light;
        }else {
            mPaletteColor = QPalette::Window;
        }
        if(nullptr != mSink){
            showDialog();
        }
    }
    repaint();
}
void VolumeButton::showConfig(){
    auto d = new QDialog(this, Qt::Popup);
    auto l = new QVBoxLayout(d);
    auto group = new QGroupBox("Volume Icon Settings:");
    auto form = new QFormLayout(group);
    l->addWidget(group);

    auto priorityCombo = new QComboBox(group);
    auto priorityControls = new QWidget(group);
    auto priorityLayout = new QVBoxLayout(priorityControls);
    auto priorityUp = new QToolButton(priorityControls);
    auto priorityDown = new QToolButton(priorityControls);
    auto priorityRemove = new QToolButton(priorityControls);
    auto priorityList = new QListWidget(group);
    auto mixerCommandEdit = new QLineEdit(group);
    auto frameShapeCombo = new QComboBox(group);
    auto frameShadowCombo = new QComboBox(group);
    auto frameLineSpin = new QSpinBox(group);
    auto frameMidLineSpin = new QSpinBox(group);

    for(auto sink : mSinkList){
        if (!mSinkPriority.contains(sink->name())){
            priorityCombo->addItem(sink->name());
        }
    }
//    priorityTool->setToolButtonStyle(Qt::ToolButtonIconOnly);

//    priorityTool->setOrientation(Qt::Vertical);
    priorityUp->setIcon(QIcon::fromTheme("arrow-up"));
    priorityUp->setToolButtonStyle(Qt::ToolButtonIconOnly);
    priorityUp->setAutoRaise(false);
    priorityDown->setIcon(QIcon::fromTheme("arrow-down"));
    priorityDown->setToolButtonStyle(Qt::ToolButtonIconOnly);
    priorityDown->setAutoRaise(false);
    priorityRemove->setIcon(QIcon::fromTheme("list-remove"));
    priorityRemove->setToolButtonStyle(Qt::ToolButtonIconOnly);
    priorityRemove->setAutoRaise(false);
    priorityLayout->addWidget(priorityUp);
    priorityLayout->addWidget(priorityDown);
    priorityLayout->addWidget(priorityRemove);
    priorityList->setMaximumHeight(100);
    priorityList->addItems(mSinkPriority);
    priorityList->setDragEnabled(false);

    mixerCommandEdit->setMinimumWidth(100);
    mixerCommandEdit->setText(mMixerCommand);
    frameShapeCombo->addItems({"NoFrame", "Box", "Panel", "StyledPanel", "HLine", "VLine", "WinPanel"});
    frameShapeCombo->setCurrentText(mShape);
    frameShadowCombo->addItems({"Plain", "Raised", "Sunken"});
    frameShadowCombo->setCurrentText(mShadow);
    frameLineSpin->setRange(0,10);
    frameLineSpin->setValue(mLineWidth);
    frameMidLineSpin->setRange(0,10);
    frameMidLineSpin->setValue(mMidLineWidth);

    connect(priorityCombo,QOverload<int>::of(&QComboBox::activated), this, [=](int index){
        QString text = priorityCombo->itemText(index);
        priorityList->addItem(text);
        mSinkPriority.append(text);
        priorityCombo->removeItem(index);
        changeSinkPriority();
    });
    connect(priorityUp, &QToolButton::clicked, this, [=]{
        int row =  priorityList->currentRow();
        if(row != -1 && row > 0){
            auto item = priorityList->takeItem(row);
            priorityList->insertItem(row - 1, item);
            auto itemString = mSinkPriority.takeAt(row);
            mSinkPriority.insert(row - 1, itemString);
            priorityList->setCurrentRow(row - 1, QItemSelectionModel::ClearAndSelect);
            changeSinkPriority();
        }
    });
    connect(priorityDown, &QToolButton::clicked, this, [=]{
        int row =  priorityList->currentRow();
        if(row != -1 && row < priorityList->count() -1){
            auto item = priorityList->takeItem(row);
            priorityList->insertItem(row + 1, item);
            auto itemString = mSinkPriority.takeAt(row);
            mSinkPriority.insert(row + 1, itemString);
            priorityList->setCurrentRow(row + 1, QItemSelectionModel::ClearAndSelect);
            changeSinkPriority();
        }
    });
    connect(priorityRemove, &QToolButton::clicked, this, [=]{
        int row =  priorityList->currentRow();
        if(row != -1){
            auto item = priorityList->takeItem(row);
            delete item;
            auto itemString = mSinkPriority.takeAt(row);
            priorityCombo->addItem(itemString);
            changeSinkPriority();
        }
    });

    connect(mixerCommandEdit,   &QLineEdit::textChanged,                        this, &VolumeButton::changeMixerCommand);
    connect(frameShapeCombo,    &QComboBox::currentTextChanged,                 this, &VolumeButton::changeShape);
    connect(frameShadowCombo,   &QComboBox::currentTextChanged,                 this, &VolumeButton::changeShadow);
    connect(frameLineSpin,      QOverload<int>::of(&QSpinBox::valueChanged),    this, &VolumeButton::changeLineWidth);
    connect(frameMidLineSpin,   QOverload<int>::of(&QSpinBox::valueChanged),    this, &VolumeButton::changeMidLineWidth);

    form->addRow("Set Priority:", priorityCombo);
    form->addRow(priorityControls, priorityList);
    form->addRow("Mixer:", mixerCommandEdit);
    form->addRow("Shape:", frameShapeCombo);
    form->addRow("Shadow:", frameShadowCombo);
    form->addRow("Line Width:", frameLineSpin);
    form->addRow("Mid Line Width:", frameMidLineSpin);

    auto center = mapFromParent(geometry().center());
    auto geo = mPanel->calculateMenuPosition(mapToGlobal(center), d->sizeHint(), 4, true);
    d->setGeometry(geo);
    d->show();
}

void VolumeButton::wheelEvent(QWheelEvent *e){
    if(nullptr == mSink)
        return;

    if(e->angleDelta().y() > 0){
        mSink->setVolume(mSink->volume() + 5);
        mSink->setMute(false);
    }
    else{
        mSink->setVolume(mSink->volume() - 5);
        mSink->setMute(false);
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    QToolTip::showText(e->globalPosistion().toPoint(), toolTip());
#else
    QToolTip::showText(e->globalPos(), toolTip());
#endif
}
void VolumeButton::showDialog(){
    auto d = new QDialog(this, Qt::Popup);
    auto box = new QGridLayout(d);
    //d->setLayout(box);

    auto sinkCombo = new QComboBox(d);
    auto volumeSlider = new QSlider(Qt::Horizontal, d);
    auto muteButton = new QPushButton(QIcon::fromTheme(mIconName),"Mute", d);
    auto mixerButton = new QPushButton("Mixer");

    for(int i = 0; i < mSinkList.count(); ++i){
        auto sink = mSinkList[i];
        sinkCombo->addItem(sink->name(), i);
        if(sink == mSink){
            sinkCombo->setCurrentIndex(i);
        }
    }

    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(mSink->volume());
    muteButton->setCheckable(true);
    muteButton->setChecked(mSink->mute());
    mixerButton->setToolTip(mMixerCommand);

    connect(sinkCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VolumeButton::changeSink);
    connect(volumeSlider, &QSlider::valueChanged, this, &VolumeButton::changeVolume);
    connect(mSink, &AudioDevice::volumeChanged, d, [=] (int v) {
        volumeSlider->blockSignals(true);
        volumeSlider->setValue(v);
        volumeSlider->blockSignals(false);
        muteButton->setIcon(QIcon::fromTheme(mIconName));
    });
    connect(muteButton, &QPushButton::toggled, this, &VolumeButton::changeMute);
    connect(mixerButton, &QPushButton::clicked, this, &VolumeButton::launchMixer);

    box->addWidget(sinkCombo, 0, 0, 1, 2);
    box->addWidget(volumeSlider, 1, 0, 1, 2);
    box->addWidget(muteButton, 2, 0, 1, 1);
    box->addWidget(mixerButton, 2, 1, 1, 1);

    auto center = mapFromParent(geometry().center());
    auto geo = mPanel->calculateMenuPosition(mapToGlobal(center), d->sizeHint(), 4, true, 1, 1, "Right");
    d->setGeometry(geo);
    d->show();
}
void VolumeButton::changeMute(bool m){
    if(nullptr != mSink){
        mSink->setMute(m);
        updateIcon();
    }
}
void VolumeButton::changeVolume(int v){
    if(nullptr != mSink){
        mSink->setVolume(v);
    }
}
void VolumeButton::updateIcon() {
    QString iconName;
    if(nullptr != mSink){
        if(mSink->volume() <= 0 || mSink->mute()) {
            iconName = "audio-volume-muted";
        } else if (mSink->volume() <= 33) {
            iconName = "audio-volume-low";
        }else if (mSink->volume() <= 66) {
            iconName = "audio-volume-medium";
        }
        else {
            iconName = "audio-volume-high";
        }
    }else {
        iconName = "audio-volume-muted";
    }
    if(iconName != mIconName){
        mIconName = iconName;
        mBtnIcon->setPixmap(QIcon::fromTheme(mIconName).pixmap(mIconQSize));
        mBtnIcon->repaint();
    }

}
VolumeButton::~VolumeButton(){

}
