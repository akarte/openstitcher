#include "mainwindow.h"
#include "innerwindow.h"
#include "ui_mainwindow.h"
#include "stitching.h"
#include "camui.h"
#include "trackingui.h"
#include <QtGui>

Q_DECLARE_METATYPE(cv::Mat)

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("Mat");

    iw = new InnerWindow();
    ui->tabWidget->addTab(iw, "unnamed");
    ui->tabWidget->setTabsClosable(true);

    QString filename = QDir::currentPath() + QString("/stitching.ini");
    QSettings settings(filename, QSettings::IniFormat);
    Stitching::InitParameters(&settings);
    config = new Config(this);
    camui = new CamUI(this);
    camui->recognizeCameras();
    trackingui = new TrackingUI(this);
    trackingui->recognizeCameras();

    connect(camui, SIGNAL(sendImage(Mat)), SLOT(receiveImage(Mat)));
    connect(trackingui, SIGNAL(sendImage(Mat)), SLOT(receiveImage(Mat)));

    qApp->installEventFilter(this);
    //setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent( QShowEvent* event ) {
    QMainWindow::showEvent( event );
}

void MainWindow::on_actionCreate_Panoramic_triggered()
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->fastPano();
}

void MainWindow::on_actionNew_Project_triggered()
{
    InnerWindow* iw = new InnerWindow();
    ui->tabWidget->addTab(iw, "unnamed");
    ui->tabWidget->setTabsClosable(true);
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    delete (InnerWindow*) ui->tabWidget->widget(index);
}

void MainWindow::on_actionDelete_Image_triggered()
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->removeSelectedItemsAtList();
}

void MainWindow::on_actionAdvanced_triggered()
{
    config->exec();
}

void MainWindow::callConfig()
{
    config->exec();
}

void MainWindow::on_actionPairwise_Matching_triggered()
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->pairwiseMatching();
}

void MainWindow::on_actionStart_Recording_triggered()
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    camui->initUI();
    camui->exec();
}

void MainWindow::receiveImage(Mat mat)
{
    // mat must be in BGR format
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->addImage(mat);
}

void MainWindow::on_actionStart_Traking_triggered()
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    trackingui->initUI();
    trackingui->exec();
}

void MainWindow::on_actionImport_Images_triggered()
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->selectImages();
}

void MainWindow::on_actionExport_Image_triggered()
{
    // Export image
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->exportImage();
}

void MainWindow::on_actionFeature_Extraction_triggered()
{
    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->findFeatures();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QMainWindow::keyPressEvent(event);

    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->getView()->keyPressSender(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    QMainWindow::keyReleaseEvent(event);

    if(!((InnerWindow*)ui->tabWidget->currentWidget())) return;
    ((InnerWindow*)ui->tabWidget->currentWidget())->getView()->keyReleaseSender(event);
}
