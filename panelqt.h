#ifndef PANELQT_H
#define PANELQT_H

#include <QtWidgets>
#include <QtGui>
#include <QtCore>
#include <QX11Info>

#include "taskbar.h"
#include "clock.h"


class PanelQt : public QFrame
{
    Q_OBJECT

public:
    PanelQt(QWidget *parent = nullptr);
    ~PanelQt();

    QString mPosition;
    int mScreen;
    int mHeight;
    QStringList mPlugins;
    QRect mGeometry;

    void init();
    void calculateGeometry();
    void reset();
    void loadSettings();
    void addPlugin();
    QRect calculateMenuPosition(QPoint pos, QSize size, int gap = 2, bool isGlobal = true);

private:
    void btnClicked();
    QHBoxLayout * mLayout;
    void updateStrut();

};
#endif // PANELQT_H
