#ifndef UTIL_H
#define UTIL_H

#include <QHash>
#include <QRect>
#include <windows.h>


class Util
{
public:
    Util();

    static QString transformTemplateStr(QString templateStr, QHash<QString, QString> kv);

    static QList<QPair<HWND, QRect>> getAllChildWindowRect(HWND parent);
};

#endif // UTIL_H
