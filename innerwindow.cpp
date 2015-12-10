#include "innerwindow.h"
#include "ui_innerwindow.h"
#include "imagelist.h"
#include "stitchimageitem.h"

#include "stitchdata.h"
#include "stitchinfo.h"
#include "faststitcher.h"
#include "stitching.h"
#include "panzoomview.h"
#include "camui.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QList>
#include <QFileDialog>
#include <QScrollBar>
#include <QLayout>
#include <QLabel>
#include <QImage>
#include <QPixmap>

#include <QElapsedTimer>

Q_DECLARE_METATYPE(cv::Mat)

InnerWindow::InnerWindow(QWidget *parent) : QWidget(parent), ui(new Ui::innerwindow)
{
    qRegisterMetaType<cv::Mat>("Mat");

    ui->setupUi(this);

    grid_size = 64;
    il = new ImageList();

    view = new PanZoomView(this);
    ui->framework->layout()->addWidget(view);
    ui->framelist->layout()->addWidget(il);

    stitchthread = new InnerWindowThread(this);
    connect(stitchthread,
            SIGNAL(updateGraphicScene(Mat, double, double, double)),
            SLOT(updateSceneCall(Mat, double, double, double)));

    connect(stitchthread,
            SIGNAL(textToUI(string)),
            SLOT(updateUItext(string)));
}

InnerWindow::~InnerWindow()
{
    delete view;
    delete il;
}

