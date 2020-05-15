// Start
// Created by Mwiza Simbeye on 01/04/2020.
//

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using namespace cv;
using namespace std;

ABSL_FLAG(bool, debug, true, "Put program in debug mode");
ABSL_FLAG(string, image_path, "board.png",
          "Path to image save initial board detection image file..");
ABSL_FLAG(int16_t, n_boards, 1, "Number of boards to locate");
ABSL_FLAG(double, image_sf, 0.5f, "Image sf");
ABSL_FLAG(double, delay, 1.f, "Delay");
ABSL_FLAG(int16_t, board_w, 9, "Board width");
ABSL_FLAG(int16_t, board_h, 6, "Board height");

/// Finding contours
Mat src_gray;
int thresh = 100;
RNG rng(12345);
int largest_contour_index = 0;
int largest_area = 0;

// Images 
Mat frame;

// Get flag params
int board_w = absl::GetFlag(FLAGS_board_w);
int board_h = absl::GetFlag(FLAGS_board_h);
int n_boards = absl::GetFlag(FLAGS_n_boards);
float delay = absl::GetFlag(FLAGS_delay);
float image_sf = absl::GetFlag(FLAGS_image_sf);

// Calculate the board size
int board_n = board_w * board_h;

Size board_sz = Size(board_w, board_h);

// Allocate storage, it is important to have the image and object points for
// calibration.
vector<vector<Point2f>> image_points;
vector<vector<Point3f>> object_points;

// Capture corner views, we loop until we have got n_boards successfully
Size image_size;

bool findChessBoard(VideoCapture capture) {
  // Check if chessboard exists in the image.
  while (image_points.size() < (size_t)n_boards) {
    Mat image0, image;
    capture >> image0;
    image_size = image0.size();
    resize(image0, image, Size(), image_sf, image_sf, INTER_LINEAR);

    // Find the board, the fun stuff.
    vector<Point2f> corners;
    bool found = findChessboardCorners(image, board_sz, corners);

    // Draw it
    drawChessboardCorners(image, board_sz, corners, found);

    // If a board was found, add it to our data.
    //

    if (found) {
      image ^= Scalar::all(255);

      Mat mcorners(corners);
      mcorners *= (1. / image_sf);
      image_points.push_back(corners);
      object_points.push_back(vector<Point3f>());
      vector<Point3f> &opts = object_points.back();
      opts.resize(board_n);

      for (int j = 0; j < board_n; j++) {
        opts[j] = Point3f((float)(j / board_w), (float)(j % board_w), 0.f);
      }
      cout << "Collected our " << (int)image_points.size() << " of " << n_boards
           << " needed chessboard images\n"
           << endl;
      imshow("Calibration", image);

      return true;
    } else {
      return false;
    }
  }
}

void thresh_callback(int, void *) {
  double maxArea;
  int maxContourId = -1;
  Mat canny_output;
  Rect bounding_rect;

  // Canny edge detection.
  Canny(src_gray, canny_output, thresh, thresh * 2);

  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(canny_output, contours, hierarchy, RETR_TREE,
               CHAIN_APPROX_SIMPLE);
  Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
  for (size_t i = 0; i < contours.size(); i++) { 
    double area = contourArea(contours.at(i));
    if (area > maxArea) {
      maxArea = area;
      maxContourId = i;
      bounding_rect = boundingRect(contours[i]);
    }
  }

  Scalar color = Scalar(0, 255, 0);
  drawContours(frame, contours,maxContourId, color, 3); // Draw the largest contour using previously stored index.

  imshow( "Chessboard", frame);
  waitKey();
}

