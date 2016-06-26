#ifndef CAMSHIFT_H
#define CAMSHIFT_H
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
using namespace cv;


class object_target{
public:
	Rect trackWindow;
	Mat hist;
	RotatedRect trackBox;
	char side_in_im;
};

bool init_cam_shift(object_target & target, Mat & image, Rect &trackWindow, int vmin = 10, int vmax = 256, int smin = 30);
bool update_target_window(object_target & target, Mat & image, int vmin = 10, int vmax = 256, int smin = 30);


#endif //CAMSHIFT_H