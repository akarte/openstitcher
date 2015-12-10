#include "imagelist.h"
#include "mainwindow.h"

#include <QListWidget>
#include <QScrollBar>
#include <QList>
#include <QIcon>
#include <QResizeEvent>
#include <QTimer>
#include <QList>
#include <QThread>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QPainter>
#include <QModelIndex>

ImageList::ImageList(QWidget *parent) : QListWidget(parent)
{
    qRegisterMetaType<QVector<int> >("QVector<int>");

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWrapping(true);
    setViewMode(QListView::IconMode);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);

    thumbnails = new QList<QPixmap>;
    mediumsize = new QList<QPixmap>;
    matimages = new QList<Mat>;

    thumbnails_size = QSize(32, 32 / 4 * 3);
    mediumsize_size = QSize(100, 100 / 4 * 3);
    waitTime = 500;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(runInThread()));
    isResizing = false;
}

ImageList::~ImageList()
{
    thumbnails->clear();
    mediumsize->clear();
    matimages->clear();

    delete thumbnails;
    delete mediumsize;
    delete matimages;
    delete timer;
}

void ImageList::runInThread()
{
    if(!isResizing || matimages->empty()) return;
    ImageListThread* imthread = new ImageListThread(this);
    imthread->start();
}

void ImageList::highdefLoad()
{
    if(!isResizing || matimages->empty()) return;
    int w = width() - 10;
    int h = w / 4 * 3;
    setIconSize(QSize(w, h));

    for(int i = 0; i < count(); i++)
    {
        QListWidgetItem* it = item(i);
        it->setIcon(QIcon(mediumsize->at(i).scaled(w,h,Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
    isResizing = false;
}

void ImageList::removeSelectedOperation()
{
    if(selectedItems().size() < 1) return;

    for(int i = 0; i < 1; i++)
    {
        int index = selectedIndexes().at(i).row();
        thumbnails->removeAt(index);
        matimages->removeAt(index);
        mediumsize->removeAt(index);
    }
    qDeleteAll(selectedItems());
}

void ImageList::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);
    if(matimages->empty()) return;

    timer->start(waitTime);

    int w = width() - 10;
    int h = w / 4 * 3;
    setIconSize(QSize(w, h));

    for(int i = 0; i < count(); i++)
    {
        QListWidgetItem* it = item(i);
        it->setIcon(QIcon(thumbnails->at(i).scaled(w,h,Qt::KeepAspectRatio, Qt::FastTransformation)));
    }
    isResizing = true;
}

void ImageList::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
}

void ImageList::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QListWidgetItem* selectedItem = itemAt(event->pos());
    if (!selectedItem)
        return;

    QPixmap pixmap = selectedItem->icon().pixmap(mediumsize_size);
    pixmap.fill(QColor(0, 0, 0, 100));
    QPainter p;
    p.begin(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(0, 0, selectedItem->icon().pixmap(mediumsize_size));
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(pixmap.rect(), QColor(0, 0, 0, 100));
    p.end();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << pixmap << event->pos();

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    //drag->setHotSpot(event->pos());

    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void ImageList::dragEnterEvent(QDragEnterEvent *event)
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

void ImageList::dragMoveEvent(QDragMoveEvent *event)
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////

ImageListThread::ImageListThread(ImageList* imagelist) : QThread(imagelist)
{
    this->imagelist = imagelist;
}

void ImageListThread::run()
{
    imagelist->highdefLoad();
}
