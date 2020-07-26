#include "pinbutton.h"
#include "taskgroup.h"
#include "taskbar.h"
#include "panelqt.h"

//pinned button icon
PinButton::~PinButton(){}
PinButton::PinButton(QIcon icon, QString className, TaskGroup * group, TaskBar * taskbar, PanelQt * panel):
    QToolButton(group),
    mIcon(icon),
    mClassName(className),
    mGroup(group),
    mTaskBar(taskbar),
    mPanel(panel),
    mMenu(new QMenu(this)),
    mAction(new QAction(this))

{
    setAutoRaise(mTaskBar->mPinAutoRaise);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::NoFocus);
    setDefaultAction(mAction);

    mAction->setIcon(mIcon);
    updateIconSize(mTaskBar->mPinIconHeight);
    updatePinWidth(mTaskBar->mPinBtnWidth);

    connect(mTaskBar, &TaskBar::pinAutoRaiseChanged, this, &PinButton::updateAutoRaise);
    connect(mTaskBar, &TaskBar::pinIconHeightChanged, this, &PinButton::updateIconSize);
    connect(mTaskBar, &TaskBar::pinBtnWidthChanged, this, &PinButton::updatePinWidth);
    connect(mAction, &QAction::triggered, this, &PinButton::actionClicked);
}
void PinButton::updateAutoRaise(bool autoRaise){
    setAutoRaise(autoRaise);
}
void PinButton::updatePinWidth(int w){
    setMinimumWidth(w);
    setMaximumWidth(w);
}
void PinButton::updateIconSize(int h){
    QSize size = QSize(h, h);
    //mAction->setIcon(mIcon.pixmap(size).scaled(size));
    setIconSize(size);
    updateGeometry();
}

void PinButton::actionClicked(){
    mGroup->startPin();
}

void PinButton::contextMenuEvent(QContextMenuEvent *event){
    Q_UNUSED(event);
    mMenu->clear();
    //_menu.set
    QAction * a;
    for (int i = 0; i < 10; ++i) {
        a = mMenu->addAction(QString("Action %1 - Action Name").arg(i));
        a->setShortcutVisibleInContextMenu(false);
    }


    //set menu position
    auto center = mapFromParent(geometry().center());
    auto menuGeo = mPanel->calculateMenuPosition(mapToGlobal(center), mMenu->sizeHint(), 4, false);
//    qDebug() << "geo" << center;
    mMenu->setGeometry(menuGeo);
    mMenu->show();
}

