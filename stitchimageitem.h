#ifndef STITCHIMAGEITEM_H
#define STITCHIMAGEITEM_H

#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QDragEnterEvent>

class StitchImageItem : public QObject, public QGraphicsPixmapItem
{
  Q_OBJECT
  //Q_PROPERTY (qreal opacity READ opacity WRITE setOpacity)

public:
    explicit StitchImageItem(const QPixmap &pixmap);
    ~StitchImageItem();
};

#endif // STITCHIMAGEITEM_H
