#ifndef SNBUTTON_H
#define SNBUTTON_H

#include <QtWidgets>

struct IconProperty{
    QString Name;
    QString Pixmap;
};

class SniAsync;
class SNFrame;

class SNButton : public QToolButton
{
    Q_OBJECT
public:
    SNButton(QString service, QString objectPath, SNFrame * frame);

    QIcon mIcon;
    QIcon mOverlayIcon;
    QIcon mAttentionIcon;
    QIcon mFallbackIcon;

    SNFrame * mFrame;
    QMenu * mMenu;

    QMap<QString, IconProperty> statusToProp;

    QString mStatus;
    QString mTitle;
    QString mIconName;
    QString mOverlayIconName;
    QString mAttentionIconName;

    SniAsync * mInterface;

    QAction * configAction();
    void changeIconSize(int s);
    void changeButtonWidth(int w);

    void updateIcon();
    void fetchIcon(QString status);

    void mouseReleaseEvent(QMouseEvent * event);
    void showMenu();
    void createMenu();
    void newStatus(QString s);
    void newTitle();
    void newIcon();
    void newOverlayIcon();
    void newAttentionIcon();
};

#endif // SNBUTTON_H
