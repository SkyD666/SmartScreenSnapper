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
    windowRectList(QList<QPair<HWND, QRect>>())
{
    // 获得桌面窗口
    HWND desktopWnd = GetDesktopWindow();
    // 获得顶层子窗口
    HWND cWnd = GetTopWindow(desktopWnd);
    // 循环取得桌面下的所有子窗口
    RECT r = {0, 0, 0, 0};
    HWND window = (HWND)(parent->winId());
    while (cWnd) {
        long exStyle = GetWindowLong(cWnd, GWL_EXSTYLE);
        if (IsWindowVisible(cWnd) && !(exStyle & WS_EX_TRANSPARENT)
                && !(exStyle & WS_EX_LAYERED)
                && !(exStyle & WS_EX_NOINHERITLAYOUT)
                && cWnd != window) {
            windowRectList.append(Util::getAllChildWindowRect(cWnd));
            GetWindowRect(cWnd, &r);
            windowRectList.append(qMakePair(cWnd, QRect(r.left, r.top, r.right - r.left, r.bottom - r.top)));
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
        if (r.second.contains(event->scenePos().toPoint())) {
            if (rectItem) {
                rectItem->setRect(r.second);
            } else {
                rectItem = addRect(r.second, QPen(QBrush(Qt::red), 5));
            }
            break;
        }
    }
}
