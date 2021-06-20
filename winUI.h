#ifndef WINDOWS_UI
#define WINDOWS_UI
#define SDL_MAIN_HANDLED
#include <iostream>
#include <windows.h>
#include <time.h>
#include <vector>
#include <string>
#include <QMainWindow>
#include <QtWidgets>


#ifdef UNICODE
#define _T(x) L##x
#else
#define _T(x) x
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class WINUI; }
QT_END_NAMESPACE

class WINUI : public QMainWindow
{
    Q_OBJECT

public:
    WINUI(QWidget *parent = nullptr);
    ~WINUI();

// protected:
//     bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::WINUI *ui;
    QLabel *gifLabel;
    QLabel *imgLabel;
    QTimer *timer;
    int imageIndex;
    QStringList filePaths;
    QList<QPixmap> images;


    QSystemTrayIcon *pSystemTray;
    QAction *settingAction;
    QAction *helpAction;
    QAction *exitAction;
    
    
    void createGiFWallPaper(const QString &dir);
    void createImageWallPaper(const QStringList &dir);
    void helpDialog();
    void removeWallPaper();
    void initSystemTray();
    void selectFiles();
    bool load();
};
#endif