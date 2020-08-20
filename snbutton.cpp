#include "snbutton.h"
#include "snframe.h"
#include "panelqt.h"
#include "sn/sniasync.h"
#include <dbusmenu-qt5/dbusmenuimporter.h>

#include <KWindowSystem>


/*! \brief specialized DBusMenuImporter to correctly create actions' icons based
 * on name
 */
class MenuImporter : public DBusMenuImporter
{
public:
    using DBusMenuImporter::DBusMenuImporter;

protected:
    virtual QIcon iconForName(const QString & name) override
    {
        return QIcon::fromTheme(name);
    }
};

SNButton::SNButton(QString service, QString objectPath, SNFrame * frame)
    : QToolButton(frame),
      mFrame(frame),
      mIsMenu(false),
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
    mInterface->propertyGetAsync(QStringLiteral("ItemIsMenu"), [=] (bool isMenu) {
        qDebug() << isMenu << service;
        mIsMenu = isMenu;
    });
    newTitle();
    statusToProp["Active"] = {"OverlayIconName", "OverlayIconPixmap"};
    statusToProp["NeedsAttention"] = {"AttentionIconName", "AttentionIconPixmap"};
    statusToProp["Passive"] = {"IconName", "IconPixmap"};

    fetchIcon("Active");
    fetchIcon("Passive");
    fetchIcon("NeedsAttention");
    //getDefaultIcon();
    connect(mFrame, &SNFrame::SNIconSizeChanged, this, &SNButton::changeIconSize);
    connect(mFrame, &SNFrame::SNButtonWidthChanged, this, &SNButton::changeButtonWidth);
}

QAction* SNButton::configAction(){
    QAction * a = new QAction(QIcon(":settings"), "Status Notifier Settings");
    connect(a, &QAction::triggered, mFrame, &SNFrame::showDialog);
    return a;
}
void SNButton::changeIconSize(int s){
    setIconSize(QSize(s, s));
}
void SNButton::changeButtonWidth(int w){
    setFixedWidth(w);
}
void SNButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        if(mIsMenu){
            showMenu();
        }
        else{
            mInterface->Activate(QCursor::pos().x(), QCursor::pos().y());
        }
    else if (event->button() == Qt::MidButton)
        mInterface->SecondaryActivate(QCursor::pos().x(), QCursor::pos().y());
    else if (Qt::RightButton == event->button())
    {
        showMenu();
    }
    QToolButton::mouseReleaseEvent(event);
}
void SNButton::createMenu(){
//    mInterface->propertyGetAsync(QLatin1String("Menu"), [=] (QDBusObjectPath path) {
//        if (!path.path().isEmpty())
//        {
//            mMenu = (new DBusMenuImporter(mInterface->service(), path.path(), this))->menu();
//            mMenu->setObjectName(QLatin1String("StatusNotifierMenu"));
//        }
//    });
}
void SNButton::showMenu(){
    mInterface->propertyGetAsync(QLatin1String("Menu"), [=] (QDBusObjectPath path) {
        if (!path.path().isEmpty())
        {
            auto importer = new MenuImporter(mInterface->service(), path.path(), this);
            connect(importer, &MenuImporter::menuUpdated, this, [=]{
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

                QPoint pos;
                if(mFrame->mPanel->mPosition == "Top"){
                    pos = mapFromParent(geometry().bottomLeft());
                    pos.setY(pos.y() + 4);
                }else {
                    pos = mapFromParent(geometry().topLeft());
                    pos.setY(pos.y() - 4 - m->sizeHint().height());
                }

                m->popup(mapToGlobal(pos));

//                auto center = mapFromParent(geometry().center());
//                auto menuGeo = mFrame->mPanel->calculateMenuPosition(mapToGlobal(center), m->sizeHint(), 4, true);
//                m->setGeometry(menuGeo);
//                m->show();
            });
            importer->updateMenu();
        }
    });
}
void SNButton::saveIcon(QString status, QIcon i){
    if (!i.isNull()){
        if (status == "Active"){
            mOverlayIcon = i;
        }
        if (status == "NeedsAttention"){
            mAttentionIcon = i;
        }
        if (status == "Passive"){
            mIcon = i;
        }
        resetIcon();
    }
}

