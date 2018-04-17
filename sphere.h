#ifndef SPHEREH
#define SPHEREH
#include "hitable.h"

#define M_PI           3.14159265358979323846 

void get_sphere_uv(const vec3& p, float& u, float& v) {

	float phi = atan2(p.z(), p.x());
	float theta = asin(p.y());
	u = 1 - (phi + M_PI) / (2 * M_PI);
	v = (theta + M_PI / 2) / M_PI;
}

class sphere : public hitable {

public:
	sphere () {}
	sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat_ptr(m) {};
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& b)  const;
	vec3 center;
	float radius;
	material *mat_ptr;
};

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {

	vec3 oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0) {
		
		float temp = (-b - sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min) {
			
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
			rec.normal = (rec.p - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
		temp = (-b + sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min) {

			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
			rec.normal = (rec.p - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}

bool sphere::bounding_box(float t0, float t1, aabb& b) const {

	b = aabb(
		center - vec3(radius, radius, radius), 
		center + vec3(radius, radius, radius));
	return true;
}
	
class moving_sphere : public hitable {

public:
	moving_sphere() {}
	moving_sphere(vec3 _center0, vec3 _center1, float _time0, float _time1, float _radius, material *mat) : center0(_center0), center1(_center1), time0(_time0), time1(_time1), radius(_radius), mat_ptr(mat) {}
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& b)  const;
	vec3 center(float time) const;
	vec3 center0, center1;
	float time0, time1;
	float radius;
	material *mat_ptr;
};

vec3 moving_sphere::center(float time) const {
	return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

bool moving_sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {

	vec3 oc = r.origin() - center(r.time());
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0) {
		
		float temp = (-b - sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min) {
			
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center(r.time())) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
		temp = (-b + sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min) {

			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center(r.time())) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}

bool moving_sphere::bounding_box(float t0, float t1, aabb& b) const {

	aabb box0 = aabb(center(t0) - vec3(radius, radius, radius), center(t0) + vec3(radius, radius, radius));
	aabb box1 = aabb(center(t1) - vec3(radius, radius, radius), center(t1) + vec3(radius, radius, radius));
	b = surrounding_box(box0, box1);
	return true;
}

#endif