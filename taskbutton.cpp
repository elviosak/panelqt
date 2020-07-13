#include "taskbutton.h"
#include "taskbar.h"
#include "panelqt.h"

void LeftAlignedTextStyle::drawItemText(QPainter * painter, const QRect & rect, int flags
            , const QPalette & pal, bool enabled, const QString & text
            , QPalette::ColorRole textRole) const
{
    QString txt = text;
    // get the button text because the text that's given to this function may be middle-elided
    if (const QToolButton *tb = dynamic_cast<const QToolButton*>(painter->device()))
        txt = tb->text();
    txt = QFontMetrics(painter->font()).elidedText(txt, Qt::ElideRight, rect.width());
    QProxyStyle::drawItemText(painter, rect, (flags & ~Qt::AlignHCenter) | Qt::AlignLeft, pal, enabled, txt, textRole);
}

TaskButton::~TaskButton(){}

TaskButton::TaskButton(WId id, QIcon icon, QString title, QString className, QActionGroup * actionGroup, TaskBar * taskbar, PanelQt * panel):
    QToolButton(panel),
    mClass(className),
    mPinned(false),
    mId(id),
    mTitle(title),
    mPanel(panel),
    mTaskbar(taskbar),
    mMenu(new QMenu(this)),
    mAction(actionGroup->addAction(new QAction(panel)))

{
    setAccessibleName(className);
    QSize size = QSize(mPanel->mHeight - 8, mPanel->mHeight - 8);
    mIcon = icon.pixmap(size);
    setIconSize(size);
    //qDebug() << iconSize() << mPanel->mHeight;
    //mIcon = icon.pixmap(iconSize());
    mAction->setCheckable(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::NoFocus);
    setDefaultAction(mAction);
    setAutoRaise(true);

    //setIconSize(QSize(mPanel->mHeight - 4, mPanel->mHeight - 4));
    mAction->setIcon(mIcon);

    mAction->setText(mTitle);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setStyle(new LeftAlignedTextStyle());




    connect(mAction, &QAction::triggered, this, &TaskButton::actionClicked);
}

void TaskButton::actionClicked(bool checked){
    qDebug() << "checked" << checked;
    if(KWindowSystem::activeWindow() == mId){
        mAction->setChecked(false);
        KWindowSystem::minimizeWindow(mId);
    }

    else
        KWindowSystem::forceActiveWindow(mId);
}
void TaskButton::setTitle(QString title){
    mTitle = title;
    mAction->setText(mTitle);
}
void TaskButton::setActionCheck(bool check){
    mAction->setChecked(check);
}
void TaskButton::buttonWidthChanged(int w){
    //setBaseSize(w, 1);
    setMinimumWidth(w);
    setMaximumWidth(w);
    updateGeometry();
}
void TaskButton::contextMenuEvent(QContextMenuEvent *event){
    Q_UNUSED(event);
    mMenu->clear();
    //_menu.set
    QAction * a;
    for (int i = 0; i < 10; ++i) {
        a = mMenu->addAction(QString("Action %1 - Action Name").arg(i));
        a->setShortcutVisibleInContextMenu(false);
    }
    a = mMenu->addAction("Close");
    connect(a, &QAction::triggered, this, &TaskButton::requestClose);


    //set menu position
    auto menuGeo = mPanel->calculateMenuPosition(geometry().center() , mMenu->sizeHint(), 4, false);
    mMenu->setGeometry(menuGeo);
    mMenu->setFixedSize(menuGeo.size());
    mMenu->show();
}
void TaskButton::updateMenu(){
    QAction * a = mMenu->addAction("Close");
    connect(a, &QAction::triggered, this, &TaskButton::requestClose);
}
void TaskButton::requestClose(){
    emit closeRequested(mId);
}
