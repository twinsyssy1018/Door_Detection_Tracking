#include "feature.h"




/*calculate the HoG feature of point (center_c , center_r).
	----------------------------------------------------------------------------------------------
	@feature: feature of point (center_c, center_r).
	@grad_angle_id: id image in the bin of the gradient angle. this image is in feature_rect size.
	@grad_mag: image of the magnitude of the gradient.
	@feature_rect: the rectangle area of the feature in the original image.
	@center_r: row coordinate of the feature point in original image.
	@center_c: column coordinate of the feature point in original image.
	----------------------------------------------------------------------------------------------
*/
bool get_hog_from_map(array<double, HoG_GRAD_BIN_SIZE * 9> & feature, const Mat & grad_angle_id, 
						const Mat & grad_mag, const Rect & feature_rect, int center_r, int center_c) {
	int relative_r = center_r - feature_rect.y;
	int relative_c = center_c - feature_rect.x;
	int half_patch_width = STD_CELL_WIDTH * STD_CELL_PER_BLOCK_ROW / 2;
	int half_patch_height = STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_COLOMN / 2;
	Point ul(relative_c - half_patch_width, relative_r - half_patch_height);
	if (ul.x < 0 || ul.y < 0 || \
		ul.x + STD_CELL_WIDTH * STD_CELL_PER_BLOCK_ROW >= grad_angle_id.cols || \
		ul.y + STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_COLOMN >= grad_angle_id.rows) {
		return false;
	}
	int r = 0, c = 0;
	int block_id = 0;
	int angle_bin_id = 0;
	int i = 0;
	for (i = 0; i < HoG_GRAD_BIN_SIZE * 9; ++i) {
		feature[i] = 0;
	}
	for (r = 0; r < STD_CELL_WIDTH * STD_CELL_PER_BLOCK_ROW; ++r) {
		for (c = 0; c < STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_COLOMN; ++c) {
			block_id = r / STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_ROW + c / STD_CELL_WIDTH;
			angle_bin_id = grad_angle_id.at<uchar>(ul.y + r, ul.x + c);
			feature[block_id * HoG_GRAD_BIN_SIZE + angle_bin_id] += grad_mag.at<double>(ul.y + r, ul.x + c);
		}
	}
	return true;
}

int get_gradient_id(int bin_size, double bin_range, double dx, double dy) {
	short sign_dx = dx>0.0 ? 2: 0;
	short sign_dy = dy > 0.0 ? 1 : 0;
	int bin_id = 0;
	double angle = 0;
	dx += 0.00001;
	switch (sign_dx | sign_dy) {
	case 0:// both are negative.
		angle = atan(dy / dx) + PI;
		break;
	case 1:// dy>0, dx <0.
		angle = atan(dy / dx) + PI;
		break;
	case 2:// dx>0, dy <0
		angle = atan(dy / dx) + 2 * PI;
		break;
	case 3:// dx>0, dy>0.
		angle = atan(dy / dx);
		break;
	default:
		break;
	}
	if (angle = 2 * PI) {
		bin_id = bin_size -1;
	}
	else {
		bin_id = int(angle / bin_range);
	}
	assert(0 <= bin_id && bin_id < bin_size);
	//cout << bin_id<< ", "<< angle<< ", "<< bin_range<< ", "<< dx<< ", "<< dy << endl;
	return bin_id;
}



