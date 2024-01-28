#include "snapbypointgraphicsscene.h"

#include "src/util.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneMouseEvent>
#include <QRegion>
#include <windows.h>

SnapByPointGraphicsScene::SnapByPointGraphicsScene(QWidget* parent)
    : QGraphicsScene(parent)
    , rectItem(nullptr)
    , windowRectList(QList<QPair<HWND, QRect>>())
{
    // 获得桌面窗口
    HWND desktopWnd = GetDesktopWindow();
    // 获得顶层子窗口
    HWND cWnd = GetTopWindow(desktopWnd);
    // 循环取得桌面下的所有子窗口
    RECT r = { 0, 0, 0, 0 };
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

void SnapByPointGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    auto devicePixelRatio = qApp->devicePixelRatio();
    auto newRect = rectItem->rect();
    newRect.setRect(
        newRect.x() * devicePixelRatio,
        newRect.y() * devicePixelRatio,
        newRect.width() * devicePixelRatio,
        newRect.height() * devicePixelRatio);

    if (event->buttons() & Qt::RightButton) {
        emit rightButtonClicked();
    } else if (event->buttons() & Qt::LeftButton) {
        emit captured(newRect);
    }
}

void SnapByPointGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    auto devicePixelRatio = qApp->devicePixelRatio();
    auto newPoint = event->scenePos().toPoint();

    for (auto r : windowRectList) {
        auto newRect = r.second;
        newRect.setRect(
            newRect.x() / devicePixelRatio,
            newRect.y() / devicePixelRatio,
            newRect.width() / devicePixelRatio,
            newRect.height() / devicePixelRatio);

        if (newRect.contains(newPoint)) {
            if (rectItem) {
                rectItem->setRect(newRect);
            } else {
                rectItem = addRect(newRect, QPen(QBrush(Qt::red), 5));
            }
            break;
        }
    }
}
