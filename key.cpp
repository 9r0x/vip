#include <QTouchEvent>
#include <QDebug>

#include <linux/uinput.h>
#include <unistd.h>
#include "keyboard.h"
#include "key.h"

Key::Key(int x, int y, int w, int h, QString label, QString stylesheet, QWidget *parent)
    : QPushButton(parent)
{
    this->label = label;

    setAttribute(Qt::WA_AcceptTouchEvents);

#ifdef MOUSE_EVENT
    connect(this, SIGNAL(pressed()),
            this, SLOT(mousePressed()));
    connect(this, SIGNAL(released()),
            this, SLOT(mouseReleased()));
#endif

    // QPushButton:hover {border:none;} removes the border around the touched key
    setStyleSheet("QPushButton {" + ((Keyboard *)parentWidget())->styleSheet + stylesheet +
                  "} QPushButton:hover {border:none;}");
    setGeometry(x, y, w, h);
    setText(label);
    show();
}

bool Key::event(QEvent *event)
{

    // The design permits, though discourages, mouse event handling
    if (event->type() == QEvent::TouchBegin)
    {
        pressed(event);
        return true;
    }
    else if (event->type() == QEvent::TouchUpdate)
    {
        updated(event);
        return true;
    }
    else if (event->type() == QEvent::TouchEnd)
    {
        released(event);
        return true;
    }

    return QPushButton::event(event);
}

#ifdef MOUSE_EVENT
void Key::mousePressed()
{
    pressed(NULL);
}

void Key::mouseReleased()
{
    released(NULL);
}
#endif

void Key::pressed([[maybe_unused]] QEvent *event)
{
}
void Key::updated([[maybe_unused]] QEvent *event) {}
void Key::released([[maybe_unused]] QEvent *event) {}

RegularKey::RegularKey(int x, int y, int w, int h, int code,
                       QString label, QString stylesheet, QWidget *parent)
    : Key(x, y, w, h, label, stylesheet, parent)
{
    this->code = code;
}

void RegularKey::emitkey(int fd, int type, int code, int val)
{
    struct input_event ie;
    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    write(fd, &ie, sizeof(ie));
}

#define KEYBOARD_FD (((Keyboard *)parentWidget())->fd)
#define EMIT_SYN emitkey(KEYBOARD_FD, EV_SYN, SYN_REPORT, 0)

void RegularKey::pressed([[maybe_unused]] QEvent *event)
{
    emitkey(KEYBOARD_FD, EV_KEY, code, 1);
    EMIT_SYN;
}

void RegularKey::released([[maybe_unused]] QEvent *event)
{
    emitkey(KEYBOARD_FD, EV_KEY, code, 0);
    EMIT_SYN;
}

ExitKey::ExitKey(int x, int y, int w, int h,
                 QString label, QString stylesheet, QWidget *parent)
    : Key(x, y, w, h, label, stylesheet, parent)
{
}

void ExitKey::released([[maybe_unused]] QEvent *event)
{
    exit(0);
}

RepositionKey::RepositionKey(int x, int y, int w, int h,
                             QString label, QString stylesheet, QWidget *parent)
    : Key(x, y, w, h, label, stylesheet, parent)
{
}

#define TOUCH_POS ((QTouchEvent *)event)->touchPoints().first().lastScreenPos()

void RepositionKey::pressed(QEvent *event)
{
    prevTouchY = TOUCH_POS.y();
}

void RepositionKey::updated(QEvent *event)
{
    qreal currentTouchY = TOUCH_POS.y();
    int deltaY = currentTouchY - prevTouchY;
    prevTouchY = currentTouchY;

    parentWidget()->move(parentWidget()->x(), parentWidget()->y() + deltaY);
}