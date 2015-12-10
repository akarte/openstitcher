#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

#include <QSettings>
#include "trackingstitching.h"


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

#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"

#include "stitching.h"
#include "featuredetector.cpp"

using namespace std;
using namespace cv;
using namespace cv::detail;

bool Stitching::preview = true;
bool Stitching::try_gpu = true;
double Stitching::work_megapix = 1.0;
double Stitching::seam_megapix = 1.0;
double Stitching::compose_megapix = -1;
float Stitching::conf_thresh = 0.6f;
string Stitching::features_type = "orb";
string Stitching::ba_cost_func = "reproj";
string Stitching::ba_refine_mask = "_____";
bool Stitching::do_wave_correct = false;
WaveCorrectKind Stitching::wave_correct = detail::WAVE_CORRECT_HORIZ;
string Stitching::warp_type = "plane";
int Stitching::expos_comp_type = ExposureCompensator::GAIN_BLOCKS;
float Stitching::match_conf = 0.8f;
string Stitching::seam_find_type = "dp_colorgrad";
int Stitching::blend_type = Blender::MULTI_BAND;
float Stitching::blend_strength = 2;

Stitching::Stitching()
{
}

Stitching::~Stitching()
{

}

void Stitching::InitParameters(QSettings* settings)
{
    // Quality & Performance variables
    preview = settings->value("Quality_Performance/preview", true).toBool();
    try_gpu = settings->value("Quality_Performance/try_gpu", true).toBool();

    // Motion estimation variables
    features_type = (string) settings->value("Motion_Estimation/features_type", "orb").toString().toUtf8();
    match_conf = settings->value("Motion_Estimation/match_confidence", 0.8).toFloat();
    work_megapix = settings->value("Motion_Estimation/work_megapix", 1.0).toDouble();
    conf_thresh = settings->value("Motion_Estimation/confidence_threshold", 0.6f).toFloat();
    ba_cost_func = (string) settings->value("Motion_Estimation/bundle_adjust_cost_func", "reproj").toString().toUtf8();
    ba_refine_mask = (string) settings->value("Motion_Estimation/bundle_adjust_refine_max", "_____").toString().toUtf8();
    do_wave_correct = settings->value("Motion_Estimation/do_wave_correct", false).toBool();
    string w_c = (string) settings->value("Motion_Estimation/wave_correct", "horizontal").toString().toUtf8();
    if(w_c == "horizontal")
        wave_correct = detail::WAVE_CORRECT_HORIZ;
    else if (w_c == "vertical")
        wave_correct = detail::WAVE_CORRECT_VERT;

    // Compositing variables
    warp_type = (string) settings->value("Compositing/warp_type", "plane").toString().toUtf8();
    seam_megapix = settings->value("Compositing/seam_megapix", 1.0).toDouble();
    seam_find_type = (string) settings->value("Compositing/seam_find_type", "dp_colorgrad").toString().toUtf8();
    compose_megapix = settings->value("Compositing/compose_megapix", -1).toDouble();
    expos_comp_type = settings->value("Compositing/exposure_compensation_method", ExposureCompensator::GAIN_BLOCKS).toInt();
    blend_type = settings->value("Compositing/blending_type", Blender::MULTI_BAND).toInt();
    blend_strength = settings->value("Compositing/blending_strength", 2.0f).toFloat();
}

