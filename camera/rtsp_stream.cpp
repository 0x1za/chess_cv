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


int main(int argc, char **argv) {
  // Initialise flag parsing.
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Set stream source
  String stream = "rtsp://192.168.1.168:8554/unicast";
  
  // Start video capture
  VideoCapture capture(stream);
  if (!capture.isOpened()) {
      cout << "\nCouldn't open the camera\n";
      return -1;
  } else if (capture.isOpened()) {
      cout << "RTSP Stream Live from " << stream << endl;
  }

  double fps = capture.get(CAP_PROP_FPS);
  string d_fps = "Frames per seconds: ";
  d_fps += to_string(int(fps)/int(1000));

  // Create window and title
  String window_name = "Chess Stream";
  namedWindow(window_name, WINDOW_NORMAL);

  while(true) {
    Mat frame;
    bool success = capture.read(frame);

    if (!success) {
        cout << "\nStream at " << stream << " ended" << endl;
        break;
    }
    putText(frame, d_fps, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0,200,0), 2); 
    imshow(window_name, frame);

    if (waitKey(10) == 27) {
        cout << "\nESC key pressed, exiting stream..." << endl;
        break;
    }
  }

  return 0;
}
