#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include <QMdiSubWindow>
#include <QListWidgetItem>
#include <QCloseEvent>

class MdiWindow : public QMdiSubWindow
{
    Q_OBJECT

signals:

    void save();

    void close();

public:
    MdiWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    void setListItemName(QString name);

    QListWidgetItem & getListItem();

    QString getName();

    void setName(QString name);

    bool isSaved();

    void setSaved(bool saved);
private:
    QListWidgetItem listItem;

    QString name;

    bool saved;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // MDIWINDOW_H
