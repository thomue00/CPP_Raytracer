// Raytracer.cpp : Defines the entry point for the console application.



#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "scene.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>


int main() {

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	{
		scene main(400, 400, 100, vec3(278, 278, -800), vec3(278, 278, 0), scene::final_scene(), 10);
		main.render("FinalScene");

		//scene main(400, 400, 200, vec3(0, 2, -3), vec3(0, 1, 0), scene::triangle_test(), 50);
		//main.render("TriangleTest");

		//scene main(400, 400, 100, vec3(0, 2, -15), vec3(0, 1, 0), scene::triangle_random(), 50);
		//main.render("TriangleArt");
	}
	end = std::chrono::system_clock::now();
	int seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
	std::cout << "Rendertime: " << seconds << "s" << std::endl;

	
	std::cout << "Press any Key to exit ...";
	getchar();
	return 0;
}

