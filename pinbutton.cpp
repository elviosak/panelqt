#include "pinbutton.h"
#include "taskgroup.h"
#include "taskbar.h"
#include "panelqt.h"

//pinned button icon
PinButton::~PinButton(){}
PinButton::PinButton(QIcon icon, QString className, TaskGroup * group, TaskBar * taskbar, PanelQt * panel):
    QFrame(group),
    mIcon(icon),
    mClassName(className),
    mGroup(group),
    mTaskBar(taskbar),
    mPanel(panel)
{
    setFrameShape(Shape::Box);
    setFrameShadow(Shadow::Plain);
    setLineWidth(1);
    setMidLineWidth(1);
    auto box = new QHBoxLayout(this);
    mBtnIcon = new QLabel(this);
    box->setMargin(0);
    box->setSpacing(0);
    //box.setAlignment(Qt::Alignment::)
    setLayout(box);
    mPaletteColor = QPalette::Window;
    QSize size(mTaskBar->mPinIconHeight, mTaskBar->mPinIconHeight);
    mBtnIcon->setPixmap(mIcon.pixmap(size));
    //mBtnIcon->setMaximumWidth(mTaskBar->mPinBtnWidth);
    mBtnIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    box->addWidget(mBtnIcon,1, Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::NoFocus);

    setMinimumWidth(mTaskBar->mPinBtnWidth);
    setMaximumWidth(mTaskBar->mPinBtnWidth);
    connect(mTaskBar, &TaskBar::pinIconHeightChanged, this, &PinButton::updateIconSize);
    connect(mTaskBar, &TaskBar::pinBtnWidthChanged, this, &PinButton::updatePinWidth);
}
void PinButton::paintEvent(QPaintEvent *event){
//    if(mPaletteColor == QPalette::Window)
//        return;

    QPainter painter(this);
    QColor color(palette().color(mPaletteColor));
    color.setAlpha(mPanel->mOpacity);
    painter.fillRect(event->rect(), color);
}
void PinButton::leaveEvent(QEvent *e){
    mHover = false;
    mPaletteColor = QPalette::Window;
    repaint();
    e->accept();
}
void PinButton::enterEvent(QEvent *e){
    mPaletteColor = QPalette::Light;
    mHover = true;
    repaint();
    e->accept();
}
void PinButton::mousePressEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
        handleClick(true);
    }
}
void PinButton::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
        handleClick(false);
        e->accept();
    }else if(e->button() == Qt::RightButton){
        e->accept();
        showMenu();
    }
}
void PinButton::handleClick(bool press){
    if(press){
        mPaletteColor = QPalette::Base;
    }else {
        if(mHover){
            mPaletteColor = QPalette::Light;
        }else {
            mPaletteColor = QPalette::Window;
        }
        mGroup->startPin();
    }
    repaint();
}
void PinButton::changeIcon(QIcon icon){
    if (!icon.isNull()){
        QSize size = QSize(mTaskBar->mIconHeight, mTaskBar->mIconHeight);
        mBtnIcon->setPixmap(mIcon.pixmap(size));
        mIcon = icon;
    }
}

void PinButton::updatePinWidth(int w){
    setMinimumWidth(w);
    setMaximumWidth(w);
}
void PinButton::updateIconSize(int h){
    QSize size = QSize(h, h);
    mBtnIcon->setPixmap(mIcon.pixmap(size));
    updateGeometry();
}

void PinButton::actionClicked(){
    mGroup->startPin();
}

void PinButton::showMenu(){
    QMenu * menu = new QMenu(this);
    //_menu.set
    QAction * a;
    a = menu->addAction(mIcon, QString("New Instance: %1").arg(mGroup->mCmd));
    connect(a, &QAction::triggered, mGroup, &TaskGroup::startPin);

    a = menu->addAction(QIcon::fromTheme("emblem-remove"), QString("Remove Pin"));
    connect(a, &QAction::triggered, mGroup, &TaskGroup::removePin);
    menu->addSeparator();


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

//    auto center = mapFromParent(geometry().center());
//    auto menuGeo = mPanel->calculateMenuPosition(mapToGlobal(center), menu->sizeHint(), 4, true);
////    qDebug() << "geo" << center;
//    menu->setGeometry(menuGeo);
//    menu->show();
}

