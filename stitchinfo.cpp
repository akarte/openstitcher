#include "stitchinfo.h"

StitchInfo::StitchInfo()
{
    this->i = -1;
    this->x = 0;
    this->y = 0;
    this->w = 0;
    this->theta = 0;
}

StitchInfo::~StitchInfo()
{
}

StitchInfo::StitchInfo(int i, double x, double y)
{
    this->i = i;
    this->x = x;
    this->y = y;
    this->w = 0;
    this->theta = 0;
}
