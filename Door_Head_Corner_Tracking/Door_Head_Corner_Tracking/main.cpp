#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <vector>
#include "track_point.h"
#include "time.h"
#include "feature.h"
using namespace std;
using namespace cv;

vector<Point> door_head_points;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		door_head_points.push_back(Point(x, y));
	}
}



int main(int argc, char** argv)
{
	bool initialized = false;
	bool tracking = false;
	double duration_ms;
	clock_t start_time, end_time;
	Point old_p1, old_p2;
	Point pt1_move, pt2_move; // the movement of the key points.
	Rect line_search_rect, feature_search_rect;
	Rect old_line_sch_rect, old_ft_sch_rect;
	Rect p_rect;
	array<array<double, HoG_GRAD_BIN_SIZE * 9>, 2> feature;
	bool repeat=false, permit = false;
	double start = double(getTickCount());
	int i = 0;
	// Read image from file 
	VideoCapture cap("C270_2.wmv");
	Mat img, search_img;
	track_state track_result;
	decision order;
	//if fail to read the image
	if (!cap.isOpened())
	{
		cout << "Error Can not open video" << endl;
		return -1;
	}
	cap >> img;
	//Create a window
	namedWindow("My Window", 1);
	//set the callback function for any mouse event
	setMouseCallback("My Window", CallBackFunc, NULL);
	while (1){
		//show the image
		imshow("My Window", img);
		char c = (char)waitKey(10);
		switch (c)
		{
		case 'n':
			cap >> img;
			door_head_points.clear();
			initialized = false;
			tracking = false;
			permit = false;
			break;
		case 's':
			if (permit == false) {
				cout << "not initialized yet" << endl;
				break;
			}

			i = 0;
			do{
				++i;
				cap >> search_img;
				start_time = clock();
				start = double(getTickCount());

				track_result = track_door(old_p1, old_p2, door_head_points, search_img, line_search_rect, feature_search_rect, feature, ALPHA, BETA, DETA);
				order = decide(track_result, i, door_head_points[0], door_head_points[1],old_p1, old_p2, search_img.size());
				cout << "track_result:" << track_result << "\n order:" << order << endl;
				if (order == decision::keep_going) {
					/*update point move vector.*/
					pt1_move = door_head_points[0] - old_p1;
					pt2_move = door_head_points[1] - old_p2;
					/*update old point record*/
					old_p1 = door_head_points[0];
					old_p2 = door_head_points[1];
					repeat = false;
				}
				else if (order == decision::retry) {
					repeat = true;
					/*enlarge the search rect*/
					get_hough_rect(pt1_move, pt2_move,
						search_img.size(), door_head_points[0],
						door_head_points[1], line_search_rect,
						ALPHA*pow(ENLARGE_RATIO, i),
						BETA*pow(ENLARGE_RATIO, i));
					get_hog_rect(pt1_move, pt2_move, search_img.size(),
						door_head_points[0], door_head_points[1],
						feature_search_rect, TAU * pow(ENLARGE_RATIO, i));
					old_line_sch_rect = line_search_rect;
					old_ft_sch_rect = feature_search_rect;
				}
				else if(order == decision::stop_car){
					cout << "stop car!" << endl;
					permit = false;
					repeat = false;
				}
				end_time = clock();
				cout << "clickes: " << end_time - start_time << "seconds: " << ((float)(end_time - start_time) / CLOCKS_PER_SEC) << endl;
				duration_ms = (double(getTickCount()) - start) * 1000 / getTickFrequency();
				cout << "It took " << duration_ms << " ms." << endl;
				p_rect.x = door_head_points[0].x - STD_CELL_WIDTH*STD_CELL_PER_BLOCK_ROW / 2;
				p_rect.y = door_head_points[0].y - STD_CELL_HEIGHT*STD_CELL_PER_BLOCK_COLOMN / 2;
				p_rect.width = STD_CELL_WIDTH*STD_CELL_PER_BLOCK_ROW;
				p_rect.height = STD_CELL_HEIGHT*STD_CELL_PER_BLOCK_COLOMN;
				circle(search_img, door_head_points[0], 3, Scalar(0, 0, 255));
				rectangle(search_img, p_rect, Scalar(255, 0, 0));
				circle(search_img, door_head_points[1], 3, Scalar(0, 0, 255));
				p_rect.x = door_head_points[1].x - STD_CELL_WIDTH*STD_CELL_PER_BLOCK_ROW / 2;
				p_rect.y = door_head_points[1].y - STD_CELL_HEIGHT*STD_CELL_PER_BLOCK_COLOMN / 2;
				rectangle(search_img, p_rect, Scalar(255, 0, 0));
				rectangle(search_img, old_ft_sch_rect, Scalar(0, 100, 0));
				rectangle(search_img, old_line_sch_rect, Scalar(0, 0, 100));
				old_line_sch_rect = line_search_rect;
				old_ft_sch_rect = feature_search_rect;
				imshow("match image", search_img);
				waitKey(1);
			} while (repeat);


			break;
		default:
			break;
		}
		if (initialized == false && door_head_points.size() == 2) {
			init_tracking(door_head_points, old_p1, old_p2, img, line_search_rect,feature_search_rect, feature, ALPHA, BETA);
			pt1_move = Point(0, 0);
			pt2_move = Point(0, 0);
			rectangle(img, feature_search_rect, Scalar(0, 100, 0));
			rectangle(img, line_search_rect, Scalar(100, 0, 0));
			old_line_sch_rect = line_search_rect;
			old_ft_sch_rect = feature_search_rect;
			//circle(img, door_head_points[0], 3, Scalar(255, 0, 0));
			//circle(img, door_head_points[1], 3, Scalar(255, 0, 0));

			//for (int i = 0; i < 2; ++i) {
			//	for (int j = 0; j < HoG_GRAD_BIN_SIZE * 9; ++j)
			//	{
			//		cout << feature[i][j] << ',';
			//	}
			//	cout << endl;
			//}
			cout << "press \'s\' to select the dst image, or press \'y\' to start matcing..." << endl;
			initialized = true;
			permit = true;
		}
		if (tracking) {
		}
	}


	// Wait until user press some key
	waitKey(0);

	return 0;

}