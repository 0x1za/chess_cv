//
// Created by Mwiza Simbeye on 13/03/2020.
//
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;

void help(char** argv ) {
    std::cout << "\n"
              << "A Sudoku OpenCV program that loads an image and extracts map.\n"
              << argv[0] <<" <path/filename>\n"
              << "For example:\n"
              << argv[0] << " ../map.jpg\n"
              << std::endl;
}


int main( int argc, char** argv ) {

    if (argc != 2) {
        help(argv);
        return 0;
    }

    // Load image
    Mat img = imread( argv[1], 0);
    Mat outerBox = Mat(img.size(), CV_8UC1);

    // Smoothing
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