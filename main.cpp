#include <QApplication>
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

    Keyboard *keyboard = new Keyboard();
    keyboard->show();

    return app.exec();
}