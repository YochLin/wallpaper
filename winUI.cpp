#include "winUI.h"
#include "ui_winUI.h"
#include <winuser.h>
#include <QPushButton>

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
    // qDebug() << wallpaper_hwnd;
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
    , ui(new Ui::WINUI), gifLabel(nullptr), imgLabel(nullptr), timer(nullptr)
{
    ui->setupUi(this);
    
    // taskBarHide(false);

    initSystemTray();

    connect(settingAction, &QAction::triggered, this, &WINUI::show);
    connect(helpAction, &QAction::triggered, this, &WINUI::helpDialog);
    connect(exitAction, &QAction::triggered, [=](){
        QApplication::exit(0);
    });
    connect(ui->actionLoad, &QAction::triggered, this, &WINUI::selectFiles);
    connect(ui->wallerpaper, &QPushButton::clicked, this, &WINUI::load);
    connect(ui->actionExit, &QAction::triggered, [=](){
        QApplication::exit(0);
    });
}

WINUI::~WINUI()
{    
    // delete ui;

    // delete gifLabel;
    // delete imgLabel;
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

    pSystemTray->showMessage(QString::fromUtf8("動態桌布"), QString::fromUtf8("可以使用我來更改桌布唷!!"));


    connect(exitAction, &QAction::triggered, [=](){
        QApplication::exit(0);
    });
}

void WINUI::helpDialog()
{
    QMessageBox message(this);

    message.setWindowTitle(QString::fromUtf8("動態桌布"));
    message.setWindowIcon(QIcon("./icon/icon.png"));
    message.setText(QString::fromUtf8("測試~~\n"
                                      "這是一款簡易的動態桌布程式，目前只能應用在windows上\n"
                                      "透過 load 就能將想要的影像放入桌布，多選取影像即能有動態效果"));
    message.exec();

}

void WINUI::removeWallPaper()
{
    delete gifLabel;
    delete imgLabel;
    delete timer;

    images.clear();
    imageIndex = 0;

    gifLabel = nullptr;
    imgLabel = nullptr;
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
    fileLoad.setFileMode(QFileDialog::ExistingFiles);
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

    removeWallPaper();

    if(filePaths.at(0).endsWith(QStringLiteral(".gif"), Qt::CaseInsensitive))
    {
        createGiFWallPaper(filePaths.at(0));
        ui->speedBox->setEnabled(false);
    }
    else if(filePaths.at(0).endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive)
        ||  filePaths.at(0).endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive)
        ||  filePaths.at(0).endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)
        ||  filePaths.at(0).endsWith(QStringLiteral(".bmp"), Qt::CaseInsensitive))
    {
        createImageWallPaper(filePaths);
        ui->speedBox->setEnabled(true);
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
    SetParent((HWND)gifLabel->winId(), getWindow());
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
        imgLabel->setPixmap(images.at(0));
        imgLabel->setScaledContents(true);
        imgLabel->showFullScreen();
        SetParent((HWND)imgLabel->winId(), getWindow());
        imgLabel->show();

        timer = new QTimer();
        if(images.count() > 1)
        {
            

            connect(timer, &QTimer::timeout, [=](){
                imgLabel->setPixmap(images.at(imageIndex));
                imageIndex = ++imageIndex % images.count();
            });

            connect(ui->speedBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int speed){
                if(timer != nullptr)
                {
                    timer->stop();
                    timer->start(speed * 10);
                }
            });
            timer->start(ui->speedBox->value() * 10);
        }
    }
}