//search_range_state get_search_range(const Point & p1, const Point & p2, Size img_size, Rect & search_rect, double alpha, double beta) {
//	// calculate the ROI.
//	//double L = cv::norm(door_head_points[0]-door_head_points[1]);
//	double L = abs(p1.x - p2.x);
//	Point up_left;
//	Point down_right;
//	short near_ud_edge = 0;
//	short near_lr_edge = 0;
//	up_left.x = (int)(min(p1.x, p2.x) - alpha * L);
//	up_left.y = (int)(min(p1.y, p2.y) - alpha * L);
//	up_left.x = (int)(up_left.x >= 0 ? up_left.x : 0);
//	up_left.y = (int)(up_left.y >= 0 ? up_left.y : 0);
//	down_right.x = (int)(max(p1.x, p2.x) + alpha* L);
//	down_right.y = (int)(max(p1.y, p2.y) + (alpha + beta) * L);
//	down_right.x = (int)(down_right.x < img_size.width-1 ? down_right.x : img_size.width-1);
//	down_right.y = (int)(down_right.y < img_size.height-1 ? down_right.y : img_size.height-1);
//
//	if (up_left.x == 0 || down_right.x == img_size.width - 1){
//		near_lr_edge = 1;
//	}
//	if (up_left.y == 0 || down_right.y == img_size.height - 1) {
//		near_ud_edge = 1;
//	}
//	search_rect.x = up_left.x;
//	search_rect.y = up_left.y;
//	search_rect.width = down_right.x - search_rect.x;
//	search_rect.height = down_right.y - search_rect.y;
//	switch (near_lr_edge<<1 | near_ud_edge)
//	{
//	case 0:
//		return search_range_state::safe;
//		break;
//	case 1:
//		return search_range_state::near_ud_edge;
//		break;
//	case 2:
//		return search_range_state::near_lr_edge;
//		break;
//	case 3:
//		return search_range_state::near_lr_ud_edge;
//		break;
//	default:
//		break;
//	}
//	throw range_error("ERROR, switch_condition out of range int get_search_range");
//}
double compare_feature(const array<double, HoG_GRAD_BIN_SIZE * 9> & f1, const array<double, HoG_GRAD_BIN_SIZE * 9> & f2) {
	double error = 0.0;
	int i = 0;
	for (i = 0; i < HoG_GRAD_BIN_SIZE * 9; ++i) {
		error += abs(f1[i] - f2[i]);
	}
	return error;
}


bool get_hog_from_local_map(array<double, HoG_GRAD_BIN_SIZE * 9> & feature, Mat & grad_angle_id, Mat & grad_mag, int center_r, int center_c) {
	int half_patch_width = STD_CELL_WIDTH * STD_CELL_PER_BLOCK_ROW / 2;
	int half_patch_height = STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_COLOMN / 2;
	Point ul(center_c - half_patch_width, center_r - half_patch_height);

	if (ul.x < 0 || ul.y < 0 || \
		ul.x + STD_CELL_WIDTH * STD_CELL_PER_BLOCK_ROW >= grad_angle_id.cols || \
		ul.y + STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_COLOMN >= grad_angle_id.rows) {
		return false;
	}
	int r = 0, c = 0;
	int block_id = 0;
	int angle_bin_id = 0;
	int i = 0;
	for (i = 0; i < HoG_GRAD_BIN_SIZE * 9; ++i) {
		feature[i] =0;
	}
	for (r = 0; r < STD_CELL_WIDTH * STD_CELL_PER_BLOCK_ROW; ++r) {
		for (c = 0; c < STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_COLOMN; ++c) {
			block_id = r / STD_CELL_HEIGHT * STD_CELL_PER_BLOCK_ROW + c / STD_CELL_WIDTH;
			angle_bin_id = grad_angle_id.at<uchar>(r, c);
			feature[block_id * HoG_GRAD_BIN_SIZE + angle_bin_id] += grad_mag.at<double>(r, c);
		}
	}
	return true;
}

bool get_grad_img(Mat & im_Ix, Mat & im_Iy, Mat & grad_angle_id, Mat & grad_mag) {
	int r = 0, c = 0;
	double bin_angle_gap = 2 * PI / HoG_GRAD_BIN_SIZE;
	for (r = 0; r < im_Ix.rows; ++r) {
		for (c = 0; c < im_Ix.cols; ++c) {
			grad_angle_id.at<uchar>(r,c) = get_gradient_id(HoG_GRAD_BIN_SIZE, bin_angle_gap, im_Ix.at<double>(r, c), im_Iy.at<double>(r, c));
			grad_mag.at<double>(r, c) = sqrt(pow(im_Iy.at<double>(r, c), 2) + pow(im_Ix.at<double>(r, c), 2));
		}
	}
	return true;
}