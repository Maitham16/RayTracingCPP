// main function
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <omp.h>
#include <thread>

#include "Classes/stdtemplate.h"
#include "Classes/Vec3.h"
#include "Classes/Hitable_list.h"
#include "Classes/Camera.h"
#include "Classes/Sphere.h"
#include "Classes/Material.h"

// return background color
Vec3 color(const Ray &ray, Hitable *World, int depth)
{
    Hit_record hit_record;
    if (World->intersect(ray, 0.001, MAXFLOAT, hit_record))
    {
        Ray scattered;
        Vec3 attenuation;
        if (depth < 50 && hit_record.material->scatter(ray, hit_record, attenuation, scattered))
        {
            return attenuation * color(scattered, World, depth + 1);
        }
        else
        {
            return Vec3(0.0, 0.0, 0.0);
        }
    }
    else
    {
        Vec3 unit_direction = unit_vector(ray.direction());
        float t = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.4, 0.5, 1.0);
    }
}

// random scene _final_scene_of_the_book
Hitable *random_scene()
{
    int n = 500;
    Hitable **list = new Hitable *[n + 1];
    list[0] = new Sphere(Vec3(0.0, -1000.0, 0.0), 1000.0, new Lambertian(Vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            float choose_material = drand48();
            Vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());
            if ((center - Vec3(4.0, 0.2, 0.0)).length() > 0.9)
            {
                if (choose_material < 0.8)
                { // diffuse
                    list[i++] = new Sphere(center, 0.2, new Lambertian(Vec3(drand48() * drand48(), drand48() * drand48(), drand48() * drand48())));
                }
                else if (choose_material < 0.95)
                { // metal
                    list[i++] = new Sphere(center, 0.2, new Metal(Vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()), 0.5 * (1 + drand48())), 0.5 * drand48()));
                }
                else
                { // glass
                    list[i++] = new Sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }
    list[i++] = new Sphere(Vec3(0.0, 1.0, 0.0), 1.0, new dielectric(1.5));
    list[i++] = new Sphere(Vec3(-4.0, 1.0, 0.0), 1.0, new Lambertian(Vec3(0.4, 0.2, 0.1)));
    list[i++] = new Sphere(Vec3(4.0, 1.0, 0.0), 1.0, new Metal(Vec3(0.7, 0.6, 0.5), 0.0));

    return new Hitable_list(list, i);
}

int main()
{
    int width = 2048;
    int height = 1024;
    int ns = 100;
    std::ofstream ofs;
    ofs.open("./output.ppm");
    std::cout << "P3\n"
              << width << " " << height << "\n255\n";
    ofs << "P3\n"
        << width << " " << height << "\n255\n";

    // scene
    Hitable *scene = random_scene();

    Vec3 left = Vec3(-2.0, -1.0, -1.0);
    Vec3 horizontal = Vec3(4.0, 0.0, 0.0);
    Vec3 vertical = Vec3(0.0, 2.0, 0.0);
    Vec3 position = Vec3(0.0, 0.0, 0.0);
    float fov = 20.0;
    float aspect = float(width) / float(height);
    Vec3 look_from = Vec3(13.0, 2.0, 3.0);
    Vec3 look_at = Vec3(0.0, 0.0, 0.0);
    Vec3 up = Vec3(0.0, 1.0, 0.0);
    float focus = 10.0;
    float aperture = 0.1;
    Camera camera(look_from, look_at, up, fov, aspect, aperture, focus);

    // Number of threads
    const int num_threads = std::thread::hardware_concurrency();

    // Calculate the height of each chunk
    int chunk_height = height / num_threads;

    // Create a vector to hold the threads
    std::vector<std::thread> threads;

    // Create a buffer to store the color data
    std::vector<std::vector<Vec3>> buffer(height, std::vector<Vec3>(width));

    // Create and launch the threads
    for (int t = 0; t < num_threads; ++t)
    {
        threads.push_back(std::thread([&, t]()
                                      {
        // Calculate the start and end y values for this chunk
        int y_start = t * chunk_height;
        int y_end = (t == num_threads - 1) ? height : y_start + chunk_height;

        // Render this chunk
        for (int j = y_start; j < y_end; ++j) {
            for (int i = 0; i < width; ++i) {
                Vec3 sceneColor(0.0, 0.0, 0.0);
                for (int s = 0; s < ns; s++) {
                    float u = float(i + drand48()) / float(width);
                    float v = float(j + drand48()) / float(height);
                    Ray ray = camera.get_ray(u, v);
                    sceneColor += color(ray, scene, 0);
                }
                sceneColor /= float(ns);
                sceneColor = Vec3(sqrt(sceneColor.r()), sqrt(sceneColor.g()), sqrt(sceneColor.b()));

                // console output
                int percent = int(float(j * width + i) / float(width * height) * 100);
                std::cout << "\r" << percent << "%";
                std::cout.flush();

                buffer[j][i] = sceneColor;
            }
        } }));
    }

    // Wait for all threads to finish
    for (auto &thread : threads)
    {
        thread.join(); 
    }

    // Output the color data from the buffer
    for (int j = height - 1; j >= 0; j--)
    {
        for (int i = 0; i < width; i++)
        {
            Vec3 &sceneColor = buffer[j][i];
            int ir = int(255.99 * sceneColor[0]);
            int ig = int(255.99 * sceneColor[1]);
            int ib = int(255.99 * sceneColor[2]);
            std::cout << ir << " " << ig << " " << ib << "\n";
            ofs << ir << " " << ig << " " << ib << "\n";
        }
    }

    return 0;
}