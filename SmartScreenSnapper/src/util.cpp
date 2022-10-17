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

QList<QRect> Util::getAllChildWindowRect(HWND parent)
{
    QList<QRect> list;
    // 获得顶层子窗口
    HWND cWnd = GetTopWindow(parent);
    // 循环取得桌面下的所有子窗口
    RECT r = {0, 0, 0, 0};
    while (cWnd) {
        if (IsWindowVisible(cWnd)) {
            list.append(getAllChildWindowRect(cWnd));
            GetWindowRect(cWnd, &r);
            list.append(QRect(r.left, r.top, r.right - r.left, r.bottom - r.top));
        }
        // 下一个子窗口（z减小）
        cWnd = GetNextWindow(cWnd, GW_HWNDNEXT);
    }
    return list;
}
