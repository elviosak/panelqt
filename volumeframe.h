#ifndef VOLUMEFRAME_H
#define VOLUMEFRAME_H

#include <QtWidgets>

class PanelQt;
class VolumeButton;
class VolumeFrame : public QFrame
{
    Q_OBJECT
public:
    VolumeFrame(PanelQt * panel);
    ~VolumeFrame();
    VolumeButton* mButton;
};

#endif // VOLUMEFRAME_H
