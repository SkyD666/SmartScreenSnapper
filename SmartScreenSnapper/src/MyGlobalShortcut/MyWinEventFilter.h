#ifndef MYWINEVENTFILTER_H
#define MYWINEVENTFILTER_H
#include <QAbstractNativeEventFilter>
class MyGlobalShortCut;

class MyWinEventFilter : public QAbstractNativeEventFilter
{
public:
    MyWinEventFilter();
    MyWinEventFilter(MyGlobalShortCut *shortcut);
    ~MyWinEventFilter();

private:
    MyGlobalShortCut *m_shortcut;

public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result);
};

#endif
