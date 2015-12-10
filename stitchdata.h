#ifndef STITCHDATA_H
#define STITCHDATA_H

#include <QDebug>

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/tracking.hpp"

using namespace std;
using namespace cv;

class StitchData
{
public:
    StitchData();
    StitchData(vector<DMatch> matches, pair<unsigned int, unsigned int> pairs);
    ~StitchData();

    vector<DMatch> matches;
    pair<unsigned int, unsigned int> pairs;
    Mat transform;
    int parent;
};

#endif // STITCHDATA_H
