#include "taskbar.h"
#include "panelqt.h"
#include "taskbutton.h"
#include "pinbutton.h"
#include "taskgroup.h"
#include "toolbuttontextstyle.hpp"

extern QHash<QString, QFrame::Shape> Shapes;
extern QHash<QString, QFrame::Shadow> Shadows;

TaskBar::~TaskBar(){}

TaskBar::TaskBar(PanelQt * panel):
    QFrame(panel),
    mPanelName(panel->mPanelName),
    mSettings(new QSettings("PanelQt", mPanelName + "/taskbar")),
    mGroupSettings(new QSettings("PanelQt", mPanelName + "/group")),
    conn(QX11Info::connection()),
    mLayout(new QHBoxLayout(this)),
    mPanel(panel),
    mActionGroup(new QActionGroup(this))
{


    //Settings
    mPinnedList = mSettings->value("pinnedList", {}).toStringList();
    mMaxBtnWidth = mSettings->value("maxBtnWidth", 200).toInt();
    mShowAllScreens = mSettings->value("showAllScreens", true).toBool();
    mIconHeight = mSettings->value("iconHeight", mPanel->mHeight-4).toInt();
    mPinIconHeight = mSettings->value("pinIconHeight", mPanel->mHeight-4).toInt();
    mPinBtnWidth = mSettings->value("pinBtnWidth", mPanel->mHeight).toInt();
    mButtonAutoRaise = mSettings->value("buttonAutoRaise", true).toBool();
    mPinAutoRaise = mSettings->value("pinAutoRaise", false).toBool();
    mButtonUnderline = mSettings->value("buttonUnderline", true).toBool();

    mShape = mSettings->value("shape", "StyledPanel").toString();
    mShadow = mSettings->value("shadow", "Raised").toString();
    mLineWidth = mSettings->value("lineWidth", 1).toInt();
    mMidLineWidth = mSettings->value("midLineWidth", 1).toInt();

    mGroupShape = mGroupSettings->value("shape", "Box").toString();
    mGroupShadow = mGroupSettings->value("shadow", "Raised").toString();
    mGroupLineWidth = mGroupSettings->value("lineWidth", 1).toInt();
    mGroupMidLineWidth = mGroupSettings->value("midLineWidth", 0).toInt();

    mElideLabel = new ElideLabel(Qt::ElideRight, Qt::AlignLeft | Qt::AlignVCenter, QMargins(4,0,4,0));
    mElideLabelUnderline = new ElideLabel(Qt::ElideRight, Qt::AlignLeft | Qt::AlignVCenter, QMargins(4,0,4,0));
    mElideLabelUnderline->mUnderline = mButtonUnderline;
    //mTBTextStyle->mUnderline = mButtonUnderline;
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    setWindowFlag(Qt::FramelessWindowHint);
    this->setContentsMargins(0,0,0,0);

    mLayout->setMargin(0);
    mLayout->setSpacing(2);
    mLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);
    setLayout(mLayout);

    changeFrame();
    setFrameShape(Shapes[mShape]);
    setFrameShadow(Shadows[mShadow]);
    setMidLineWidth(mMidLineWidth);

    loadPinned();

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &TaskBar::windowAdded);
    connect(KWindowSystem::self(), QOverload<WId, NET::Properties, NET::Properties2>::of(&KWindowSystem::windowChanged), this, &TaskBar::windowChanged);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &TaskBar::windowRemoved);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &TaskBar::activeWindowChanged);
    repaint();


    KWindowInfo info(104857605, NET::WMAllProperties, NET::WM2AllProperties);

    qDebug() << "telegram" << info.windowClassName() <<info.state();

}
void TaskBar::groupChangeShape(QString s){
    mGroupShape = s;
    mGroupSettings->setValue("shape", s);
    emit groupShapeChanged(s);
}
void TaskBar::groupChangeShadow(QString s){
    mGroupShadow = s;
    mGroupSettings->setValue("shadow", s);
    emit groupShadowChanged(s);
}
void TaskBar::groupChangeLineWidth(int w){
    mGroupLineWidth = w;
    mGroupSettings->setValue("lineWidth", w);
    emit groupLineWidthChanged(w);
}
void TaskBar::groupChangeMidLineWidth(int w){
    mGroupMidLineWidth = w;
    mGroupSettings->setValue("midLineWidth", w);
    emit groupMidLineWidthChanged(w);
}

