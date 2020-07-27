#ifndef TOOLBUTTONTEXTSTYLE_HPP
#define TOOLBUTTONTEXTSTYLE_HPP

#include <QtWidgets>
#include "taskbutton.h"

class ElideLabel : public QProxyStyle
{
public:
    Qt::TextElideMode mElide;
    QFlags<Qt::AlignmentFlag> mAlign;
    QMargins mMargins;
    bool mUnderline;
    ElideLabel(Qt::TextElideMode elide, QFlags<Qt::AlignmentFlag> align, QMargins margins):
        QProxyStyle(),
        mElide(elide),
        mAlign(align),
        mMargins(margins),
        mUnderline(false)
    {}
    void drawItemText(QPainter * painter, const QRect & rect, int flags
                , const QPalette & pal, bool enabled, const QString & text
                , QPalette::ColorRole textRole) const
    {
        Q_UNUSED(flags);
        Q_UNUSED(text);
        auto lbl = dynamic_cast<QLabel*>(painter->device());
        QString txt = nullptr == lbl ? text : lbl->text();

        QFontMetrics metrics = QFontMetrics(painter->font());
        QRect r(rect.marginsRemoved(mMargins));
        txt = metrics.elidedText(txt, mElide, r.width());

        QProxyStyle::drawItemText(painter, r, mAlign, pal, enabled, txt, textRole);

        if(mUnderline){
            QRect br =  metrics.boundingRect(r, mAlign, txt);
            painter->drawLine(br.bottomLeft(), br.bottomRight());
            painter->end();
        }
    }
};
class ElideLabelUnderline : public QProxyStyle
{
public:
    Qt::TextElideMode mElide;
    QFlags<Qt::AlignmentFlag> mAlign;
    QMargins mMargins;
    bool mUnderline;
    ElideLabelUnderline(Qt::TextElideMode elide, QFlags<Qt::AlignmentFlag> align, QMargins margins):
        QProxyStyle(),
        mElide(elide),
        mAlign(align),
        mMargins(margins),
        mUnderline(true)
    {}
    void drawItemText(QPainter * painter, const QRect & rect, int flags
                , const QPalette & pal, bool enabled, const QString & text
                , QPalette::ColorRole textRole) const
    {
        Q_UNUSED(flags);
        Q_UNUSED(text);
        auto lbl = dynamic_cast<QLabel*>(painter->device());
        QString txt = nullptr == lbl ? text : lbl->text();
        QRect r(rect.marginsRemoved(mMargins));
        QFontMetrics metrics = QFontMetrics(painter->font());
        txt = metrics.elidedText(txt, mElide, r.width());
        QProxyStyle::drawItemText(painter, r, mAlign, pal, enabled, txt, textRole);

        QRect br =  metrics.boundingRect(r, mAlign, txt);
        painter->drawLine(br.bottomLeft(), br.bottomRight());
        painter->end();
    }
};
#endif // TOOLBUTTONTEXTSTYLE_HPP
