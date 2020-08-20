#include "snframe.h"
#include "panelqt.h"
#include "snbutton.h"
#include "sn/statusnotifierwatcher.h"

extern QHash<QString, QFrame::Shape> Shapes;
extern QHash<QString, QFrame::Shadow> Shadows;
SNFrame::~SNFrame(){
    delete mWatcher;
}
SNFrame::SNFrame(PanelQt * panel)
    : QFrame(panel),
      mPanelName(panel->mPanelName),
      mSettings(new QSettings("PanelQt", mPanelName + "/statusnotifier")),
      mPanel(panel),
      mLayout(new QHBoxLayout(this))
{
    setAttribute(Qt::WA_NoMousePropagation);
    mIconSize = mSettings->value("iconSize", 24).toInt();
    mButtonWidth = mSettings->value("buttonWidth", 30).toInt();

    mShape = mSettings->value("shape", "Box").toString();
    mShadow = mSettings->value("shadow", "Raised").toString();
    mLineWidth = mSettings->value("lineWidth", 1).toInt();
    mMidLineWidth = mSettings->value("midLineWidth", 1).toInt();

    //setLayout(mLayout);
    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    //qsetSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    changeFrame();

    // Watcher stuff
    QString dbusName = QString("org.kde.StatusNotifierHost-%1-%2").arg(QCoreApplication::applicationPid()).arg(mPanelName);

    if (!QDBusConnection::sessionBus().registerService(dbusName))
        qDebug() << QDBusConnection::sessionBus().lastError().message();

    mWatcher = new StatusNotifierWatcher;
    mWatcher->RegisterStatusNotifierHost(dbusName);

    connect(mWatcher, &StatusNotifierWatcher::StatusNotifierItemRegistered,
            this, &SNFrame::onItemAdded);
    connect(mWatcher, &StatusNotifierWatcher::StatusNotifierItemUnregistered,
            this, &SNFrame::onItemRemoved);
    qDebug() << "registered" << mWatcher->RegisteredStatusNotifierItems();
}
void SNFrame::changeIconSize(int s){
    mIconSize = s;
    mSettings->setValue("iconSize", s);
    emit SNIconSizeChanged(s);
}
void SNFrame::changeButtonWidth(int w){
    mButtonWidth = w;
    mSettings->setValue("buttonWidth", w);
    emit SNButtonWidthChanged(w);
}
void SNFrame::mouseReleaseEvent(QMouseEvent *event){
    Q_UNUSED(event);
    if(event->button() == Qt::RightButton){
        showDialog();
        event->accept();
        return;
    }
    event->ignore();

}
void SNFrame::showDialog(){
    auto dialog = new QDialog(this, Qt::Popup);
    dialog->setWindowTitle("Status Notifier Settings");

    auto form = new QFormLayout(dialog);

    auto iconSizeSpin = new QSpinBox(dialog);
    auto buttonWidthSpin = new QSpinBox(dialog);

    iconSizeSpin->setRange(12, 128);
    iconSizeSpin->setValue(mIconSize);
    buttonWidthSpin->setRange(12, 128);
    buttonWidthSpin->setValue(mButtonWidth);

    connect(iconSizeSpin,       QOverload<int>::of(&QSpinBox::valueChanged), this, &SNFrame::changeIconSize);
    connect(buttonWidthSpin,    QOverload<int>::of(&QSpinBox::valueChanged), this, &SNFrame::changeButtonWidth);

    form->addRow("Icon size:", iconSizeSpin);
    form->addRow("Button width:", buttonWidthSpin);

    auto frameGroup = new QGroupBox("Frame Config", dialog);
    auto frameForm =new QFormLayout(frameGroup);


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

    connect(frameShapeCombo,    &QComboBox::currentTextChanged,                 this, &SNFrame::changeShape);
    connect(frameShadowCombo,   &QComboBox::currentTextChanged,                 this, &SNFrame::changeShadow);
    connect(frameLineSpin,      QOverload<int>::of(&QSpinBox::valueChanged),    this, &SNFrame::changeLineWidth);
    connect(frameMidLineSpin,   QOverload<int>::of(&QSpinBox::valueChanged),    this, &SNFrame::changeMidLineWidth);

    frameForm->addRow("Shape:", frameShapeCombo);
    frameForm->addRow("Shadow:", frameShadowCombo);
    frameForm->addRow("Line Width:", frameLineSpin);
    frameForm->addRow("Mid Line Width:", frameMidLineSpin);

    form->addRow(frameGroup);

    auto center = geometry().center();
    qDebug() << center;
    auto dialogGeo = mPanel->calculateMenuPosition(center, dialog->sizeHint(), 4, false);
    qDebug() << "menuGeo" << dialogGeo;
    dialog->setGeometry(dialogGeo);
    dialog->show();
}

void SNFrame::changeShape(QString s){
    mShape = s;
    mSettings->setValue("shape", s);
    changeFrame();
}
void SNFrame::changeShadow(QString s){
    mShadow = s;
    mSettings->setValue("shadow", s);
    changeFrame();
}
void SNFrame::changeLineWidth(int w){
    mLineWidth = w;
    mSettings->setValue("lineWidth", w);
    changeFrame();
}
void SNFrame::changeMidLineWidth(int w){
    mMidLineWidth = w;
    mSettings->setValue("midLineWidth", w);
    changeFrame();
}
void SNFrame::changeFrame(){
    setFrameShape(Shapes[mShape]);
    setFrameShadow(Shadows[mShadow]);
    setLineWidth(mLineWidth);
    setMidLineWidth(mMidLineWidth);
};
void SNFrame::onItemAdded(const QString &serviceAndPath)
{
    int slash = serviceAndPath.indexOf(QLatin1Char('/'));
    QString serv = serviceAndPath.left(slash);
    QString path = serviceAndPath.mid(slash);

    qDebug() << "itemAdded" << serviceAndPath << serv << path;
    SNButton *btn = new SNButton(serv, path, this);
    mLayout->addWidget(btn);
    mButtonList[serviceAndPath] = btn;
}

void SNFrame::onItemRemoved(const QString &serviceAndPath)
{
    qDebug() << serviceAndPath;

    if (!mButtonList.contains(serviceAndPath)) {
        return;
    }
    SNButton * btn = mButtonList.take(serviceAndPath);
    btn->deleteLater();
}
