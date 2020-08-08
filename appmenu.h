#ifndef APPMENU_H
#define APPMENU_H

#include <QtWidgets>
#include <QtDBus>

struct CategoryData{
    QString Name;
    QString Label;
    QString Icon;
};

struct ActionEntry{
    QString Name;
    QString Exec;
};

struct Entry {
    QString Name;
    QString GenericName;
    QString Comment;
    QString Exec;
    QString Path;
    QString Icon;
    QString StartupWMClass;
    bool Terminal;
    QStringList OnlyShowIn;
    QStringList Actions;
    QStringList Categories;
    QList<ActionEntry> ActionEntries;
};

Q_DECLARE_METATYPE(CategoryData)
Q_DECLARE_METATYPE(ActionEntry)
Q_DECLARE_METATYPE(Entry)
struct CustomAction
{
    QString Name;
    QString Icon;
    QString Exec;
    bool Enabled;
};
Q_DECLARE_METATYPE(CustomAction);


class PanelQt;
class AppMenu : public QToolButton
{
    Q_OBJECT

public:
    AppMenu(PanelQt * panel);
    ~AppMenu();

    int id = qRegisterMetaType<CategoryData>();
    QSettings * mSettings;
    PanelQt * mPanel;
    QMenu * mMenu;
    QDialog * mDialog;
    QLineEdit * mSearchEdit;
    QStringList mDirs;
    QList<CategoryData> mCategories;
    QList<CategoryData> mDefaultCategories;
    QMap<QString,bool> mOrderedCategories;

    QStringList mListSystemActions;
    QList<CustomAction> mSystemActions;
    QList<CustomAction> mDefaultSystemActions;
    //QList<QStringList> mSystemActions;
    QHash<QString, QList<Entry>> mCategoryMap;
    QStringList mShownCategories;
    QList<Entry> mEntries;
    bool mShowAllActions;
    QString mTerminalCommand;
    QIcon mDefaultIcon;


    double mHScale;
    double mVScale;
    QString mWhichIcon;
    QString mIconName;
    QString mIconPath;
    int mIconSize;
    int mButtonWidth;

    QSize mIconQSize;

    void changeSystemActions();
    void loadSystemActions();
    void changeTerminalCommand(QString cmd);
    void changeShowAllActions(bool s);
    void changeShownCategories(QString s, bool add);
    void changeHScale(double s);
    void changeVScale(double s);
    void changeWhichIcon(QString w);
    void changeIconName(QString n);
    void changeIconPath(QString p);
    void changeIconSize(int s);
    void changeButtonWidth(int w);
    void changeIcon();

    void showConfig();
    void dialogClosed();
    void createDialog();
    void showDialog();
    void showMenu();
    QList<Entry> findEntries();
    QHash<QString, QList<Entry>> categorizeEntries();
    QHash<QString, QVariant> readDesktopFile(QString p);

    void contextMenuEvent(QContextMenuEvent *event) override;
    void launch(Entry e);
    void launchCmd(QString cmd);
public slots:
    Q_NOREPLY void toggleMenu();
};

#endif // APPMENU_H
