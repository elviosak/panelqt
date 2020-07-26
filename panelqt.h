#ifndef PANELQT_H
#define PANELQT_H

#include <QtWidgets>
#include <QtGui>
#include <QtCore>
#include <QX11Info>

class AppMenu;
class TaskBar;
class SNFrame;
class Clock;

class PanelQt : public QFrame
{
    Q_OBJECT

public:
    PanelQt(QString panelname, QWidget *parent = nullptr);
    ~PanelQt();

    QSettings * mSettings;
    QString mPanelName;
    QStringList mPlugins;
    QRect mGeometry;
    QColor mBackgroundColor;
    int mOpacity;
    QFont mSystemFont;
    bool mUseSystemFont;
    QFont mFont;
    QString mPosition;
    int mScreen;
    int mHeight;

    QString mShape;
    QString mShadow;
    int mLineWidth;
    int mMidLineWidth;

    AppMenu * mAppMenu;
    TaskBar * mTaskBar;
    SNFrame * mSNFrame;
    Clock * mClock;

    void changeShape(QString s);
    void changeShadow(QString s);
    void changeLineWidth(int w);
    void changeMidLineWidth(int w);
    void changeFrame();

    void setUseSystemFont(bool useSystemFont);
    void setPanelFont(QFont f);
    void setPanelScreen(int screen);
    void setPanelPosition(QString p);
    void setPanelHeight(int h);
    void updatePanelGeometry();

    void paintEvent(QPaintEvent * event);
    void setPanelOpacity(int o);

    void init();
    void calculateGeometry();
    void reset();
    void loadSettings();
    void addPlugin();
    QRect calculateMenuPosition(QPoint pos, QSize size, int gap = 2, bool isGlobal = true, float hScale = 1, float vScale = 1);
    void mousePressEvent(QMouseEvent * event);
    void showDialog(QPoint pos);

signals:


private:
    void btnClicked();
    QHBoxLayout * mLayout;
    void updateStrut();
};
#endif // PANELQT_H