void Stitching::SaveParameters(QSettings *settings)
{
    // Quality & Performance variables
    settings->setValue("Quality_Performance/preview", preview);
    settings->setValue("Quality_Performance/try_gpu", try_gpu);

    // Motion estimation variables
    settings->setValue("Motion_Estimation/features_type", QString::fromStdString(features_type));
    settings->setValue("Motion_Estimation/match_confidence", (double) match_conf);
    settings->setValue("Motion_Estimation/work_megapix", work_megapix);
    settings->setValue("Motion_Estimation/confidence_threshold", (double) conf_thresh);
    settings->setValue("Motion_Estimation/bundle_adjust_cost_func", QString::fromStdString(ba_cost_func));
    settings->setValue("Motion_Estimation/bundle_adjust_refine_max", QString::fromStdString(ba_refine_mask));
    settings->setValue("Motion_Estimation/do_wave_correct", do_wave_correct);
    if(wave_correct == detail::WAVE_CORRECT_HORIZ)
        settings->setValue("Motion_Estimation/wave_correct", "horizontal");
    else if (wave_correct == detail::WAVE_CORRECT_VERT)
        settings->setValue("Motion_Estimation/wave_correct", "vertical");

    // Compositing variables
    settings->setValue("Compositing/warp_type", QString::fromStdString(warp_type));
    settings->setValue("Compositing/seam_megapix", seam_megapix);
    settings->setValue("Compositing/seam_find_type", QString::fromStdString(seam_find_type));
    settings->setValue("Compositing/compose_megapix", compose_megapix);
    settings->setValue("Compositing/exposure_compensation_method", expos_comp_type);
    settings->setValue("Compositing/blending_type", blend_type);
    settings->setValue("Compositing/blending_strength", (double) blend_strength);

    settings->sync();
}

