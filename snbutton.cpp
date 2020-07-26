#include "snbutton.h"
#include "snframe.h"
#include "panelqt.h"
#include "sn/sniasync.h"
#include <dbusmenu-qt5/dbusmenuimporter.h>

SNButton::SNButton(QString service, QString objectPath, SNFrame * frame)
    : QToolButton(frame),
      mFrame(frame),
      mStatus("Passive")
{
    setAttribute(Qt::WA_NoMousePropagation);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setAutoRaise(true);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    mFallbackIcon = QIcon::fromTheme("application-x-desktop");

    setIconSize(QSize(mFrame->mIconSize, mFrame->mIconSize));
    setFixedWidth(mFrame->mButtonWidth);
    setContentsMargins(0,0,0,0);

    //Watcher stuff
    mInterface = new SniAsync(service, objectPath, QDBusConnection::sessionBus(), this);

    connect(mInterface, &SniAsync::NewStatus, this, &SNButton::newStatus);
    connect(mInterface, &SniAsync::NewTitle, this, &SNButton::newTitle);
    connect(mInterface, &SniAsync::NewIcon, this, &SNButton::newIcon);
    connect(mInterface, &SniAsync::NewOverlayIcon, this, &SNButton::newOverlayIcon);
    connect(mInterface, &SniAsync::NewAttentionIcon, this, &SNButton::newAttentionIcon);

    mInterface->propertyGetAsync(QStringLiteral("Status"), [=] (QString s) {
        newStatus(s);
    });

    statusToProp["Active"] = {"OverlayIconName", "OverlayIconPixmap"};
    statusToProp["NeedsAttention"] = {"AttentionIconName", "AttentionIconPixmap"};
    statusToProp["Passive"] = {"IconName", "IconPixmap"};

    fetchIcon("Active");
    fetchIcon("Passive");
    fetchIcon("NeedsAttention");

    connect(mFrame, &SNFrame::SNIconSizeChanged, this, &SNButton::changeIconSize);
    connect(mFrame, &SNFrame::SNButtonWidthChanged, this, &SNButton::changeButtonWidth);
}
QAction* SNButton::configAction(){
    QAction * a = new QAction("Status Notifier Settings");
    connect(a, &QAction::triggered, mFrame, &SNFrame::showDialog);
    return a;
}
void SNButton::changeIconSize(int s){
    qDebug() << "changeIconSize" << s;
    setIconSize(QSize(s, s));
}
void SNButton::changeButtonWidth(int w){
    qDebug() << "changeButtonWidth" << w;
    setFixedWidth(w);
}
void SNButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        mInterface->Activate(QCursor::pos().x(), QCursor::pos().y());
    else if (event->button() == Qt::MidButton)
        mInterface->SecondaryActivate(QCursor::pos().x(), QCursor::pos().y());
    else if (Qt::RightButton == event->button())
    {
        showMenu();
    }
    QToolButton::mouseReleaseEvent(event);
}
void SNButton::createMenu(){
    mInterface->propertyGetAsync(QLatin1String("Menu"), [=] (QDBusObjectPath path) {
        if (!path.path().isEmpty())
        {
            qDebug() << "menu imported";
            mMenu = (new DBusMenuImporter(mInterface->service(), path.path(), this))->menu();
            mMenu->setObjectName(QLatin1String("StatusNotifierMenu"));
        }
    });
}
void SNButton::showMenu(){
    mInterface->propertyGetAsync(QLatin1String("Menu"), [=] (QDBusObjectPath path) {
        if (!path.path().isEmpty())
        {
            auto importer = new DBusMenuImporter(mInterface->service(), path.path(), this);
            connect(importer, &DBusMenuImporter::menuUpdated, this, [=]{
                QMenu * m = new QMenu(importer->menu()->title());
                m->addAction(configAction());
                m->addSeparator();
                auto objList = importer->menu()->children();
                QAction * menuAction = importer->menu()->menuAction();
                for(auto obj : objList){
                    QAction * a = qobject_cast<QAction*>(obj);
                    if(nullptr != a && a != menuAction){
                        m->addAction(a);
                    }
                }
                auto center = mapFromParent(geometry().center());
                auto menuGeo = mFrame->mPanel->calculateMenuPosition(mapToGlobal(center), m->sizeHint(), 4, false);
                m->setGeometry(menuGeo);
                m->show();
            });
            importer->updateMenu();
        }
    });
}
void SNButton::fetchIcon(QString status)
{
    QString nameProperty = statusToProp[status].Name;
    QString pixmapProperty = statusToProp[status].Pixmap;
    mInterface->propertyGetAsync(nameProperty, [this, status, nameProperty, pixmapProperty] (QString iconName) {
        qDebug() << "status" << status << nameProperty << pixmapProperty << "iconname" << iconName << "fromthemenull" << QIcon::fromTheme(iconName).isNull();
        if (!iconName.isEmpty() && !QIcon::fromTheme(iconName).isNull())
        {
            setIcon(QIcon::fromTheme(iconName));
        }
        else{
            mInterface->propertyGetAsync(pixmapProperty, [this, status, pixmapProperty] (IconPixmapList iconPixmaps)
            {
                qDebug() << "iconpixmaps empty" << iconPixmaps.empty();
                if (iconPixmaps.empty()){
                    return;
                }
                QIcon nextIcon;
                for (IconPixmap iconPixmap: iconPixmaps)
                {
                    if (!iconPixmap.bytes.isNull())
                    {
                        QImage image((uchar*) iconPixmap.bytes.data(), iconPixmap.width,
                                     iconPixmap.height, QImage::Format_ARGB32);

                        const uchar *end = image.constBits() + image.sizeInBytes();
                        uchar *dest = reinterpret_cast<uchar*>(iconPixmap.bytes.data());
                        for (const uchar *src = image.constBits(); src < end; src += 4, dest += 4)
                            qToUnaligned(qToBigEndian<quint32>(qFromUnaligned<quint32>(src)), dest);

                        nextIcon.addPixmap(QPixmap::fromImage(image));
                    }
                }
                qDebug() << "next icon null" << nextIcon.isNull();
                if(!nextIcon.isNull()){
                    setIcon(nextIcon);
                }
            });
        }
    });
}
void SNButton::updateIcon(){
    if (mStatus == "Active" && !mOverlayIcon.isNull())
        setIcon(mOverlayIcon);
    else if (mStatus == "NeedsAttention" && !mAttentionIcon.isNull())
        setIcon(mAttentionIcon);
    else if (mStatus == "Passive" && !mIcon.isNull())
        setIcon(mIcon);
    else if (!mOverlayIcon.isNull())
        setIcon(mOverlayIcon);
    else if (!mAttentionIcon.isNull())
        setIcon(mAttentionIcon);
    else
        setIcon(mFallbackIcon);
}

void SNButton::newStatus(QString s){
    if(s != mStatus){
        fetchIcon(s);
        mStatus = s;
    }
}
void SNButton::newTitle(){
    mInterface->propertyGetAsync(QStringLiteral("Title"), [this] (QString title) {
        qDebug() << title;
        mTitle = title;
    });
}
void SNButton::newIcon(){
    fetchIcon(mStatus);
}
void SNButton::newOverlayIcon(){
    fetchIcon(mStatus);
}
void SNButton::newAttentionIcon(){
    fetchIcon(mStatus);
}
