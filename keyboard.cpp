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

Keyboard::Keyboard(QString fileName, QWidget *parent) : KXmlGuiWindow(parent)
{
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
    setupRows(layout);
    setupCustomKeys(layout);
}

Keyboard::~Keyboard()
{
    ioctl(fd, UI_DEV_DESTROY);
    ::close(fd);
}

void Keyboard::initvdev()
{
    struct uinput_setup usetup;
    struct uinput_abs_setup abs_usetup_x;
    struct uinput_abs_setup abs_usetup_y;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    // https://www.kernel.org/doc/html/v6.4/input/event-codes.html
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < 248; i++)
        ioctl(fd, UI_SET_KEYBIT, i);
    // Without BTN_LEFT, the mouse will not work
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);

    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_ABSBIT, ABS_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);

    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_POINTER);
    ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0xFFFF;
    usetup.id.product = 0xFFFF;
    strncpy(usetup.name, "Virtual Input", UINPUT_MAX_NAME_SIZE);
    ioctl(fd, UI_DEV_SETUP, &usetup);

    // ABS setup is independent
    // Without this, typing would not work
    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();

    memset(&abs_usetup_x, 0, sizeof(abs_usetup_x));
    abs_usetup_x.code = ABS_X;
    abs_usetup_x.absinfo.maximum = screenSize.width();
    ioctl(fd, UI_ABS_SETUP, &abs_usetup_x);

    memset(&abs_usetup_y, 0, sizeof(abs_usetup_y));
    abs_usetup_y.code = ABS_Y;
    abs_usetup_y.absinfo.maximum = screenSize.height();
    ioctl(fd, UI_ABS_SETUP, &abs_usetup_y);

    ioctl(fd, UI_DEV_CREATE);
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

    if (layout->contains("styleSheet"))
        styleSheet = (*layout)["styleSheet"].toString();
}

void Keyboard::setupRows(QSharedPointer<QJsonObject> layout)
{
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
        int x = 0;
        for (int j = 0; j < nKeys; ++j)
        {
            QJsonObject keyObject = rowKeys[j].toObject();
            int width = (int)(unitWidth * rowKeys[j].toObject()["columnSpan"].toDouble());
            setupKey(layout, &keyObject, x, y, width, height);
            x += width;
        }

        y += height;
    }
}

void Keyboard::setupCustomKeys(QSharedPointer<QJsonObject> layout)
{
    QJsonArray customKeys = (*layout)["customKeys"].toArray();
    int nKeys = customKeys.size();
    for (int i = 0; i < nKeys; ++i)
    {
        QJsonObject keyObject = customKeys[i].toObject();
        int x = keyObject["x"].toDouble() * totalWidth;
        int y = keyObject["y"].toDouble() * totalHeight;
        int w = keyObject["w"].toDouble() * totalWidth;
        int h = keyObject["h"].toDouble() * totalHeight;
        if (x < 0 || x >= totalWidth || y < 0 || y >= totalHeight ||
            w <= 0 || w > totalWidth || h <= 0 || h > totalHeight)
        {
            qWarning() << "Invalid Custom Key";
            exit(1);
        }
        setupKey(layout, &keyObject, x, y, w, h);
    }
}

void Keyboard::setupKey(QSharedPointer<QJsonObject> layout, QJsonObject *keyObject,
                        int x, int y, int w, int h)
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

        QString keyStylesheet = "";
        if (keyObject->contains("styleSheet"))
            keyStylesheet = (*keyObject)["styleSheet"].toString();

        RegularKey *key = new RegularKey(x, y, w, h, code, label, keyStylesheet, this);
        keys.append(key);
    }
    else if (type == "mouse")
    {
        QString mouseType;
        int mouseX, mouseY;

        if (!keyObject->contains("mouseType"))
        {
            qWarning() << "Invalid Mouse Key";
            exit(1);
        }
        else
            // Type checking in instantiation
            mouseType = (*keyObject)["mouseType"].toString();

        if (!keyObject->contains("mouseX") || !keyObject->contains("mouseY"))
        {
            qWarning() << "Invalid Mouse Key";
            exit(1);
        }
        else
        {
            mouseX = (int)((*keyObject)["mouseX"].toDouble() * totalWidth);
            mouseY = (int)((*keyObject)["mouseY"].toDouble() * totalHeight);
        }

        if (keyObject->contains("label"))
            label = (*keyObject)["label"].toString();
        else
            label = mouseType;

        QString keyStylesheet = "";
        if (keyObject->contains("styleSheet"))
            keyStylesheet = (*keyObject)["styleSheet"].toString();

        MouseKey *key = new MouseKey(x, y, w, h, mouseType, mouseX, mouseY,
                                     label, keyStylesheet, this);
        keys.append(key);
    }
    else if (type == "special")
    {
        QString keyStylesheet = "";
        int code = (*keyObject)["code"].toInt();

        if (keyObject->contains("label"))
            label = (*keyObject)["label"].toString();
        else
            label = QString::number(code);
        if (keyObject->contains("styleSheet"))
            keyStylesheet = (*keyObject)["styleSheet"].toString();

        switch (code)
        {
        case SPECIAL_EXIT:
        {
            ExitKey *key = new ExitKey(x, y, w, h,
                                       label, keyStylesheet, this);
            keys.append(key);
            break;
        }
        case SPECIAL_REPOSITION:
        {
            RepositionKey *key = new RepositionKey(x, y, w, h,
                                                   label, keyStylesheet, this);
            keys.append(key);
            break;
        }
        case SPECIAL_PLACEHOLDER:
        {
            break;
        }
        default:
        {
            qWarning() << "Invalid Special Key";
            exit(1);
        }
        }
    }
    else if (type == "macro")
    {
        QString keyStylesheet = "";
        int macro = (*keyObject)["macro"].toInt();

        if (keyObject->contains("label"))
            label = (*keyObject)["label"].toString();
        else
            label = macro;
        if (keyObject->contains("styleSheet"))
            keyStylesheet = (*keyObject)["styleSheet"].toString();

        MacroKey *key = new MacroKey(x, y, w, h,
                                     (*layout)["macros"].toArray()[macro].toArray(),
                                     label, keyStylesheet, this);
        keys.append(key);
    }
    else
    {
        qWarning() << "Invalid Key type";
        exit(1);
    }
}