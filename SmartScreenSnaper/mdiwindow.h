#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include <QMdiSubWindow>
#include <QListWidgetItem>
#include <QCloseEvent>

class MdiWindow : public QMdiSubWindow
{
    Q_OBJECT

signals:
    void zoom(int n);

    void save();

    void close();

public:
    MdiWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    void wheelEvent(QWheelEvent *event) override;

    void setListItemName(QString name);

    QListWidgetItem & getListItem();

    QString getName();

    void setName(QString name);

    bool isSaved();

    void setSaved(bool saved);

    void setXScale(double xScale);

    double getXScale();

    void setYScale(double yScale);

    double getYScale();

private:
    QListWidgetItem listItem;

    QString name;

    bool saved;

    //目前xScale和yScale相同
    double xScale;

    double yScale;

    QWidget* parent;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MDIWINDOW_H
