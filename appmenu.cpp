
#include "appmenu.h"
#include "panelqt.h"


AppMenu::AppMenu(PanelQt * panel)
    :QToolButton(panel),
    mSettings(new QSettings("PanelQt", panel->mPanelName + "/appmenu")),
    mPanel(panel)
{
    setAttribute(Qt::WA_NoMousePropagation);
    mDefaultCategories.append({"AudioVideo", "Multimedia", "applications-multimedia"});
    mDefaultCategories.append({"Audio", "Audio", "audio"});
    mDefaultCategories.append({"Video", "Video", "video"});
    mDefaultCategories.append({"Development", "Development", "applications-development"});
    mDefaultCategories.append({"Education", "Education", "applications-education"});
    mDefaultCategories.append({"Game", "Games", "applications-games"});
    mDefaultCategories.append({"Graphics", "Graphics", "applications-graphics"});
    mDefaultCategories.append({"Network", "Network", "applications-network"});
    mDefaultCategories.append({"Office", "Office", "applications-office"});
    mDefaultCategories.append({"Science", "Science", "applications-science"});
    mDefaultCategories.append({"Utility", "Utility", "applications-utilities"});
    mDefaultCategories.append({"Other", "Other", "applications-other"});
    mDefaultCategories.append({"System", "System", "system"});
    mDefaultCategories.append({"Settings", "Settings", "settings"});

    mOrderedCategories = {
        {"AudioVideo", true},
        {"Audio", true},
        {"Video", true},
        {"Development", true},
        {"Education", true},
        {"Game", true},
        {"Graphics", true},
        {"Network", true},
        {"Office", true},
        {"Science", true},
        {"Utility", true},
        {"Other", true},
        {"System", true},
        {"Settings", true}
    };

    mShownCategories = mSettings->value("shownCategories", QStringList(mOrderedCategories.keys())).toStringList();

    for (auto cat: mDefaultCategories){
        if (mShownCategories.contains(cat.Name)){
            mCategories.append(cat);
        }
    }

    mShowAllActions = mSettings->value("showAllActions", false).toBool();
    mHScale = mSettings->value("hScale", 1).toDouble();
    mVScale = mSettings->value("vScale", 1.5).toDouble();
    mWhichIcon = mSettings->value("whichIcon", "Default").toString();
    mIconName = mSettings->value("iconName", "").toString();
    mIconPath = mSettings->value("iconPath", "").toString();
    mTerminalCommand = mSettings->value("terminalCommand", "qterminal -e").toString();
    mIconSize = mSettings->value("iconSize", mPanel->mHeight).toInt();
    mButtonWidth = mSettings->value("buttonWidth", 50).toInt();

    mDefaultIcon = QIcon(":/menu");
    mIconQSize = QSize(mIconSize, mIconSize);



    mDefaultSystemActions = {
        {"Log Out", "system-log-out", "lxqt-leave --logout", true},
        {"Lock Screen", "system-lock-screen", "lxqt-leave --lockscreen", true},
        {"Suspend", "system-suspend", "lxqt-leave --suspend", true},
        {"Hibernate", "system-suspend-hibernate","lxqt-leave --hibernate", false},
        {"Reboot", "system-reboot", "lxqt-leave --reboot", true},
        {"Shutdown", "system-shutdown", "lxqt-leave --shutdown", true}
    };

    loadSystemActions();

    setAutoRaise(true);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumWidth(mButtonWidth);

    changeIcon();


    mDirs = QString::fromUtf8(qgetenv("XDG_DATA_DIRS")).split(":");
    mEntries = findEntries();
    mCategoryMap = categorizeEntries();
    createDialog();

    connect(this, &QToolButton::clicked, this, &AppMenu::toggleMenu);
    connect(mDialog, &QDialog::finished, this, &AppMenu::dialogClosed);

    //Register DBus Interface
    QString mService = QLatin1String("org.panelqt.");
    mService.append(mPanel->mPanelName);
    bool conn = QDBusConnection::sessionBus().isConnected();
    if(!conn) {
        qDebug() << "AppMenu: failed to find DBus connection";
    } else{
        bool regService = QDBusConnection::sessionBus().registerService(mService);
        if(!regService){
            qDebug() << "AppMenu: failed to register DBus service:" << mService << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }else{
            bool regObject = QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);
            if(!regObject) qDebug() << "AppMenu: failed to register DBus object"
                                    << qPrintable(QDBusConnection::sessionBus().lastError().message());;
        }
    }
}
void AppMenu::loadSystemActions(){
   // mListSystemActions = mSettings->value("listSystemActions", QStringList(mDefaultSystemActions.keys())).toStringList();
    for(auto a : mDefaultSystemActions){
        CustomAction ac;
        mSettings->beginGroup("System-" + a.Name);
        ac.Name = mSettings->value("name", a.Name).toString();
        ac.Icon = mSettings->value("icon", a.Icon).toString();
        ac.Exec = mSettings->value("exec", a.Exec).toString();
        ac.Enabled = mSettings->value("enabled", a.Enabled).toBool();
        mSettings->endGroup();
        mSystemActions.append(ac);
    }
}
void AppMenu::changeSystemActions(){
    qDebug() << "saving actions";
    //mSettings->setValue("listSystemActions", mListSystemActions);
    for(auto a : mSystemActions){
        mSettings->beginGroup("System-" + a.Name);
        mSettings->setValue("name", a.Name);
        mSettings->setValue("icon", a.Icon);
        mSettings->setValue("exec", a.Exec);
        mSettings->setValue("enabled", a.Enabled);
        mSettings->endGroup();
    }
    createDialog();
}
void AppMenu::toggleMenu(){
    //Q_UNUSED(t);
    if(mDialog->isVisible()){
        mDialog->hide();
        mSearchEdit->setText("");
        //return false;
    } else {
        auto center = geometry().topLeft();
        auto geo = mPanel->calculateMenuPosition(center, mDialog->sizeHint(), 4, false, mHScale, mVScale, "Left");
        mDialog->setGeometry(geo);
        mDialog->show();
        mSearchEdit->setFocus();
        //return true;
    }
}
void AppMenu::contextMenuEvent(QContextMenuEvent *event){
    Q_UNUSED(event);
    showConfig();
}
void AppMenu::showConfig(){
    auto d = new QDialog(this, Qt::Popup);
    connect(d, &QDialog::finished, this, &AppMenu::changeSystemActions);
    auto hbox = new QHBoxLayout(d);

    auto generalGroup = new QGroupBox("AppMenu Settings:", d);
    auto generalForm = new QFormLayout(generalGroup);

    auto showActionsCheck = new QCheckBox(d);
    auto hScaleSpin = new QDoubleSpinBox(d);
    auto vScaleSpin = new QDoubleSpinBox(d);
    auto terminalCommandEdit = new QLineEdit(d);


    showActionsCheck->setChecked(mShowAllActions);
    hScaleSpin->setMinimumWidth(60);
    hScaleSpin->setRange(1, 2);
    hScaleSpin->setSingleStep(0.05);
    hScaleSpin->setValue(mHScale);
    vScaleSpin->setRange(1, 2);
    vScaleSpin->setValue(mVScale);
    vScaleSpin->setSingleStep(0.05);
    terminalCommandEdit->setText(mTerminalCommand);

    connect(showActionsCheck, &QCheckBox::toggled, this, &AppMenu::changeShowAllActions);
    connect(hScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AppMenu::changeHScale);
    connect(vScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AppMenu::changeVScale);
    connect(terminalCommandEdit, &QLineEdit::textChanged, this, &AppMenu::changeTerminalCommand);

    generalForm->addRow("Show all actions:", showActionsCheck);
    generalForm->addRow("Horizontal scale:", hScaleSpin);
    generalForm->addRow("Vertical scale:", vScaleSpin);
    generalForm->addRow("Command for terminal apps:", terminalCommandEdit);

    hbox->addWidget(generalGroup);

    auto systemGroup = new QGroupBox("System Actions:", d);
    auto systemForm = new QFormLayout(systemGroup);

    for(int i = 0; i < mSystemActions.count(); ++i){
        auto actionsGroup = new QGroupBox(mSystemActions[i].Name, systemGroup);
        actionsGroup->setCheckable(true);
        auto actionsForm = new QFormLayout(actionsGroup);

        auto actionIconEdit = new QLineEdit(mSystemActions[i].Icon);
        auto actionExecEdit = new QLineEdit(mSystemActions[i].Exec);
        actionsForm->addRow("Icon:", actionIconEdit);
        actionsForm->addRow("Exec", actionExecEdit);
        connect(actionsGroup, &QGroupBox::toggled, this, [=](bool checked){
            actionIconEdit->setEnabled(checked);
            actionExecEdit->setEnabled(checked);
            mSystemActions[i].Enabled = checked;
            qDebug() << "toggled";
            //changeSystemActions();
        });
        connect(actionIconEdit, &QLineEdit::textChanged, this, [=](QString text){
            mSystemActions[i].Icon = text;
            //changeSystemActions();
        });
        connect(actionExecEdit, &QLineEdit::textChanged, this, [=](QString text){
            mSystemActions[i].Exec = text;
            //changeSystemActions();
        });
        actionsGroup->setChecked(mSystemActions[i].Enabled);
        systemForm->addRow(actionsGroup);
    }

    hbox->addWidget(systemGroup);


    auto categoriesGroup = new QGroupBox("Show Categories:");
    auto categoriesForm = new QFormLayout(categoriesGroup);

    for (auto i = mOrderedCategories.begin(); i != mOrderedCategories.end(); ++i){
        auto catCheck = new QCheckBox(categoriesGroup);
        catCheck->setChecked(mShownCategories.contains(i.key()));
        connect(catCheck, &QCheckBox::toggled, this, [=] (bool add) { changeShownCategories(i.key(), add);});
        categoriesForm->addRow(i.key(), catCheck);
    }

    hbox->addWidget(categoriesGroup);

    auto center = geometry().center();
    auto geo = mPanel->calculateMenuPosition(center, d->sizeHint(), 4, false);
    d->setGeometry(geo);
    d->show();
    mSearchEdit->setFocus();
}
void AppMenu::changeTerminalCommand(QString cmd){
    mTerminalCommand = cmd;
    mSettings->setValue("terminalCommand", cmd);
}
void AppMenu::changeShowAllActions(bool s){
    mShowAllActions = s;
    mSettings->setValue("showAllActions", s);
    createDialog();
}
void AppMenu::changeShownCategories(QString s, bool add){
    if(add){
        mShownCategories.append(s);
    }else {
        mShownCategories.removeOne(s);
    }
    mSettings->setValue("shownCategories", mShownCategories);
    mCategories.clear();
    for (auto cat: mDefaultCategories){
        if (mShownCategories.contains(cat.Name)){
            mCategories.append(cat);
        }
    }
    mCategoryMap = categorizeEntries();
    createDialog();
}
void AppMenu::changeHScale(double s){
    mHScale = s;
    mSettings->setValue("hScale", s);
}
void AppMenu::changeVScale(double s){
    mVScale = s;
    mSettings->setValue("vScale", s);
}
void AppMenu::changeWhichIcon(QString w){
    mWhichIcon = w;
    mSettings->setValue("whichIcon", w);
    changeIcon();
}
void AppMenu::changeIconName(QString n){
    mIconName = n;
    mSettings->setValue("iconName", n);
    changeIcon();
}
void AppMenu::changeIconPath(QString p){
    mIconPath = p;
    mSettings->setValue("iconPath", p);
    changeIcon();
}
void AppMenu::changeIconSize(int s){
    mIconSize = s;
    mSettings->setValue("iconSize", s);
    changeIcon();
}
void AppMenu::changeButtonWidth(int w){
    mButtonWidth = w;
    mSettings->setValue("buttonWidth", w);
    setMinimumWidth(mButtonWidth);
}
void AppMenu::changeIcon(){
    QIcon i;
    if(mWhichIcon == "Name"){
        i = QIcon::fromTheme(mIconName);
    }else if(mWhichIcon == "Path"){
        i = QIcon(mIconPath);
    }
    if(i.isNull()){
        i = mDefaultIcon;
    }
    setIcon(i);
    setIconSize(mIconQSize);
}



