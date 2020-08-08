#ifndef SNFRAME_H
#define SNFRAME_H

#include <QtWidgets>
#include <QtDBus/QtDBus>


class SNButton;
class StatusNotifierWatcher;

class PanelQt;

class SNFrame : public QFrame
{
    Q_OBJECT
public:
    SNFrame(PanelQt * panel);
    ~SNFrame();
    QString mPanelName;
    QSettings * mSettings;
    PanelQt * mPanel;
    QHBoxLayout * mLayout;

    int mIconSize;
    int mButtonWidth;

    void changeIconSize(int s);
    void changeButtonWidth(int w);

    QString mShape;
    QString mShadow;
    int mLineWidth;
    int mMidLineWidth;

    void showDialog();
    void changeShape(QString s);
    void changeShadow(QString s);
    void changeLineWidth(int w);
    void changeMidLineWidth(int w);
    void changeFrame();

    StatusNotifierWatcher * mWatcher;

    QMap<QString, SNButton*> mButtonList;

    void onItemAdded(const QString &serviceAndPath);
    void onItemRemoved(const QString &serviceAndPath);

    void mouseReleaseEvent(QMouseEvent * event) override;
signals:
    void SNIconSizeChanged(int s);
    void SNButtonWidthChanged(int w);
};

#endif // SNFRAME_H
