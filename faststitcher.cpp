#include "faststitcher.h"
#include "stitchdata.h"
#include "stitchinfo.h"

#include <vector>
#include <algorithm>
#include <QTime>

using namespace std;
using namespace cv;

Q_DECLARE_METATYPE(std::string)

int FastStitcher::max_orb_features = 1000;
float FastStitcher::match_threshold = 0.7f;

FastStitcher::FastStitcher()
{

}

FastStitcher::~FastStitcher()
{

}

string IntToString( int Number )
{
    ostringstream ss;
    ss << Number;
    return ss.str();
}

string DoubleToString(double Number)
{
    ostringstream ss;
    ss << Number;
    return ss.str();
}

bool weightsorter(const pair<unsigned int, unsigned int> &i, const pair<unsigned int, unsigned int> &j)
{
    return i.second >= j.second;
}

bool matchsorter(const StitchData &i, const StitchData &j)
{
    return i.matches.size() >= j.matches.size();
}

void FastStitcher::stitch(vector<Mat> &images,
                          vector<StitchInfo> &imageinfo)
{
    emit appendText("Extracting keypoints...");
    QTime myTimer;
    myTimer.start();
    /// EXTRACT KEYPOINTS
    // ORB Detector (will detect max_orb_features number of orb descriptors
    OrbFeatureDetector detector(FastStitcher::max_orb_features);
    // Keypoints vector
    vector<vector<KeyPoint> > keypointlist;
    // Detect all keypoints
    for(size_t i = 0; i < images.size(); i++){
        // Get image keypoints
        keypointlist.push_back(keypointDetection(images.at(i), detector));
    }

    emit appendText("Computing descriptors...");
    /// COMPUTE DESCRIPTORS
    OrbDescriptorExtractor extractor;
    // Descriptor vector
    vector<Mat> descriptorlist;
    for(size_t i = 0; i < images.size(); i++){
        // Get image descriptors
        descriptorlist.push_back(descriptorExtraction(images.at(i), keypointlist.at(i), extractor));
    }

    emit appendText("Matching images...");
    /// MATCH IMAGES
    vector< StitchData > matchlist;
    vector<pair<unsigned int, unsigned int> > weightmap(images.size());
    getMatches(images, matchlist, weightmap, descriptorlist);

    emit appendText("Sorting...");
    /// WEIGHT SORT COMPARISON
    weightSort(weightmap);

    emit appendText("Removing repetitive matches...");
    /// CLEAR MATCHES
    // Remove repeated matched images to translate only once every image
    vector< StitchData > matchlist_aux;
    clearMatches(matchlist, matchlist_aux, weightmap);

    emit appendText("Re-sorting matches...");
    /// SORT MATCHES
    sortItems(matchlist_aux, matchlist);

    emit appendText("Getting transformation data...");
    /// GET TRANSFORMATION
    // Get transformations between images if were found any goodMatches
    vector< StitchData >::iterator it = matchlist.begin();
    while(it != matchlist.end()) {
        StitchData &item = *it;

        vector<DMatch> match = item.matches;
        pair<int,int> pairs = item.pairs;

        Mat rigidTransform = getHomography(match,
                              keypointlist.at(pairs.first),
                              keypointlist.at(pairs.second));

        if(rigidTransform.empty()) {
            it = matchlist.erase(it);
        } else {
            item.transform = rigidTransform;
            ++it;
        }
    }

    emit appendText("Applying transformation data...");
    /// APPLYING TRANSFORMATIONS
    vector< StitchInfo > imageinfo_aux(weightmap.size());
    for(int i = 0; i < weightmap.size(); i++){
        StitchInfo* info = &imageinfo_aux.at(i);
        info->i = weightmap.at(i).first;
        info->w = weightmap.at(i).second;
        info->x = 0;
        info->y = 0;
    }

    weightmap.clear();

    StitchInfo* info = &imageinfo_aux.at(0);

    emit appendText(
                "Pivot image:" + IntToString(info->i)
                + ", Traslation (" + IntToString(info->x)
                + "," + IntToString(info->y) + ")");

    applyTransformations(matchlist, imageinfo_aux, *info);

    imageinfo = imageinfo_aux;
    int nMilliseconds = myTimer.elapsed();

    emit appendText("Total time: " + IntToString(nMilliseconds));
}

