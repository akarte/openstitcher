#include "panzoomview.h"
#include "stitchimageitem.h"

//Qt includes
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTextStream>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QGuiApplication>

#include <QPropertyAnimation>

#include "mainwindow.h"

PanZoomView::PanZoomView(QWidget* parent) : QGraphicsView(parent) {

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //Set-up the scene
    QGraphicsScene* scene = new QGraphicsScene(this);
    setScene(scene);

    setMouseTracking(true);

    //Use ScrollHand Drag Mode to enable Panning
    setDragMode(ScrollHandDrag);
    // there is no dragginG when initializing UI
    DRAGGING = false;
    // Init collections
    selectedlist = new QList<StitchImageItem*>();

    // Init rectangle color and opacity
    selectionRect = new QGraphicsRectItem();
    selectionRect->setBrush(* new QBrush(Qt::blue));
    selectionRect->setOpacity(0.35);

    // Apparently useless
    setFocus();
}

PanZoomView::~PanZoomView()
{
    delete scene();
}

void PanZoomView::keyPressSender(QKeyEvent *event)
{
    PanZoomView::keyPressEvent(event);
}

void PanZoomView::keyReleaseSender(QKeyEvent *event)
{
    PanZoomView::keyReleaseEvent(event);
}

/**
  * Zoom the view in and out.
  */
void PanZoomView::wheelEvent(QWheelEvent* event) {

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Scale the view / do the zoom
    double scaleFactor = 1.15;
    if(event->delta() > 0) {
        // Zoom in
        scale(scaleFactor, scaleFactor);
    } else {
        // Zooming out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
    // Don't call superclass handler here
    // as wheel is normally used for moving scrollbars
}

void PanZoomView::initScene()
{
    DRAGGING = false;
    // clear elements
    selectedlist->clear();
    // For every item inside, check if are StitchImageItems
    foreach(QGraphicsItem *item, scene()->items()){
        if(StitchImageItem* v = dynamic_cast<StitchImageItem*>(item)){
            scene()->removeItem(v);
        }
    }
}

void PanZoomView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    if(dragMode()== ScrollHandDrag) {return;}
    // I don't care for right click
    if (event->buttons() == Qt::RightButton) {return;}
    // I only care for ctrl+left click
    if(!event->modifiers().testFlag(Qt::ControlModifier)) {return;}

    // If left click and ctrl is pressed, check if dragging or updating selectionRect size
    if(DRAGGING){
        QPoint pos = mapToScene(event->pos().x(), event->pos().y()).toPoint();
        foreach(StitchImageItem *item, *selectedlist){
            QPoint newpos, oldpos = item->pos().toPoint();

            newpos.setX(oldpos.x() + (pos.x() - offset.x()));
            newpos.setY(oldpos.y() + (pos.y() - offset.y()));

            item->setPos(newpos);
        }
        offset = mapToScene(event->pos().x(), event->pos().y()).toPoint();
    } else {
        QPoint touchPoint = mapToScene(event->pos().x(), event->pos().y()).toPoint();
        QRectF rect = selectionRect->rect();
        int x = rect.x();
        int y = rect.y();
        int w = touchPoint.x() - x;
        int h = touchPoint.y() - y;
        float o = selectionRect->opacity();
        updateSelectionRectangle(x, y, w, h, o);
    }
}

void PanZoomView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    // Clear selections if right click is trigger
    if(event->button() == Qt::RightButton){
        if(!selectedlist->isEmpty()){
            selectedlist->clear();
        }
    } else {
        if(dragMode()== ScrollHandDrag) {return;}
        // I don't care if not ctrl+click
        if(!event->modifiers().testFlag(Qt::ControlModifier)) {return;}

        // Add selection rect if not exist before do changes with it
        if(!scene()->items().contains(selectionRect)) scene()->addItem(selectionRect);

        // If I want to move objects, then check if in current position
        // there's something selected
        // If someting is selected: change mouse state to "let's drag!"
        // else, continue doing stuffs
        if(shouldDrag(event->pos())){
            offset = mapToScene(event->pos().x(), event->pos().y()).toPoint();
            DRAGGING = true;
        } else {
            DRAGGING = false;
            // Get touch point and update/start selecting
            QPoint touchPoint = mapToScene(event->pos().x(), event->pos().y()).toPoint();
            int x = touchPoint.x();
            int y = touchPoint.y();
            updateSelectionRectangle(x, y, 0, 0, 0.35f);
        }
    }
}

