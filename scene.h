#pragma once
#ifndef SCENEH
#define SCENEH

#include <float.h>
#include "camera.h"
#include "ThreadPool.h"
#include <string>
#include "hitable_list.h"
#include "aarect.h"
#include "sphere.h"
#include "triangle.h"
#include "material.h"
#include "constant_medium.h"
#include "bhv_node.h"
#include "box.h"
#include "ThreadPool.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class scene {
private:

	int nx, ny, ns, max_depth;
	vec3 look_from, look_at;
	float focus_dist, aperture, vfov;
	camera *cam;
	vec3 **colors;
	hitable *world;

	bool save(std::string name) const;
	vec3 trace(const ray& r, int depth) const;

public:

	scene() {}
	scene(int _width, int _height, int _samples, vec3 _lookfrom, vec3 _lookat, hitable *_world, int _maxdepth = 50, float _focusdist = 10.0, float _aperture = 0.0, float _vfov = 40) : nx(_width), ny(_height), ns(_samples), look_from(_lookfrom), look_at(_lookat), world(_world), max_depth(_maxdepth), focus_dist(_focusdist), aperture(_aperture), vfov(_vfov) {

		this->cam = new camera(look_from, look_at, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, focus_dist, 0, 1);
		this->colors = new vec3*[nx * ny];
	}

	bool render(std::string name = "output") const;
	
	
	static hitable* earth(vec3 pos);
	static hitable* simple_light_scene();
	static hitable* triangle_test();
	static hitable* triangle_random();
	static hitable* random_scene();
	static hitable* cornell_box();
	static hitable* cornell_box_smoke();
	static hitable* final_scene();

};

vec3 scene::trace(const ray& r, int depth) const {

	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {

		ray scattered;
		vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		if (depth < max_depth && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return emitted + attenuation * trace(scattered, depth + 1);
		}
		else {
			return emitted;
		}
	}
	else {

		// sky
		vec3 unitDir = r.direction();
		float t = 0.5f*(unitDir.y() + 1.0f);
		return ((1.0f - t)*vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f)) * 0.3f;

		//return vec3(0, 0, 0);
	}
}

bool scene::render(std::string name) const {

	int c = 0;
	ThreadPool::ParallelFor(0, ny, [&](int y) {

		//std::cout << "Processing Line: " << y << "\n";
		for (int x = 0; x < nx; x++) {

			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {

				float u = float(x + random_float()) / float(nx);
				float v = float(y + random_float()) / float(ny);
				ray r = cam->get_ray(u, v);
				//vec3 p = r.point_at_parameter(2.0);
				col += trace(r, 0);
			}

			col /= float(ns);
			vec3 *c = new vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			c->Clamp01();
			colors[x + y * nx] = c;
			
		}
		c++;
		std::cout << "Process: " << c++ << "/" << ny * 2 << "\n";
	});

	save(name);
	return true;
}

bool scene::save(std::string name) const {

	uint8_t *bytes = new uint8_t[nx * ny * 3];
	for (int y = ny - 1; y >= 0; y--) {

		for (int x = 0; x < nx; x++) {

			vec3 col = *colors[x + (ny - y - 1) * nx];
			int ir = int(255.99  * col[0]);
			int ig = int(255.99  * col[1]);
			int ib = int(255.99  * col[2]);

			bytes[((x + y * nx) * 3 + 0)] = uint8_t(ir);
			bytes[((x + y * nx) * 3 + 1)] = uint8_t(ig);
			bytes[((x + y * nx) * 3 + 2)] = uint8_t(ib);
		}
	}
	std::cout << "Wrote PNG: " << name << ".png" << std::endl;
	std::string path = "_ImgOutput/" + name + ".png";
	stbi_write_png(path.c_str(), nx, ny, 3, bytes, 0);

	delete(colors);
	return true;
}





hitable* scene::earth(vec3 pos) {

	int nx, ny, nn;
	unsigned char *tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
	material *mat_earth = new lambertian(new image_texture(tex_data, nx, ny));
	return new sphere(pos, 1, mat_earth);
}

