#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "config.h"
#include "camui.h"
#include "trackingui.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showEvent(QShowEvent* event);
    void callConfig();

private slots:
    void on_actionCreate_Panoramic_triggered();
    void on_actionNew_Project_triggered();
    void on_tabWidget_tabCloseRequested(int index);
    void on_actionDelete_Image_triggered();
    void on_actionAdvanced_triggered();
    void on_actionPairwise_Matching_triggered();
    void on_actionStart_Recording_triggered();
    void receiveImage(Mat mat);

    void on_actionStart_Traking_triggered();

    void on_actionImport_Images_triggered();

    void on_actionExport_Image_triggered();

    void on_actionFeature_Extraction_triggered();
private:
    Ui::MainWindow *ui;
    QWidget *iw;
    Config *config;
    CamUI *camui;
    TrackingUI *trackingui;

protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
};

#endif // MAINWINDOW_H
