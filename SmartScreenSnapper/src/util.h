#ifndef UTIL_H
#define UTIL_H

#include <QHash>


class Util
{
public:
    Util();

    static QString transformTemplateStr(QString templateStr, QHash<QString, QString> kv);
};

#endif // UTIL_H
