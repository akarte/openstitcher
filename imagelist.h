#ifndef IMAGELIST_H
#define IMAGELIST_H

#include <QListWidget>
#include <QList>
#include <QIcon>
#include <QThread>
#include <QPoint>

#include "opencv2/opencv.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

class ImageList : public QListWidget
{
    Q_OBJECT
public:
    explicit ImageList(QWidget *parent = 0);
    ~ImageList();

    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void removeSelectedOperation();
    void highdefLoad();

    QList<QPixmap>* thumbnails;
    QList<QPixmap>* mediumsize;
    QList<Mat>* matimages;

    QSize thumbnails_size;
    QSize mediumsize_size;
    int waitTime;
    QTimer *timer;
    bool isResizing;
    QPoint dragStartPosition;
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
signals:

public slots:
    void runInThread();
};
#endif // IMAGELIST_H
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IMAGELISTTHREAD_H
#define IMAGELISTTHREAD_H

class ImageListThread : public QThread
{
    Q_OBJECT
public:
    explicit ImageListThread(ImageList* imagelist);
private:
    ImageList* imagelist;

protected:
    void run();
};
#endif // IMAGELISTTHREAD_H
