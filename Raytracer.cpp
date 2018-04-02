// Raytracer.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "float.h"
#include "hitable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "bhv_node.h"
#include <algorithm>

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>

class ThreadPool {

public:

    template<typename Index, typename Callable>
    static void ParallelFor(Index start, Index end, Callable func) {
        // Estimate number of threads in the pool
        const static unsigned nb_threads_hint = std::thread::hardware_concurrency();
        const static unsigned nb_threads = (nb_threads_hint == 0u ? 8u : nb_threads_hint);

        // Size of a slice for the range functions
        Index n = end - start + 1;
        Index slice = (Index) std::round(n / static_cast<double> (nb_threads));
        slice = std::max(slice, Index(1));

        // [Helper] Inner loop
        auto launchRange = [&func] (int k1, int k2) {
            for (Index k = k1; k < k2; k++) {
                func(k);
            }
        };

        // Create pool and launch jobs
        std::vector<std::thread> pool;
        pool.reserve(nb_threads);
        Index i1 = start;
        Index i2 = std::min(start + slice, end);
        for (unsigned i = 0; i + 1 < nb_threads && i1 < end; ++i) {
            pool.emplace_back(launchRange, i1, i2);
            i1 = i2;
            i2 = std::min(i2 + slice, end);
        }
        if (i1 < end) {
            pool.emplace_back(launchRange, i1, end);
        }

        // Wait for jobs to finish
        for (std::thread &t : pool) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    // Serial version for easy comparison
    template<typename Index, typename Callable>
    static void SequentialFor(Index start, Index end, Callable func) {
        for (Index i = start; i < end; i++) {
            func(i);
        }
    }

};

vec3 color(const ray& r, hitable *world, int depth) {

	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {

		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {

		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0);
		return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}

hitable *random_scene() {

	int n = 500;
	texture *checker = new checker_texture(
		new constant_texture(vec3(0.8, 0.0, 0.0)), 
		new constant_texture(vec3(0.9, 0.9, 0.9)));
	texture *perlinTex = new noise_texture(1);

	hitable **list = new hitable*[n + 1];
	//list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(perlinTex));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(perlinTex));
	int i = 2;
	bool generateRandom = false;
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
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1, new dialectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1, new metal(vec3(0.7, 0.6, 0.5), 0.0));
	//return new hitable_list(list, i);
	return new bhv_node(list, i, 0, 1);
}

int main() {

	int nx = 1200; // width
	int ny = 800; // height
	int ns = 5; // samples per ray

	hitable *world = random_scene();

	vec3 look_from = vec3(13, 2, 3);
	vec3 lookat = vec3(0, 0, 0);
	float dist_to_focus = 10;
	float aperture = 0.0;
	camera cam(look_from, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus, 0, 1);
	
	// Image Container
	vec3 **colors = new vec3*[nx * ny];

	ThreadPool::ParallelFor(0, ny, [&](int y) {

		std::cout << "Processing Line: " << y << "\n";
		for (int x = 0; x < nx; x++) {

			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {

				float u = float(x + random_float()) / float(nx);
				float v = float(y + random_float()) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}

			col /= float(ns);
			vec3 *c = new vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			colors[x + y * nx] = c;
			
		}
	});

	// Init Filestream
	std::ofstream myfile ("output.ppm");
	if (myfile.is_open()) {

		// Write PPM Header
		myfile << "P3\n" << nx << " " << ny << "\n255\n";

		for (int y = ny - 1; y >= 0; y--) {

			for (int x = 0; x < nx; x++) {

				vec3 col = *colors[x + y * nx];
				int ir = int(255.99  * col[0]);
				int ig = int(255.99  * col[1]);
				int ib = int(255.99  * col[2]);
				// Write each pixel to filestream
				myfile << ir << " " << ig << " " << ib << "\n";
			}
		}
		myfile.close();
	}
	else {

		std::cout << "Can't open file via ofstream";
	}
	delete(colors);

	std::cout << "Press any Key to exit ...";
	getchar();
	return 0;
}