void SNButton::resetIcon(){
    if (mStatus == "Active" && !mOverlayIcon.isNull())
        setIcon(mOverlayIcon);
    else if (mStatus == "NeedsAttention" && !mAttentionIcon.isNull())
        setIcon(mAttentionIcon);
    else if (!mIcon.isNull())
        setIcon(mIcon);
    else if (!mOverlayIcon.isNull())
        setIcon(mOverlayIcon);
    else if (!mAttentionIcon.isNull())
        setIcon(mAttentionIcon);
    else
        setIcon(mFallbackIcon);
    update();
}
void SNButton::fetchIcon(QString status)
{
    QString nameProperty = statusToProp[status].Name;
    QString pixmapProperty = statusToProp[status].Pixmap;
    mInterface->propertyGetAsync(QLatin1String("IconThemePath"), [this, status, nameProperty, pixmapProperty] (QString themePath) {
        mInterface->propertyGetAsync(nameProperty, [this, status, pixmapProperty, themePath] (QString iconName) {
            QIcon nextIcon;
            if (!iconName.isEmpty())
            {

                if (QIcon::hasThemeIcon(iconName)){
                    nextIcon = QIcon::fromTheme(iconName);
                }
                else
                {
                    QDir themeDir(themePath);
                    if (themeDir.exists())
                    {
                        if (themeDir.exists(iconName + QStringLiteral(".png"))){
                            nextIcon = QIcon(themeDir.filePath(iconName + QStringLiteral(".png")));
                        }else if (themeDir.cd(QStringLiteral("hicolor")) || (themeDir.cd(QStringLiteral("icons")) && themeDir.cd(QStringLiteral("hicolor"))))
                        {
                            const QStringList sizes = themeDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
                            for (const QString &dir : sizes)
                            {
                                const QStringList dirs = QDir(themeDir.filePath(dir)).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
                                for (const QString &innerDir : dirs)
                                {
                                    QString file = themeDir.absolutePath() + QLatin1Char('/') + dir + QLatin1Char('/') + innerDir + QLatin1Char('/') + iconName + QStringLiteral(".png");
                                    if (QFile::exists(file)){
                                        nextIcon =  QIcon(file);
                                    }
                                }
                            }
                        }
                    }
                }


            }
            qDebug() <<"iconName" << iconName << nextIcon.isNull();
            if(!nextIcon.isNull()){
                saveIcon(status, nextIcon);
            }
            else
            {
                mInterface->propertyGetAsync(pixmapProperty, [this, status, pixmapProperty] (IconPixmapList iconPixmaps) {
                    if (iconPixmaps.empty()){
                        qDebug() << "iconPixmaps empty";
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
                    qDebug() <<"pixmap" << nextIcon.isNull();
                    saveIcon(status, nextIcon);
                });
            }
        });
    });
}

void SNButton::newStatus(QString s){
    if(s != mStatus){
        fetchIcon(s);
        mStatus = s;
    }
}
void SNButton::newTitle(){
    mInterface->propertyGetAsync(QStringLiteral("Title"), [this] (QString title) {
        qDebug() << "title" << title;
        mTitle = title;
        setToolTip(mTitle);
    });
}
void SNButton::newIcon(){
    qDebug() << "newIcon" << mStatus;
    fetchIcon(mStatus);
}
void SNButton::newOverlayIcon(){
    qDebug() << "newOverlayIcon" << mStatus;
    fetchIcon(mStatus);
}
void SNButton::newAttentionIcon(){
    qDebug() << "newAttentionIcon" << mStatus;
    fetchIcon(mStatus);
}
