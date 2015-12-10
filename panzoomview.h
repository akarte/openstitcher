#ifndef PANZOOMVIEW_H
#define PANZOOMVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QList>
#include "stitchimageitem.h"

class PanZoomView : public QGraphicsView
{
public:
    PanZoomView(QWidget* parent = NULL);
    ~PanZoomView();
    void keyPressSender(QKeyEvent* event);
    void keyReleaseSender(QKeyEvent* event);
    void initScene();
    QRectF calculateSceneBoundingRect();

protected:
    //Take over the interaction
    virtual void wheelEvent(QWheelEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

private:
    void updateSelectionRectangle(int x, int y, int w, int h, const float o);
    void doSelectInsideRect(const QPoint &pos);
    void restoreOpacity();
    bool shouldDrag(const QPoint &pos);

    QGraphicsRectItem* selectionRect;
    /// Save offset of every image in selected list
    QPoint offset;
    /// Current items selected
    QList<StitchImageItem*>* selectedlist;

    bool DRAGGING;
};

#endif // PANZOOMVIEW_H
