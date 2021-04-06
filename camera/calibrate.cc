//
// Created by Mwiza Simbeye on 01/04/2020.
//
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "opencv2/opencv.hpp"
#include <iostream>

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using namespace cv;
using namespace std;

DEFINE_string(image_path, "", "Path to image for camara calibration.");
DEFINE_int32(n_boards, 0, "Number of boards to calibrate");
DEFINE_double(image_sf, 0.5f, "Image sf");
DEFINE_int32(board_w, 0, "Board width");
DEFINE_int32(board_h, 0, "Board height");

int main(int argc, char **argv) {
  // Initialise flag parsing.
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Get flag params
  int board_w = FLAGS_board_w;
  int board_h = FLAGS_board_h;
  int n_boards = FLAGS_n_boards;
  float image_sf = FLAGS_image_sf;

  // Calculate the board size
  int board_n = board_w * board_h;

  Size board_sz = Size(board_w, board_h);

  // Start video capture
  VideoCapture capture(0);
  if (!capture.isOpened()) {
    cout << "\nCouldn't open the camera\n";
    return -1;
  }

  // Allocate storage, it is important to have the image and object points for
  // calibration.
  vector<vector<Point2f>> image_points;
  vector<vector<Point3f>> object_points;

  // Capture corner views, we loop until we have got n_boards successfully
  //
  double last_captured_timestamp = 0;
  Size image_size;

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
    double timestamp = (double)clock() / CLOCKS_PER_SEC;

    if (found && timestamp - last_captured_timestamp > 1) {
      last_captured_timestamp = timestamp;
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
    }
    imshow("Calibration", image);
    if ((waitKey(30) & 255) == 27)
      return -1;
  }

  destroyWindow("Calibration");
  cout << "\n\n*** CALIBRATING THE CAMERA...\n" << endl;

  // Calibrate the Camera
  Mat intrinsic_matrix, distortion_coeff;
  double err =
      calibrateCamera(object_points, image_points, image_size, intrinsic_matrix,
                      distortion_coeff, noArray(), noArray(),
                      CALIB_ZERO_TANGENT_DIST | CALIB_FIX_PRINCIPAL_POINT);
  // Save the intrinsic and distortion values.
  cout << "*** DONE\n\nProjection error is " << err
       << "\nStoring Intrinsics.xml and Distortions.xml files\n\n";
  FileStorage fs("intrinsics.xml", FileStorage::WRITE);

  fs << "image_width" << image_size.width << "image_height" << image_size.height
     << "camera_matrix" << intrinsic_matrix << "distortion_coefficients"
     << distortion_coeff;
  fs.release();

  // Load matrix back in!!
  fs.open("intrinsics.xml", FileStorage::READ);
  cout << "\nimage_width: " << (int)fs["image_width"];
  cout << "\nimage_height: " << (int)fs["image_height"];

  Mat intrinsic_matrix_loaded, distortion_coeffs_loaded;
  fs["camera_matrix"] >> intrinsic_matrix_loaded;
  fs["distortion_coefficients"] >> distortion_coeffs_loaded;
  cout << "\nintrinsic matrix:" << intrinsic_matrix_loaded;
  cout << "\ndistortion coefficients: " << distortion_coeffs_loaded << endl;

  // Build the undistort map to be used on all subsequent frames.
  Mat map1, map2;
  initUndistortRectifyMap(intrinsic_matrix_loaded, distortion_coeffs_loaded,
                          Mat(), intrinsic_matrix_loaded, image_size, CV_16SC2,
                          map1, map2);
  // Run camera to screen.
  for (;;) {
    Mat image, image0;
    capture >> image0;
    if (image0.empty())
      break;
    remap(image0, image, map1, map2, INTER_LINEAR,
            BORDER_CONSTANT, Scalar());
    imshow("Undistorted", image);
    if ((waitKey(30) & 255) == 27)
      break;
  }

  return 0;
}
