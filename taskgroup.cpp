#include "taskgroup.h"
#include "pinbutton.h"
#include "taskbutton.h"
#include "taskbar.h"
#include "panelqt.h"

extern QHash<QString, QFrame::Shape> Shapes;
extern QHash<QString, QFrame::Shadow> Shadows;

TaskGroup::~TaskGroup(){}

TaskGroup::TaskGroup(QString className, QIcon icon, TaskBar* taskbar, PanelQt * panel, bool pinned, QString cmd):
    QFrame(taskbar),
    mClassName(className),
    mIcon(icon),
    mTaskBar(taskbar),
    mPanel(panel),
    mPinned(pinned),
    mCmd(cmd),
    mWindowCount(0),
    mLayout(new QHBoxLayout(this)),
    mPinButton(new PinButton(mIcon, mClassName, this, mTaskBar, mPanel))
{
    setAttribute(Qt::WA_NoMousePropagation);
    mShape = mTaskBar->mGroupShape;
    mShadow = mTaskBar->mGroupShadow;
    mLineWidth = mTaskBar->mGroupLineWidth;
    mMidLineWidth = mTaskBar->mGroupMidLineWidth;

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    setFrameShape(Shape::NoFrame);
    setWindowFlag(Qt::FramelessWindowHint);
    this->setContentsMargins(0,0,0,0);

    setAcceptDrops(true);

    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    mLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);
    //setLayout(mLayout);

    mLayout->addWidget(mPinButton);
    changeFrame();


    connect(mTaskBar, &TaskBar::groupShapeChanged, this, &TaskGroup::changeShape);
    connect(mTaskBar, &TaskBar::groupShadowChanged, this, &TaskGroup::changeShadow);
    connect(mTaskBar, &TaskBar::groupLineWidthChanged, this, &TaskGroup::changeLineWidth);
    connect(mTaskBar, &TaskBar::groupMidLineWidthChanged, this, &TaskGroup::changeMidLineWidth);
    connect(mTaskBar, &TaskBar::buttonWidthChanged, this, &TaskGroup::changeButtonWidth);
}
void TaskGroup::changeButtonWidth(int w){
    int pinwidth = mTaskBar->mPinBtnWidth;
    int btnwidth = w;
    int newwidth = mPinned && mWinList.count() == 0 ? pinwidth + frameWidth() * 2: btnwidth * mWinList.count() + frameWidth();
    setMaximumWidth(newwidth);
    repaint();
}
void TaskGroup::changeShape(QString s){
    mShape = s;
    changeFrame();
}
void TaskGroup::changeShadow(QString s){
    mShadow = s;
    changeFrame();
}
void TaskGroup::changeLineWidth(int w){
    mLineWidth = w;
    changeFrame();
}
void TaskGroup::changeMidLineWidth(int w){
    mMidLineWidth = w;
    changeFrame();
}
void TaskGroup::changeFrame(){
    setFrameShape(Shapes[mShape]);
    setFrameShadow(Shadows[mShadow]);
    setLineWidth(mLineWidth);
    setMidLineWidth(mMidLineWidth);
};
void TaskGroup::setCmd(QString cmd)
{
    mCmd = cmd;
}
void TaskGroup::addPin(){
    QString cmd = QInputDialog::getText(this, QString("Insert Command for %1").arg(mClassName), "Command:", QLineEdit::Normal, mClassName);
    if(cmd != ""){
        setCmd(cmd);
        mPinned = true;
        mTaskBar->savePinned(mClassName, mIcon, mCmd);
    }
}
void TaskGroup::raiseWindow(WId id){
    KWindowSystem::raiseWindow(id);
    KWindowSystem::forceActiveWindow(id);
}
void TaskGroup::removePin(){
    mPinned = false;
    mPinButton->hide();
    mTaskBar->removePin(mClassName);
}
void TaskGroup::startPin(){
    if(mCmd.isEmpty())
        return;

#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
        QStringList splitCmd = QProcess::splitCommand(mCmd);
        qDebug()<< "launch" << mCmd << splitCmd;
        QProcess::startDetached(splitCmd.takeFirst(), splitCmd);
#else
        QProcess::startDetached(mCmd);
#endif
}
void TaskGroup::setTitle(WId id, QString title){
    if(mWinList.contains(id))
        mWinList[id]->setTitle(title);
}
void TaskGroup::setActive(WId id){
    if(mWinList.contains(id))
        mWinList[id]->setActionCheck(true);
}

void TaskGroup::addWindow(WId id, QString title, QIcon icon){
    if(!mWinList.contains(id)){
        auto btn = new TaskButton(id, icon, title, mClassName, this, mTaskBar, mPanel);
        mLayout->addWidget(btn, 1);
        //addWidget(btn);
        mWinList[id] = btn;
        mPinButton->hide();
        connect(btn, &TaskButton::closeRequested, mTaskBar, &TaskBar::closeRequested);

        emit buttonAdded(id);
    }
}
void TaskGroup::changeIcon(WId id, QIcon icon){
    if(mWinList.contains(id))
        mWinList[id]->changeIcon(icon);
}
void TaskGroup::removeWindow(WId id){
    if(mWinList.contains(id)){
        qDebug() << "removing" << id << mClassName;
        auto btn = mWinList[id];
        btn->deleteLater();
        mWinList.remove(id);
        if(mWinList.count() == 0){
            if (mPinned){
                mPinButton->show();
                emit buttonRemoved(id);
            }else {
                emit removeGroup(mClassName);
            }
        }
        else {
            emit buttonRemoved(id);
        }
    }
}
void TaskGroup::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}
void TaskGroup::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "enter group";
    event->acceptProposedAction();
    //setFrameShadow(QFrame::Sunken);
    if(event->mimeData()->text() == "Drag TaskGroup"){
        emit dragEntered(this, event->source());
    }
//    else {
//        auto btn = qobject_cast<TaskButton *>(this->childAt(event->pos()));
//        if(nullptr != btn){
//            btn->dragEnterEvent(event);
//        }
//    }
}
void TaskGroup::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
            return;
        if ((event->pos() - dragStartPosition).manhattanLength()
             < QApplication::startDragDistance())
            return;

    QDrag *drag = new QDrag(this);

//    QPixmap * pix = new QPixmap(size());
//    render(pix);
//    QPixmap newPix(size());

//    QPainter painter;
//    painter.begin(&newPix);
//    painter.setRenderHint(QPainter::Antialiasing);
//    //painter.setOpacity();
//    painter.drawPixmap(0,0, *pix);
//    painter.end();
    drag->setHotSpot(QPoint(16,16));
    QPixmap pix = mIcon.pixmap(32,32);
    pix.scaled(32,32);
    drag->setPixmap(pix);
    drag->setDragCursor(QCursor(Qt::DragMoveCursor).pixmap(), Qt::MoveAction);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText("Drag TaskGroup");
    drag->setMimeData(mimeData);

    setFrameShadow(QFrame::Sunken);
    drag->exec(Qt::MoveAction);
    setFrameShadow(QFrame::Raised);
    drag->deleteLater();


   // Qt::DropAction dropAction = drag->exec();
}
