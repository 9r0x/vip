#include <QTouchEvent>
#include <QDebug>
#include <QJsonArray>
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

void Key::emitKey(int fd, int type, int code, int val)
{
    struct input_event ie;
    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    ssize_t bytes_written = write(fd, &ie, sizeof(ie));
    if (bytes_written < 0)
    {
        qWarning() << "write() failed";
        exit(1);
    }
}

#define KEYBOARD_FD (((Keyboard *)parentWidget())->fd)
#define EMIT_SYN emitKey(KEYBOARD_FD, EV_SYN, SYN_REPORT, 0)

RegularKey::RegularKey(int x, int y, int w, int h, int code,
                       QString label, QString stylesheet, QWidget *parent)
    : Key(x, y, w, h, label, stylesheet, parent)
{
    this->code = code;
}

void RegularKey::pressed([[maybe_unused]] QEvent *event)
{
    emitKey(KEYBOARD_FD, EV_KEY, code, 1);
    EMIT_SYN;
}

void RegularKey::released([[maybe_unused]] QEvent *event)
{
    emitKey(KEYBOARD_FD, EV_KEY, code, 0);
    EMIT_SYN;
}

MouseKey::MouseKey(int x, int y, int w, int h, QString mouseType, int mouseX, int mouseY,
                   QString label, QString stylesheet, QWidget *parent)
    : Key(x, y, w, h, label, stylesheet, parent)
{
    if (mouseType == "abs")
        this->mouseType = EV_ABS;
    else if (mouseType == "rel")
        this->mouseType = EV_REL;
    else
    {
        qWarning() << "Invalid mouse type";
        exit(1);
    }
    this->mouseX = mouseX;
    this->mouseY = mouseY;
}

void MouseKey::released([[maybe_unused]] QEvent *event)
{
    if (mouseType == EV_ABS)
    {
        // Uinput seems to not re-send events that does not alter existing value
        // though other devices(mouse, track pad, etc) may have changed it
        // Use (0,0) to refresh it
        emitKey(KEYBOARD_FD, EV_ABS, ABS_X, 0);
        emitKey(KEYBOARD_FD, EV_ABS, ABS_Y, 0);
    }
    // REL_X = ABS_X = 0x00
    // REL_Y = ABS_Y = 0x01
    emitKey(KEYBOARD_FD, mouseType, 0, mouseX);
    emitKey(KEYBOARD_FD, mouseType, 1, mouseY);
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

MacroKey::MacroKey(int x, int y, int w, int h, QJsonArray macro,
                   QString label, QString stylesheet, QWidget *parent)
    : Key(x, y, w, h, label, stylesheet, parent)
{
    this->macro = macro;
}

void MacroKey::released([[maybe_unused]] QEvent *event)
{
    processMacro(macro, 0, macro.size());
}

void MacroKey::processMacro(const QJsonArray &p, int start, int n)
{
    if (start + n > p.size())
        qWarning() << "MacroKey::processMacro() : Invalid macro size";

    int i = start;
    while (i < start + n)
    {
        QJsonArray action = p[i].toArray();
        int size = action.size();
        switch (size)
        {
        case 1:
        {
            usleep(action[0].toInt());
            break;
        }
        case 2:
        {
            int lines = action[0].toInt();
            int repeat = action[1].toInt();
            for (int j = 0; j < repeat; j++)
                processMacro(p, i + 1, lines);
            i += lines;
            break;
        }
        case 3:
        {
            int type = action[0].toInt();
            int code = action[1].toInt();
            int val = action[2].toInt();
            emitKey(KEYBOARD_FD, type, code, val);
            break;
        }
        default:
        {
            qWarning() << "Invalid macro size at index" << i;
            exit(1);
        }
        }
        i++;
    }
}