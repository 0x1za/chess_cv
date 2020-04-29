//
// Created by Mwiza Simbeye on 01/04/2020.
//
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include "gflags/gflags.h"
#include "glog/logging.h"

using namespace cv;
using namespace std;

DEFINE_bool(reset, true, "Include 'advanced' options in the menu listing");
DEFINE_string(image_path, "", "Path to image for camara calibration.");

int main(int argc, char** argv) {
    // Initialise flag parsing.
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    
    CHECK(!FLAGS_image_path.empty())
      << "-image_path is missing.";

    cout << FLAGS_image_path << endl;

    // Load image
    Mat img = imread(FLAGS_image_path, 0);
    Mat outerBox = Mat(img.size(), CV_8UC1);

    // Smoothin
    GaussianBlur(img, img, Size(11, 11), 0);
    adaptiveThreshold(img, outerBox, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5, 2);
    bitwise_not(outerBox, outerBox);

    // Dilation
    Mat kernel = (Mat_<uchar>(3,3) << 0,1,0,1,1,1,0,1,0);
    dilate(outerBox, outerBox, kernel);

    if( img.empty() ) return -1;
    imshow("Map", outerBox);
    waitKey(0);
    destroyWindow("Map");
}
