#include "snapbypointgraphicsscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPolygonItem>
#include <QRegion>
#include <QDebug>
#include <windows.h>
#include "src/snapfrompointdialog.h"
#include "src/util.h"

SnapByPointGraphicsScene::SnapByPointGraphicsScene(QWidget *parent) :
    QGraphicsScene(parent),
    rectItem(nullptr),
    windowRectList(QList<QRect>())
{
    // 获得桌面窗口
    HWND desktopWnd = GetDesktopWindow();
    // 获得顶层子窗口
    HWND cWnd = GetTopWindow(desktopWnd);
    // 循环取得桌面下的所有子窗口
    RECT r = {0, 0, 0, 0};
    HWND window = (HWND)(parent->winId());
    while (cWnd) {
        if (IsWindowVisible(cWnd) && cWnd != window) {
            windowRectList.append(Util::getAllChildWindowRect(cWnd));
            GetWindowRect(cWnd, &r);
            windowRectList.append(QRect(r.left, r.top, r.right - r.left, r.bottom - r.top));
        }

        // 下一个子窗口（z减小）
        cWnd = GetNextWindow(cWnd, GW_HWNDNEXT);
    }
}

SnapByPointGraphicsScene::~SnapByPointGraphicsScene()
{

}


void SnapByPointGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton) {
        emit rightButtonClicked();
    } else if (event->buttons() & Qt::LeftButton) {
        emit captured(rectItem->rect());
    }
}

void SnapByPointGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    for (auto r : windowRectList) {
        if (r.contains(event->scenePos().toPoint())) {
            if (rectItem) {
                rectItem->setRect(r);
            } else {
                rectItem = addRect(r, QPen(QBrush(Qt::red), 5));
            }
            break;
        }
    }
}
