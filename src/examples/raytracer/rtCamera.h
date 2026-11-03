#pragma once

#include "hittable.h"
#include <thread>

class rtCamera
{
	public:
	double aspect_ratio 	= 1.;
	int image_width 	= 100;
	int image_height;
	int samples_per_pixel 	= 10;
	int max_depth 		= 10;
	
	double vfov  		= 90;

	glm::vec3 lookfrom 	= glm::vec3(0.);
	glm::vec3 lookat 	= glm::vec3(0., 0., 1.);
	glm::vec3 vup  		= glm::vec3(0., 1., 0.);
	
	double defocus_angle = 0;
	double focus_dist = 10;

	std::vector<unsigned char> rayTracePixels;
	std::vector<std::thread> threads;
	

	void render(const hittable& world)
	{
		initialize();	
	
		uint32_t index = -1;
		for (int j = 0; j < image_height; j++)
		{
			for (int i = 0; i < image_width; i++)
			{
				glm::vec3 pixel_color{0.};
				
//				threads.emplace_back([&](){
					sample_pixel(i, j, pixel_color, world);
//				});	

				std::cout << "\rLoading: " << double((double)index / (double)(image_height * image_width * 3)) * 100.;
				write_color(index, pixel_samples_scale * pixel_color);
			};
//			std::cout << "\rDone.             \n";
		};
	};

	private:
	double pixel_samples_scale;
	glm::vec3 center;
	glm::vec3 pixel00_loc;
	glm::vec3 pixel_delta_u;
	glm::vec3 pixel_delta_v;
	glm::vec3 u, v, w;
	glm::vec3 defocus_disk_u;
	glm::vec3 defocus_disk_v;

	void initialize()
	{
		image_height = int(image_width/aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		pixel_samples_scale = 1.0/samples_per_pixel;

		rayTracePixels.resize(image_width * image_height * 3);

		center = lookfrom;

		auto theta = degrees_to_radians(vfov);
		auto h = std::tan(theta/2);
		
		auto viewport_height = 2. * h * focus_dist;
		auto viewport_width = viewport_height * ((double(image_width)/image_height));

		w = glm::normalize(lookfrom - lookat);
		u = glm::normalize(glm::cross(vup, w));
		v = glm::cross(w, u);

		auto viewport_u = viewport_width*u; 
		auto viewport_v = viewport_height*-v; 

		pixel_delta_u = viewport_u/image_width;
		pixel_delta_v = viewport_v/image_height;
	
		auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
	
		auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
		defocus_disk_u = u * defocus_radius;
		defocus_disk_v = v * defocus_radius;
	};

	void sample_pixel(const int& i, const int& j, glm::vec3& pixel_color, const hittable& world)
	{
		for (int sample = 0; sample < samples_per_pixel; sample++)
		{
			ray r = get_ray(i, j);
			pixel_color += ray_color(r, max_depth, world);
		};
	};

	ray get_ray(int i, int j)
	{
		auto offset = sample_square();
		auto pixel_sample = pixel00_loc 
				+ ((i + offset.x) * pixel_delta_u)
				+ ((j + offset.y) * pixel_delta_v);
		
		auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		
		return ray(ray_origin, ray_direction);
	};

	glm::vec3 sample_square() const
	{
		return glm::vec3(random_double() - 0.5, random_double() - 0.5, 0.);
	};

	glm::vec3 defocus_disk_sample() const
	{
		auto p = glm_ext::random_in_unit_disk();
		return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
	};

	glm::vec3 ray_color(const ray& r, int depth, const hittable& world)
	{
		if (depth <= 0)
		{
			return glm::vec3(0.);
		};
	
		hit_record rec;
		if (world.hit(r, interval(0.001, infinity), rec))
		{
			ray scattered;
			glm::vec3 attenuation;
			if (rec.mat->scatter(r, rec, attenuation, scattered))
			{
				return attenuation * ray_color(scattered, depth - 1, world);
			};
		
			return glm::vec3(0.);
		};
	
		glm::vec3 unit_direction(glm::normalize(r.direction()));
		auto a = 0.5*(unit_direction.y + 1.0);
		return (1.0 - a) * glm::vec3(1.) + a * glm::vec3(0.5, 0.7, 1.);
	};

	double linear_to_gamma(double linear_component)
	{
		if (linear_component > 0)
		{
			return std::sqrt(linear_component);
		};
		return 0;
	};
	
	void write_color(uint32_t& index, const glm::vec3 pixel_color)
	{
		auto r = pixel_color.r; 
		auto g = pixel_color.g;
		auto b = pixel_color.b;
		
		r = linear_to_gamma(r); 
		g = linear_to_gamma(g);
		b = linear_to_gamma(b);

		static const interval intensity(0.000, 0.999);
		int ir = int(256 * intensity.clamp(r));
		int ig = int(256 * intensity.clamp(g));
		int ib = int(256 * intensity.clamp(b));
		
		rayTracePixels[++index] = ir;
		rayTracePixels[++index] = ig;
		rayTracePixels[++index] = ib;
	};

};