void Stitching::createPano(vector<Mat> nombre_imagenes, Mat &pano)
{
    cv::setBreakOnError(true);

    // Check if have enough images
    int num_images = static_cast<int>(nombre_imagenes.size());
    if (num_images < 2) return;

    double work_scale = 1, seam_scale = 1, compose_scale = 1;
    bool is_work_scale_set = false, is_seam_scale_set = false, is_compose_scale_set = false;

    // 1 : Find Features
    Ptr<FeaturesFinder> finder;
    if (features_type == "orb") finder = new OrbFeaturesFinder();
    else return;

    vector<Mat> full_img(num_images);
    vector<Mat> img(num_images);

    vector<ImageFeatures> features(num_images);
    vector<Mat> images(num_images);
    vector<Size> full_img_sizes(num_images);
    double seam_work_aspect = 1;

//#pragma omp parallel for
    for (int i = 0; i < num_images; ++i)
    {
        full_img[i] = nombre_imagenes[i];
        full_img_sizes[i] = full_img[i].size();

        if (full_img[i].empty()) continue;

        if (work_megapix < 0)
        {
            img[i] = full_img[i];
            work_scale = 1;
            is_work_scale_set = true;
        }
        else
        {
            if (!is_work_scale_set)
            {
                work_scale = min(1.0, sqrt(work_megapix * 1e6 / full_img[i].size().area()));
                is_work_scale_set = true;
            }
            resize(full_img[i], img[i], Size(), work_scale, work_scale);
        }
        if (!is_seam_scale_set)
        {
            seam_scale = min(1.0, sqrt(seam_megapix * 1e6 / full_img[i].size().area()));
            seam_work_aspect = seam_scale / work_scale;
            is_seam_scale_set = true;
        }

        // Find features for each image
        (*finder)(img[i], features[i]);
        features[i].img_idx = i;

        resize(full_img[i], img[i], Size(), seam_scale, seam_scale);
        images[i] = img[i].clone();
    }

    finder->collectGarbage();

    // 2 : Prepare to find pairwise for each image
    vector<MatchesInfo> pairwise_matches;
    BestOf2NearestMatcher matcher(try_gpu, match_conf);
    matcher(features, pairwise_matches);
    matcher.collectGarbage();

    // Leave only images we are sure are from the same panorama
    vector<int> indices = leaveBiggestComponent(features, pairwise_matches, conf_thresh);
    vector<Mat> img_subset;
    vector<Size> full_img_sizes_subset;
    for (size_t i = 0; i < indices.size(); ++i)
    {
        img_subset.push_back(images[indices[i]]);
        full_img_sizes_subset.push_back(full_img_sizes[indices[i]]);
    }

    images = img_subset;
    full_img_sizes = full_img_sizes_subset;

    // Check if we still have enough images
    num_images = static_cast<int>(images.size());
    if (num_images < 2) return;

    // Begin homography estimator
    HomographyBasedEstimator estimator;
    vector<CameraParams> cameras;
    estimator(features, pairwise_matches, cameras);

    for (size_t i = 0; i < cameras.size(); ++i)
    {
        Mat R;
        cameras[i].R.convertTo(R, CV_32F);
        cameras[i].R = R;
    }

    Ptr<detail::BundleAdjusterBase> adjuster;
    if (ba_cost_func == "reproj") adjuster = new detail::BundleAdjusterReproj();
    else if (ba_cost_func == "ray") adjuster = new detail::BundleAdjusterRay();
    else return;
    adjuster->setConfThresh(conf_thresh);
    Mat_<uchar> refine_mask = Mat::zeros(3, 3, CV_8U);
    if (ba_refine_mask[0] == 'x') refine_mask(0,0) = 1;
    if (ba_refine_mask[1] == 'x') refine_mask(0,1) = 1;
    if (ba_refine_mask[2] == 'x') refine_mask(0,2) = 1;
    if (ba_refine_mask[3] == 'x') refine_mask(1,1) = 1;
    if (ba_refine_mask[4] == 'x') refine_mask(1,2) = 1;
    adjuster->setRefinementMask(refine_mask);
    (*adjuster)(features, pairwise_matches, cameras);

    // Find median focal length
    vector<double> focals;
    for (size_t i = 0; i < cameras.size(); ++i)
    {
        focals.push_back(cameras[i].focal);
    }

    sort(focals.begin(), focals.end());
    float warped_image_scale;
    if (focals.size() % 2 == 1) warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
    else                        warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5f;

    if (do_wave_correct)
    {
        vector<Mat> rmats;
        for (size_t i = 0; i < cameras.size(); ++i)
            rmats.push_back(cameras[i].R);
        waveCorrect(rmats, wave_correct);
        for (size_t i = 0; i < cameras.size(); ++i)
            cameras[i].R = rmats[i];
    }

    // 4 : Warp images
    vector<Point> corners(num_images);
    vector<Mat> masks_warped(num_images);
    vector<Mat> images_warped(num_images);
    vector<Size> sizes(num_images);
    vector<Mat> masks(num_images);

    // Preapre images masks
    for (int i = 0; i < num_images; ++i)
    {
        masks[i].create(images[i].size(), CV_8U);
        masks[i].setTo(Scalar::all(255));
    }

    // Warp images and their masks

    Ptr<WarperCreator> warper_creator;
#if defined(HAVE_OPENCV_GPU)
    if (try_gpu && gpu::getCudaEnabledDeviceCount() > 0)
    {
        if (warp_type == "plane")               warper_creator = new cv::PlaneWarperGpu();
        else if (warp_type == "cylindrical")    warper_creator = new cv::CylindricalWarperGpu();
        else if (warp_type == "spherical")      warper_creator = new cv::SphericalWarperGpu();
    }
    else
#endif
    {
        if (warp_type == "plane")                               warper_creator = new cv::PlaneWarper();
        else if (warp_type == "cylindrical")                    warper_creator = new cv::CylindricalWarper();
        else if (warp_type == "spherical")                      warper_creator = new cv::SphericalWarper();
        else if (warp_type == "fisheye")                        warper_creator = new cv::FisheyeWarper();
        else if (warp_type == "stereographic")                  warper_creator = new cv::StereographicWarper();
        else if (warp_type == "compressedPlaneA2B1")            warper_creator = new cv::CompressedRectilinearWarper(2, 1);
        else if (warp_type == "compressedPlaneA1.5B1")          warper_creator = new cv::CompressedRectilinearWarper(1.5, 1);
        else if (warp_type == "compressedPlanePortraitA2B1")    warper_creator = new cv::CompressedRectilinearPortraitWarper(2, 1);
        else if (warp_type == "compressedPlanePortraitA1.5B1")  warper_creator = new cv::CompressedRectilinearPortraitWarper(1.5, 1);
        else if (warp_type == "paniniA2B1")                     warper_creator = new cv::PaniniWarper(2, 1);
        else if (warp_type == "paniniA1.5B1")                   warper_creator = new cv::PaniniWarper(1.5, 1);
        else if (warp_type == "paniniPortraitA2B1")             warper_creator = new cv::PaniniPortraitWarper(2, 1);
        else if (warp_type == "paniniPortraitA1.5B1")           warper_creator = new cv::PaniniPortraitWarper(1.5, 1);
        else if (warp_type == "mercator")                       warper_creator = new cv::MercatorWarper();
        else if (warp_type == "transverseMercator")             warper_creator = new cv::TransverseMercatorWarper();
    }
    if (warper_creator.empty()) return;

    Ptr<RotationWarper> warper = warper_creator->create(static_cast<float>(warped_image_scale * seam_work_aspect));

    vector<Mat> images_warped_f(num_images);

//#pragma omp parallel for
    for (int i = 0; i < num_images; ++i)
    {
        Mat_<float> K;
        cameras[i].K().convertTo(K, CV_32F);
        float swa = (float)seam_work_aspect;
        K(0,0) *= swa; K(0,2) *= swa;
        K(1,1) *= swa; K(1,2) *= swa;

        corners[i] = warper->warp(images[i], K, cameras[i].R, INTER_LINEAR, BORDER_REFLECT, images_warped[i]);
        sizes[i] = images_warped[i].size();

        warper->warp(masks[i], K, cameras[i].R, INTER_NEAREST, BORDER_CONSTANT, masks_warped[i]);

        images_warped[i].convertTo(images_warped_f[i], CV_32F);
    }

    // 5 : Stiching
    Ptr<ExposureCompensator> compensator = ExposureCompensator::createDefault(expos_comp_type);
    compensator->feed(corners, images_warped, masks_warped);

    Ptr<SeamFinder> seam_finder;
    if (seam_find_type == "no")             seam_finder = new detail::NoSeamFinder();
    else if (seam_find_type == "voronoi")   seam_finder = new detail::VoronoiSeamFinder();
    else if (seam_find_type == "gc_color")
    {
#if defined(HAVE_OPENCV_GPU)
        if (try_gpu && gpu::getCudaEnabledDeviceCount() > 0)
            seam_finder = new detail::GraphCutSeamFinderGpu(GraphCutSeamFinderBase::COST_COLOR);
        else
#endif
            seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR);
    }
    else if (seam_find_type == "gc_colorgrad")
    {
#if defined(HAVE_OPENCV_GPU)
        if (try_gpu && gpu::getCudaEnabledDeviceCount() > 0)
            seam_finder = new detail::GraphCutSeamFinderGpu(GraphCutSeamFinderBase::COST_COLOR_GRAD);
        else
#endif
            seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR_GRAD);
    }
    else if (seam_find_type == "dp_color")      seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR);
    else if (seam_find_type == "dp_colorgrad")  seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR_GRAD);
    if (seam_finder.empty())return;

    seam_finder->find(images_warped_f, corners, masks_warped);

    // Release unused memory
    images.clear();
    images_warped.clear();
    images_warped_f.clear();
    masks.clear();

    // 6 : Composing
    Mat img_warped, img_warped_s;
    Mat dilated_mask, seam_mask, mask, mask_warped;
    Ptr<Blender> blender;
    double compose_work_aspect = 1;

