#include "taskbutton.h"
#include "taskbar.h"
#include "panelqt.h"
#include "taskbutton.h"
#include "taskgroup.h"
#include "taskbar.h"
#include "panelqt.h"
#include "pinbutton.h"
#include "toolbuttontextstyle.hpp"

TaskButton::~TaskButton(){}

TaskButton::TaskButton(WId id, QIcon icon, QString title, QString className, QActionGroup * actionGroup, TaskGroup * group, TaskBar * taskbar, PanelQt * panel):
    QToolButton(group),
    mClass(className),
    mPinned(false),
    mId(id),
    mIcon(icon),
    mTitle(title),
    mGroup(group),
    mTaskBar(taskbar),
    mPanel(panel),
    mMenu(new QMenu(this)),
    mAction(actionGroup->addAction(new QAction(panel)))

{
    setAutoRaise(mTaskBar->mButtonAutoRaise);
    setAccessibleName(className);
    mAction->setCheckable(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    setFocusPolicy(Qt::NoFocus);
    setDefaultAction(mAction);
    setMaximumWidth(mTaskBar->mMaxBtnWidth);
    setAcceptDrops(true);
    mAction->setIcon(mIcon);
    updateIconSize(mTaskBar->mIconHeight);

    mAction->setText(mTitle);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setStyle(mTaskBar->mTBTextStyle);

    connect(mTaskBar, &TaskBar::buttonAutoRaiseChanged, this, &TaskButton::updateAutoRaise);
    connect(mTaskBar, &TaskBar::iconHeightChanged, this, &TaskButton::updateIconSize);
    //connect(mTaskBar, &TaskBar::buttonWidthChanged, this, &TaskButton::buttonWidthChanged);
    connect(mAction, &QAction::triggered, this, &TaskButton::actionClicked);
}
void TaskButton::changeIcon(QIcon icon){
    mAction->setIcon(icon);
    mIcon = icon;
}
void TaskButton::updateAutoRaise(bool autoRaise){
    setAutoRaise(autoRaise);
}
void TaskButton::updateIconSize(int h){
    QSize size = QSize(h, h);
    //mAction->setIcon(mIcon.pixmap(size).scaled(size));
    setIconSize(size);
    updateGeometry();
}
void TaskButton::actionClicked(bool checked){
    qDebug() << "checked" << checked;
    if(KWindowSystem::activeWindow() == mId){
        mAction->setChecked(false);
        KWindowSystem::minimizeWindow(mId);
    }

    else
        KWindowSystem::forceActiveWindow(mId);
}
void TaskButton::setTitle(QString title){
    mTitle = title;
    mAction->setText(mTitle);
}
void TaskButton::setActionCheck(bool check){
    mAction->setChecked(check);
}
void TaskButton::buttonWidthChanged(int w){
    //qDebug() << "buttonWidthChanged" << mId << w;
    //setMinimumWidth(w);
    setMaximumWidth(w);
    updateGeometry();
}
void TaskButton::dragEnterEvent(QDragEnterEvent *e){
     qDebug() << "enter button";
    if (e->mimeData()->text() != "Drag TaskGroup"){

        KWindowSystem::raiseWindow(mId);
        KWindowSystem::forceActiveWindow(mId);
    } /*else {
        e->ignore();
    }*/
    e->accept();
}
void TaskButton::moveApplicationToPrevNextMonitor(bool next)
{
    KWindowInfo info(mId, NET::WMState | NET::XAWMState);
    if (isMinimized())
        KWindowSystem::unminimizeWindow(mId);
    KWindowSystem::forceActiveWindow(mId);
    const QRect& windowGeometry = KWindowInfo(mId, NET::WMFrameExtents).frameGeometry();
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.size() > 1){
        for (int i = 0; i < screens.size(); ++i)
        {
            QRect screenGeometry = screens[i]->geometry();
            if (screenGeometry.intersects(windowGeometry))
            {
                int targetScreen = i + (next ? 1 : -1);
                if (targetScreen < 0)
                    targetScreen += screens.size();
                else if (targetScreen >= screens.size())
                    targetScreen -= screens.size();
                QRect targetScreenGeometry = screens[targetScreen]->geometry();
                int X = windowGeometry.x() - screenGeometry.x() + targetScreenGeometry.x();
                int Y = windowGeometry.y() - screenGeometry.y() + targetScreenGeometry.y();
                NET::States state = KWindowInfo(mId, NET::WMState).state();
                //      NW geometry |     y/x      |  from panel
                const int flags = 1 | (0b011 << 8) | (0b010 << 12);
                KWindowSystem::clearState(mId, NET::MaxHoriz | NET::MaxVert | NET::Max | NET::FullScreen);
                NETRootInfo(QX11Info::connection(), NET::Properties(), NET::WM2MoveResizeWindow).moveResizeWindowRequest(mId, flags, X, Y, 0, 0);
                QTimer::singleShot(200, this, [this, state]
                {
                    KWindowSystem::setState(mId, state);
                    KWindowSystem::raiseWindow(mId);
                });
                break;
            }
        }
    }
}
void TaskButton::tileWindow(TilePosition pos, int perc){

    KWindowInfo info(mId, NET::WMFrameExtents | NET::WMGeometry | NET::WMState | NET::XAWMState);
    if (info.isMinimized())
        KWindowSystem::unminimizeWindow(mId);
    KWindowSystem::forceActiveWindow(mId);

    auto screen = QApplication::screens()[mPanel->mScreen];
    auto screenGeo = screen->geometry();

    int frameLeft = qAbs(info.frameGeometry().left() - info.geometry().left());
    int frameRight = qAbs(info.frameGeometry().right() - info.geometry().right());
    int frameTop = qAbs(info.frameGeometry().top() - info.geometry().top());
    int frameBottom = qAbs(info.frameGeometry().bottom() - info.geometry().bottom());

    //QRect windowGeo = KWindowInfo(mId, NET::WMFrameExtents).frameGeometry();
    if(mPanel->mPosition == "Bottom"){
        screenGeo.setBottom(screenGeo.bottom() - mPanel->mHeight);
    }else if (mPanel->mPosition == "Top") {
        screenGeo.setTop(screenGeo.top() + mPanel->mHeight);
    }
//    int width = qRound(static_cast<double>(screenGeo.width() * perc / 100));
    int height = screenGeo.height();
//    int x;
//    if (pos == TilePosition::Left) x = screenGeo.left();
//    if (pos == TilePosition::Center){
//        int sidePerc = (100 - perc) / 2;
//        int sideWidth =  qRound(static_cast<double>(screenGeo.width() * sidePerc / 100));
//        x = screenGeo.left() + sideWidth;
//    }
//    if (pos == TilePosition::Right) x = screenGeo.right() - width;

  int y = screenGeo.top();
    int x;
    if(pos==TilePosition::Left){
        x = screenGeo.left();
    }
    if(pos== TilePosition::Center){
        int leftWidth = (screenGeo.width() - perc)/2;
        x = screenGeo.left() + leftWidth;
    }
    if(pos==TilePosition::Right){
        int leftWidth = screenGeo.width() - perc;
        x = screenGeo.left() + leftWidth;
    }

    int width = perc - frameLeft - frameRight;
    height = height - frameTop - frameBottom;



    qDebug() << screenGeo << x<<y<<width<<height;
    //NET::States state = KWindowInfo(mId, NET::WMState).state();
    //      NW geometry |     y/x        |  from panel
    const int flags = 1 | (0b01111 << 8) | (0b010 << 12);
    KWindowSystem::clearState(mId, NET::MaxHoriz | NET::MaxVert | NET::Max | NET::FullScreen);
    NETRootInfo(QX11Info::connection(), NET::Properties(), NET::WM2MoveResizeWindow).moveResizeWindowRequest(mId, flags, x, y, width, height);
    QTimer::singleShot(200, this, [this]
    {
        KWindowSystem::setState(mId, NET::MaxVert);
        KWindowSystem::raiseWindow(mId);
    });

}
void TaskButton::contextMenuEvent(QContextMenuEvent *event){
    Q_UNUSED(event);
    mMenu->clear();
    //_menu.set
    QAction * a;
    // pin actions
    if(mGroup->mPinned){
        auto cmd = mGroup->mCmd;
        a = mMenu->addAction(mIcon, QString("New Instance: %1").arg(cmd));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::startPin);

        a = mMenu->addAction(QString("Remove Pin"));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::removePin);
    }else {
        a = mMenu->addAction(QString("Add Pin"));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::addPin);
    }
    mMenu->addSeparator();

    // taskbar
    a = mMenu->addAction("Panel/TaskBar Settings");
    connect(a, &QAction::triggered, this, [=] {
        auto center = mapFromParent(geometry().center());

        mPanel->showDialog(mapToGlobal(center));
    });
    mMenu->addSeparator();
    a = mMenu->addAction("Tile Left: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Left, 640);
    });
    a = mMenu->addAction("Tile Center: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Center, 640);
    });
    a = mMenu->addAction("Tile Right: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Right, 640);
    });
    mMenu->addSeparator();
    a = mMenu->addAction("Tile Left: 1280 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Left, 1280);
    });
    a = mMenu->addAction("Tile Right: 1280 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Right, 1280);
    });
    mMenu->addSeparator();
    a = mMenu->addAction("Close");
    connect(a, &QAction::triggered, this, &TaskButton::requestClose);


    //set menu position
    auto center = mapFromParent(geometry().center());
    auto menuGeo = mPanel->calculateMenuPosition(mapToGlobal(center), mMenu->sizeHint(), 4, false);
//    qDebug() << "geo" << center;
    mMenu->setGeometry(menuGeo);
    mMenu->show();
}
void TaskButton::updateMenu(){
    QAction * a = mMenu->addAction("Close");
    connect(a, &QAction::triggered, this, &TaskButton::requestClose);
}
void TaskButton::requestClose(){
    emit closeRequested(mId);
}
