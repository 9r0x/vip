#pragma once

#include <QPushButton>

extern int fd;

/* Use Q_OBJECT if signal+button handler is used */

class Key : public QPushButton
{
#ifdef MOUSE_EVENT
    Q_OBJECT
#endif

public:
    explicit Key(int x, int y, int w, int h,
                 QString label, QString stylesheet, QWidget *parent);
    QString label;

#ifdef MOUSE_EVENT
public slots:
    void mousePressed();
    void mouseReleased();
#endif

protected:
    bool event(QEvent *event) override;
    void emitKey(int fd, int type, int code, int val);
    // Use vitual so that event() call the overriden methods
    virtual void pressed(QEvent *event);
    virtual void updated(QEvent *event);
    virtual void released(QEvent *event);
};

class RegularKey : public Key
{
#ifdef MOUSE_EVENT
    Q_OBJECT
#endif
public:
    explicit RegularKey(int x, int y, int w, int h,
                        int code, QString label, QString stylesheet, QWidget *parent);
    int code;

protected:
    void pressed(QEvent *event) override;
    void released(QEvent *event) override;
};

class MouseKey : public Key
{
#ifdef MOUSE_EVENT
    Q_OBJECT
#endif
public:
    explicit MouseKey(int x, int y, int w, int h, QString mouseType, int mouseX, int mouseY,
                      QString label, QString stylesheet, QWidget *parent);
    int mouseType, mouseX, mouseY;

protected:
    void released(QEvent *event) override;
};

#define SPECIAL_EXIT 0
class ExitKey : public Key
{
#ifdef MOUSE_EVENT
    Q_OBJECT
#endif
public:
    explicit ExitKey(int x, int y, int w, int h,
                     QString label, QString stylesheet, QWidget *parent);

protected:
    void released(QEvent *event) override;
};

#define SPECIAL_REPOSITION 1
class RepositionKey : public Key
{
public:
    explicit RepositionKey(int x, int y, int w, int h,
                           QString label, QString stylesheet, QWidget *parent);

protected:
    qreal prevTouchY;

    void pressed(QEvent *event) override;
    void updated(QEvent *event) override;
};

#define SPECIAL_PLACEHOLDER 2