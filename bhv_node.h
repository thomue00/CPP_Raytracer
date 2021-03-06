#ifndef BHV_NODEH
#define BHV_NODEH

#include "hitable.h"
#include <iostream>

int box_x_compare(const void * a, const void * b) {
	
	aabb box_left, box_right;
	hitable *ah = *(hitable**)a;
	hitable *bh = *(hitable**)b;
	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right)) {
		std::cerr << "No bounding box in bhv_node constructor \n";
	}
	if (box_left.min().x() - box_right.min().x() < 0.0) {
		return -1;
	}
	else {
		return 1;
	}
}
int box_y_compare(const void * a, const void * b) {
	
	aabb box_left, box_right;
	hitable *ah = *(hitable**)a;
	hitable *bh = *(hitable**)b;
	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right)) {
		std::cerr << "No bounding box in bhv_node constructor \n";
	}
	if (box_left.min().y() - box_right.min().y() < 0.0) {
		return -1;
	}
	else {
		return 1;
	}
}
int box_z_compare(const void * a, const void * b) {
	
	aabb box_left, box_right;
	hitable *ah = *(hitable**)a;
	hitable *bh = *(hitable**)b;
	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right)) {
		std::cerr << "No bounding box in bhv_node constructor \n";
	}
	if (box_left.min().z() - box_right.min().z() < 0.0) {
		return -1;
	}
	else {
		return 1;
	}
}

class bhv_node: public hitable {

public:

	bhv_node () {}
	bhv_node(hitable **l, int n, float time0, float time1);

	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& b)  const;

	hitable *left;
	hitable *right;
	aabb box;
};

inline bhv_node::bhv_node(hitable **l, int n, float time0, float time1) {

	aabb *boxes = new aabb[n];
	float *left_area = new float[n];
	float *right_area = new float[n];
	aabb main_box;
	bool dummy = l[0]->bounding_box(time0, time1, main_box);
	for (int i = 1; i < n; i++) {

		aabb new_box;
		bool dummy = l[i]->bounding_box(time0, time1, new_box);
		main_box = surrounding_box(new_box, main_box);
	}
	int axis = main_box.longest_axis();
	if (axis == 0) {
		qsort(l, n, sizeof(hitable *), box_x_compare);
	}
	else if (axis == 1) {
		qsort(l, n, sizeof(hitable *), box_y_compare);
	}
	else {
		qsort(l, n, sizeof(hitable *), box_z_compare);
	}
	for (int i = 0; i < n; i++) {
		bool dummy = l[i]->bounding_box(time0, time1, boxes[i]);
	}
	left_area[0] = boxes[0].area();
	aabb left_box = boxes[0];
	for (int i = 1; i < n - 1; i++) {

		left_box = surrounding_box(left_box, boxes[i]);
		left_area[i] = left_box.area();
	}
	right_area[n - 1] = boxes[n - 1].area();
	aabb right_box = boxes[n - 1];
	for (int i = n - 2; i > 0; i--) {

		right_box = surrounding_box(right_box, boxes[i]);
		right_area[i] = right_box.area();
	}
	float min_SAH = FLT_MAX;
	int min_SAH_idx;
	for (int i = 0; i < n - 1; i++) {

		float SAH = i * left_area[i] + (n - i - 1)*right_area[i + 1];
		if (SAH < min_SAH) {

			min_SAH_idx = i;
			min_SAH = SAH;
		}
	}

	if (n == 1) {
		left = right = l[0];
	}
	else if (n == 2) {
		left = l[0];
		right = l[1];
	}
	else {
		left = new bhv_node(l, n / 2, time0, time1);
		right = new bhv_node(l + n / 2, n - n / 2, time0, time1);
	}

	box = main_box;

	/*int axis = int(3 * random_float());
	if (axis == 0) {
		qsort(l, n, sizeof(hitable *), box_x_compare);
	}
	else if (axis == 1) {
		qsort(l, n, sizeof(hitable *), box_y_compare);
	}
	else if (axis == 2) {
		qsort(l, n, sizeof(hitable *), box_z_compare);
	}
	if (n == 1) {
		left = right = l[0];
	}
	else if (n == 2) {
		left = l[0];
		right = l[1];
	}
	else {

		left = new bhv_node(l, n / 2, time0, time1);
		right = new bhv_node(l + n/ 2, n -  n / 2, time0, time1);
	}
	aabb box_left, box_right;
	if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right)) {
		std::cerr << "No bounding box in bhv_node constructor \n";
	}
	box = surrounding_box(box_left, box_right);*/
}

bool bhv_node::hit(const ray& r, float tmin, float tmax, hit_record& rec) const {

	if (box.hit(r, tmin, tmax)) {

		hit_record left_rec, right_rec;
		bool hit_left = left->hit(r, tmin, tmax, left_rec);
		bool hit_right = right->hit(r, tmin, tmax, right_rec);
		if (hit_left && hit_right) {
			
			if (left_rec.t < right_rec.t) {
				rec = left_rec;
			}
			else {
				rec = right_rec;
			}
			return true;
		}
		else if (hit_left) {

			rec = left_rec;
			return true;
		}
		else if (hit_right) {
			
			rec = right_rec;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

bool bhv_node::bounding_box(float t0, float t1, aabb& b) const {
	
	b = box;
	return true;
}
#endif