#include "taskbar.h"
#include "panelqt.h"

TaskBar::~TaskBar(){}

TaskBar::TaskBar(PanelQt * panel):
    QFrame(panel),
    conn(QX11Info::connection()),
    mLayout(new QHBoxLayout(this)),
    mPanel(panel),
    mShowAllScreens(false),
    mActionGroup(new QActionGroup(this)),
    mMaxBtnWidth(200)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    setFrameShape(Shape::StyledPanel);
    setFrameShadow(Shadow::Raised);

    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    mLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);
    setLayout(mLayout);

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &TaskBar::windowAdded);
    connect(KWindowSystem::self(), QOverload<WId, NET::Properties, NET::Properties2>::of(&KWindowSystem::windowChanged), this, &TaskBar::windowChanged);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &TaskBar::windowRemoved);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &TaskBar::activeWindowChanged);
}

void TaskBar::closeRequested(WId id){
    qDebug() << "close" << id;

    NETRootInfo info(conn, NET::Properties(), NET::Properties2());
    info.closeWindowRequest(id);

//    Display * disp = QX11Info::display();

//    XEvent event;
//    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

//    event.xclient.type = ClientMessage;
//    event.xclient.serial = 0;
//    event.xclient.send_event = True;
//    event.xclient.message_type = XInternAtom(disp, "_NET_CLOSE_WINDOW", False);
//    event.xclient.window = id;
//    event.xclient.format = 32;
//    event.xclient.data.l[0] = 0;
//    event.xclient.data.l[1] = 0;
//    event.xclient.data.l[2] = 0;
//    event.xclient.data.l[3] = 0;
//    event.xclient.data.l[4] = 0;
//    XSendEvent(disp, DefaultRootWindow(disp), False, mask, &event);

//    if(event.xerror.error_code)
//        qDebug() << "error code" << event.xerror.error_code;

}
void TaskBar::addPin(QString className){

}

void TaskBar::addWindow(WId id){
    KWindowInfo info(id, NET::WMName | NET::WMState | NET::WMPid, NET::WM2WindowClass | NET::WM2DesktopFileName | NET::WM2AppMenuServiceName | NET::WM2AppMenuObjectPath);
    QIcon icon = KWindowSystem::icon(id);
    QString title = info.name();
    QString className = info.windowClassClass();
    auto btn = new TaskButton(id, icon, title, className, mActionGroup, this, mPanel);
    mWinList[id] = btn;
    qDebug() << "addwindow" << className << info.windowClassName() << info.applicationMenuServiceName() << info.applicationMenuObjectPath() << info.desktopFileName();

    int newIndex = mLayout->count();
    for (int i = newIndex -1; i >= 0; --i) {
        auto currentClass = mLayout->itemAt(i)->widget()->accessibleName();
        if(className == currentClass){
            newIndex = i + 1;
            break;
        }
    }
    newIndex = newIndex > 0 ? newIndex : 0;
    mLayout->insertWidget(newIndex, btn);
    //mLayout->addWidget(btn, 0, index);
    qDebug() << "inserted" << className << "index:" << newIndex;
    connect(btn, &TaskButton::closeRequested, this, &TaskBar::closeRequested);
    connect(this, &TaskBar::buttonWidthChanged, btn, &TaskButton::buttonWidthChanged);
    QTimer::singleShot(0, this, &TaskBar::recalculateButtons);
}
void TaskBar::recalculateButtons(){
    //qDebug() << "hint" << sizeHint().width() << "size" << size().width();

    int count = mLayout->count(); // - 1;
    if (count > 0){
        int btnWidth = size().width() / count;
        //qDebug() << "btnWidth" << btnWidth;
        if (btnWidth > mMaxBtnWidth)
            btnWidth = mMaxBtnWidth;
        emit buttonWidthChanged(btnWidth);
    }

}
bool TaskBar::acceptWindow(WId id, bool changed){
    if(mWinList.contains(id) && !changed)
        return false;


    KWindowInfo info(id, NET::WMState | NET::WMWindowType | NET::WMGeometry, NET::WM2WindowClass | NET::WM2TransientFor | NET::WM2DesktopFileName);
    if (!mShowAllScreens){
        auto screenGeo = QApplication::screens()[mPanel->mScreen]->geometry();
        if(!info.geometry().intersects(screenGeo))
            return false;
    }

    NET::States rejectedStates = NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher;
    //NET::WindowTypes acceptedTypes = NET::NormalMask | NET::DialogMask; //| NET::Unknown;
    if(info.hasState(rejectedStates)){
        //qDebug() << info.windowClassClass() << "rejectedStates";
        return false;
    }
    NET::WindowType type = info.windowType(NET::AllTypesMask);
    if(type > 0 || type == 5){ // -1 is unknown
        //qDebug() << info.windowClassClass() << "acceptedTypes" << type;
        return false;
    }

    WId tfor = info.transientFor();
    if (tfor == 0 || tfor == id || tfor == QX11Info::appRootWindow()){
        //qDebug() << "********" <<info.windowClassClass() << "desktop" << info.desktopFileName();
        return true;
    }
    //qDebug() << info.windowClassClass() << "transientFor";
    return false;
}
void TaskBar::activeWindowChanged(WId id){
//    auto btn = mWinMap[id];
//    if(btn)
//        btn->setChecked(true);
}

void TaskBar::windowAdded(WId id){
    if(acceptWindow(id))
        addWindow(id);
}
void TaskBar::windowRemoved(WId id){
    if(mWinList.contains(id)){
        TaskButton * btn = mWinList[id];
        mLayout->removeWidget(btn);
        btn->deleteLater();
        mWinList.remove(id);
        recalculateButtons();
    }
}
void TaskBar::windowChanged(WId id, NET::Properties properties, NET::Properties2 properties2){
    Q_UNUSED(properties2);
    KWindowInfo info(id, NET::WMAllProperties, NET::WM2WindowClass);
    if(mWinList.contains(id)){
        //qDebug() << "***** check" << info.windowClassClass();
        auto btn = mWinList[id];
        if (properties & NET::WMName){
            //qDebug() << "name" << info.windowClassClass();
            btn->setTitle(info.name());
        }
        if(properties & NET::WMState || properties & NET::WMWindowType){
            //qDebug() << "state|type" << info.windowClassClass();
            if(!acceptWindow(id, true)){
                //qDebug() << "removed" << info.windowClassClass();
                windowRemoved(id);
                return;
            }
            btn->setActionCheck(info.state() & NET::Focused);
        }
    }
    else if (acceptWindow(id)) {
        addWindow(id);
    }
}
