#ifndef STITCHING_H
#define STITCHING_H

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <QSettings>

#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"

#include "stitching.h"

using namespace std;
using namespace cv;
using namespace cv::detail;

class Stitching
{
public:
    explicit Stitching();
    ~Stitching();

    void createPano(vector<Mat> nombre_imagenes, cv::Mat &pano);
    void findFeatures(Mat &image, ImageFeatures &feature, int img_idx);
    void matchingViewer(Mat &base, ImageFeatures &baseFeatures,
                        Mat &current, ImageFeatures &currentFeatures, Mat &viewer);

    void testFeatures(std::vector<Mat> imagelist);
    void addDistortions(std::vector<Mat> &imagelist, Mat original);
    void rotate(cv::Mat& src, double angle, cv::Mat& dst);

    Mat equalizeIntensity(const Mat& inputImage);

    static void InitParameters(QSettings *settings);
    static void SaveParameters(QSettings *settings);
    static bool preview;
    static bool try_gpu;
    static double work_megapix;
    static double seam_megapix;
    static double compose_megapix;
    static float conf_thresh;
    static string features_type;
    static string ba_cost_func;
    static string ba_refine_mask;
    static bool do_wave_correct;
    static WaveCorrectKind wave_correct;
    static string warp_type;
    static int expos_comp_type;
    static float match_conf;
    static string seam_find_type;
    static int blend_type;
    static float blend_strength;
};

#endif // STITCHING_H
