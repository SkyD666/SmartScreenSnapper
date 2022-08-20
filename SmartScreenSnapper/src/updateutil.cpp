#include "updateutil.h"
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "const.h"

UpdateUtil::UpdateUtil() {

}

void UpdateUtil::checkUpdate(QNetworkAccessManager *manager) {
    manager->get(QNetworkRequest(QUrl(Const::checkUpdateUrl)));
}

bool UpdateUtil::isNewVersion(QString version) {
    return QApplication::applicationVersion() != version;
}

GitHubRelease* UpdateUtil::getData(QObject *parent, QByteArray raw) {
    GitHubRelease* githubRelease = new GitHubRelease(parent);

    QJsonParseError jsonParseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(raw, &jsonParseError);
    QJsonObject jsonObject = jsonDocument.object();

    QJsonValue name = jsonObject.take("name");
    QJsonValue tagName = jsonObject.take("tag_name");
    QJsonValue htmlUrl = jsonObject.take("html_url");
    QJsonValue body = jsonObject.take("body");
    QJsonValue publishedAt = jsonObject.take("published_at");
    if (name.isUndefined() || tagName.isUndefined() || htmlUrl.isUndefined() ||
            body.isUndefined() || publishedAt.isUndefined()) {
        return nullptr;
    }
    githubRelease->name = name.toString();
    githubRelease->tagName = tagName.toString();
    githubRelease->htmlUrl = htmlUrl.toString();
    githubRelease->body = body.toString();
    githubRelease->publishedAt = publishedAt.toString();

    return githubRelease;
}

GitHubRelease::GitHubRelease(QObject *parent): QObject(parent) {}
