#ifndef COMMONUTIL_H
#define COMMONUTIL_H

#include <QString>
#include <QNetworkAccessManager>

class GitHubRelease;

class UpdateUtil {

public:

    UpdateUtil();

    static void checkUpdate(QNetworkAccessManager *manager);

    static bool isNewVersion(QString version);

    static GitHubRelease* getData(QObject *parent, QByteArray raw);
};


class GitHubRelease: public QObject {
    Q_OBJECT
public:
    GitHubRelease(QObject *parent = nullptr);
    QString tagName;
    QString name;
    QString publishedAt;
    QString htmlUrl;
    QString body;
};
#endif // COMMONUTIL_H
