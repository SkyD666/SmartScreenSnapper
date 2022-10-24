#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include <QMdiSubWindow>
#include <QListWidgetItem>
#include <QCloseEvent>

namespace Ui {
class MdiWindowWidget;
}

class MdiWindow : public QMdiSubWindow
{
    Q_OBJECT

signals:
    void zoom(int n);

    void save();

    void close();

public:
    MdiWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    void setPixmap(QPixmap pixmap);

    void setListItemName(QString name);

    QListWidgetItem & getListItem();

    QString getName();

    void setName(QString name);

    bool isSaved();

    void setSaved(bool saved);

    void setScale(double scale);

    double getScale();

    QPixmap getPixmap();

    bool saveByDialog();

    bool saveByPath(QString filePath);

private:
    Ui::MdiWindowWidget *ui;

    QListWidgetItem listItem;

    QWidget *containerWidget;

    QString name;

    bool saved;

    double imageScale;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MDIWINDOW_H
