#include "util.h"

#include <QStringBuilder>

Util::Util()
{

}

QString Util::transformTemplateStr(QString templateStr, QHash<QString, QString> kv)
{
    QString finalStr;
    QString key;
    bool left = false;
    for (QChar c : templateStr) {
        if (c == '<') {
            if (left) return finalStr;
            left = true;
        } else if (c == '>') {
            finalStr.append(kv[key]);
            key.clear();
            left = false;
        } else if (left) {
            key.append(c);
        } else {
            finalStr.append(c);
        }
    }
    return finalStr;
}
