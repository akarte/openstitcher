#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>

#include <QElapsedTimer>

#include <string>     // std::string, std::to_string

using namespace cv;
using namespace std;

// Global variables
Mat src, src_gray;
Mat harris_dst; Mat harris_copy; Mat Mc;

int harris_qualityLevel = 50;
int max_qualityLevel = 100;

double harris_minVal; double harris_maxVal;

RNG rng(12345);

const char* harris_window = "Harris corner detector";

// Timer to count time (duh)
QElapsedTimer timer;
int elapsedTime;
ofstream myfile;

// Function headers
void harris_detector(Mat &image, int index);
int harris_function(int &index, int, void* );
void sift_detector(Mat &image, int index);
void surf_detector(Mat &image, int index);
void orb_detector(Mat &image, int index);
string NumberToString (int Number );

void harris_detector(Mat &image, int index)
{
    // Open file (closed in orb)
    myfile.open("data_test.txt", std::ios_base::app);

    // Load source image and convert it to gray
    src = image;
    cvtColor( src, src_gray, COLOR_BGR2GRAY );

    // Count time
    timer.start();

    // Set parameters
    int blockSize = 3; int apertureSize = 3;

    // Harris matrix -- Using cornerEigenValsAndVecs
    harris_dst = Mat::zeros( src_gray.size(), CV_32FC(6) );
    Mc = Mat::zeros( src_gray.size(), CV_32FC1 );

    cornerEigenValsAndVecs( src_gray, harris_dst, blockSize, apertureSize, BORDER_DEFAULT );

    /* calculate Mc */
    for( int j = 0; j < src_gray.rows; j++ )
    { for( int i = 0; i < src_gray.cols; i++ )
        {
            float lambda_1 = harris_dst.at<Vec6f>(j, i)[0];
            float lambda_2 = harris_dst.at<Vec6f>(j, i)[1];
            Mc.at<float>(j,i) = lambda_1*lambda_2 - 0.04f*pow( ( lambda_1 + lambda_2 ), 2 );
        }
    }

    minMaxLoc( Mc, &harris_minVal, &harris_maxVal, 0, 0, Mat() );

    // elapsed time
    elapsedTime = timer.elapsed();

    /* Create Window and Trackbar */
    //imshow( harris_window + NumberToString(index), WINDOW_AUTOSIZE );
    //createTrackbar( " Quality Level:", harris_window, &harris_qualityLevel, max_qualityLevel, harris_function );
    int features = harris_function(index, 0, 0 );

    myfile << "\n" + NumberToString(index) + "\t" + NumberToString(features) + "\t0\t" + NumberToString(elapsedTime) + "\t";
}

void sift_detector(Mat &image, int index)
{
    // Count time
    timer.start();

    SiftFeatureDetector detector( 0.05, 5.0 );
    SiftDescriptorExtractor extractor;
    vector<cv::KeyPoint> keypoints;

    // Get keypoints
    detector.detect(image, keypoints);

    // Computing descriptors
    Mat descriptors;
    extractor.compute( image, keypoints, descriptors);

    // elapsed time
    int features = keypoints.size();
    int descript = descriptors.rows;
    elapsedTime = timer.elapsed();
    myfile << NumberToString(features) + "\t" +
              NumberToString(descript) + "\t" +
              NumberToString(elapsedTime) + "\t";

    // Add results to image and save.
    Mat output;
    drawKeypoints(image, keypoints, output, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    imshow("SIFT detector" + NumberToString(index), output);
}

void surf_detector(Mat &image, int index)
{
    // Count time
    timer.start();

    SurfFeatureDetector detector(400);
    SurfDescriptorExtractor extractor;
    vector<cv::KeyPoint> keypoints;

    // Get keypoints
    detector.detect(image, keypoints);

    // Computing descriptors
    Mat descriptors;
    extractor.compute( image, keypoints, descriptors);

    // elapsed time
    int features = keypoints.size();
    int descript = descriptors.rows;
    elapsedTime = timer.elapsed();
    myfile << NumberToString(features) + "\t" +
              NumberToString(descript) + "\t" +
              NumberToString(elapsedTime) + "\t";

    // Add results to image and save.
    Mat output;
    drawKeypoints(image, keypoints, output, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    imshow("SURF detector" + NumberToString(index), output);
}

void orb_detector(Mat &image, int index)
{
    // Count time
    timer.start();

    OrbFeatureDetector detector(600);
    OrbDescriptorExtractor extractor;
    vector<cv::KeyPoint> keypoints;

    // get keypoints
    detector.detect(image, keypoints);

    // Computing descriptors
    Mat descriptors;
    extractor.compute( image, keypoints, descriptors);

    // elapsed time
    int features = keypoints.size();
    int descript = descriptors.rows;
    elapsedTime = timer.elapsed();
    myfile << NumberToString(features) + "\t" +
              NumberToString(descript) + "\t" +
              NumberToString(elapsedTime) + "\t";

    // Add results to image and save.
    Mat output;
    drawKeypoints(image, keypoints, output, Scalar::all(255), 4);
    imshow("ORB detector" + NumberToString(index), output);

    std::stringstream sstm;
    sstm << "image" << index << ".jpg";
    string result = sstm.str();

    imwrite( result, output );

    // close file
    myfile.close();
}

void orbMatching(Mat &img1, Mat &img2)
{
    OrbFeatureDetector detector(600);
    OrbDescriptorExtractor extractor;
    vector<cv::KeyPoint> keypoints1, keypoints2;

    // get keypoints
    detector.detect(img1, keypoints1);
    detector.detect(img2, keypoints2);

    // Computing descriptors
    Mat descriptors1, descriptors2;
    extractor.compute( img1, keypoints1, descriptors1);
    extractor.compute( img2, keypoints2, descriptors2);

    //-- Step 3: Matching descriptor vectors with a brute force matcher
    BFMatcher matcher(NORM_L2);
    std::vector< DMatch > matches;
    matcher.match( descriptors1, descriptors2, matches );

    //-- Draw matches
    Mat img_matches;
    drawMatches( img1, keypoints1, img2, keypoints2, matches, img_matches, Scalar::all(255) );

    //-- Show detected matches
    imshow("Matches", img_matches );

    std::stringstream sstm;
    sstm << "match" << 1 << ".jpg";
    string result = sstm.str();

    imwrite( result, img_matches );
}

int harris_function(int &index, int, void* )
{
    int features = 0;
    harris_copy = src.clone();

    if( harris_qualityLevel < 1 ) { harris_qualityLevel = 1; }

    for( int j = 0; j < src_gray.rows; j++ )
    { for( int i = 0; i < src_gray.cols; i++ )
        {
            if( Mc.at<float>(j,i) > harris_minVal + ( harris_maxVal - harris_minVal )*harris_qualityLevel/max_qualityLevel )
            {
                if (features > 500) continue;
                features++; circle( harris_copy, Point(i,j), 4, Scalar( rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255) ), -1, 8, 0 ); }
        }
    }
    imshow( harris_window + NumberToString(index), harris_copy );

    return features;
}

string NumberToString( int Number )
{
    ostringstream ss;
    ss << Number;
    return ss.str();
}
