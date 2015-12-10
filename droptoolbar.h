#ifndef DROPTOOLBAR_H
#define DROPTOOLBAR_H

#include <QToolBar>

class DropToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit DropToolBar(QWidget *parent = 0);

protected:
    void dragEnterEvent(QDragEnterEvent * event);
    void dragMoveEvent(QDragMoveEvent * event);
    void dropEvent(QDropEvent * event);

signals:

public slots:

};

#endif // DROPTOOLBAR_H
