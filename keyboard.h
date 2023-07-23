#pragma once

#include <KXmlGuiWindow>
#include "key.h"

class Keyboard : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit Keyboard(QWidget *parent = nullptr);
    ~Keyboard();
    int fd;

private:
    QVector<QPushButton *> keys;
    int totalWidth;
    int totalHeight;

    void initvdev();
    QSharedPointer<QJsonObject> loadLayout(QString);
    void setupUI(QSharedPointer<QJsonObject> layout);
    void setupKey(QJsonObject *keyObject,
                  int x, int y, int w, int h);
};