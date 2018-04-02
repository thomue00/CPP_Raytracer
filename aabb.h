#ifndef AABBH
#define AABBH

#include "vec3.h"
#include "ray.h"

// AABB
inline float ffmin(float a, float b) { return a < b ? a : b; }
inline float ffmax(float a, float b) { return a > b ? a : b; }

class aabb {

public:

	aabb() {}
	aabb(const vec3& a, const vec3& b) {
		_min = a;
		_max = b;
	}

	vec3 min() const { return _min; }
	vec3 max() const { return _max; }

	bool hit(const ray& ray, float tmin, float tmax) const {

		for (int i = 0; i < 3; i++) {

			float invD = 1.0f / ray.direction()[i];
			float t0 = (min()[i] - ray.origin()[i]) * invD;
			float t1 = (max()[i] - ray.origin()[i]) * invD;
			if (invD < 0.0) {
				std::swap(t0, t1);
			}
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			//float t0 = ffmin((_min[i] - ray.origin()[i]) / ray.direction()[i], (_max[i] - ray.origin()[i]) / ray.direction()[i]);
			//float t1 = ffmax((_min[i] - ray.origin()[i]) / ray.direction()[i], (_max[i] - ray.origin()[i]) / ray.direction()[i]);
			//tmin = ffmax(t0, tmin);
			//tmax = ffmin(t1, tmax);
			if (tmax <= tmin) {
				return false;
			}
		}

		return true;
	}
	vec3 _min, _max;
};

aabb surrounding_box(aabb box0, aabb box1) {

	vec3 small(fmin(box0.min().x(), box1.min().x()), fmin(box0.min().y(), box1.min().y()), fmin(box0.min().z(), box1.min().z()));
	vec3 big(fmax(box0.max().x(), box1.max().x()), fmax(box0.max().y(), box1.max().y()), fmax(box0.max().z(), box1.max().z()));
	return aabb(small, big);
}

#endif