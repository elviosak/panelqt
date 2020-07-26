#ifndef TOOLBUTTONTEXTSTYLE_HPP
#define TOOLBUTTONTEXTSTYLE_HPP

#include <QtWidgets>

class ToolButtonTextStyle : public QProxyStyle
{
public:
    Qt::TextElideMode mElide;
    QFlags<Qt::AlignmentFlag> mAlign;
    QMargins mMargins;
    bool mUnderline;
    ToolButtonTextStyle(Qt::TextElideMode elide, QFlags<Qt::AlignmentFlag> align, QMargins margins):
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
        QToolButton * tb = dynamic_cast<QToolButton*>(painter->device());
        QString txt = nullptr == tb ? text : tb->text();
        QRect r(rect.marginsRemoved(mMargins));
        QFontMetrics metrics = QFontMetrics(painter->font());
        txt = metrics.elidedText(txt, mElide, r.width());
        QProxyStyle::drawItemText(painter, r, mAlign, pal, enabled, txt, textRole);
        if(nullptr != tb && tb->isChecked() && mUnderline){
            QRect br =  metrics.boundingRect(r, mAlign, txt);
            painter->drawLine(br.bottomLeft(), br.bottomRight());
            painter->end();
        }
    }
};
#endif // TOOLBUTTONTEXTSTYLE_HPP
