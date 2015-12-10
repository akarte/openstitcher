#ifndef FASTSTITCHER_H
#define FASTSTITCHER_H

#include <QDebug>

#include "stitchdata.h"
#include "stitchinfo.h"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/tracking.hpp"

using namespace std;
using namespace cv;

class FastStitcher : public QObject
{
    Q_OBJECT
public:
    explicit FastStitcher();
    ~FastStitcher();

    static int max_orb_features;
    static float match_threshold;

    void stitch(vector<Mat> &images,
                vector<StitchInfo> &imageinfo);

private:
    vector<KeyPoint> keypointDetection(Mat image, OrbFeatureDetector &detector);

    Mat descriptorExtraction(Mat image,
                             vector<KeyPoint> keypoints,
                             OrbDescriptorExtractor &extractor);

    vector<DMatch> matchImages(Mat &baseD, Mat &currD);

    Mat getHomography(vector<DMatch> goodMatches,
            vector<KeyPoint> baseK,
            vector<KeyPoint> currK);

    int ratioTest(vector<vector<DMatch> > &matches, float ratio);

    void symmetryTest(
            const vector<vector<DMatch> >& matches1,
            const vector<vector<DMatch> >& matches2,
            vector<DMatch>& symMatches);

    bool alreadyExist(StitchData,
                      vector<StitchData>);

    void clearMatches(vector< StitchData > &matchlist,
                      vector< StitchData > &matchlist_aux,
                      vector< pair<unsigned int, unsigned int> > &weightmap);

    void weightSort(vector< pair<unsigned int, unsigned int> > &weightmap);

    void getMatches(vector<Mat> &images, vector< StitchData > &matchlist,
                                  vector< pair<unsigned int, unsigned int> > &weightmap, vector<Mat> &descriptorlist);


    void sortItems(vector<StitchData> &list,
                   vector<StitchData> &sortedlist);

    void removeIfExist(unsigned int &number, vector< unsigned int > &list);
    void addIfNotExist(unsigned int &number, vector< unsigned int > &list);
    void appendToList(unsigned int id, vector< StitchData > &list, vector< StitchData > &sortedlist, vector< unsigned int > &bufferlist);
    void appendToList(vector< unsigned int > &bufferlist, vector< StitchData > &list, vector< StitchData > &sortedlist);

    void applyTransformations(vector<StitchData> matchlist,
                              vector<StitchInfo> &imageinfo_aux, StitchInfo pivot);
    int getWeightIndex(int index,
                       vector<StitchInfo> vectorlist);

    void moveToTop(vector<StitchInfo> &list, StitchInfo *item );
    void moveToBottom(vector< StitchInfo > &list, StitchInfo *item );

signals:
    void appendText(string text2append);
};

#endif // FASTSTITCHER_H
