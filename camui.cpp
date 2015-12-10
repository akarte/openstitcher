#include "camui.h"
#include "ui_camui.h"
#include "panzoomview.h"
#include "stitching.h"

#include "opencv2/opencv.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QPropertyAnimation>
#include <QThread>

using namespace cv;
using namespace std;

Q_DECLARE_METATYPE(cv::Mat)

CamUI::CamUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CamUI)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("Mat");

    view = new PanZoomView(this);
    ui->frameview->layout()->addWidget(view);
    ui->btnsnap->setDefaultAction(ui->actionTakeSnap);
    camthread = new CamUIThread(this, view);

    picture = new CustomGraphicsPixmapItem(QPixmap());
    view->scene()->addItem(picture);

    connect(camthread, SIGNAL(updatePixmap(QPixmap)), SLOT(showPixmap(QPixmap)));
    connect(camthread, SIGNAL(hasSnapTaken(Mat)), SLOT(addSnap(Mat)));
}

void CamUI::initUI()
{
    ui->checkBox->setChecked(false);
    ui->btnsnap->setEnabled(false);
    ui->combocameras->setEnabled(true);
    ui->btnreload->setEnabled(true);

    camthread->takeSnap(false);
    camthread->stopVideo();
    picture->setPixmap(QPixmap());
}

void CamUI::recognizeCameras()
{
    // Fill combobox with all cameras in system!!! <<<<<
    // Count camera devices
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

void CamUI::showPixmap(QPixmap pixmap)
{
    picture->setPixmap(pixmap);
    view->scene()->setSceneRect(pixmap.rect());
}

void CamUI::addSnap(Mat mat)
{
    QPropertyAnimation *animation = new QPropertyAnimation(picture, "opacity");
    animation->setDuration(1000);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start();

    // ADD SNAP TO IMAGE LIST
    emit sendImage(mat);
}

void CamUI::closeEvent(QCloseEvent *event)
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

CamUI::~CamUI()
{
    delete ui;
}

void CamUI::on_btnreload_clicked()
{
    recognizeCameras();
}

void CamUI::on_checkBox_clicked(bool checked)
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

void CamUI::on_actionTakeSnap_triggered()
{
    if(!ui->checkBox->isChecked() && !this->isVisible()) return;
    camthread->takeSnap(camthread->isRunning());
}

void CamUI::on_combocameras_activated(int index)
{
    camthread->setCameraIndex(index);
}

QImage CamUI::cvMatToQImage( const cv::Mat &inMat, QImage::Format format )
{
    return QImage(inMat.data, inMat.cols, inMat.rows, inMat.step, format);
}

QPixmap CamUI::cvMatToQPixmap( const cv::Mat &inMat, QImage::Format format )
{
    return QPixmap::fromImage( cvMatToQImage( inMat, format ) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

CamUIThread::CamUIThread(QObject* parent, PanZoomView* view) : QThread(parent)
{
    this->view = view;
    this->takesnap = false;
    this->cameraIndex = 0;
}

bool CamUIThread::startVideo()
{
    this->capturing = true;
    capture.open(cameraIndex);
    return capture.isOpened();
}

void CamUIThread::run()
{
    if(!view->scene()) return;

    while(capturing)
    {
        Mat frame;
        if(!capture.read(frame)) capturing = false;
        if(takesnap)
        {
            takesnap = false;
            // Send frame in BGR format
            emit hasSnapTaken(frame);
        }
        // From here, frame'll be in RGB format
        Mat aux;
        cvtColor(frame, aux, CV_BGR2RGB);
        QPixmap pm;
        pm = CamUI::cvMatToQPixmap(aux, QImage::Format_RGB888);
        emit updatePixmap(pm);
    }
    emit updatePixmap(QPixmap());
    capture.release();
}

void CamUIThread::stopVideo()
{
    this->capturing = false;
}

void CamUIThread::takeSnap(bool takesnap)
{
    this->takesnap = takesnap;
}

void CamUIThread::setCameraIndex(int cameraIndex)
{
    this->stopVideo();
    this->cameraIndex = cameraIndex;
}
