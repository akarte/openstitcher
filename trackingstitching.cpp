#include "trackingstitching.h"
#include <QDebug>

using namespace std;
using namespace cv;

string TrackingStitching::features_type = "orb";
float TrackingStitching::match_threshold = 0.7f;

TrackingStitching::TrackingStitching(string feature_type)
{
    this->features_type = feature_type;
}

TrackingStitching::~TrackingStitching()
{

}

void TrackingStitching::stitch(Mat base, Mat curr, Mat &stit)
{
    /// EXTRACT KEYPOINTS
    OrbFeatureDetector detector(1000);
    vector<KeyPoint> baseK, currK;
    // Get base image keypoints
    if(!base.empty()) baseK = keypointDetection(base, detector);
    else {return;}

    // Get current image keypoints
    if(!curr.empty()) currK = keypointDetection(curr, detector);
    else {stit = base; return;}

    /// COMPUTE DESCRIPTORS
    OrbDescriptorExtractor extractor;
    Mat baseD, currD;
    // Get base descriptos
    baseD = descriptorExtraction(base, baseK, extractor);
    currD = descriptorExtraction(curr, currK, extractor);

    /// MATCH IMAGES
    vector<DMatch> goodMatches;
    // Get good matches if were found any descriptors
    if(baseD.rows > 0 && currD.rows > 0)
        goodMatches = matchImages(baseD, currD);
    else
    {
        //Draw matches
        drawMatches( base, baseK, curr, currK, goodMatches,
                     stit, Scalar::all(255), Scalar::all(255),
                     vector< char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
        return;
    }

    /// GET TRANSFORMATION
    // Get Homography if were found any goodMatches
    if(goodMatches.size() > 0) stit = getHomography(goodMatches, base, curr, baseK, currK);

    if(stit.empty()){
        //Draw matches
        drawMatches( base, baseK, curr, currK, goodMatches,
                     stit, Scalar::all(255), Scalar::all(255),
                     vector< char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    }
}

///
/// \brief TrackingStitching::keypointDetection Detect keypoints from a given image
/// \param image    Source image where keypoints will be computed
/// \return keypoints vector
///
vector<KeyPoint> TrackingStitching::keypointDetection(Mat image, OrbFeatureDetector &detector)
{
    vector<cv::KeyPoint> keypoints;
    // get keypoints
    detector.detect(image, keypoints);

    return keypoints;
}

///
/// \brief TrackingStitching::descriptorExtraction Extracts descriptors and return it in a Mat file
/// \param image    Image where descriptors will be computed
/// \param keypoints    Localization of prospect descriptors
/// \return Mat file containing descritors
///
Mat TrackingStitching::descriptorExtraction(Mat image, vector<KeyPoint> keypoints, OrbDescriptorExtractor &extractor)
{
    // Descritor matrix
    Mat descriptors;
    extractor.compute( image, keypoints, descriptors );
    return descriptors;
}

///
/// \brief TrackingStitching::matchImages match two images based on descriptors previewsly computed
/// \param base base image
/// \param current current image to be stitched
/// \return stitched image
///
vector<DMatch> TrackingStitching::matchImages(Mat baseD, Mat currD)
{
    // Matcher object
    //BFMatcher matcher(features_type == "orb" ? cv::NORM_HAMMING : NORM_L2);
    FlannBasedMatcher matcher(new cv::flann::LshIndexParams(20, 20, 2));
    // matches object
    std::vector< vector< DMatch > > matches1, matches2;
    // match base to current
    matcher.knnMatch(baseD, currD, matches1, 2);
    // match current to base
    matcher.knnMatch(currD, baseD, matches2, 2);

    qDebug() << matches1.size();
    qDebug() << matches2.size();

    // Thresholding
    int removed = ratioTest(matches1, match_threshold);
    removed = ratioTest(matches2, match_threshold);

    // Remove non symmetrical matchs
    vector<cv::DMatch> symMatches;
    symmetryTest(matches1,matches2,symMatches);

    qDebug() << "you just clicked ok";
    qDebug() << "you just clicked ok";


    qDebug() << symMatches.size();
    return symMatches;
}

Mat TrackingStitching::getHomography(vector<DMatch> goodMatches, Mat base, Mat curr, vector<KeyPoint> baseK, vector<KeyPoint> currK)
{
    // Get Points
    std::vector<Point2f> baseP, currP;

    for( int i = 0; i < goodMatches.size(); i++ )
    {
      // Get the keypoints from the good matches
      baseP.push_back( baseK[ goodMatches[i].queryIdx ].pt );
      currP.push_back( currK[ goodMatches[i].trainIdx ].pt );
    }
    // Estimate rigid transformation (only rotation, translation, scalation allowed)
    Mat R = cv::estimateRigidTransform(currP, baseP, false);

    // If no R stimated return
    if(R.cols == 0)
    {
        return Mat();
    }

    // Transform R matrix to Homography matrix
    cv::Mat H = cv::Mat(3,3,R.type());
    H.at<double>(0,0) = R.at<double>(0,0);
    H.at<double>(0,1) = R.at<double>(0,1);
    H.at<double>(0,2) = R.at<double>(0,2);

    H.at<double>(1,0) = R.at<double>(1,0);
    H.at<double>(1,1) = R.at<double>(1,1);
    H.at<double>(1,2) = R.at<double>(1,2);

    H.at<double>(2,0) = 0.0;
    H.at<double>(2,1) = 0.0;
    H.at<double>(2,2) = 1.0;

    Mat warping;

    //warp images
    // Make a desplasment to fit base Image in center of homography
    int x_offset = base.cols;
    int y_offset = base.rows;
    // create an offset matrix
    Mat offset = (Mat_<double>(3,3) << 1, 0, x_offset, 0, 1, y_offset, 0, 0, 1);
    // Apply it to Homography
    H = H * offset;
    // Warp perspective
    warpPerspective(curr, warping, H, Size( base.cols * 3, base.rows * 3 ), INTER_NEAREST);
    // ROI
    Mat roi1(warping, Rect(x_offset, y_offset, base.cols, base.rows));
    base.copyTo(roi1);

    return warping;
}

/// Clear matches for which NN ratio is > than threshold
/// return the number of removed points
/// (corresponding entries being cleared,
/// i.e. size will be 0)
int TrackingStitching::ratioTest(std::vector<std::vector<cv::DMatch> > &matches, float ratio)
{
    int removed=0;
    // for all matches
    for (std::vector<std::vector<cv::DMatch> >::iterator
         matchIterator= matches.begin();
         matchIterator!= matches.end(); ++matchIterator) {
        // if 2 NN has been identified
        if (matchIterator->size() > 1) {
            // check distance ratio
            if ((*matchIterator)[0].distance/
                    (*matchIterator)[1].distance > ratio) {
                matchIterator->clear(); // remove match
                removed++;
            }
        } else { // does not have 2 neighbours
            matchIterator->clear(); // remove match
            removed++;
        }
    }
    return removed;
}

/// Insert symmetrical matches in symMatches vector
void TrackingStitching::symmetryTest(
        const std::vector<std::vector<cv::DMatch> >& matches1,
        const std::vector<std::vector<cv::DMatch> >& matches2,
        std::vector<cv::DMatch>& symMatches)
{
    // for all matches image 1 -> image 2
    for (std::vector<std::vector<cv::DMatch> >::
         const_iterator matchIterator1= matches1.begin();
         matchIterator1!= matches1.end(); ++matchIterator1) {

        // ignore deleted matches
        if (matchIterator1->size() < 2) continue;

        // for all matches image 2 -> image 1
        for (std::vector<std::vector<cv::DMatch> >::
             const_iterator matchIterator2= matches2.begin();
             matchIterator2!= matches2.end();
             ++matchIterator2) {
            // ignore deleted matches
            if (matchIterator2->size() < 2) continue;
            // Match symmetry test
            if ((*matchIterator1)[0].queryIdx ==
                    (*matchIterator2)[0].trainIdx &&
                    (*matchIterator2)[0].queryIdx ==
                    (*matchIterator1)[0].trainIdx) {
                // add symmetrical match
                symMatches.push_back(
                            cv::DMatch((*matchIterator1)[0].queryIdx,
                            (*matchIterator1)[0].trainIdx,
                        (*matchIterator1)[0].distance));
                break; // next match in image 1 -> image 2
            }
        }
    }
}
