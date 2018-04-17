#pragma once
#ifndef TRIANGLEH
#define TRIANGLEH

#include "hitable.h"
#define EPS 0.0000001

class triangle : public hitable {

public:
	triangle() {}
	triangle(vec3 _v0, vec3 _v1, vec3 _v2, material *m) : v0(_v0), v1(_v1), v2(_v2), mat_ptr(m) { }
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& b) const;

	vec3 v0, v1, v2;
	material *mat_ptr;
};



bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {

	vec3 e1, e2, h, s, q;
	float a, f, u, v;
	e1 = v1 - v0;
	e2 = v2 - v0;
	h = cross(r.direction(), e2);
	a = dot(e1, h);
	if (a > -EPS && a < EPS) {
		return false;
	}

	f = 1 / a;
	s = r.origin() - v0;
	u = f * dot(s, h);
	if (u < 0.0 || u > 1.0) {
		return false;
	}

	q = cross(s, e1);
	v = f * dot(r.direction(), q);
	if (v < 0.0 || u + v > 1.0) {
		return false;
	}

	float t = f * dot(e2, q);
	if (t > EPS) {

		rec.t = t;
		//rec.p = r.origin() * r.direction() * t;
		rec.p = r.point_at_parameter(t);
		rec.mat_ptr = mat_ptr;
		rec.normal = cross(e1, e2);
		return true;
	}
	else {
		return false;
	}
}

bool triangle::bounding_box(float t0, float t1, aabb& b) const {
	
	vec3 min(ffmin(ffmin(v0.x(), v1.x()), v2.x()),
		ffmin(ffmin(v0.y(), v1.y()), v2.y()),
		ffmin(ffmin(v0.z(), v1.z()), v2.z()));
	vec3 max(ffmax(ffmin(v0.x(), v1.x()), v2.x()),
		ffmax(ffmin(v0.y(), v1.y()), v2.y()),
		ffmax(ffmin(v0.z(), v1.z()), v2.z()));
	b = aabb(min, max);
	if (abs(b.max().x() - b.min().x()) < 0.0001f) {
		b.min().e[0] -= 0.0001f;
		b.max().e[0] += 0.0001f;
	}
	if (abs(b.max().y() - b.min().y()) < 0.0001f) {
		b.min().e[1] -= 0.0001f;
		b.max().e[1] += 0.0001f;
	}
	if (abs(b.max().z() - b.min().z()) < 0.0001f) {
		b.min().e[2] -= 0.0001f;
		b.max().e[2] += 0.0001f;
	}

	return true;
}

#endif