tuple<Mat, Mat> cleanImage(Mat img) {
  // Ideally we want to resize the image but we will do that later
  // TODO (mwizasimbeye11): Add resize op
  Mat gray;
  Mat outerBox = Mat(img.size(), CV_8UC1);
  // cvtColor(img, gray, CV_BGR2GRAY);
  imshow("B2G", img);
  waitKey(0);

  // Smoothin
  // GaussianBlur(img, img, Size(11, 11), 0);
  adaptiveThreshold(img, outerBox, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,
                    125, 1);
  Mat kernel = (Mat_<uchar>(5, 5) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
  dilate(outerBox, outerBox, kernel);

  if (absl::GetFlag(FLAGS_debug)) {
    imwrite("adapt_board.png", outerBox);
    imshow("Map", outerBox);
    waitKey(0);
    destroyWindow("Adaptive Threshold");
  }

  return {outerBox, img};
}

Mat findCornersMask(Mat adaptiveThresh, Mat img) {
  Mat imageCopy;
  imageCopy = img.clone();
  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(adaptiveThresh, contours, hierarchy, RETR_TREE,
               CHAIN_APPROX_SIMPLE);

  Rect bounding_rect;
  double Lratio;
  double Lperimeter;
  double ratio;
  vector<Point> largest;
  double Larea;
  int largest_area = 0;
  double perimeter;
  double area;
  int largest_contour_index = 0;

  cout << "Contours: " << sizeof(contours) << endl;

  for (int i = 0, size = sizeof(contours); i != size; ++i) {
    area = contourArea(contours[i]);
    perimeter = arcLength(contours[i], true);

    if (i == 0) {
      Lratio = 0;
    }

    if (perimeter > 0) {
      ratio = area / perimeter;
      if (ratio > Lratio) {
        largest = contours[i];
        Lratio = ratio;
        Lperimeter = perimeter;
        Larea = area;
      }
    }
    cout << Larea << endl;
    double a = contourArea(contours[i]);
    if (a > largest_area) {
      largest_area = a;
      largest_contour_index = i;
      cout << i << " area  " << a << endl;
      bounding_rect = boundingRect(contours[i]);
    }
  }
  Scalar color(0, 255, 0);
  // drawContours(imageCopy, largest, -1, color, CV_FILLED, 8, hierarchy);
  drawContours(img, contours, largest_contour_index, color, CV_FILLED, 8,
               hierarchy);
  rectangle(img, bounding_rect, Scalar(0, 0, 255), 2, 8, 0);
  imshow("Components", img);
  waitKey(0);

  return imageCopy;
}

int main(int argc, char **argv) {
  bool found = false;
  // Initialise flag parsing.
  absl::ParseCommandLine(argc, argv);
  // gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Start camera and take a picture.
  VideoCapture camera(0);
  if (!camera.isOpened()) {
    cerr << "ERROR: Could not open camera" << endl;
    return 1;
  }
  Mat image;
  while (found == false) {
    camera >> frame;
    if (frame.empty())
      break; // end of video stream
    // Find chessboard
    bool chessboard_find = findChessBoard(camera);
    if (chessboard_find) {
      cout << "Found: " << (bool)chessboard_find << endl;
      imwrite(absl::GetFlag(FLAGS_image_path), frame);
      // imwrite(FLAGS_image_path, frame);
      found = true;
      if (absl::GetFlag(FLAGS_debug)) {
        imshow("Board", frame);
        waitKey(0);
        destroyWindow("Board");
      }
      // Board detection ops
      // We will perform adaptive thresholding on the image to extract the
      // board.
      src_gray = imread(absl::GetFlag(FLAGS_image_path), 0);
      // cout << src_gray << endl;
      // tuple<Mat, Mat> result = cleanImage(src_gray);
      // findCornersMask(get<0>(result), get<1>(result));
      blur(src_gray, src_gray, Size(3, 3));
      const char *source_window = "Source";
      namedWindow(source_window);
      imshow(source_window, src_gray);
      const int max_thresh = 255;
      createTrackbar("Canny thresh:", source_window, &thresh, max_thresh,
                     thresh_callback);
      thresh_callback(0, 0);
      waitKey();

      // stop capturing if the chessboard is detected.
    } else {
      cout << "INFO: Searching for chessboard...." << endl;
    }
  }

  // the camera will be closed automatically upon exit
  // cap.close();
  return 0;
}
