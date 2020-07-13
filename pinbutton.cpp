#include "pinbutton.h"
#include "taskbar.h"
#include "panelqt.h"


//pinned button icon
PinButton::~PinButton(){}
PinButton::PinButton(QIcon icon, QString className, PanelQt * panel):
    QToolButton(panel),
    mClass(className),
    mIcon(icon),
    mPanel(panel),
    mMenu(new QMenu(this)),
    mAction(new QAction(panel))

{
    //mAction->setCheckable(true);
    setAutoRaise(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::NoFocus);
    setDefaultAction(mAction);
    setIconSize(QSize(mPanel->mHeight -4,mPanel->mHeight -4));
    mAction->setIcon(mIcon);

    connect(mAction, &QAction::triggered, this, &PinButton::actionClicked);
}


void PinButton::actionClicked(bool checked){
    qDebug() << "checked" << checked;

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
    a = mMenu->addAction("Close");
    connect(a, &QAction::triggered, this, &PinButton::requestClose);


    //set menu position
    auto menuGeo = mPanel->calculateMenuPosition(geometry().center() , mMenu->sizeHint(), 4, false);
    mMenu->setGeometry(menuGeo);
    mMenu->setFixedSize(menuGeo.size());
    mMenu->show();
}
void PinButton::updateMenu(){
    QAction * a = mMenu->addAction("Close");
    connect(a, &QAction::triggered, this, &PinButton::requestClose);
}
void PinButton::requestClose(){

}
