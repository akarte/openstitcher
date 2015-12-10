#ifndef TRACKINGUI_H
#define TRACKINGUI_H

#include <QDialog>
#include <QGraphicsScene>
#include <QThread>

#include "opencv2/opencv.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "panzoomview.h"
#include "customgraphicspixmapitem.h"

#include "stitching.h"

using namespace cv;
using namespace std;

class TrackingUIThread;
namespace Ui {
class TrackingUI;
}

class TrackingUI : public QDialog
{
    Q_OBJECT

public:
    explicit TrackingUI(QWidget *parent = 0);
    ~TrackingUI();
    static QImage cvMatToQImage( const cv::Mat &inMat, QImage::Format format );
    static QPixmap cvMatToQPixmap( const cv::Mat &inMat, QImage::Format format );
    void initUI();
    void closeEvent(QCloseEvent *event);
    void recognizeCameras();
signals:
    void sendImage(Mat mat);
private slots:
    void showPixmap(QPixmap pixmap);
    void addSnap(Mat mat);
    void on_actionTakeSnap_triggered();
    void on_combocameras_activated(int index);
    void on_checktracking_clicked(bool checked);
    void on_btnreload_clicked();

    void on_actionAdvanced_triggered();

private:
    Ui::TrackingUI *ui;
    PanZoomView* view;
    TrackingUIThread* camthread;
    CustomGraphicsPixmapItem* picture;
};

#endif // TrackingUI_H
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TrackingUITHREAD_H
#define TrackingUITHREAD_H

class TrackingUIThread : public QThread
{
    Q_OBJECT
public:
    explicit TrackingUIThread(QObject *parent, PanZoomView* view);
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
    Mat base;
    vector<cv::KeyPoint> baseFeatures;

protected:
    void run();
};
#endif // TRACKINGUI_H
