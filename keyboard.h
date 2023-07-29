#pragma once

#include <KXmlGuiWindow>
#include "key.h"

class Keyboard : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit Keyboard(QString fileName, QWidget *parent = nullptr);
    ~Keyboard();
    int fd;
    QString styleSheet;

protected:
    QVector<QPushButton *> keys;
    int totalWidth;
    int totalHeight;

    void initvdev();
    QSharedPointer<QJsonObject> loadLayout(QString);
    void setupUI(QSharedPointer<QJsonObject> layout);
    void setupRows(QSharedPointer<QJsonObject> layout);
    void setupCustomKeys(QSharedPointer<QJsonObject> layout);
    void setupKey(QSharedPointer<QJsonObject> layout, QJsonObject *keyObject,
                  int x, int y, int w, int h);
};