//#pragma omp parallel for
    for (int img_idx = 0; img_idx < num_images; ++img_idx)
    {
        // Read image and resize it if necessary
        if (!is_compose_scale_set)
        {
            if (compose_megapix > 0)    compose_scale = min(1.0, sqrt(compose_megapix * 1e6 / full_img[img_idx].size().area()));
            is_compose_scale_set = true;

            // Compute relative scales
            //compose_seam_aspect = compose_scale / seam_scale;
            compose_work_aspect = compose_scale / work_scale;

            // Update warped image scale
            warped_image_scale *= static_cast<float>(compose_work_aspect);
            warper = warper_creator->create(warped_image_scale);

            // Update corners and sizes
            for (int i = 0; i < num_images; ++i)
            {
                // Update intrinsics
                cameras[i].focal *= compose_work_aspect;
                cameras[i].ppx *= compose_work_aspect;
                cameras[i].ppy *= compose_work_aspect;

                // Update corner and size
                Size sz = full_img_sizes[i];
                if (std::abs(compose_scale - 1) > 1e-1)
                {
                    sz.width = cvRound(full_img_sizes[i].width * compose_scale);
                    sz.height = cvRound(full_img_sizes[i].height * compose_scale);
                }

                Mat K;
                cameras[i].K().convertTo(K, CV_32F);
                Rect roi = warper->warpRoi(sz, K, cameras[i].R);
                corners[i] = roi.tl();
                sizes[i] = roi.size();
            }
        }
        if (abs(compose_scale - 1) > 1e-1)
            resize(full_img[img_idx], img[img_idx], Size(), compose_scale, compose_scale);
        else
            img[img_idx] = full_img[img_idx];
        full_img[img_idx].release();
        Size img_size = img[img_idx].size();

        Mat K;
        cameras[img_idx].K().convertTo(K, CV_32F);

        // Warp the current image
        warper->warp(img[img_idx], K, cameras[img_idx].R, INTER_LINEAR, BORDER_REFLECT, img_warped);

        // Warp the current image mask
        mask.create(img_size, CV_8U);
        mask.setTo(Scalar::all(255));
        warper->warp(mask, K, cameras[img_idx].R, INTER_NEAREST, BORDER_CONSTANT, mask_warped);

        // Compensate exposure
        compensator->apply(img_idx, corners[img_idx], img_warped, mask_warped);

        img_warped.convertTo(img_warped_s, CV_16S);
        img_warped.release();
        img[img_idx].release();
        mask.release();

        dilate(masks_warped[img_idx], dilated_mask, Mat());
        resize(dilated_mask, seam_mask, mask_warped.size());
        mask_warped = seam_mask & mask_warped;

        if (blender.empty())
        {
            blender = Blender::createDefault(blend_type, try_gpu);
            Size dst_sz = resultRoi(corners, sizes).size();
            float blend_width = sqrt(static_cast<float>(dst_sz.area())) * blend_strength / 100.f;
            if (blend_width < 1.f)  blender = Blender::createDefault(Blender::NO, try_gpu);
            else if (blend_type == Blender::MULTI_BAND)
            {
                MultiBandBlender* mb = dynamic_cast<MultiBandBlender*>(static_cast<Blender*>(blender));
                mb->setNumBands(static_cast<int>(ceil(log(blend_width)/log(2.)) - 1.));
            }
            else if (blend_type == Blender::FEATHER)
            {
                FeatherBlender* fb = dynamic_cast<FeatherBlender*>(static_cast<Blender*>(blender));
                fb->setSharpness(1.f/blend_width);
            }
            blender->prepare(corners, sizes);
        }

        // Blend the current image
        blender->feed(img_warped_s, mask_warped, corners[img_idx]);
    }

    Mat result, result_mask;
    blender->blend(result, result_mask);

    //////////////////////////////////////////////////////////////////////////////////////////////
    result.convertTo(pano, CV_8UC3);
    cvtColor(pano, pano, CV_BGR2RGB);
    //////////////////////////////////////////////////////////////////////////////////////////////
}

