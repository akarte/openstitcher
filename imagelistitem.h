#ifndef IMAGELISTITEM_H
#define IMAGELISTITEM_H

#include <QListWidgetItem>

class ImageListItem : public QListWidgetItem
{
public:
    ImageListItem();
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
};

#endif // IMAGELISTITEM_H
