#ifndef CAMUI_H
#define CAMUI_H

#include <QDialog>
#include <QGraphicsScene>
#include <QThread>

#include "opencv2/opencv.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "panzoomview.h"
#include "customgraphicspixmapitem.h"

using namespace cv;
using namespace std;

class CamUIThread;
namespace Ui {
class CamUI;
}

class CamUI : public QDialog
{
    Q_OBJECT

public:
    explicit CamUI(QWidget *parent = 0);
    ~CamUI();
    static QImage cvMatToQImage( const cv::Mat &inMat, QImage::Format format );
    static QPixmap cvMatToQPixmap( const cv::Mat &inMat, QImage::Format format );
    void initUI();
    void closeEvent(QCloseEvent *event);
    void recognizeCameras();
signals:
    void sendImage(Mat mat);
private slots:
    void on_checkBox_clicked(bool checked);
    void showPixmap(QPixmap pixmap);
    void addSnap(Mat mat);
    void on_actionTakeSnap_triggered();

    void on_combocameras_activated(int index);

    void on_btnreload_clicked();

private:
    Ui::CamUI *ui;
    PanZoomView* view;
    CamUIThread* camthread;
    CustomGraphicsPixmapItem* picture;
};

#endif // CAMUI_H
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMUITHREAD_H
#define CAMUITHREAD_H

class CamUIThread : public QThread
{
    Q_OBJECT
public:
    explicit CamUIThread(QObject *parent, PanZoomView* view);
    void stopVideo();
    bool startVideo();
    void takeSnap(bool takesnap);
    void setCameraIndex(int cameraIndex);

signals:
    void updatePixmap(QPixmap pixmap);
    void hasSnapTaken(Mat mat);
private:
    PanZoomView* view;
    bool capturing;
    bool takesnap;
    VideoCapture capture;
    int cameraIndex;

protected:
    void run();
};
#endif // CAMUITHREAD_H
