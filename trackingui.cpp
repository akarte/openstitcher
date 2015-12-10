#include "trackingui.h"
#include "ui_trackingui.h"
#include "panzoomview.h"
#include "trackingstitching.h"
#include "mainwindow.h"

#include "opencv2/opencv.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QPropertyAnimation>
#include <QThread>

using namespace cv;
using namespace std;

Q_DECLARE_METATYPE(cv::Mat)

TrackingUI::TrackingUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrackingUI)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("Mat");

    view = new PanZoomView(this);
    ui->frameview->layout()->addWidget(view);
    ui->btnsnap->setDefaultAction(ui->actionTakeSnap);
    ui->btnadvanced->setDefaultAction(ui->actionAdvanced);
    camthread = new TrackingUIThread(this, view);

    picture = new CustomGraphicsPixmapItem(QPixmap());
    view->scene()->addItem(picture);

    connect(camthread, SIGNAL(updatePixmap(QPixmap)), SLOT(showPixmap(QPixmap)));
    connect(camthread, SIGNAL(hasSnapTaken(Mat)), SLOT(addSnap(Mat)));
}

void TrackingUI::initUI()
{
    ui->checktracking->setChecked(false);
    ui->btnsnap->setEnabled(false);
    ui->combocameras->setEnabled(true);
    ui->btnreload->setEnabled(true);

    camthread->takeSnap(false);
    camthread->stopVideo();
    picture->setPixmap(QPixmap());
}

void TrackingUI::recognizeCameras()
{
    // Fill combobox with all cameras in system!!! <<<<<
    // Count camera devices, only add the first 10 cameras (if exists)
    int maxTested = 10;
    int numOfCams = 0;
    ui->combocameras->clear();
    for (int i = 0; i < maxTested; i++){
        VideoCapture icamera(i);
        bool res = icamera.isOpened();
        icamera.release();
        if (res)
        {
            ui->combocameras->addItem("Camera " + QString::number(numOfCams), numOfCams);
            numOfCams++;
        }
    }
}

void TrackingUI::showPixmap(QPixmap pixmap)
{
    picture->setPixmap(pixmap);
    view->scene()->setSceneRect(pixmap.rect());
}

void TrackingUI::addSnap(Mat mat)
{
    QPropertyAnimation *animation = new QPropertyAnimation(picture, "opacity");
    animation->setDuration(1000);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start();

    // ADD SNAP TO IMAGE LIST
    emit sendImage(mat);
}

void TrackingUI::closeEvent(QCloseEvent *event)
{
    camthread->stopVideo();
    picture->setPixmap(QPixmap());
    for(;;)
    {
        if(!camthread->isRunning())
            break;
    }
    QDialog::closeEvent(event);
}

TrackingUI::~TrackingUI()
{
    delete ui;
}

void TrackingUI::on_btnreload_clicked()
{
    recognizeCameras();
}

void TrackingUI::on_checktracking_clicked(bool checked)
{
    ui->btnsnap->setEnabled(checked);
    ui->combocameras->setEnabled(!checked);
    ui->btnreload->setEnabled(!checked);
    if(checked)
    {
        for(;;)
        {
            if(!camthread->isRunning())
                break;
        }
        if(camthread->startVideo()) camthread->start();
    }
    else
    {
        if(camthread) camthread->stopVideo();
    }
}

void TrackingUI::on_actionTakeSnap_triggered()
{
    if(!ui->checktracking->isChecked() && !this->isVisible()) return;
    camthread->takeSnap(camthread->isRunning());
}

void TrackingUI::on_actionAdvanced_triggered()
{
    ((MainWindow*)this->parentWidget())->callConfig();
}

void TrackingUI::on_combocameras_activated(int index)
{
    camthread->setCameraIndex(index);
}

QImage TrackingUI::cvMatToQImage( const cv::Mat &inMat, QImage::Format format )
{
    return QImage(inMat.data, inMat.cols, inMat.rows, inMat.step, format);
}

QPixmap TrackingUI::cvMatToQPixmap( const cv::Mat &inMat, QImage::Format format )
{
    return QPixmap::fromImage( cvMatToQImage( inMat, format ) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

TrackingUIThread::TrackingUIThread(QObject* parent, PanZoomView* view) : QThread(parent)
{
    this->view = view;
    this->takesnap = false;
    this->cameraIndex = 0;
}

bool TrackingUIThread::startVideo()
{
    this->capturing = true;
    capture.open(cameraIndex);
    return capture.isOpened();
}

void TrackingUIThread::run()
{
    if(!view->scene()) return;

    while(capturing)
    {
        // Current image in camera
        Mat frame;
        // Final view
        Mat viewer;
        // Read image in BGR format
        if(!capture.read(frame)) capturing = false;
        if(takesnap)
        {
            base = frame;
            takesnap = false;
            emit hasSnapTaken(base);
        }
        QPixmap pm;
        if(base.empty()) viewer = frame;
        else
        {
            TrackingStitching stitcher("orb");
            stitcher.stitch(base, frame, viewer);
            //ImageFeatures frameFeatures;
            //stitcher.findFeatures(frame, frameFeatures, 1);
            //stitcher.matchingViewer(base, baseFeatures, frame, frameFeatures, viewer);
        }
        cvtColor(viewer, viewer, CV_BGR2RGB);
        pm = TrackingUI::cvMatToQPixmap(viewer, QImage::Format_RGB888);
        emit updatePixmap(pm);
    }
    emit updatePixmap(QPixmap());
    base.release();
    capture.release();
}

void TrackingUIThread::stopVideo()
{
    this->capturing = false;
}

void TrackingUIThread::takeSnap(bool takesnap)
{
    this->takesnap = takesnap;
}

void TrackingUIThread::setCameraIndex(int cameraIndex)
{
    this->stopVideo();
    this->cameraIndex = cameraIndex;
}

