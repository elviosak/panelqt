#include "panelqt.h"

PanelQt::PanelQt(QWidget *parent):
    QFrame(parent),
    mLayout(new QHBoxLayout(this))
{
    mScreen = 1; //QX11Info::appScreen();
    mPosition = "Bottom"; // Top or Bottom
    //mPosition = "Top";
    mHeight = 32;
    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    setLayout(mLayout);
    setFocusPolicy(Qt::NoFocus);
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus;
    setWindowFlags(flags);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    QApplication::setAttribute(Qt::AA_DontShowShortcutsInContextMenus);

    init();
}

void PanelQt::init(){
    calculateGeometry();
    setGeometry(mGeometry);
    setFixedSize(mGeometry.size());
    auto taskbar = new TaskBar(this);
    mLayout->addWidget(taskbar);

    auto spacer = new QFrame();
    spacer->setFrameShape(Shape::StyledPanel);
    spacer->setFrameShadow(Shadow::Raised);
    spacer->setBaseSize(mGeometry.width(), 1);
    spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    mLayout->addWidget(spacer);

    mLayout->addSpacing(4);
    //mLayout->addWidget(new BtButton(this));

    mLayout->addWidget(new Clock(this));
    updateStrut();
}
QRect PanelQt::calculateMenuPosition(QPoint pos, QSize size, int gap, bool isGlobal){
    auto screenGeo = QApplication::screens()[mScreen]->geometry();
    auto menuGeo = isGlobal ? QRect(pos, size) : QRect(mapToGlobal(pos), size);
    qDebug() << "calculateMenuPosition" << pos << mapToGlobal(pos) << isGlobal << menuGeo;

    menuGeo.moveLeft(menuGeo.left() - size.width()/2); //center menu

    if(mPosition == "Bottom")
        screenGeo.moveBottom(screenGeo.bottom() - mHeight);
    if(mPosition == "Top")
        screenGeo.moveTop(screenGeo.top() + mHeight);

    qDebug() << "moveTop" << menuGeo;

    if (menuGeo.top() < screenGeo.top() + gap){
        menuGeo.moveTop(screenGeo.top() + gap);
        qDebug() << "moveTop" << menuGeo;
    }
    if (menuGeo.bottom() > screenGeo.bottom() - gap){
        menuGeo.moveBottom(screenGeo.bottom() - gap);
        qDebug() << "moveBottom" << menuGeo;
    }
    if (menuGeo.left() < screenGeo.left() + gap){
        menuGeo.moveLeft(screenGeo.left() + gap);
        qDebug() << "moveLeft" << menuGeo;
    }
    if (menuGeo.right() > screenGeo.right() - gap){
        menuGeo.moveRight(screenGeo.right() - gap);
        qDebug() << "moveRight" << menuGeo;
    }

    qDebug() << "calculateMenuPosition" << screenGeo << menuGeo;
    return menuGeo;
}
void PanelQt::calculateGeometry(){
    QRect scr_geo = QApplication::screens()[mScreen]->geometry();
    if (mPosition == "Bottom"){
        mGeometry = QRect(scr_geo.bottomLeft() - QPoint(0, mHeight), scr_geo.bottomRight());
    }
    else if(mPosition == "Top") {
        mGeometry = QRect(scr_geo.topLeft(), scr_geo.topRight() + QPoint(0, mHeight));
    }
}

void PanelQt::updateStrut(){

//    auto geo = QApplication::screens()[mScreen] -> geometry();
//    WId win = winId();
//    KWindowSystem::setStrut(win, geo.left(), geo.right(), geo.bottom() + mHeight, geo.bottom());

    auto virtGeo = QApplication::screens()[mScreen]->virtualGeometry();
    WId win = winId();                //the id of the window
    int left_width = 0,     //width of the strut at the left edge
        left_start = 0,     //starting y coordinate of the strut at the left edge
        left_end = 0,       //ending y coordinate of the strut at the left edge
        right_width = 0,    //width of the strut at the right edge
        right_start = 0,    //starting y coordinate of the strut at the right edge
        right_end = 0,      //ending y coordinate of the strut at the right edge
        top_width = 0,      //width of the strut at the top edge
        top_start = 0,      //starting x coordinate of the strut at the top edge
        top_end = 0,        //ending x coordinate of the strut at the top edge
        bottom_width = 0,   //width of the strut at the bottom edge
        bottom_start = 0,   //starting x coordinate of the strut at the bottom edge
        bottom_end = 0;     //ending x coordinate of the strut at the bottom edge

    if(mPosition == "Bottom"){
        bottom_width = virtGeo.bottom() - mGeometry.bottom() + mHeight;
        bottom_start = mGeometry.left();
        bottom_end = mGeometry.right();
    }
    if(mPosition == "Top"){
        top_width = mGeometry.top() + mHeight;
        top_start = mGeometry.left();
        top_end = mGeometry.right();
    }
    KWindowSystem::setExtendedStrut	(win, left_width, left_start, left_end, right_width, right_start, right_end, top_width, top_start, top_end, 	bottom_width, bottom_start, bottom_end);
}

PanelQt::~PanelQt()
{
    //delete ui;
}

