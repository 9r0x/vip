#pragma once

#include <QPushButton>

extern int fd;

class Key : public QPushButton
{
    Q_OBJECT
public:
    explicit Key(int x, int y, int w, int h, int code, QString label);
    int code;
    QString label;

public slots:
    void pressed();
    void released();

private:
    void emitkey(int fd, int type, int code, int val);
};

class ExitKey : public QPushButton
{
    Q_OBJECT
public:
    explicit ExitKey(int x, int y, int w, int h, QString label);
    QString label;

public slots:
    void released();
};