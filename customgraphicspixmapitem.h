#ifndef CUSTOMGRAPHICSPIXMAPITEM_H
#define CUSTOMGRAPHICSPIXMAPITEM_H

#include <QGraphicsPixmapItem>

class CustomGraphicsPixmapItem  : public QObject, public QGraphicsPixmapItem
{
  Q_OBJECT
  Q_PROPERTY (qreal opacity READ opacity WRITE setOpacity)

public:
    explicit CustomGraphicsPixmapItem(const QPixmap &pixmap);

signals:

public slots:
};

#endif // CUSTOMGRAPHICSPIXMAPITEM_H
