#include <linux/uinput.h>
#include <unistd.h>
#include "keyboard.h"
#include "key.h"

Key::Key(int x, int y, int w, int h, int code, QString label)
    : QPushButton()
{
    this->code = code;
    this->label = label;

    connect(this, SIGNAL(pressed()), this, SLOT(pressed()));
    connect(this, SIGNAL(released()), this, SLOT(released()));

    setGeometry(x, y, w, h);
    setText(label);
    show();
}

void Key::emitkey(int fd, int type, int code, int val)
{
    struct input_event ie;
    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    write(fd, &ie, sizeof(ie));
}

#define KEYBOARD_FD (((Keyboard *)parent())->fd)
#define EMIT_SYN emitkey(KEYBOARD_FD, EV_SYN, SYN_REPORT, 0)

void Key::pressed()
{
    emitkey(KEYBOARD_FD, EV_KEY, code, 1);
    EMIT_SYN;
}

void Key::released()
{
    emitkey(KEYBOARD_FD, EV_KEY, code, 0);
    EMIT_SYN;
}

ExitKey ::ExitKey(int x, int y, int w, int h, QString label)
{
    this->label = label;

    connect(this, SIGNAL(released()), this, SLOT(released()));

    setGeometry(x, y, w, h);
    setText(label);
    show();
}

void ExitKey::released()
{
    exit(0);
}