#include "droptoolbar.h"
#include "imagelist.h"
#include <QMimeData>
#include <QDragMoveEvent>
#include <QDragEnterEvent>

DropToolBar::DropToolBar(QWidget *parent) :
    QToolBar(parent)
{
    setAcceptDrops(true);
}

void DropToolBar::dragEnterEvent(QDragEnterEvent * event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == (QObject*)this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void DropToolBar::dragMoveEvent(QDragMoveEvent * event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == (QObject*)this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void DropToolBar::dropEvent(QDropEvent * event)
{
    QAction* action = actionAt(event->pos());
    if(!action) event->ignore();

    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        ImageList* imagelist = (ImageList*) event->source();
        imagelist->removeSelectedOperation();
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}
