#ifndef EXTRAS_H
#define EXTRAS_H

#include <QFrame>
#include <QHash>
#include <QString>

QHash<QString, QFrame::Shape> Shapes = {
    {"NoFrame", QFrame::NoFrame},
    {"Box", QFrame::Box},
    {"Panel", QFrame::Panel},
    {"StyledPanel", QFrame::StyledPanel},
    {"HLine", QFrame::HLine},
    {"VLine", QFrame::VLine},
    {"WinPanel", QFrame::WinPanel}
};
QHash<QString, QFrame::Shadow> Shadows = {
    {"Plain", QFrame::Plain},
    {"Raised", QFrame::Raised},
    {"Sunken", QFrame::Sunken}
};

#endif // EXTRAS_H
