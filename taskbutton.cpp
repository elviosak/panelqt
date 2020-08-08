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

TaskButton::TaskButton(WId id, QIcon icon, QString title, QString className, TaskGroup * group, TaskBar * taskbar, PanelQt * panel):
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
{
    auto box = new QHBoxLayout(this);
    box->setMargin(0);
    box->setSpacing(0);
    //setLayout(box);
    auto innerFrame = new QFrame;
    innerFrame->setFrameShape(Shape::NoFrame);
    box->addSpacing(2);
    box->addWidget(innerFrame);
    auto hbox = new QHBoxLayout(innerFrame);
    hbox->setDirection(QBoxLayout::LeftToRight);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    //innerFrame->setLayout(hbox);
    mBtnIcon = new QLabel;
    mBtnText = new QLabel;

    mBtnText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);



    hbox->addWidget(mBtnIcon);
    hbox->addWidget(mBtnText);

    changeIcon(mIcon);
    changeText(title);

    setLineWidth(1);
    setContentsMargins(0,0,0,0);
    setActive(mActive);

    setAccessibleName(className);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    setFocusPolicy(Qt::NoFocus);
    setAcceptDrops(true);

    connect(mTaskBar, &TaskBar::iconHeightChanged, this, &TaskButton::changeIconSize);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &TaskButton::activeWindowChanged);
    connect(mTaskBar, &TaskBar::buttonWidthChanged, this, &TaskButton::buttonWidthChanged);
}
void TaskButton::paintEvent(QPaintEvent *event){
//    if(mPaletteColor == QPalette::Window)
//        return;

    QPainter painter(this);
    QColor color(palette().color(mPaletteColor));
    color.setAlpha(mPanel->mOpacity);
    painter.fillRect(event->rect(), color);
}
void TaskButton::leaveEvent(QEvent *e){
    setActive(mActive);
    e->accept();
}
void TaskButton::enterEvent(QEvent *e){
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
        mBtnIcon->setPixmap(mIcon.pixmap(size));
        mIcon = icon;
    }
}
void TaskButton::changeText(QString t){
    mTitle = t;
    mBtnText->setText(t);
}
void TaskButton::setActive(bool active){
    mActive = active;
    if(active){
        setFrameShape(Shape::StyledPanel);
        setFrameShadow(Shadow::Sunken);
        mBtnText->setStyle(mTaskBar->mElideLabelUnderline);
        mPaletteColor = QPalette::Base;
    }else {
        setFrameShape(Shape::StyledPanel);
        setFrameShadow(Shadow::Raised);
        mBtnText->setStyle(mTaskBar->mElideLabel);
        mPaletteColor = QPalette::Window;
    }
    repaint();
}

void TaskButton::changeIconSize(int h){
    QSize size = QSize(h, h);
    mBtnIcon->setPixmap(mIcon.pixmap(size).scaled(size));
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

    int x = 0;
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
    //      NW geometry |height/width/y/x|  from panel
    const int flags = 1 | (0b01111 << 8) | (0b010 << 12);
    KWindowSystem::clearState(mId, NET::MaxHoriz | NET::MaxVert | NET::Max | NET::FullScreen);
    NETRootInfo(QX11Info::connection(), NET::Properties(), NET::WM2MoveResizeWindow).moveResizeWindowRequest(mId, flags, x, y, width, height);
    QTimer::singleShot(200, this, [this]
    {
        //KWindowSystem::setState(mId, NET::MaxVert);
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
    QDialog * d = new QDialog(this, Qt::Popup);
    auto box = new QHBoxLayout(d);
    box->setMargin(0);
    box->setSpacing(0);
//    auto wid = new QWidget;
//    auto geo = QApplication::screens()[mPanel->mScreen]->geometry();
//    geo.moveTop(geo.top() + 500);
//    qDebug() << "geo" << geo;
//    wid->setGeometry(geo);
//    wid->show();


    QMenu * menu = new QMenu("Menu", d);
    //box->addWidget(menu);

    QAction * a;
    // pin actions
    if(mGroup->mPinned){
        a = menu->addAction(mGroup->mIcon, QString("New Instance: %1").arg(mGroup->mCmd));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::startPin);

        a = menu->addAction(QIcon::fromTheme("emblem-remove"), QString("Remove Pin"));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::removePin);
    }else {
        a = menu->addAction(QIcon::fromTheme("emblem-added"), QString("Add Pin"));
        connect(a, &QAction::triggered, mGroup, &TaskGroup::addPin);
    }
    menu->addSeparator();

    // taskbar
    a = menu->addAction(QIcon(":settings"), "Panel/TaskBar Settings");
    connect(a, &QAction::triggered, this, [=] {
        auto center = mapFromParent(geometry().center());

        mPanel->showDialog(mapToGlobal(center));
    });
    menu->addSeparator();
    a = menu->addAction(QIcon(":tile-left1"), "Tile Left: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Left, 640);
    });
    a = menu->addAction(QIcon(":tile-center1"), "Tile Center: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Center, 640);
    });
    a = menu->addAction(QIcon(":tile-right1"), "Tile Right: 640 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Right, 640);
    });
    menu->addSeparator();
    a = menu->addAction(QIcon(":tile-left2"), "Tile Left: 1280 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Left, 1280);
    });
    a = menu->addAction(QIcon(":tile-right2"), "Tile Right: 1280 px");
    connect(a, &QAction::triggered, this, [=]{
        tileWindow(TilePosition::Right, 1280);
    });
    menu->addSeparator();
    a = menu->addAction(QIcon::fromTheme("window-close"), "Close");
    connect(a, &QAction::triggered, this, &TaskButton::requestClose);

//    box->addWidget(menu);
//    auto btn = new QPushButton("btn");
//    btn->setFlat(true);
//    box->addWidget(btn);


    //menu->show();
    //set menu position
    QPoint pos;
    if(mPanel->mPosition == "Top"){
        pos = mapFromParent(geometry().bottomLeft());
        pos.setY(pos.y() + 4);
    }else {
        pos = mapFromParent(geometry().topLeft());
        pos.setY(pos.y() - 4 - menu->sizeHint().height());
    }

    menu->popup(mapToGlobal(pos));
//    qDebug() << "menu" << menu->sizeHint();
//    auto menuGeo = mPanel->calculateMenuPosition(mapToGlobal(center), d->sizeHint(), 4, true);
    //auto menuGeo = mPanel->calculateMenuPosition(mapToGlobal(center), menu->sizeHint(), 4, true);
//    qDebug() << "geo" << center;
   // d->setGeometry(menuGeo);
    //menu->setGeometry(menuGeo);
    //d->show();
    //menu->show();
}

void TaskButton::requestClose(){
    emit closeRequested(mId);
}
