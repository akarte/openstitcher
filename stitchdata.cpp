#include "stitchdata.h"

#include <vector>

using namespace std;
using namespace cv;

StitchData::StitchData()
{
    this->parent = 0;
}

StitchData::StitchData(vector<DMatch> matches, pair<unsigned int, unsigned int> pairs)
{
    this->matches = matches;
    this->pairs = pairs;
    this->parent = -1;
}

StitchData::~StitchData()
{
}

