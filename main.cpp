#include <QApplication>
#include <KLocalizedString>
#include <unistd.h>
#include "keyboard.h"

int main(int argc, char *argv[])
{
    setuid(0);
    unsetenv("XDG_RUNTIME_DIR");
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("vip");

    Keyboard *keyboard = new Keyboard();
    keyboard->show();

    return app.exec();
}