void Stitching::findFeatures(Mat &image, ImageFeatures &feature, int img_idx)
{
    cv::setBreakOnError(true);

    Ptr<FeaturesFinder> finder;
    if (features_type == "orb")
        finder = new OrbFeaturesFinder(Size(1, 1), 1000, 1.3f, 5);
    //else return;

    (*finder)(image, feature);
    feature.img_idx = img_idx;

    finder->collectGarbage();
}

Mat Stitching::equalizeIntensity(const Mat& inputImage)
{
    if(inputImage.channels() >= 3)
    {
        Mat ycrcb;

        cvtColor(inputImage,ycrcb,CV_BGR2YCrCb);

        vector<Mat> channels;
        split(ycrcb,channels);

        equalizeHist(channels[0], channels[0]);

        Mat result;
        merge(channels,ycrcb);

        cvtColor(ycrcb,result,CV_YCrCb2BGR);

        return result;
    }
    return Mat();
}

void Stitching::matchingViewer(Mat &base, ImageFeatures &baseFeatures, Mat &current, ImageFeatures &currentFeatures, Mat &viewer)
{
    current = equalizeIntensity(current);

    if( !current.data || !base.data ) return;
    ////////////////////////////////////////////////////////////////////////////////////// GET KEYPOINTS
    // Image Features vector, this'll save all features detected in current image and base image
    vector<ImageFeatures> features(2);
    features[0] = currentFeatures;
    features[1] = baseFeatures;
    ////////////////////////////////////////////////////////////////////////////////////// MATCH
    // Get matches
    vector<MatchesInfo> pairwise_matches;
    BestOf2NearestMatcher matcher(try_gpu, match_conf);
    matcher(features, pairwise_matches);
    matcher.collectGarbage();

    if(pairwise_matches.size() > 3)
    {
        if(pairwise_matches[2].num_inliers > 0)
        {
            //warp images
            Mat warping;
            Mat H = pairwise_matches[1].H;
            int x_offset = current.cols - (current.cols / 2);
            int y_offset = current.rows - (current.rows / 2);
            Mat offset = (Mat_<double>(3,3) << 1, 0, x_offset, 0, 1, y_offset, 0, 0, 1);
            H = H * offset;
            //warpAffine(current, warping, H, Size(current.cols * 2, current.rows * 2));
            warpPerspective(current, warping, H, Size(current.cols * 2, current.rows * 2), INTER_NEAREST);
            viewer = Mat(Size(warping.cols, warping.rows), CV_8UC3);
            Mat roi1(viewer, Rect(x_offset, y_offset, base.cols, base.rows));
            base.copyTo(roi1);
            viewer = warping + viewer;
        }
    }
    /*
    if(pairwise_matches.size() > 3)
    {
        if(pairwise_matches[2].num_inliers > 0)
        {
            drawMatches(
                base,       features[0].keypoints,
                current,    features[1].keypoints,
                pairwise_matches[1].matches, viewer, Scalar::all(255), Scalar::all(255));
        }
    }
    */
    if(viewer.empty())
    {
        drawMatches(
                base,       features[1].keypoints,
                current,    features[0].keypoints,
                pairwise_matches[0].matches, viewer, Scalar::all(255), Scalar::all(255));
    }

    pairwise_matches.clear();
    features.clear();
}