void PanZoomView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if(dragMode()== ScrollHandDrag) {return;}
    // I don't care for right click
    if (event->buttons() == Qt::RightButton) {return;}
    // I only care for ctrl+left click
    if(!event->modifiers().testFlag(Qt::ControlModifier)) {return;}

    // Get touch point
    QPoint touchPoint = mapToScene(event->pos().x(), event->pos().y()).toPoint();
    int x = touchPoint.x();
    int y = touchPoint.y();
    // If left click and ctrl is pressed, check if dragging or updating selectionRect size
    if(!DRAGGING){
        // Select objects inside current rect
        doSelectInsideRect(event->pos());
        // Init rectangle boundaries
        updateSelectionRectangle(x, y, 0, 0, 0);
    } else {
        // clear elements
        selectedlist->clear();
        // restore transparency
        restoreOpacity();
    }
    scene()->setSceneRect(calculateSceneBoundingRect());

    DRAGGING = false;
}

QRectF PanZoomView::calculateSceneBoundingRect()
{
    QRectF rect;
    bool isfirst = true;
    if(scene()->items().empty()) {return rect;}
    foreach(QGraphicsItem *item, scene()->items()){
        if(StitchImageItem* v = dynamic_cast<StitchImageItem*>(item)){
            if(isfirst) {
                isfirst = false;
                rect = item->sceneBoundingRect();
                continue;
            }
            QRectF irect = item->sceneBoundingRect();
            if(rect.left() > irect.left()){
                rect.setX(irect.x());
            }
            if(rect.right() < irect.right()){
                rect.setRight(irect.right());
            }
            if(rect.top() > irect.top()){
                rect.setY(irect.y());
            }
            if(rect.bottom() < irect.bottom()){
                rect.setBottom(irect.bottom());
            }
        }
    }
    return rect;
}


bool PanZoomView::shouldDrag(const QPoint &pos)
{
    if(selectedlist->empty()) {return false;}

    QGraphicsItem *item = itemAt(pos);
    if(StitchImageItem* v = dynamic_cast<StitchImageItem*>(item)){
        // If are, set opacity to half
        if(selectedlist->contains(v)){
            return true;
        }
    }
    return false;
}

void PanZoomView::doSelectInsideRect(const QPoint &pos)
{
    // Init rect
    QRectF* rect = new QRectF();
    int x, y, w, h;
    x = selectionRect->rect().x();
    y = selectionRect->rect().y();
    w = selectionRect->rect().width();
    h = selectionRect->rect().height();

    // If no dragging only click, then select item
    if(w == 0 && h == 0){
        QGraphicsItem *item = itemAt(pos);
        if(StitchImageItem* v = dynamic_cast<StitchImageItem*>(item)){
            // Move item to top:
            scene()->removeItem(v);
            scene()->addItem(v);
            // If are, set opacity to half
            if(!selectedlist->contains(v)){selectedlist->push_back(v);}
            v->setOpacity(0.5);
        } else {
            // clear elements
            selectedlist->clear();
            // restore transparency
            restoreOpacity();
        }
        delete rect; rect = NULL;
        return;
    }

    // Set position, width and height based on selectionrect
    if(w < 0){
        rect->setX(x - (w * -1));
        rect->setWidth(w*-1);
    } else {
        rect->setX(x);
        rect->setWidth(w);
    }

    if(h < 0){
        rect->setY(y - (h * -1));
        rect->setHeight(h*-1);
    } else {
        rect->setY(y);
        rect->setHeight(h);
    }
    // Check elements that intersect with current selectionrect
    QList<QGraphicsItem*> itemsselected = scene()->items(*rect, Qt::IntersectsItemShape);
    delete rect; rect = NULL;
    // restore transparency
    restoreOpacity();

    // If no item are inside, clear selection
    if (itemsselected.isEmpty()){return;}
    // For every item inside, check if are StitchImageItems
    foreach(QGraphicsItem *item, itemsselected){
        if(StitchImageItem* v = dynamic_cast<StitchImageItem*>(item)){
            // If are, set opacity to half
            if(!selectedlist->contains(v)) selectedlist->push_back(v);
            v->setOpacity(0.5);
        }
    }
}

void PanZoomView::restoreOpacity(){
    foreach(QGraphicsItem *item, items()){
        if(StitchImageItem* v = dynamic_cast<StitchImageItem*>(item)){
            // If are, set opacity to 1
            v->setOpacity(1);
        }
    }
}

void PanZoomView::updateSelectionRectangle(int x, int y,
                                           int w, int h, const float o)
{
    QRectF* r = new QRectF(x, y, w, h);
    // Update r in view
    selectionRect->setRect(*r);
    selectionRect->setOpacity(o);
    delete r;
    r = NULL;
}

void PanZoomView::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
    if ( (event->key() == Qt::Key_Control) ){
        setDragMode(NoDrag);
    } else {
        setDragMode(ScrollHandDrag);
    }
}

void PanZoomView::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsView::keyReleaseEvent(event);
    if ( (event->key() == Qt::Key_Control) ){
        setDragMode(ScrollHandDrag);
    } else {
        setDragMode(NoDrag);
    }
}
