#include <QApplication>
#include <QDir>
#include <KLocalizedString>
#include <unistd.h>
#include "keyboard.h"

int main(int argc, char *argv[])
{
    if (setuid(0) == -1)
    {
        qWarning("setuid(0) failed");
        return 1;
    }
    unsetenv("XDG_RUNTIME_DIR");
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("vip");

    QString configRoot = "/etc/vip.d";
    QString fileName = QDir(configRoot).entryList({"*.json"}).first();
    qDebug() << "Using" << configRoot + "/" + fileName;
    Keyboard *keyboard = new Keyboard(configRoot + "/" + fileName);
    keyboard->show();

    return app.exec();
}