void InnerWindow::selectImages()
{
    int w = il->width() - 10;
    int h = w / 4 * 3;
    il->setIconSize(QSize(w, h));
    QStringList fileNames = QFileDialog::getOpenFileNames(
                this, tr("Select Image"),QDir::currentPath(),
                tr("All files (*.*);;JPEG (*.jpg *.jpeg);;TIFF (*.tif);;PNG (*.png)"));
    for (int i = 0; i < fileNames.size(); ++i)
    {
        QPixmap t_pxm = QPixmap(fileNames.at(i));
        il->thumbnails->append(t_pxm.scaled(il->thumbnails_size, Qt::KeepAspectRatio, Qt::FastTransformation));
        il->mediumsize->append(t_pxm.scaled(il->mediumsize_size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // Read all Mat in BGR format
        Mat matimage(imread(fileNames.at(i).toLocal8Bit().constData()));
        il->matimages->append(matimage);

        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon(QIcon(il->mediumsize->last()));
        il->addItem(item);
    }
}

void InnerWindow::addImage(Mat mat)
{
    // mat must be in BGR format
    il->matimages->append(mat);
    Mat aux;
    cvtColor(mat, aux, CV_BGR2RGB);
    // From here mat'll be in RGB format
    QPixmap pm = CamUI::cvMatToQPixmap(aux,  QImage::Format_RGB888);
    il->thumbnails->append(pm.scaled(il->thumbnails_size, Qt::KeepAspectRatio, Qt::FastTransformation));
    il->mediumsize->append(pm.scaled(il->mediumsize_size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    int w = il->width() - 10;
    int h = w / 4 * 3;
    il->setIconSize(QSize(w, h));

    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon(QIcon(il->mediumsize->last().scaled(w,h,Qt::KeepAspectRatio, Qt::FastTransformation)));
    il->addItem(item);
}

void InnerWindow::fastPano(){
    // Create a object that store current image paths
    vector<Mat> imagelist;
    for(int i = 0; i < il->matimages->size(); i++){
        imagelist.push_back(il->matimages->at(i));
    }

    if(imagelist.empty()) return;

    // Get scene where image will be added
    scene = view->scene();
    // Clear it before
    view->initScene();

    ui->infotext->insertPlainText("\n\n=============================================");
    ui->infotext->insertPlainText("Initializing panoramic algorythm");

    if(!stitchthread->isRunning()){
        stitchthread->initStitch(imagelist);
        stitchthread->start();
    }
}

void InnerWindow::updateSceneCall(Mat item, double xTranslate, double yTranslate, double theta){
    // Needs to convert all images to
    // QImage in a RGBA format (with alpha channel)
    Mat aux =  item.clone();
    cvtColor(aux, aux, CV_BGR2RGB);
    StitchImageItem* graphicpixmap = new StitchImageItem(CamUI::cvMatToQPixmap(aux, QImage::Format_RGB888));
    aux.release();

    scene->addItem(graphicpixmap);
    graphicpixmap->setPos(xTranslate, yTranslate);
    graphicpixmap->setRotation(theta);
    scene->setSceneRect(scene->itemsBoundingRect());
}

void InnerWindow::updateUItext(string text2append)
{
    ui->infotext->appendPlainText(text2append.c_str());
}

void InnerWindow::createPano()
{
    // Create a stitcher object

    Stitching stitcher = Stitching();
    // Create a object that store current image paths

    vector<Mat> imagelist;
    for(int i = 0; i < il->matimages->size(); i++)
        imagelist.push_back(il->matimages->at(i));

    // Object that will store the panoramic image
    Mat pano;

    // Create panoramic

    // Timer to count time (duh)
    QElapsedTimer timer;
    timer.start();

    // object that create a panoramic image based on an given image list
    stitcher.createPano(imagelist, pano);

    // If there's no panoramic created, exit
    if(pano.empty()) return;

    // Stop time
    int elapsed = timer.elapsed();

    // Needs to convert the current panoramic image to
    // QImage in a RGB format (without alpha channel)
    QPixmap pm = CamUI::cvMatToQPixmap(pano, QImage::Format_RGB888);

    // Add panoramic image created to UI

    // Get scene where image will be added
    scene = view->scene();
    // Clear it before
    scene->clear();
    // Add image
    scene->addPixmap(pm);
    // Get rect to fit to image
    scene->setSceneRect(pm.rect());
    // Print time taken to create panoramic image
    scene->addText(QString::number(elapsed))->setDefaultTextColor(Qt::white);
    // Release image
    pano.release();
}

void InnerWindow::findFeatures()
{

    stitch = new Stitching();
    std::vector<Mat> imagelist;
    for(int i = 0; i < il->matimages->size(); i++)
    {
        imagelist.push_back(il->matimages->at(i));
    }
    stitch->testFeatures(imagelist);

}

void InnerWindow::pairwiseMatching()
{
    if(!stitch) return;
    std::vector<Mat> imagelist;
    for(int i = 0; i < il->matimages->size(); i++)
    {
        imagelist.push_back(il->matimages->at(i));
    }
    //stitch->pairwiseMatching(imagelist);
}

void InnerWindow::drawGrid()
{
    QPen pen(QColor(220,220,220, 255));
    pen.setWidth(1);
    int width = ui->framework->width();
    int height = ui->framework->height();
    while(width > 0)
    {
        width-=grid_size;
        scene->addLine(width, 0, width, view->height(),pen);
    }
    while(height > 0)
    {
        height-=grid_size;
        scene->addLine(0, height, view->width(), height,pen);
    }
}

void InnerWindow::removeSelectedItemsAtList()
{
    il->removeSelectedOperation();
}

PanZoomView *InnerWindow::getView()
{
    return view;
}

void InnerWindow::exportImage()
{
    // If there's nothing to export (if there's no image) then return
    if(!(scene) || scene->sceneRect().isNull() || scene->sceneRect().isEmpty()) return;

    // Open dialog to select path to save image (only jpg and png format for now)
    QString fileName = QFileDialog::getSaveFileName(
                this, tr("Save Image"),QDir::currentPath(),
                tr("JPEG (*.jpg *.jpeg);;PNG (*.png)"));

    // If image name is empty or null, return
    if(fileName.isNull() || fileName.isEmpty()) return;

    // Create an image object
    // Create the image with the exact size of the shrunk scene
    scene->setSceneRect(view->calculateSceneBoundingRect());
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);

    // If image is empty, return
    if(image.isNull()) return;

    // Fill emptyness with transparency
    // Start all pixels transparent
    image.fill(Qt::transparent);

    // Necessary to put rendered object
    QPainter painter(&image);

    // Render whatever that is in scene right now
    scene->render(&painter);

    // Save it...
    image.save(fileName);
}

InnerWindowThread::InnerWindowThread(QObject* parent) : QThread(parent)
{
    qRegisterMetaType<std::string>("string");
}

void InnerWindowThread::initStitch(vector<Mat> &images){
    this->imagelist = images;
}

void InnerWindowThread::appendUItext(string text2append)
{
    emit textToUI(text2append);
}

void InnerWindowThread::run(){
    vector<StitchInfo> imageinfo;

    // Create a stitcher object
    FastStitcher* stitcher = new FastStitcher();

    connect(stitcher, SIGNAL(appendText(string)),
            SLOT(appendUItext(string)));
    // object that create a panoramic image based on an given image list
    stitcher->stitch(imagelist, imageinfo);
    if(imageinfo.empty()){
        return;
    }

    for(unsigned int i = 0; i < imageinfo.size(); i++){
        StitchInfo info = imageinfo.at(i);
        emit updateGraphicScene(imagelist.at(info.i), info.x, info.y, /*info.theta*/0);
    }

    delete stitcher;
    stitcher = NULL;
}
