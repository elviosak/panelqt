#include "panelqt.h"
#include "extras.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString name = argc > 1 ? argv[1] : "panel1";
    PanelQt w(name);
    w.show();
    return a.exec();
}