void FastStitcher::moveToTop(vector< StitchInfo > &list, StitchInfo *item ){
    for(vector< StitchInfo >::iterator it = list.begin(); it != list.end();) {
        StitchInfo data = *it;
        if(data.i == item->i){
            std::rotate(list.begin(), it, it + 1);
            break;
        }
        ++it;
    }
}

void FastStitcher::moveToBottom(vector< StitchInfo > &list, StitchInfo *item ){
    for(vector< StitchInfo >::iterator it = list.begin(); it != list.end();) {
        StitchInfo data = *it;
        if(data.i == item->i){
            std::rotate(it, it + 1, list.end());
            break;
        }
        ++it;
    }
}

void FastStitcher::applyTransformations(vector< StitchData > matchlist,
                                        vector< StitchInfo > &imageinfo_aux,
                                        StitchInfo pivot){
    // Move current pivot to bottom of list
    moveToBottom(imageinfo_aux, &pivot);

    for(vector< StitchData >::iterator it = matchlist.begin(); it != matchlist.end();) {
        StitchData data = *it;
        double xtran = data.transform.at<double>(0, 2);
        double ytran = data.transform.at<double>(1, 2);
        double theta1 = data.transform.at<double>(1, 0);
        double theta2 = data.transform.at<double>(1, 1);

        if(data.pairs.first == pivot.i){
            it = matchlist.erase(it);
            unsigned int index = getWeightIndex(data.pairs.second, imageinfo_aux);
            StitchInfo* info = &imageinfo_aux.at(index);

            emit appendText(
                "Pivot type: PIVOT-left, Image index: " + IntToString(pivot.x) +
                " current position: (" + DoubleToString(xtran) + "," + DoubleToString(ytran) + ")");

            info->x = pivot.x + xtran;
            info->y = pivot.y + ytran;
            info->theta = atan2(theta1, theta2);
            moveToTop(imageinfo_aux, info);
            emit appendText(
                "...indexed to " + IntToString(pivot.x) +
                " : moved to(" + DoubleToString(info->x) + "," + DoubleToString(info->y) + ")");
        } else if(data.pairs.second == pivot.i){
            it = matchlist.erase(it);
            unsigned int index = getWeightIndex(data.pairs.first, imageinfo_aux);
            StitchInfo* info = &imageinfo_aux.at(index);

            emit appendText(
                "Pivot type: right-PIVOT " + IntToString(pivot.x) +
                " Translation: (" + DoubleToString(xtran) + "," + DoubleToString(ytran) + ")");

            info->x = pivot.x - xtran;
            info->y = pivot.y - ytran;
            info->theta = atan2(theta2, theta1);
            moveToTop(imageinfo_aux, info);

            emit appendText(
                "...indexed to " + IntToString(pivot.x) +
                " : moved to(" + DoubleToString(info->x) + "," + DoubleToString(info->y) + ")");
        } else {
            ++it;
        }
    }

    emit appendText("");

    if(!matchlist.empty()){
        StitchInfo* newinfo = &imageinfo_aux.at(0);
        emit appendText(
                    "Pivot image:" + IntToString(newinfo->i)
                    + ", Traslation (" + IntToString(newinfo->x)
                    + "," + IntToString(newinfo->y) + ")");

        applyTransformations(matchlist, imageinfo_aux, *newinfo);
    }
}

int FastStitcher::getWeightIndex(int index, vector< StitchInfo > vectorlist){
    for(int i = 0; i < vectorlist.size(); i++){
        StitchInfo info = vectorlist.at(i);
        if(index == info.i) return i;
    }
    return -1;
}