void TaskBar::changeShape(QString s){
    mShape = s;
    mSettings->setValue("shape", s);
    changeFrame();
}
void TaskBar::changeShadow(QString s){
    mShadow = s;
    mSettings->setValue("shadow", s);
    changeFrame();
}
void TaskBar::changeLineWidth(int w){
    mLineWidth = w;
    mSettings->setValue("lineWidth", w);
    changeFrame();
}
void TaskBar::changeMidLineWidth(int w){
    mMidLineWidth = w;
    mSettings->setValue("midLineWidth", w);
    changeFrame();
}
void TaskBar::changeFrame(){
    setFrameShape(Shapes[mShape]);
    setFrameShadow(Shadows[mShadow]);
    setLineWidth(mLineWidth);
    setMidLineWidth(mMidLineWidth);
};
void TaskBar::setButtonUnderline(bool u){
    mButtonUnderline = u;
    mSettings->setValue("buttonUnderline", u);
    mElideLabelUnderline->mUnderline = u;
    repaint();
}
void TaskBar::setButtonAutoRaise(bool autoRaise){
    mButtonAutoRaise = autoRaise;
    mSettings->setValue("buttonAutoRaise", autoRaise);
    emit buttonAutoRaiseChanged(autoRaise);
}
void TaskBar::setPinAutoRaise(bool autoRaise){
    mPinAutoRaise = autoRaise;
    mSettings->setValue("pinAutoRaise", autoRaise);
    emit pinAutoRaiseChanged(autoRaise);
}

void TaskBar::setMaxBtnWidth(int w){
    mMaxBtnWidth = w;
    mSettings->setValue("maxBtnWidth", w);
    emit buttonWidthChanged(w);
    recalculateButtonsDelay();
}
void TaskBar::resetTaskBar(){
    mIdList.clear();
    mClassList.clear();
    mActionGroup->deleteLater();
    mActionGroup = new QActionGroup(this);
    while (mLayout->count()>0) {
        mLayout->takeAt(0)->widget()->deleteLater();
    }
    loadPinned();

    auto list = KWindowSystem::windows();
    for (auto i = list.begin(); i != list.end(); ++i) {
        windowAdded(*i);
    }
}
void TaskBar::setShowAllScreens(bool show){
    mShowAllScreens = show;
    mSettings->setValue("showAllScreens", show);
    resetTaskBar();
}
void TaskBar::setPinBtnWidth(int w){
    mPinBtnWidth = w;
    mSettings->setValue("pinBtnWidth", w);
    emit pinBtnWidthChanged(w);
    recalculateButtonsDelay();
}
void TaskBar::setPinIconHeight(int h){
    mPinIconHeight = h;
    mSettings->setValue("pinIconHeight", h);
    emit pinIconHeightChanged(h);
}
void TaskBar::setIconHeight(int h){
    mIconHeight = h;
    mSettings->setValue("iconHeight", h);
    emit iconHeightChanged(h);
}
void TaskBar::loadPinned(){
    qDebug() << "loadPinned" << mPinnedList;
    if(mPinnedList.count() > 0){
        for (auto i = mPinnedList.begin(); i != mPinnedList.end(); ++i) {
            QString className = (*i);
            mSettings->beginGroup("pinned-" + className);
            QIcon icon = mSettings->value("icon").value<QIcon>();
            QString cmd = mSettings->value("cmd").toString();
            addGroup(className, icon, true, cmd);
            mSettings->endGroup();
        }
    }
}
void TaskBar::savePinned(QString className, QIcon icon, QString cmd){
    mSettings->beginGroup("pinned-" + className);
    mSettings->setValue("icon", icon);
    mSettings->setValue("cmd", cmd);
    mSettings->endGroup();
    savePinList();
}
void TaskBar::removePin(QString className){
    mSettings->remove("pinned-" + className);
    savePinList();
}
void TaskBar::savePinList(){
    //qDebug() << "savepinlist";
    QStringList newList;
    for (int i = 0; i < mLayout->count(); ++i) {
        TaskGroup * tg = qobject_cast<TaskGroup*>(mLayout->itemAt(i)->widget());
        if(tg->mPinned)
            newList.append(tg->mClassName);
    }
    if(newList != mPinnedList){
        //qDebug() << "old list" << mPinnedList << "new" << newList;
        mPinnedList = newList;
        mSettings->setValue("pinnedList", mPinnedList);
    }
}
void TaskBar::closeRequested(WId id){
    qDebug() << "close" << id;

    NETRootInfo info(conn, NET::Properties(), NET::Properties2());
    info.closeWindowRequest(id);
}
void TaskBar::addGroup(QString className, QIcon icon, bool pinned, QString cmd){
    mClassList[className] = new TaskGroup(className, icon, this, mPanel, pinned, cmd);
    mLayout->addWidget(mClassList[className]);
    connect(mClassList[className], &TaskGroup::buttonAdded, this, &TaskBar::recalculateButtonsDelay);
    connect(mClassList[className], &TaskGroup::buttonRemoved, this, &TaskBar::buttonRemoved);
    connect(mClassList[className], &TaskGroup::removeGroup, this, &TaskBar::removeGroup);
    connect(mClassList[className], &TaskGroup::dragEntered, this, &TaskBar::swapGroups);
}

