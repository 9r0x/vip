#include <QApplication>
#include <QPushButton>
#include <QScreen>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <linux/uinput.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include "keyboard.h"
#include "key.h"

Keyboard::Keyboard(QWidget *parent) : KXmlGuiWindow(parent)
{
    QString fileName = ":/config/layout";
    totalWidth = 0;
    totalHeight = 0;

    initvdev();

    QSharedPointer<QJsonObject> layout = loadLayout(fileName);
    if (!layout)
    {
        qWarning() << "Invalid Config File";
        exit(1);
    }
    setupUI(layout);

    QJsonArray rows = (*layout)["rows"].toArray();
    int nRows = rows.size();
    double rowSpanSum = 0.0;

    //  pass 1: loop rows to normalize height
    for (int i = 0; i < nRows; ++i)
    {
        double rowSpan = 1.0;
        QJsonObject row = rows[i].toObject();
        if (row.contains("rowSpan"))
        {
            rowSpan = row["rowSpan"].toDouble();
            if (rowSpan <= 0)
            {
                qWarning() << "Invalid Row Span for row" << i;
                exit(1);
            }
        }
        else
            row.insert("rowSpan", rowSpan);
        rowSpanSum += rowSpan;
        rows[i] = row;
    }

    double unitHeight = totalHeight / rowSpanSum;

    //  pass 2: loop rows for creating keys
    int y = 0;
    for (int i = 0; i < nRows; ++i)
    {
        QJsonObject row = rows[i].toObject();
        if (!row.contains("keys"))
        {
            qWarning() << "Invalid Row";
            exit(1);
        }
        QJsonArray rowKeys = row["keys"].toArray();
        int nKeys = rowKeys.size();
        int height = (int)(unitHeight * row["rowSpan"].toDouble());
        double columnSpanSum = 0.0;

        // pass 1: loop keys to normalize width
        for (int j = 0; j < nKeys; ++j)
        {
            QJsonObject keyObject = rowKeys[j].toObject();
            double columnSpan = 1.0;

            if (keyObject.contains("columnSpan"))
            {
                columnSpan = keyObject["columnSpan"].toDouble();
                if (columnSpan <= 0)
                {
                    qWarning() << "Invalid Column Span for (row, key)" << i << j;
                    exit(1);
                }
            }
            else
                keyObject.insert("columnSpan", columnSpan);
            columnSpanSum += columnSpan;
            rowKeys[j] = keyObject;
        }

        double unitWidth = (totalWidth - (nKeys - 1) * 0) / columnSpanSum;

        // pass 2: resize keys by their columnSpan
        // TODO spacing
        int x = 0;
        for (int j = 0; j < nKeys; ++j)
        {
            QJsonObject keyObject = rowKeys[j].toObject();
            int width = (int)(unitWidth * rowKeys[j].toObject()["columnSpan"].toDouble());
            setupKey(&keyObject, x, y, width, height);
            x += width;
        }

        y += height;
    }
}

Keyboard::~Keyboard()
{
    ioctl(fd, UI_DEV_DESTROY);
    ::close(fd);
}

void Keyboard::initvdev()
{
    struct uinput_setup usetup;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    // https://www.kernel.org/doc/html/v6.4/input/event-codes.html
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < 256; i++)
        ioctl(fd, UI_SET_KEYBIT, i);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    // TODO config
    usetup.id.vendor = 0xFFFF;
    usetup.id.product = 0xFFFF;
    strncpy(usetup.name, "Virtual Keyboard", UINPUT_MAX_NAME_SIZE);

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);
}

void Keyboard::setupUI(QSharedPointer<QJsonObject> layout)
{
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();
    int x = 0;
    int y = 0;

    if (layout->contains("totalWidth"))
        totalWidth = (*layout)["totalWidth"].toInt();
    else
        totalWidth = screenSize.width();
    if (layout->contains("totalHeight"))
        totalHeight = (*layout)["totalHeight"].toInt();
    else
        totalHeight = screenSize.height() / 2;
    if (layout->contains("x"))
        x = (*layout)["x"].toInt();
    else
        x = (screenSize.width() - totalWidth) / 2;
    if (layout->contains("y"))
        y = (*layout)["y"].toInt();
    else
        y = screenSize.height() - totalHeight;

    setGeometry(x, y, totalWidth, totalHeight);
}

QSharedPointer<QJsonObject> Keyboard::loadLayout(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Invalid Config File";
        exit(1);
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error == QJsonParseError::NoError)
    {
        QSharedPointer<QJsonObject> layout = QSharedPointer<QJsonObject>::create(doc.object());
        return layout;
    }
    else
    {
        qWarning() << "Invalid JSON";
        exit(1);
    }
}

void Keyboard::setupKey(QJsonObject *keyObject, int x, int y, int w, int h)
{
    int code = 0;
    QString label;

    // TODO additional types, crrently assume .type = key
    // switch over the key types
    QString type = (*keyObject)["type"].toString();

    if (type == "key")
    {

        if (!keyObject->contains("code"))
        {
            qWarning() << "Invalid Key";
            exit(1);
        }
        else
        {
            // cat /usr/include/linux/input-event-codes.h
            code = (*keyObject)["code"].toInt();
            if (code < 0 || code >= KEY_MAX)
            {
                qWarning() << "Invalid Key";
                exit(1);
            }
        }

        // & -> &&
        if (keyObject->contains("label"))
            label = (*keyObject)["label"].toString();
        else
            label = QString::number(code);

        Key *key = new Key(x, y, w, h, code, label);
        keys.append(key);
        key->setParent(this);
    }
    else if (type == "exit")
    {
        if (keyObject->contains("label"))
            label = (*keyObject)["label"].toString();
        else
            label = QString::number(code);

        ExitKey *key = new ExitKey(x, y, w, h, label);
        keys.append(key);
        key->setParent(this);
    }
    else
    {
        qWarning() << "Invalid Key type";
        exit(1);
    }
}