void FastStitcher::clearMatches(vector< StitchData > &matchlist,
                                vector< StitchData > &matchlist_aux,
                                vector< pair<unsigned int, unsigned int> > &weightmap){

    for(unsigned int i = 0; i < weightmap.size(); i++){
        pair<unsigned int, unsigned int> weightpair = weightmap.at(i);

        for(vector< StitchData >::iterator it = matchlist.begin(); it != matchlist.end();) {
            StitchData current = *it;
            if (weightpair.first == current.pairs.first || weightpair.first == current.pairs.second){
                if(matchlist_aux.size() >= i + 1){
                    StitchData amatch = matchlist_aux.at(i);
                    if(amatch.matches.size() < current.matches.size()){
                        matchlist_aux.at(i) = current;
                        emit appendText(
                            "... Position "
                            + IntToString(i) + " replaced by pair ["
                            + IntToString(current.pairs.first) + ","
                            + IntToString(current.pairs.second) + "]");
                        it = matchlist.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    matchlist_aux.push_back(current);
                    emit appendText(
                        "Position "
                        + IntToString(i) + " pair ["
                        + IntToString(current.pairs.first) + ","
                        + IntToString(current.pairs.second) + "]");
                    it = matchlist.erase(it);
                }
            } else {
                ++it;
            }
        }
    }
    matchlist.clear();
}

void FastStitcher::weightSort(vector< pair<unsigned int, unsigned int> > &weightmap){
    // sort weightmap from bigest to lowest
    std::sort(weightmap.begin(), weightmap.end(), weightsorter);
}

void FastStitcher::getMatches(vector<Mat> &images,
                              vector< StitchData > &matchlist,
                              vector<pair<unsigned int, unsigned int> > &weightmap,
                              vector< Mat> &descriptorlist){
    for(unsigned int i = 0; i < images.size(); i++){
        for(unsigned int j = i; j < images.size(); j++){
            // don't compare with itself
            if(i == j) continue;
            // don't compare with itself inverted
            bool skipinverted = false;
            for(unsigned int k = 0; k < matchlist.size(); k++){
                StitchData kpair = matchlist.at(k);
                if(kpair.pairs.first == j && kpair.pairs.second == i){
                    skipinverted = true;
                    break;
                }
            }
            if(skipinverted) continue;

            // get match between images
            vector<DMatch> match = matchImages(descriptorlist.at(i), descriptorlist.at(j));
            // if match exist between current image, add match and index
            if(match.size() > 1){
                matchlist.push_back(StitchData(match, pair<int, int>(i,j)));
                weightmap.at(i).first = i;
                weightmap.at(i).second += match.size();

                weightmap.at(j).first = j;
                weightmap.at(j).second += match.size();

                emit appendText(
                    "Base image: [" + IntToString(i)
                    + "] match with image [" + IntToString(j)
                    + "] with [" + IntToString(match.size()) + "] inline descriptors");
            }
        }
    }
}

void FastStitcher::sortItems(vector< StitchData > &list,
                             vector< StitchData > &sortedlist){
    if(sortedlist.empty()){
        // add first item to sortedlist
        sortedlist.push_back(list.at(0));
        // remove this element
        list.erase(list.begin());
    }

    // last item in sortedlist
    vector< unsigned int > bufferlist;
    bufferlist.push_back(sortedlist.at(sortedlist.size() - 1).pairs.first);
    bufferlist.push_back(sortedlist.at(sortedlist.size() - 1).pairs.second);
    // append childs
    appendToList(bufferlist, list, sortedlist);
}

void FastStitcher::appendToList(vector< unsigned int > &bufferlist,
                                vector< StitchData > &list,
                                vector< StitchData > &sortedlist){
    for(vector< unsigned int >::iterator it = bufferlist.begin(); it != bufferlist.end();){
        if(bufferlist.empty()) return;
        unsigned int i = *it;
        appendToList(i, list, sortedlist, bufferlist);
        removeIfExist(i, bufferlist);
        ++it;
    }
    if(!list.empty() && !bufferlist.empty())  appendToList(bufferlist, list, sortedlist);
}

void FastStitcher::appendToList(unsigned int id,
                                vector< StitchData > &list,
                                vector< StitchData > &sortedlist,
                                vector< unsigned int > &bufferlist){
    for(vector< StitchData >::iterator it = list.begin(); it != list.end();){
        StitchData item = *it;
        item.parent = id;
        if(id == item.pairs.first){
            sortedlist.push_back(item);
            addIfNotExist(item.pairs.second, bufferlist);
            it = list.erase(it);
         } else if(id == item.pairs.second){
            sortedlist.push_back(item);
            addIfNotExist(item.pairs.first, bufferlist);
            it = list.erase(it);
        } else {
            ++it;
        }
    }
}

void FastStitcher::addIfNotExist(unsigned int &number,
                                 vector< unsigned int > &list){
    bool exist = false;
    for(unsigned int i = 0; i < list.size(); i++){
        if(list.at(i) == number) exist = true;
    }
    if(!exist) list.push_back(number);
}

void FastStitcher::removeIfExist(unsigned int &number, vector< unsigned int > &list){
    for(vector< unsigned int >::iterator it = list.begin(); it != list.end();){
        unsigned int i = *it;
        if(i == number){
            it = list.erase(it);
        } else {
            ++it;
        }
    }
}

bool FastStitcher::alreadyExist(StitchData item,
                                vector< StitchData > list){
    for(unsigned int i = 0; i < list.size(); i++){
        pair<unsigned int, unsigned int> curr = list.at(i).pairs;
        if(item.pairs.first == curr.first && item.pairs.second == curr.second)
            return true;
    }
    return false;
}

///
/// \brief TrackingStitching::keypointDetection Detect keypoints from a given image
/// \param image    Source image where keypoints will be computed
/// \return keypoints vector
///
vector<KeyPoint> FastStitcher::keypointDetection(Mat image, OrbFeatureDetector &detector)
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
Mat FastStitcher::descriptorExtraction(Mat image, vector<KeyPoint> keypoints, OrbDescriptorExtractor &extractor)
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
vector<DMatch> FastStitcher::matchImages(Mat &baseD, Mat &currD)
{
    // Matcher object
    BFMatcher matcher(cv::NORM_HAMMING);
    //FlannBasedMatcher matcher(new cv::flann::LshIndexParams(20, 20, 2));
    // matches object
    std::vector< vector< DMatch > > matches1, matches2;
    // match base to current
    matcher.knnMatch(baseD, currD, matches1, 2);
    // match current to base
    matcher.knnMatch(currD, baseD, matches2, 2);

    // Thresholding
    int removed = ratioTest(matches1, match_threshold);
    removed = ratioTest(matches2, match_threshold);

    // Remove non symmetrical matchs
    vector<cv::DMatch> symMatches;
    symmetryTest(matches1,matches2,symMatches);

    return symMatches;
}

Mat FastStitcher::getHomography(vector<DMatch> goodMatches,
                                vector<KeyPoint> baseK, vector<KeyPoint> currK)
{
    // Get Points
    std::vector<Point2f> baseP, currP;

    for( unsigned int i = 0; i < goodMatches.size(); i++ )
    {
      // Get the keypoints from the good matches
      baseP.push_back( baseK[ goodMatches[i].queryIdx ].pt );
      currP.push_back( currK[ goodMatches[i].trainIdx ].pt );
    }
    // Estimate rigid transformation (only rotation, translation, scalation allowed)
    Mat R = cv::estimateRigidTransform(currP, baseP, false);
    // If no R stimated return
    if(R.cols == 0) {
        return Mat();
    }
    return R;
}

/// Clear matches for which NN ratio is > than threshold
/// return the number of removed points
/// (corresponding entries being cleared,
/// i.e. size will be 0)
int FastStitcher::ratioTest(std::vector<std::vector<cv::DMatch> > &matches, float ratio)
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
void FastStitcher::symmetryTest(
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