void TaskBar::addWindow(WId id){
    KWindowInfo info(id, NET::WMName | NET::WMState | NET::WMPid, NET::WM2WindowClass | NET::WM2DesktopFileName | NET::WM2AppMenuServiceName | NET::WM2AppMenuObjectPath);
    QIcon icon = KWindowSystem::icon(id);
    QString title = info.name();
    QString className = info.windowClassName();
    //TaskGroup * group;
    if(!mClassList.contains(className)){
        addGroup(className, icon);
    }
    //group = mClassList[className];
    mClassList[className]->addWindow(id, title, icon);
    mIdList[id] = mClassList[className];
}
void TaskBar::swapGroups(QObject *enteredObj, QObject *draggedObj){
    auto dst = qobject_cast<TaskGroup*>(enteredObj);
    auto src = qobject_cast<TaskGroup*>(draggedObj);
    if(dst && src){
        int srcindex = mLayout->indexOf(src);
        int dstindex = mLayout->indexOf(dst);
        qDebug() << "swap src:" << src->mClassName << "dst" << dst->mClassName;
        auto item = mLayout->takeAt(srcindex);
        mLayout->insertItem(dstindex, item);
        QTimer::singleShot(0, this, &TaskBar::savePinList);
    }
}
void TaskBar::buttonRemoved(WId id){
    if(mIdList.contains(id)){
        mIdList.remove(id);
        recalculateButtonsDelay();
    }
}
void TaskBar::removeGroup(QString className){
    if(mClassList.contains(className)){
        auto group = mClassList[className];
        group->deleteLater();
        mClassList.remove(className);
        recalculateButtonsDelay();
    }
}
void TaskBar::recalculateButtonsDelay(){
    QTimer::singleShot(0, this, &TaskBar::recalculateButtons);
}
void TaskBar::recalculateButtons(){
    //qDebug() << "hint" << sizeHint().width() << "size" << size().width();
    int pinnedCount = 0;
    int windowCount = 0;
    for (auto i = mClassList.begin(); i != mClassList.end(); ++i) {
        int currentWindows = i.value()->mWinList.count();
        windowCount += currentWindows;
        if(currentWindows == 0 && i.value()->mPinned)
            pinnedCount++;
    }
    int btnWidth = windowCount > 0 ? (size().width() - mLayout->count() * mLayout->spacing() - pinnedCount * mPinBtnWidth)/windowCount : mMaxBtnWidth;
    btnWidth = btnWidth > mMaxBtnWidth ? mMaxBtnWidth : btnWidth;
    //qDebug() << "recalculateButtons" << btnWidth;

    emit buttonWidthChanged(btnWidth);
}
bool TaskBar::acceptWindow(WId id, bool changed){
    if(mIdList.contains(id) && !changed)
        return false;

    KWindowInfo info(id, NET::WMWindowType | NET::WMState |NET::WMGeometry, NET::WM2TransientFor);
    if (!info.valid())
        return false;
    auto screenGeo = QApplication::screens()[mPanel->mScreen]->geometry();

    if (!mShowAllScreens && !info.geometry().intersects(screenGeo)){
        return false;
    }

    QFlags<NET::WindowTypeMask> ignoreList;
    ignoreList |= NET::DesktopMask;
    ignoreList |= NET::DockMask;
    ignoreList |= NET::SplashMask;
    ignoreList |= NET::ToolbarMask;
    ignoreList |= NET::MenuMask;
    ignoreList |= NET::PopupMenuMask;
    ignoreList |= NET::NotificationMask;



    if (NET::typeMatchesMask(info.windowType(NET::AllTypesMask), ignoreList))
        return false;

    if (info.state() & NET::SkipTaskbar)
        return false;

    // WM_TRANSIENT_FOR hint not set - normal window
    WId transFor = info.transientFor();
    if (transFor == 0 || transFor == id || transFor == (WId) QX11Info::appRootWindow())
        return true;

    info = KWindowInfo(transFor, NET::WMWindowType);

    QFlags<NET::WindowTypeMask> normalFlag;
    normalFlag |= NET::NormalMask;
    normalFlag |= NET::DialogMask;
    normalFlag |= NET::UtilityMask;

    return !NET::typeMatchesMask(info.windowType(NET::AllTypesMask), normalFlag);
//    if(mIdList.contains(id) && !changed)
//        return false;

//    KWindowInfo info(id, NET::WMState | NET::WMWindowType | NET::WMGeometry, NET::WM2WindowClass | NET::WM2TransientFor | NET::WM2DesktopFileName);
//    if (!mShowAllScreens){
//        auto screenGeo = QApplication::screens()[mPanel->mScreen]->geometry();
//        if(!info.geometry().intersects(screenGeo))
//            return false;
//    }

//    NET::States rejectedStates = NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher;
//    //NET::WindowTypes acceptedTypes = NET::NormalMask | NET::DialogMask; //| NET::Unknown;
//    if(info.hasState(rejectedStates)){
//        //qDebug() << info.windowClassName() << "rejectedStates";
//        return false;
//    }
//    NET::WindowType type = info.windowType(NET::AllTypesMask);
//    if(type > 0 || type == 5){ // -1 is unknown
//        //qDebug() << info.windowClassName() << "acceptedTypes" << type;
//        return false;
//    }
//    return true;
}
void TaskBar::activeWindowChanged(WId id){
    Q_UNUSED(id);
//    auto btn = mWinMap[id];
//    if(btn)
//        btn->setChecked(true);
}

