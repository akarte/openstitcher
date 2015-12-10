#ifndef STITCHINFO_H
#define STITCHINFO_H


class StitchInfo
{
public:
    StitchInfo();
    StitchInfo(int i, double x, double y);
    ~StitchInfo();
    /// Image index
    int i;
    /// X position
    double x;
    /// Y position
    double y;
    /// Angle rotation
    double theta;
    /// Weight based of numer of
    /// descriptors found
    int w;
};

#endif // STITCHINFO_H