void Stitching::testFeatures(std::vector<Mat> imagelist)
{
    if(imagelist.empty()) return;

    cv::setBreakOnError(true);
    //addDistortions(imagelist, imagelist.at(0));

    if(imagelist.size() == 2){
        Mat img1, img2;
        img1 = imagelist.at(0);
        img2 = imagelist.at(1);

        TrackingStitching stitcher("orb");
        Mat viewer;
        stitcher.stitch(img1, img2, viewer);
        imshow("asd", viewer);
    } else {
        for(int i = 0; i < imagelist.size(); i++){
            // Harris test
            //harris_detector(imagelist.at(i), i);
            //sift_detector(imagelist.at(i), i);
            //surf_detector(imagelist.at(i), i);
            orb_detector(imagelist.at(i), i);
        }
    }
}
void Stitching::addDistortions(std::vector<Mat> &imagelist, Mat original)
{
    // noise
    Mat noisy = original.clone();
    Mat noiser = original.clone();
    randn(noisy,10,10);
    imagelist.push_back(noiser + noisy);

    // rotation
    cv::Mat rotated;
    rotate(original, -30, rotated);
    imagelist.push_back(rotated);

    // scaled down
    int sizew = (int) (original.cols * 0.7);
    int sizeh = (int) (original.rows * 0.7);
    Size size(sizew,sizeh);//the dst image size,e.g.100x100
    Mat scaledown;//dst image
    resize(original,scaledown, size);//resize image
    imagelist.push_back(scaledown);

    // scaled up
    int sizew2 = (int) (original.cols * 1.3);
    int sizeh2 = (int) (original.rows * 1.3);
    Size size2(sizew2,sizeh2);//the dst image size,e.g.100x100
    Mat scaledup;//dst image
    resize(original,scaledup,size2);//resize image
    imagelist.push_back(scaledup);
}

/**
 * Rotate an image
 */
void Stitching::rotate(cv::Mat& src, double angle, cv::Mat& dst)
{
    int len = std::max(src.cols, src.rows);
    cv::Point2f pt(len/2., len/2.);
    cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);

    cv::warpAffine(src, dst, r, cv::Size(len, len));
}
