#include "winUI.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    
    WINUI winui;
    winui.setWindowIcon(QIcon("./icon/icon.png"));
    // winui.show();
    winui.hide();

    return app.exec();
};