void AppMenu::dialogClosed(){
    mSearchEdit->setText("");
}
void AppMenu::showDialog(){
    if(mDialog->isVisible()){
        mDialog->hide();
        mSearchEdit->setText("");
    } else {
        auto center = geometry().center();
        auto geo = mPanel->calculateMenuPosition(center, mDialog->sizeHint(), 4, false, mHScale, mVScale);
        mDialog->setGeometry(geo);
        mDialog->show();
        mSearchEdit->setFocus();
    }
}
void AppMenu::showMenu(){
    auto center = geometry().center();
   // qDebug() << center;
    auto menuGeo = mPanel->calculateMenuPosition(center, mMenu->sizeHint(), 4, false);
   // qDebug() << "AppMenu menuGeo" << menuGeo;
    mMenu->setGeometry(menuGeo);
    mMenu->show();
}
void AppMenu::launch(Entry e){
    QString Exec = e.Exec;
    if(e.Terminal){
        QString TerminalCommand = mTerminalCommand.isEmpty() ? "qterminal -e" : mTerminalCommand;
        Exec = TerminalCommand + " " + Exec;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
    QStringList splitCmd = QProcess::splitCommand(Exec);
    qDebug()<< "launch" << e.Exec << splitCmd;
    QProcess::startDetached(splitCmd.takeFirst(), splitCmd, e.Path);

#else
    QProcess::startDetached(exec);
#endif
}
void AppMenu::launchCmd(QString cmd){
    QString Exec = cmd;
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
    QStringList splitCmd = QProcess::splitCommand(Exec);
    QProcess::startDetached(splitCmd.takeFirst(), splitCmd);

#else
    QProcess::startDetached(exec);
#endif
}
QHash<QString, QList<Entry>> AppMenu::categorizeEntries(){
    QHash<QString, QList<Entry>> categoryMap;
    for (auto e: mEntries){
        if(e.OnlyShowIn.count()==0){
            bool added = false;
            for (auto cat : mCategories){
                if (e.Categories.contains(cat.Name)){
                    categoryMap[cat.Name].append(e);
                    added = true;
                }
            }
            if(!added){
                categoryMap["Other"].append(e);
            }
        }
    }
    return categoryMap;
}

QHash<QString, QVariant> AppMenu::readDesktopFile(QString p){
    QFile file(p);
    QHash<QString, QVariant> res;
   // qDebug() << p;
    QStringList list;
    if(file.open(QIODevice::ReadOnly)){
        QString fileData = QLatin1String(file.readAll());
        list = fileData.split("\n");
        file.close();
    }
    QString current = "";
    for (QString line : list){
        if(line.startsWith("[") && line.endsWith("]")){
            current = line.replace("[", "").replace("]", "").trimmed() + "/";
        }else {
            int index = line.indexOf("=");
            if (index != -1) {
                QString left = line.left(index);
                QString key = current + left;
                QVariant val = line.mid(index + 1);
                res[key] = val;
            }
        }
    }
   // qDebug()<< res["Name"] << res;
    return res;

}
QList<Entry> AppMenu::findEntries(){
    QStringList done;
    QList<Entry> entries;
    for (QString d: mDirs){
        QDir dir(d.append("/applications"));
        if (dir.exists()){
            QList<QFileInfo> infoList = dir.entryInfoList(QDir::Files, QDir::Name);
            for (QFileInfo info: infoList){
                QString path = info.canonicalFilePath();
                if (!done.contains(path)){
                    QHash<QString, QVariant> s = readDesktopFile(path);
                    done.append(path);
                    if (s.value("Desktop Entry/Type").toString() == "Application" &&
                        s.value("Desktop Entry/NoDisplay", false).toBool() == false &&
                        s.value("Desktop Entry/Hidden", false).toBool() == false
                        )
                    {
                        Entry entry;
                        QString pre = "Desktop Entry/";
                        entry.Name = s.value(pre+"Name", "").toString();
                        entry.GenericName = s.value(pre+"GenericName", "").toString();
                        entry.Comment = s.value(pre+"Comment", "").toString();
                        QString Exec = s.value(pre+"Exec", "").toString();
                        QRegExp rx("%[fFuUick]");
                        entry.Exec = Exec.replace(rx, "").trimmed();
                        entry.Path = s.value(pre+"Path", "").toString();
                        entry.Icon = s.value(pre+"Icon", "").toString();
                        entry.StartupWMClass = s.value(pre+"StartupWMClass", "").toString();
                        entry.Terminal = s.value(pre+"Terminal", false).toBool();
                        entry.OnlyShowIn = s.value(pre+"OnlyShowIn", "").toString().split(";", Qt::SkipEmptyParts);
                        entry.Actions = s.value(pre+"Actions", "").toString().split(";", Qt::SkipEmptyParts);
                        QString catString = s.value(pre+"Categories", "").toString();
                        QStringList list = s.value(pre+"Categories", "").toStringList();
                        entry.Categories = catString.split(";", Qt::SkipEmptyParts);
                        entry.ActionEntries = {};

                        if(entry.Actions.count() > 0){
                            for(QString ac :entry.Actions){
                                ActionEntry action;
                                pre = QString("Desktop Action %1").arg(ac) + "/";
                                action.Name = s.value(pre+"Name", "").toString();
                                action.Exec = s.value(pre+"Exec", "").toString().replace(rx, "").trimmed();
                                entry.ActionEntries.append(action);
                            }
                        }
                        entries.append(entry);
                    }
                }
            }
        }
    }
    return entries;
}
void AppMenu::createDialog(){
//    if(nullptr != mDialog){
//        delete mDialog;
//    }
    mDialog = new QDialog(this, Qt::Popup);
    auto vbox = new QVBoxLayout(mDialog);

    auto box = new QHBoxLayout;
    box->setSpacing(0);
    vbox->addLayout(box);

    auto leftW = new QWidget(mDialog);

    auto left = new QVBoxLayout(leftW);
    left->setMargin(0);
    left->setSpacing(0);
    auto m = leftW->contentsMargins();
    leftW->setContentsMargins(m.left(), m.top(), 0, m.bottom());

    box->addWidget(leftW);


    auto rightW = new QWidget(mDialog);
    auto right = new QStackedLayout(rightW);

    box->addWidget(rightW);
    QActionGroup * categoryGroup = new QActionGroup(mDialog);
    QAction * a;
    QToolButton * tb;
    for (int i = 0; i < mCategories.count(); ++i) {
        auto cat = mCategories.at(i);
        a = new QAction(QIcon::fromTheme(cat.Icon, QIcon::fromTheme("application-x-desktop")), cat.Label);
        a->setCheckable(true);
        categoryGroup->addAction(a);
        connect(a, &QAction::triggered, this, [=]{
            right->setCurrentIndex(i);
        });
        connect(a, &QAction::hovered, this, [=]{
            a->trigger();
        });
        tb = new QToolButton(mDialog);
        tb->setToolTip("");
        tb->setAutoRaise(true);
        tb->setDefaultAction(a);
        tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        tb->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
        left->addWidget(tb);
    }

    for (int i = 0; i < mCategories.count(); ++i) {
        auto cat = mCategories.at(i).Name;
        auto list = new QListWidget();
        QMap<QListWidgetItem*, Entry> items;
        list->setSelectionMode(QAbstractItemView::SingleSelection);
        //list->setBackgroundRole(QPalette::Button);
        list->setMouseTracking(true);
        for (Entry e : mCategoryMap[cat]){
            if(e.Actions.count() > 0 && mShowAllActions){
                for(ActionEntry a: e.ActionEntries){
                    Entry ae = e;
                    ae.Name = e.Name + " - " + a.Name;
                    ae.Exec = a.Exec;
                    auto aitem = new QListWidgetItem(QIcon::fromTheme(ae.Icon, QIcon::fromTheme("application-x-desktop")), ae.Name);
                    QString adesc = QString(
                        "GenericName: %1\n"\
                        "Comment: %2\n"\
                        "Icon: %3\n"\
                        "Exec: %4\n"\
                        "Categories:%6")
                        .arg(ae.GenericName)
                        .arg(ae.Comment)
                        .arg(ae.Icon)
                        .arg(ae.Exec)
                        .arg(ae.Categories.join(";")
                    );
                    aitem->setToolTip(adesc);
                    list->addItem(aitem);
                    items[aitem] = ae;
                }
            }
            else{
                auto item = new QListWidgetItem(QIcon::fromTheme(e.Icon, QIcon::fromTheme("application-x-desktop")), e.Name);
                QString desc = QString(
                        "GenericName: %1\n"\
                        "Comment: %2\n"\
                        "Icon: %3\n"\
                        "Exec: %4\n"\
                        "Actions: %5\n"\
                        "Categories:%6")
                        .arg(e.GenericName)
                        .arg(e.Comment)
                        .arg(e.Icon)
                        .arg(e.Exec)
                        .arg(e.Actions.join(";"))
                        .arg(e.Categories.join(";"));
                item->setToolTip(desc);
                list->addItem(item);
                items[item] = e;
            }
        }
        connect(list, &QListWidget::itemEntered, this, [=](QListWidgetItem * item){
            list->setCurrentItem(item);
        });
        connect(list, &QListWidget::itemClicked, this, [=](QListWidgetItem * item){
            list->scrollToTop();
            list->setCurrentRow(0);
            launch(items[item]);
            mDialog->reject();
        });
        list->sortItems();
        QShortcut * listShortcut = new QShortcut(list);
        listShortcut->setKey(Qt::Key_Left);
        listShortcut->setContext(Qt::WidgetShortcut);
        connect(listShortcut, &QShortcut::activated, list, [=]{
            list->clearSelection();
            mDialog->setFocus();
        });
        listShortcut = new QShortcut(list);
        listShortcut->setKey(QKeySequence(Qt::Key_Enter));
        listShortcut->setContext(Qt::WidgetShortcut);
        connect(listShortcut, &QShortcut::activated, list, [=]{
            auto currentItem = list->currentItem();
            if(currentItem){
                list->scrollToTop();
                launch(items[currentItem]);
                mDialog->reject();
            }

        });
        listShortcut = new QShortcut(list);
        listShortcut->setKey(QKeySequence(Qt::Key_Return));
        listShortcut->setContext(Qt::WidgetShortcut);
        connect(listShortcut, &QShortcut::activated, list, [=]{
            auto currentItem = list->currentItem();
            if(currentItem){
                list->scrollToTop();
                launch(items[currentItem]);
                mDialog->reject();
            }

        });
        listShortcut = new QShortcut(list);
        listShortcut->setKey(QKeySequence(Qt::Key_Space));
        listShortcut->setContext(Qt::WidgetShortcut);
        connect(listShortcut, &QShortcut::activated, list, [=]{
            auto currentItem = list->currentItem();
            if(currentItem){
                list->scrollToTop();
                launch(items[currentItem]);
                mDialog->reject();
            }

        });
        right->addWidget(list);
    }
    categoryGroup->actions()[0]->trigger();

    //Search

    int i = left->count();
    auto searchBox = new QHBoxLayout;
    auto lbl = new QLabel;
    auto searchIcon = QIcon::fromTheme("system-search", QIcon::fromTheme("application-x-desktop"));
    lbl->setPixmap(searchIcon.pixmap(24, 24));
    mSearchEdit = new QLineEdit;
    auto searchList = new QListWidget();
    QHash<QString, QListWidgetItem*> nameToItem;
    searchBox->addWidget(lbl);
    searchBox->addWidget(mSearchEdit);
    vbox->addLayout(searchBox);
    QMap<QListWidgetItem*, Entry> searchItems;
    searchList->setSelectionMode(QAbstractItemView::SingleSelection);
    searchList->setMouseTracking(true);
    for (Entry e : mEntries){
        auto item = new QListWidgetItem(QIcon::fromTheme(e.Icon, QIcon::fromTheme("application-x-desktop")), e.Name);
        QString desc = QString(
            "GenericName: %1\n"\
            "Comment: %2\n"\
            "Icon: %3\n"\
            "Exec: %4\n"\
            "Actions: %5\n"\
            "Categories:%6")
            .arg(e.GenericName)
            .arg(e.Comment)
            .arg(e.Icon)
            .arg(e.Exec)
            .arg(e.Actions.join(";"))
            .arg(e.Categories.join(";")
        );
        item->setToolTip(desc);
        QString name = e.Name + " " + e.Exec.split(" ")[0];
        nameToItem[name] = item;
        //hiddenList->addItem(item);
        searchItems[item] = e;
        if(e.Actions.count()>0){
            for(ActionEntry a: e.ActionEntries){
                Entry ae = e;
                ae.Name = e.Name + " - " + a.Name;
                ae.Exec = a.Exec;
                auto aitem = new QListWidgetItem(QIcon::fromTheme(ae.Icon, QIcon::fromTheme("application-x-desktop")), ae.Name);
                QString adesc = QString(
                    "GenericName: %1\n"\
                    "Comment: %2\n"\
                    "Icon: %3\n"\
                    "Exec: %4\n"\
                    "Categories:%6")
                    .arg(ae.GenericName)
                    .arg(ae.Comment)
                    .arg(ae.Icon)
                    .arg(ae.Exec)
                    .arg(ae.Categories.join(";")
                );
                aitem->setToolTip(adesc);
                QString name = ae.Name;// + " " + e.Exec.split(" ")[0];
                nameToItem[name] = aitem;

                searchItems[aitem] = ae;
            }
        }
    }
    searchList->sortItems();

    connect(mSearchEdit, &QLineEdit::textChanged, this, [=](QString text){
        qDebug() << "searching" << text << "nameToItem" << nameToItem.size();
        //auto found = hiddenList->findItems(text, Qt::MatchContains);
        //qDebug() << "found" << found.count();
        //searchList->clear();
        while (searchList->count() > 0) {
            searchList->takeItem(0);
        }
        auto actions = categoryGroup->actions();
        for(auto action : actions){
            action->setChecked(false);
        }
        right->setCurrentIndex(i);

        QStringList matches = nameToItem.keys().filter(text, Qt::CaseInsensitive);
        for(auto match : matches){
            searchList->addItem(nameToItem[match]);
        }

        if(searchList->count()>0){
            searchList->sortItems();
            searchList->setCurrentRow(0);
        }
        qDebug() << "list count" <<searchList->count();
    });

    QShortcut * listShortcut = new QShortcut(searchList);
    listShortcut->setKey(Qt::Key_Left);
    listShortcut->setContext(Qt::WidgetShortcut);
    connect(listShortcut, &QShortcut::activated, searchList, [=]{
        searchList->clearSelection();
        mDialog->setFocus();
    });
    listShortcut = new QShortcut(searchList);
    listShortcut->setKey(QKeySequence(Qt::Key_Enter));
    listShortcut->setContext(Qt::WidgetShortcut);
    connect(listShortcut, &QShortcut::activated, searchList, [=]{
        if(searchList->count()>0){
            auto currentItem = searchList->currentItem();
            if(nullptr != currentItem){
                searchList->scrollToTop();
                launch(searchItems[currentItem]);
                mDialog->reject();
            }
        }
    });
    listShortcut = new QShortcut(searchList);
    listShortcut->setKey(QKeySequence(Qt::Key_Return));
    listShortcut->setContext(Qt::WidgetShortcut);
    connect(listShortcut, &QShortcut::activated, searchList, [=]{
        if(searchList->count()>0){
            auto currentItem = searchList->currentItem();
            if(nullptr != currentItem){
                searchList->scrollToTop();
                launch(searchItems[currentItem]);
                mDialog->reject();
            }
        }
    });
    listShortcut = new QShortcut(searchList);
    listShortcut->setKey(QKeySequence(Qt::Key_Space));
    listShortcut->setContext(Qt::WidgetShortcut);
    connect(listShortcut, &QShortcut::activated, searchList, [=]{
        if(searchList->count()>0){
            auto currentItem = searchList->currentItem();
            if(nullptr != currentItem){
                searchList->scrollToTop();
                launch(searchItems[currentItem]);
                mDialog->reject();
            }
        }
    });
    right->addWidget(searchList);

    //////
    auto tool = new QToolBar;
    tool->setToolButtonStyle(Qt::ToolButtonIconOnly);
    //        0 Name;
    //        1 Icon;
    //        2 Exec;
    //        3 Enabled;
    for (auto customAction: mSystemActions){
        if(customAction.Enabled){
            QString iconString = customAction.Icon;
            QIcon icon = QIcon::fromTheme(iconString);
            if(icon.isNull()){
                if(QFile::exists(iconString)){
                    icon = QIcon(iconString);
                }
                else {
                    icon = QIcon::fromTheme("application-x-desktop");
                }
            }

//            if(QIcon::hasThemeIcon(customAction.Icon)){
//                icon = QIcon::fromTheme(customAction.Icon);
//            }else {
//                if(QFile::exists(customAction.Icon)){
//                    icon = QIcon(customAction.Icon);
//                }
//            }
            a = new QAction(icon, customAction.Name);
            connect(a, &QAction::triggered, this, [=] {launchCmd(customAction.Exec);});
            tool->addAction(a);
        }
    }
//    a = new QAction(QIcon::fromTheme("system-log-out"), "Log Off");
//    tool->addAction(a);
//    a = new QAction(QIcon::fromTheme("system-suspend"), "Suspend");
//    tool->addAction(a);
//    a = new QAction(QIcon::fromTheme("system-reboot"), "Restart");
//    tool->addAction(a);
//    a = new QAction(QIcon::fromTheme("system-shutdown"), "Shutdown");
//    tool->addAction(a);
    searchBox->addWidget(tool);

    connect(searchList, &QListWidget::itemEntered, this, [=](QListWidgetItem * item){
        searchList->setCurrentItem(item);
    });
    connect(searchList, &QListWidget::itemClicked, this, [=](QListWidgetItem * item){
        launch(searchItems[item]);
        mDialog->reject();
    });



    //Shortcuts



    QShortcut * s = new QShortcut(mSearchEdit);
    s->setKey(Qt::Key_Up);
    s->setContext(Qt::WidgetShortcut);
    connect(s, &QShortcut::activated, mSearchEdit, [=]{
        if(right->currentIndex() == i){
            right->currentWidget()->setFocus();
            auto listWidget = qobject_cast<QListWidget*>(right->currentWidget());
            int row = listWidget->currentRow();
            row = qMax(0, row - 1);
            listWidget->setCurrentRow(row);
        }
        else{
            auto actions = categoryGroup->actions();
            actions.last()->trigger();
            mDialog->setFocus();
        }
    });
    s = new QShortcut(mSearchEdit);
    s->setKey(Qt::Key_Down);
    s->setContext(Qt::WidgetShortcut);
    connect(s, &QShortcut::activated, mSearchEdit, [=]{
        if(right->currentIndex() == i){
            right->currentWidget()->setFocus();
            auto listWidget = qobject_cast<QListWidget*>(right->currentWidget());
            int row = listWidget->currentRow();
            row = qMin(listWidget->count() -1, row + 1);
            listWidget->setCurrentRow(row);
        }
        else{
            auto actions = categoryGroup->actions();
            actions.at(0)->trigger();
            mDialog->setFocus();
        }
    });



    s = new QShortcut(mSearchEdit);
    s->setKey(QKeySequence(Qt::Key_Enter));
    s->setContext(Qt::WidgetShortcut);
    connect(s, &QShortcut::activated, mSearchEdit, [=]{
        if(searchList->count()>0){
            auto currentItem = searchList->currentItem();
            qDebug() << "Enter" << currentItem->text() << searchItems[currentItem].Name <<searchItems[currentItem].Exec;
            if(nullptr != currentItem){
                searchList->scrollToTop();
                launch(searchItems[currentItem]);
                mDialog->reject();
            }
        }

    });
    s = new QShortcut(mSearchEdit);
    s->setKey(QKeySequence(Qt::Key_Return));
    s->setContext(Qt::WidgetShortcut);
    connect(s, &QShortcut::activated, mSearchEdit, [=]{
        if(searchList->count()>0){
            auto currentItem = searchList->currentItem();
            qDebug() << "Enter" << currentItem->text() << searchItems[currentItem].Name <<searchItems[currentItem].Exec;
            if(nullptr != currentItem){
                searchList->scrollToTop();
                launch(searchItems[currentItem]);
                mDialog->reject();
            }
        }

    });


    s = new QShortcut(mDialog);
    s->setKey(Qt::Key_Up);
    s->setContext(Qt::WidgetShortcut);
    connect(s, &QShortcut::activated, mDialog, [=]{
        auto actions = categoryGroup->actions();
        if(nullptr == categoryGroup->checkedAction()){
            actions.at(0)->trigger();
            return;
        }
        int index = 0;
        for(int i = 0; i < actions.count(); ++i){
            if(actions.at(i)->isChecked()){
                index = i;
                break;
            }
        }
        if(index > 0){
            actions.at(index - 1)->trigger();
        }else{
            mSearchEdit->setFocus();
        }
    });
    s = new QShortcut(mDialog);
    s->setKey(Qt::Key_Down);
    s->setContext(Qt::WidgetShortcut);
    connect(s, &QShortcut::activated, mDialog, [=]{
        auto actions = categoryGroup->actions();
        if(nullptr == categoryGroup->checkedAction()){
            actions.at(0)->trigger();
            return;
        }
        int index = actions.count() -1;
        for(int i = 0; i < actions.count(); ++i){
            if(actions.at(i)->isChecked()){
                index = i;
                break;
            }
        }
        if(index < actions.count() - 1){
            actions.at(index + 1)->trigger();
        }else{
            mSearchEdit->setFocus();
        }
    });

    s = new QShortcut(mDialog);
    s->setKey(Qt::Key_Right);
    s->setContext(Qt::WidgetShortcut);
    connect(s, &QShortcut::activated, mDialog, [=]{
       qDebug() << "right" << leftW->hasFocus() << rightW->hasFocus() << mDialog->hasFocus();
       right->currentWidget()->setFocus();
       qobject_cast<QListWidget*>(right->currentWidget())->setCurrentRow(0);
    });
}

AppMenu::~AppMenu(){

}
