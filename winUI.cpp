#include "winUI.h"
#include "ui_winUI.h"
#include <winuser.h>
#include <QPushButton>

HWND findDesktopWindow()
{
    return FindWindow(_T("ProgMan"), _T("Program Manger"));
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    HWND p = FindWindowEx(hwnd, NULL, _T("SHELLDLL_DefView"), NULL);
    HWND *ret = (HWND*)lParam;
    if(p)
    {
        *ret = FindWindowEx(NULL, hwnd, _T("WorkerW"), NULL);
    }
    return true;
}

HWND getWindow()
{
    HWND progman = FindWindow(_T("ProgMan"), _T("Program Manager"));

    if(progman == NULL)
    {
        qDebug() << "not get desktop interfacce";
    }

    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    HWND wallpaper_hwnd = nullptr;
    EnumWindows(EnumWindowsProc, (LPARAM)&wallpaper_hwnd);
    qDebug() << wallpaper_hwnd;
    return wallpaper_hwnd;
}

void taskBarHide(bool hide)
{
    LPARAM lParam = hide ? ABS_AUTOHIDE : ABS_ALWAYSONTOP;

    APPBARDATA apBar;
    memset(&apBar,0,sizeof(apBar));
    apBar.cbSize = sizeof(apBar);
    apBar.hWnd = FindWindow(_T("Shell_TrayWnd"), NULL);

    if(apBar.hWnd != NULL)
    {
        apBar.lParam = lParam;
        SHAppBarMessage(ABM_SETSTATE, &apBar);
    }

}


WINUI::WINUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WINUI)
{
    ui->setupUi(this);
    
    // taskBarHide(false);

    initSystemTray();

    connect(ui->actionLoad, &QAction::triggered, this, &WINUI::selectFiles);
    connect(ui->wallerpaper, &QPushButton::clicked, this, &WINUI::load);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
}

WINUI::~WINUI()
{    
    delete ui;

    delete gifLabel;
    delete imgLabel;
}

void WINUI::initSystemTray()
{
    pSystemTray = new QSystemTrayIcon(this);
    
    // 創建兩個項目的QMenu
    QMenu *trayIconMenu = new QMenu();

    settingAction = new QAction(QString::fromUtf8("設置"));
    helpAction = new QAction(QString::fromUtf8("幫助"));
    exitAction = new QAction(QString::fromUtf8("離開"));

    trayIconMenu->addAction(settingAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(helpAction);
    trayIconMenu->addAction(exitAction);
    
    pSystemTray->setContextMenu(trayIconMenu);
    pSystemTray->setToolTip(QString::fromUtf8("應用程式"));
    pSystemTray->setIcon(QIcon("./icon/icon.png"));

    connect(pSystemTray , &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason reason){
        if (reason == QSystemTrayIcon::Trigger)
            this->show();
    });
    
    pSystemTray->show();

    pSystemTray->showMessage(QString::fromUtf8("標題"), QString::fromUtf8("內容"));


    connect(exitAction, &QAction::triggered, [=](){
        QApplication::exit(0);
    });
}

void WINUI::selectFiles()
{
    // QStringList dir = QFileDialog::getOpenFileNames(this, tr("Open GIF files"),
    //                                     "./",
    //                                     tr("Image Files(*.bmp)"));
    // qDebug() << dir;

    QFileDialog fileLoad(this);

    fileLoad.setWindowTitle(QStringLiteral("選擇文件"));
    fileLoad.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList formatLists;
    formatLists.append("一般影像(*.jpg *.jpeg *.png *.bmp)");
    formatLists.append("動態影像(*.gif)");
    fileLoad.setFileMode(QFileDialog::ExistingFile);
    fileLoad.setNameFilters(formatLists);

    if(fileLoad.exec() == QFileDialog::Accepted)
    {
        filePaths = fileLoad.selectedFiles();
        qDebug() << filePaths;
    }    
}

bool WINUI::load()
{
    if(filePaths.count() <= 0)
    {
        return false;
    }

    if(filePaths.at(0).endsWith(QStringLiteral(".gif"), Qt::CaseInsensitive))
    {
        createGiFWallPaper(filePaths.at(0));
    }
    else if(filePaths.at(0).endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive)
        ||  filePaths.at(0).endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive)
        ||  filePaths.at(0).endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)
        ||  filePaths.at(0).endsWith(QStringLiteral(".bmp"), Qt::CaseInsensitive))
    {
        createImageWallPaper(filePaths);
    }
    return true;
}

// bool WINUI::eventFilter(QObject *object, QEvent *event)
// {
//     if (object == m_pImageLbl || object == m_pMovieLbl || object == m_pVedioLbl)
//     {
//         switch (event->type())
//         {
//         case QEvent::Show:
//             m_pCharacterLbl->setParent(qobject_cast<QWidget*>(object));
//             if (m_pCharacterVisibleBox->isChecked() && object != m_pVedioLbl)
//                 m_pCharacterLbl->show();
//             break;
//         case QEvent::Hide:
//             m_pCharacterLbl->hide();
//             m_pCharacterLbl->setParent(this);
//             break;
//         case QEvent::Enter:
//             SetParent((HWND)qobject_cast<QWidget*>(object)->winId(), findDeskTopWindow());
//             break;
//         default:
//             break;
//         }
//     }

//     return QWidget::eventFilter(object, event); 
// }

/*
*   採用 QMovie 讀取 GIF 
*/
void WINUI::createGiFWallPaper(const QString &dir)
{
    QMovie *gif = new QMovie(dir);
    gifLabel = new QLabel();

    gifLabel->installEventFilter(this);
    gifLabel->setWindowFlag(Qt::FramelessWindowHint);
    gifLabel->setMovie(gif);
    gifLabel->setScaledContents(true);
    gifLabel->showFullScreen();
    gif->setParent(gifLabel);
    SetParent((HWND)gifLabel->winId(), findDesktopWindow());
    gifLabel->show();
    gif->start();
}


/*
*/
void WINUI::createImageWallPaper(const QStringList &dir)
{
    QPixmap img;
    for(auto path: dir)
    {
        if(img.load(path))
            images.append(img);
    }

    if(images.count() > 0)
    {
        imgLabel = new QLabel();
        imgLabel->installEventFilter(this);
        imgLabel->setWindowFlag(Qt::FramelessWindowHint);
        imgLabel->setScaledContents(true);
        imgLabel->setPixmap(images.at(0));
        imgLabel->showFullScreen();
        // setParent((HWND)imgLabel->winId(), findDesktopWindow());
        imgLabel->show();

        if(images.count() > 1)
        {
            QTimer *timer = new QTimer(imgLabel);

            connect(timer, &QTimer::timeout, [=](){
                imgLabel->setPixmap(images.at(imageIndex));
                imageIndex = ++imageIndex % images.count();
            });

            connect(ui->speedBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int speed){
                if(timer != nullptr)
                {
                    timer->stop();
                    timer->start(speed * 1000);
                }
            });
            timer->start(ui->speedBox->value() * 1000);
        }
    }
}