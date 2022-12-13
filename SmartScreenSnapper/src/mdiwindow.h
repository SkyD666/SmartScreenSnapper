#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include <QMdiSubWindow>
#include <QListWidgetItem>
#include <QCloseEvent>
#include <QMenu>
#include "screenshothelper.h"

namespace Ui {
class MdiWindowWidget;
}

class MdiWindow : public QMdiSubWindow
{
    Q_OBJECT

signals:
    void zoom(int n);

    void onClose();

public:
    enum { MdiWindowRole = Qt::UserRole + 1 };

    MdiWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    ~MdiWindow();

    void setPixmap(QPixmap pixmap);

    void setListItemName(QString name);

    QListWidgetItem & getListItem();

    QString getName();

    void setName(QString name);

    bool isSaved();

    void setScale(double scale);

    double getScale();

    QPixmap getPixmap();

    bool saveByDialog();

    bool saveByPath(QString filePath);

    ScreenShotHelper::ShotType getShotType() const;

    void setShotType(ScreenShotHelper::ShotType newShotType);

private:

    void setSaved(bool saved);

    Ui::MdiWindowWidget *ui;

    QListWidgetItem listItem;

    QWidget *containerWidget;

    QString name;

    bool saved;

    double imageScale;

    ScreenShotHelper::ShotType shotType;

    QMenu *contextMenu;

    void initActions();

protected:
    void closeEvent(QCloseEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // MDIWINDOW_H
