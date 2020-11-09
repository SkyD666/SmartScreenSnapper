#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QAbstractNativeEventFilter>

class Application: public QApplication, public QAbstractNativeEventFilter
{
public:
    Application(int argc, char *argv[]);

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};

#endif // APPLICATION_H
