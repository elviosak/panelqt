#ifndef VOLUMEBUTTON_H
#define VOLUMEBUTTON_H

#include <QtWidgets>


class PanelQt;
class PulseAudioEngine;
class AudioDevice;
class VolumeFrame;

class VolumeButton : public QFrame
{
    Q_OBJECT

public:
    VolumeButton(VolumeFrame * frame, PanelQt * panel);
    ~VolumeButton();

    QSettings * mSettings;
    PanelQt * mPanel;
    VolumeFrame * mFrame;
    QLabel * mBtnIcon;

    QString mShape;
    QString mShadow;
    int mLineWidth;
    int mMidLineWidth;

    void changeShape(QString s);
    void changeShadow(QString s);
    void changeLineWidth(int w);
    void changeMidLineWidth(int w);
    void changeFrame();
    QStringList mSinkPriority;
    void changeSinkPriority();
    QList<AudioDevice *> sortSinks(QList<AudioDevice*> sinkList);

    int mIconSize;
    int mButtonWidth;
    QIcon mIcon;
    QSize mIconQSize;
    QString mIconName;
    QString mMixerCommand;

    int mVolume;
    PulseAudioEngine * mEngine;
    QList<AudioDevice *> mSinkList;
    AudioDevice * mSink = nullptr;
    QProgressBar * mBar;

    QPalette::ColorRole mPaletteColor;
    bool mHover;

    void updateIcon();
    void changeVolume(int v);
    void changeMute(bool m);
    void changeSink(int i);
    void changeSinkList();
    void changeMixerCommand(QString cmd);
    void launchMixer();

    void handleClick(bool press); // press or release
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;

    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

    void showConfig();
    void showDialog();
    void volumeChanged(int v);

};

#endif // VOLUMEBUTTON_H
