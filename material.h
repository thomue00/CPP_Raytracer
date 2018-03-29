#ifndef MATERIALH
#define MATERIALH
struct hit_record;
#include "ray.h"
#include "hitable.h"

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record rec, vec3& attenuattion, ray& scattered) const = 0;
};
#endif