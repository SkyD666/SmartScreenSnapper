#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include "screenshothelper.h"
#include <QCloseEvent>
#include <QListWidgetItem>
#include <QMdiSubWindow>
#include <QMenu>
#include <QUndoStack>

namespace Ui {
class MdiWindowWidget;
}

class MdiWindow : public QMdiSubWindow {
    Q_OBJECT

signals:
    void zoom(int n);

    void onClose();

public:
    enum { MdiWindowRole = Qt::UserRole + 1 };

    MdiWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    ~MdiWindow();

    void setLayers(QList<QPair<QPixmap, QPoint>> layers);

    void setListItemName(QString name);

    QListWidgetItem& getListItem();

    QString getName();

    void setName(QString name);

    bool isSaved();

    void setSaved(bool saved);

    void setScale(double scale);

    double getScale();

    QPixmap getPixmap();

    bool saveByDialog();

    bool saveByPath(QString filePath);

    ScreenShotHelper::ShotType getShotType() const;

    void setShotType(ScreenShotHelper::ShotType newShotType);

    QUndoStack* getUndoStack() const;

private:
    Ui::MdiWindowWidget* ui;

    qreal initDpi = devicePixelRatio();

    QListWidgetItem listItem;

    QWidget* containerWidget;

    QString name;

    bool saved;

    double imageScale;

    ScreenShotHelper::ShotType shotType;

    QMenu* contextMenu;

    void initActions();

    QUndoStack* undoStack;

protected:
    void closeEvent(QCloseEvent* event) override;

    void contextMenuEvent(QContextMenuEvent* event) override;
};

#endif // MDIWINDOW_H
