#include "volumeframe.h"
#include "panelqt.h"
#include "volumebutton.h"

VolumeFrame::VolumeFrame(PanelQt * panel)
    :QFrame(panel)
{
    mButton = new VolumeButton(this, panel);
    auto box = new QHBoxLayout(this);
    box->addWidget(mButton);
    box->setMargin(0);
    box->setSpacing(0);
    //setLayout(box);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
}
VolumeFrame::~VolumeFrame(){}