void TaskBar::windowAdded(WId id){
    if(acceptWindow(id))
        addWindow(id);
}
void TaskBar::windowRemoved(WId id){
    qDebug() << "windowRemoved" << id;
    if(mIdList.contains(id)){
        qDebug() << "windowRemoved contains" << id;
        TaskGroup * group = mIdList[id];
        group->removeWindow(id);
        mIdList.remove(id);
    }
}
void TaskBar::windowChanged(WId id, NET::Properties properties, NET::Properties2 properties2){
    Q_UNUSED(properties2);
    KWindowInfo info(id, NET::WMAllProperties, NET::WM2WindowClass);
    if(mIdList.contains(id)){
        //qDebug() << "***** check" << info.windowClassName();
        auto group = mIdList[id];
        if (properties & NET::WMName){
            //qDebug() << "name" << info.windowClassName();
            group->setTitle(id, info.name());
        }
        if(properties & NET::WMState || properties & NET::WMWindowType){
            //qDebug() << "state|type" << info.windowClassName();
            if(!acceptWindow(id, true)){
                //qDebug() << "removed" << info.windowClassName();
                windowRemoved(id);
                return;
            }
            if(info.state() & NET::Focused)
                group->setActive(id);
        }
        if(properties & NET::WMIcon){
            group->changeIcon(id, KWindowSystem::icon(id));
        }
    }
    else if (acceptWindow(id)) {
        addWindow(id);
    }
}
