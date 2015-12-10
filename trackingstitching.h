#ifndef TRACKINGSTITCHING_H
#define TRACKINGSTITCHING_H

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/tracking.hpp"

using namespace std;
using namespace cv;

class TrackingStitching
{
public:
    TrackingStitching(string feature_type);
    ~TrackingStitching();

    static string features_type;
    static float match_threshold;

    void stitch(Mat base, Mat curr, Mat &stit);

private:
    vector<KeyPoint> keypointDetection(Mat image, OrbFeatureDetector &detector);
    Mat descriptorExtraction(Mat image, vector<KeyPoint> keypoints, OrbDescriptorExtractor &extractor);
    vector<DMatch> matchImages(Mat baseD, Mat currD);
    Mat getHomography(vector<DMatch> goodMatches, Mat base, Mat curr, vector<KeyPoint> baseK, vector<KeyPoint> currK);
    int ratioTest(std::vector<std::vector<cv::DMatch> > &matches, float ratio);
    void symmetryTest(
            const std::vector<std::vector<cv::DMatch> >& matches1,
            const std::vector<std::vector<cv::DMatch> >& matches2,
            std::vector<cv::DMatch>& symMatches);

};

#endif // TRACKINGSTITCHING_H
