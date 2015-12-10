#ifndef InnerWindowTHREAD_H
#define InnerWindowTHREAD_H

#include "imagelist.h"
#include "panzoomview.h"
#include "stitchdata.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "stitching.h"

#include <QWidget>
#include <QGraphicsScene>

class InnerWindowThread : public QThread
{
    Q_OBJECT
public:
    explicit InnerWindowThread(QObject *parent);
    void initStitch(vector<Mat> &images);

signals:
    void updateGraphicScene(Mat item, double xTranslate, double yTranslate, double theta);
    void textToUI(string text2ui);

public slots:
    void appendUItext(string text2append);

private:
    vector<Mat> imagelist;

protected:
    void run();
};
#endif // TRACKINGUI_H

//////////////////////////////////////////////////////

#ifndef INNERWINDOW_H
#define INNERWINDOW_H

namespace Ui {
class innerwindow;
}

class InnerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InnerWindow(QWidget *parent = 0);
    ~InnerWindow();
    /**
     * @brief selectImages opens a dialog to select images to be stitched
     */
    void selectImages();
    /**
     * @brief createPano create a panoramic image and add to the UI component
     */
    void createPano();
    void fastPano();
    void findFeatures();
    void pairwiseMatching();
    void addImage(Mat mat);
    void exportImage();

    void drawGrid();
    void removeSelectedItemsAtList();
    PanZoomView* getView();

private slots:
    void updateSceneCall(Mat item, double xTranslate, double yTranslate, double theta);
    void updateUItext(string text2append);

private:
    Ui::innerwindow *ui;
    /**
     * @brief thumbnail_width thumbnail width of image
     */
    int thumbnail_width;

    InnerWindowThread *stitchthread;
    /**
     * @brief thumbnail_height thumbnail height of image
     */
    int thumbnail_height;
    int grid_size;

    /**
     * @brief il Widget to display current images to be stitched
     */
    ImageList* il;
    /**
     * @brief view Widget that let manipulate an stitched image
     */
    PanZoomView* view;
    /**
     * @brief scene Widget to display panoramic image
     */
    QGraphicsScene* scene;
    Stitching* stitch;
};

#endif // INNERWINDOW_H
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