hitable* scene::simple_light_scene() {

	texture *pertext = new noise_texture(4);
	hitable **list = new hitable*[4];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	list[2] = new sphere(vec3(0, 6, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[3] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	return new hitable_list(list, 4);
}

hitable* scene::triangle_test() {

	texture *checker = new checker_texture(
		new constant_texture(vec3(0.8, 0.0, 0.0)),
		new constant_texture(vec3(0.9, 0.9, 0.9)));

	texture *blue = new constant_texture(vec3(0, 1, 0));

	hitable **list = new hitable*[4];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	list[1] = new triangle(vec3(0, 0, 0), vec3(1, 0, 20), vec3(1, 1, 0), new lambertian(blue));
	list[2] = new sphere(vec3(0, 6, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[3] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	return new hitable_list(list, 4);
}

hitable* scene::triangle_random() {

	texture *checker = new checker_texture(
		new constant_texture(vec3(0.8, 0.0, 0.0)),
		new constant_texture(vec3(0.9, 0.9, 0.9)));

	texture *blue = new constant_texture(vec3(0, 1, 0));

	hitable **list = new hitable*[21];
	int l = 0;
	list[l++] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	//list[l++] = new sphere(vec3(0, 6, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	//list[l++] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));

	for (int i = 0; i < 20; i++) {
		list[l++] = new triangle(random_in_unit_sphere() * 5, random_in_unit_sphere() * 5, random_in_unit_sphere() * 5, new lambertian(new constant_texture(random_in_unit_sphere())));
	}

	return new hitable_list(list, l);
}

hitable* scene::random_scene() {

	int n = 500;
	texture *checker = new checker_texture(
		new constant_texture(vec3(0.8, 0.0, 0.0)),
		new constant_texture(vec3(0.9, 0.9, 0.9)));
	texture *perlinTex = new noise_texture(5);

	hitable **list = new hitable*[n + 1];
	//list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));

	int i = 1;
	bool generateRandom = true;
	if (generateRandom) {

		for (int a = -11; a < 11; a++) {

			for (int b = -11; b < 11; b++) {

				float choose_mat = random_float();
				vec3 center(a + 0.9 * random_float(), 0.2, b + 0.9 * random_float());
				if ((center - vec3(4, 0.2, 0)).length() > 0.9) {

					if (choose_mat < 0.8) {

						// diffuse
						//list[i++] = new moving_sphere(center, center + vec3(0, 0.5 * random_float(), 0), 0, 1, 0.2, new lambertian(vec3(random_float() * random_float(), random_float() * random_float(), random_float() * random_float())));
						list[i++] = new sphere(center, 0.2, new lambertian(new constant_texture(vec3(random_float() * random_float(), random_float() * random_float(), random_float() * random_float()))));
					}
					else if (choose_mat < 0.95) {

						// metal
						list[i++] = new sphere(center, 0.2, new metal(vec3(0.5 * (1 + random_float()), 0.5 * (1 + random_float()), 0.5 * (1 + random_float())), 0.5 * random_float()));
					}
					else {

						// glass
						list[i++] = new sphere(center, 0.2, new dialectric(1.5));
					}
				}
			}
		}
		list[i++] = new sphere(vec3(0, 1, 0), 1, new dialectric(1.5));
		//list[i++] = new sphere(vec3(-4, 1, 0), 1, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
		list[i++] = scene::earth(vec3(-4, 1, 0));
		list[i++] = new sphere(vec3(4, 1, 0), 1, new metal(vec3(0.7, 0.6, 0.5), 0.0));
	}

	//return new hitable_list(list, i);
	return new bhv_node(list, i, 0, 1);
}

hitable* scene::cornell_box() {

	hitable **list = new hitable*[8];
	int i = 0;
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material *light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));
	return new hitable_list(list, i);
}

hitable* scene::cornell_box_smoke() {

	hitable **list = new hitable*[8];
	int i = 0;
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material *light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));

	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));

	hitable *b1 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	hitable *b2 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

	list[i++] = new constant_medium(b1, 0.01, new constant_texture(vec3(1, 1, 1)));
	list[i++] = new constant_medium(b2, 0.01, new constant_texture(vec3(0, 0, 0)));
	return new hitable_list(list, i);
}

hitable* scene::final_scene() {

	int nb = 20;
	hitable **list = new hitable*[30];
	hitable **boxlist = new hitable*[100000];
	hitable **boxlist2 = new hitable*[100000];

	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *ground = new lambertian(new constant_texture(vec3(0.48, 0.83, 0.53)));

	int b = 0;
	for (int i = 0; i < nb; i++) {

		for (int j = 0; j < nb; j++) {

			int w = 100;
			int x0 = -1000 + i * w;
			int z0 = -1000 + j * w;
			int y0 = 0;
			int x1 = x0 + w;
			int z1 = z0 + w;
			int y1 = 100 * (random_float() + 0.01);
			boxlist[b++] = new box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
		}
	}
	int l = 0;
	list[l++] = new bhv_node(boxlist, b, 0, 1);
	material *light = new diffuse_light(new constant_texture(vec3(7, 7, 7)));
	list[l++] = new xz_rect(123, 423, 147, 412, 554, light);
	vec3 center(400, 400, 200);
	list[l++] = new moving_sphere(center, center + vec3(30, 0, 0), 0, 1, 50, new lambertian(new constant_texture(vec3(0.7, 0.3, 0.1))));
	list[l++] = new sphere(vec3(260, 150, 45), 50, new dialectric(1.5));
	list[l++] = new sphere(vec3(0, 150, 145), 50, new metal(vec3(0.8, 0.8, 0.9), 10.0));
	hitable *boundary = new sphere(vec3(360, 150, 145), 70, new dialectric(1.5));
	list[l++] = boundary;
	list[l++] = new constant_medium(boundary, 0.2, new constant_texture(vec3(0.2, 0.4, 0.9)));
	boundary = new sphere(vec3(0, 0, 0), 5000, new dialectric(1.5));
	list[l++] = new constant_medium(boundary, 0.0001, new constant_texture(vec3(1.0, 1.0, 1.0)));
	int nx, ny, nn;
	unsigned char *tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
	material *emat = new lambertian(new image_texture(tex_data, nx, ny));
	list[l++] = new sphere(vec3(400, 200, 400), 100, emat);
	texture *pertext = new noise_texture(0.1);
	list[l++] = new sphere(vec3(220, 280, 300), 80, new lambertian(pertext));
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxlist2[j] = new sphere(vec3(165 * random_float(), 165 * random_float(), 165 * random_float()), 10, white);
	}
	list[l++] = new translate(new rotate_y(new bhv_node(boxlist2, ns, 0.0, 1.0), 15), vec3(-100, 270, 395));
	return new hitable_list(list, l);
}

#endif