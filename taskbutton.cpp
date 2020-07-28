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
    QFrame(group),
    mClass(className),
    mPinned(false),
    mId(id),
    mIcon(icon),
    mTitle(title),
    mGroup(group),
    mTaskBar(taskbar),
    mPanel(panel),
    mActive(KWindowSystem::activeWindow() == mId)
    //mAction(actionGroup->addAction(new QAction(panel)))

{
    auto box = new QHBoxLayout;
    box->setMargin(0);
    box->setSpacing(0);
    setLayout(box);
    auto innerFrame = new QFrame;
    innerFrame->setFrameShape(Shape::NoFrame);
    box->addSpacing(2);
    box->addWidget(innerFrame);
    auto hbox = new QHBoxLayout();
    hbox->setDirection(QBoxLayout::LeftToRight);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    innerFrame->setLayout(hbox);
    mBtnIcon = new QLabel;
    mBtnText = new QLabel;

    mBtnText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);



    hbox->addWidget(mBtnIcon);//,1, Qt::AlignVCenter);
    hbox->addWidget(mBtnText);//,10, Qt::AlignVCenter);

    changeIcon(mIcon);
    changeText(title);

    setLineWidth(1);
    setContentsMargins(0,0,0,0);
    setActive(mActive);

    setAccessibleName(className);
    //mAction->setCheckable(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    setFocusPolicy(Qt::NoFocus);
    setAcceptDrops(true);

    connect(mTaskBar, &TaskBar::iconHeightChanged, this, &TaskButton::updateIconSize);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &TaskButton::activeWindowChanged);
    connect(mTaskBar, &TaskBar::buttonWidthChanged, this, &TaskButton::buttonWidthChanged);
    //connect(mAction, &QAction::triggered, this, &TaskButton::actionClicked);
}
void TaskButton::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    QColor color(palette().color(mPaletteColor));
    color.setAlpha(mPanel->mOpacity);
    painter.fillRect(event->rect(), color);
}
void TaskButton::leaveEvent(QEvent *e){
    qDebug() << "hover leave";
    //setBackgroundRole(QPalette::Window);
    setActive(mActive);
    //mBtnText->setStyle(mActive ? mTaskBar->mElideLabelUnderline : mTaskBar->mElideLabel);
    e->accept();
}
void TaskButton::enterEvent(QEvent *e){
    qDebug() << "hover enter";
    //setBackgroundRole(QPalette::Light);
    mPaletteColor = QPalette::Light;
    repaint();
    e->accept();
}
void TaskButton::activeWindowChanged(WId id){
    mActive = mId == id;
    setActive(mActive);
}
void TaskButton::changeIcon(QIcon icon){
    if (!icon.isNull()){
        QSize size = QSize(mTaskBar->mIconHeight, mTaskBar->mIconHeight);
        //mBtnIcon->setMinimumWidth(mTaskBar->mIconHeight);
        mBtnIcon->setPixmap(mIcon.pixmap(size));
        mIcon = icon;
    }
}
void TaskButton::changeText(QString t){
    mTitle = t;
    mBtnText->setText(t);
    //mBtnText->setMaximumWidth(sizeHint().width()-mTaskBar->mIconHeight);
}
void TaskButton::setActive(bool active){
    mActive = active;
    if(active){
        setFrameShape(Shape::StyledPanel);
        setFrameShadow(Shadow::Sunken);
        mBtnText->setStyle(mTaskBar->mElideLabelUnderline);
        //setBackgroundRole(QPalette::Base);
        mPaletteColor = QPalette::Base;
    }else {
        setFrameShape(Shape::StyledPanel);
        setFrameShadow(Shadow::Raised);
        mBtnText->setStyle(mTaskBar->mElideLabel);
        //setBackgroundRole(QPalette::Button);
        mPaletteColor = QPalette::Window;
    }
    repaint();
}

void TaskButton::updateIconSize(int h){
    QSize size = QSize(h, h);
    //mAction->setIcon(mIcon.pixmap(size).scaled(size));
    mBtnIcon->setPixmap(mIcon.pixmap(size).scaled(size));
    //updateGeometry();
}
void TaskButton::actionClicked(bool checked){
    qDebug() << "checked" << checked;

    if(KWindowSystem::activeWindow() == mId){
        setActive(false);
        KWindowSystem::minimizeWindow(mId);
    }
    else
        KWindowSystem::forceActiveWindow(mId);
}
void TaskButton::setTitle(QString title){
    mTitle = title;
    mBtnText->setText(mTitle);
}
void TaskButton::setActionCheck(bool check){
    setActive(check);
}
void TaskButton::buttonWidthChanged(int w){
    //qDebug() << "buttonWidthChanged" << mId << w;
    //setMinimumWidth(w);
    setMaximumWidth(w - frameWidth());
    updateGeometry();
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

    if(mPanel->mPosition == "Bottom"){
        screenGeo.setBottom(screenGeo.bottom() - mPanel->mHeight);
    }else if (mPanel->mPosition == "Top") {
        screenGeo.setTop(screenGeo.top() + mPanel->mHeight);
    }

    int height = screenGeo.height();
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
void TaskButton::dragEnterEvent(QDragEnterEvent *e){
    qDebug() << "enter button";
   if (e->mimeData()->text() != "Drag TaskGroup"){
       KWindowSystem::raiseWindow(mId);
       KWindowSystem::forceActiveWindow(mId);
       e->accept();
   } else {
       e->ignore();
   }

}

void TaskButton::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        actionClicked(true);
    }else if(event->button() == Qt::MidButton){
        qDebug() << "btn clicked" << mClass;
    }else if(event->button() == Qt::RightButton){
        showMenu();
    }
    event->accept();
}
void TaskButton::showMenu(){
    QMenu * menu = new QMenu(this);
    QAction * a;
    // pin actions
    if(mGroup->mPinned){
        a = menu->addAction(mIcon, QString("New Instance: %1").arg(mGroup->mCmd));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::startPin);

        a = menu->addAction(QString("Remove Pin"));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::removePin);
    }else {
        a = menu->addAction(QString("Add Pin"));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::addPin);
    }
    menu->addSeparator();

    // taskbar
    a = menu->addAction("Panel/TaskBar Settings");
    connect(a, &QAction::triggered, this, [=] {
        auto center = mapFromParent(geometry().center());

        mPanel->showDialog(mapToGlobal(center));
    });
    menu->addSeparator();
    a = menu->addAction("Tile Left: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Left, 640);
    });
    a = menu->addAction("Tile Center: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Center, 640);
    });
    a = menu->addAction("Tile Right: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Right, 640);
    });
    menu->addSeparator();
    a = menu->addAction("Tile Left: 1280 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Left, 1280);
    });
    a = menu->addAction("Tile Right: 1280 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Right, 1280);
    });
    menu->addSeparator();
    a = menu->addAction("Close");
    connect(a, &QAction::triggered, this, &TaskButton::requestClose);


    //set menu position
    auto center = mapFromParent(geometry().center());
    auto menuGeo = mPanel->calculateMenuPosition(mapToGlobal(center), menu->sizeHint(), 4, false);
//    qDebug() << "geo" << center;
    menu->setGeometry(menuGeo);
    menu->show();
}

void TaskButton::requestClose(){
    emit closeRequested(mId);
}
