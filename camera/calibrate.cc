/*
 * This file is part of chess_cv.camera.calibrate
 *
 * Created by Mwiza Simbeye on 01/04/2020.
 *
 * This program calibrates cameras.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "opencv2/opencv.hpp"
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using namespace cv;
using namespace std;

ABSL_FLAG(string, image_path, "", "Path to image for camara calibration.");
ABSL_FLAG(int, n_boards, 10, "Number of boards to calibrate");
ABSL_FLAG(float, image_sf, 0.5f, "Image sf");
ABSL_FLAG(int, board_w, 7, "Board width");
ABSL_FLAG(int, board_h, 9, "Board height");
ABSL_FLAG(bool, use_fisheye, false, "Use fish eye camera");
ABSL_FLAG(bool, use_rtsp, false, "Capture video from RTSP");
ABSL_FLAG(string, rtsp_url, "rtsp://192.168.1.26:8554/unicast",
          "URL for RTSP video.");

int main(int argc, char **argv) {
  // Initialise flag parsing.
  absl::ParseCommandLine(argc, argv);

  // Get flag params
  int board_w = absl::GetFlag(FLAGS_board_w);
  int board_h = absl::GetFlag(FLAGS_board_h);
  int n_boards = absl::GetFlag(FLAGS_n_boards);
  float image_sf = absl::GetFlag(FLAGS_image_sf);
  bool use_rtsp = absl::GetFlag(FLAGS_use_rtsp);
  bool use_fisheye = absl::GetFlag(FLAGS_use_fisheye);
  string rtsp_url = absl::GetFlag(FLAGS_rtsp_url);

  // Calculate the board size
  int board_n = board_w * board_h;

  VideoCapture capture;

  Size board_sz = Size(board_w, board_h);

  if (use_rtsp) {
    VideoCapture capture(rtsp_url);
    if (!capture.isOpened()) {
      cout << "\nCouldn't open the camera\n";
      return -1;
    } else if (capture.isOpened()) {
      cout << "RTSP Stream Live from " << rtsp_url << endl;
    }

    double fps = capture.get(CAP_PROP_FPS);
    string d_fps = "Frames per seconds: ";
    d_fps += to_string(int(fps) / int(1000));

    // Create window and title
    String window_name = "Chess Stream";
    namedWindow(window_name, WINDOW_NORMAL);

    while (true) {
      Mat frame;
      bool success = capture.read(frame);

      if (!success) {
        cout << "\nStream at " << rtsp_url << " ended" << endl;
        break;
      }
      putText(frame, d_fps, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.6,
              Scalar(0, 200, 0), 2);
      imshow(window_name, frame);

      if (waitKey(10) == 27) {
        cout << "\nESC key pressed, exiting stream..." << endl;
        break;
      }
    }
  } else {
    // Start video capturei with default camera.
    VideoCapture capture(0);
    if (!capture.isOpened()) {
      cout << "\nCouldn't open the camera\n";
      return -1;
    }
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
           << " needed chessboard image(s)" << endl;
    }
    imshow("Calibration", image);
    if ((waitKey(30) & 255) == 27)
      return -1;
  }

  // Destroy calibration preview window.
  destroyWindow("Calibration");
  cout << "\n*** CALIBRATING CAMERA...\n" << endl;

  // Calibrate the Camera
  Mat intrinsic_matrix, distortion_coeff;
  double err =
      calibrateCamera(object_points, image_points, image_size, intrinsic_matrix,
                      distortion_coeff, noArray(), noArray(),
                      CALIB_ZERO_TANGENT_DIST | CALIB_FIX_PRINCIPAL_POINT);
  // Save the intrinsic and distortion values.
  cout << "*** DONE!\n\nProjection error is " << err
       << "\nSaving intrinsics.xml and distortions.xml files\n";
  FileStorage fs("intrinsics.xml", FileStorage::WRITE);

  fs << "image_width" << image_size.width << "image_height" << image_size.height
     << "camera_matrix" << intrinsic_matrix << "distortion_coefficients"
     << distortion_coeff;
  fs.release();

  // Load matrix back in!!
  fs.open("intrinsics.xml", FileStorage::READ);
  cout << "\nimage_width: " << static_cast<int>(fs["image_width"]);
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
    remap(image0, image, map1, map2, INTER_LINEAR, BORDER_CONSTANT, Scalar());
    imshow("Undistorted", image);
    if ((waitKey(30) & 255) == 27)
      break;
  }

  return 0;
}
