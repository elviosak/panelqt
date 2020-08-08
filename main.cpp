#include "panelqt.h"
#include "extras.h"

#include <QApplication>
#include <QtDBus>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString name;
    bool start = true;
    if(argc > 1){
        if(QLatin1String(argv[1]) == QLatin1String("--panel"))
        {
            if(argv[2]){
                name = QLatin1String(argv[2]);
            }
        }
        if(QLatin1String(argv[1]) == QLatin1String("--toggle")){
            start = false;
            if(argv[2]){
                name = QLatin1String(argv[2]);
            }
        }
    }
    if(name.isEmpty()){
        name = QLatin1String("panel1");
    }
    if(!start){
        // command: dbus-send --type=method_call --dest=org.panelqt.panel1 / local.panelqt.AppMenu.toggleMenu
        qDebug() << "toggle panel:" << name;
        QString mService = QLatin1String("org.panelqt.");
        mService.append(name);
        QDBusInterface * iface = new QDBusInterface(mService, QLatin1String("/"));
        iface->call(QLatin1String("toggleMenu"));
        qDebug() << "toggled";
        return 0;
    }
    qDebug() << "starting panel:" << name;
    PanelQt w(name);
    w.show();

    return a.exec();
}
