#pragma once

#include "hittable.h"

class rtCamera
{
	public:
	double aspect_ratio = 1.;
	int image_width = 100;
	int image_height;
	
	std::vector<unsigned char> rayTracePixels;
	
	void render(const hittable& world)
	{
		initialize();	
	
		std::cout << "Image height: " << image_height << '\n';
		uint32_t index = -1;
		for (int j = 0; j < image_height; j++)
		{
			std::cout << "\rScanlines remaining: " << image_height - j << '\n';
			for (int i = 0; i < image_width; i++)
			{
				auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
				auto ray_direction = pixel_center - center;;
				ray r(center, ray_direction);

				glm::vec3 pixel_color{ray_color(r, world)};

				write_color(index, pixel_color);
//				std::cout << "Index: " << index << '\n';
			};
			std::cout << "\rDone.             \n";
		};
	};

	private:
	glm::vec3 center;
	glm::vec3 pixel00_loc;
	glm::vec3 pixel_delta_u;
	glm::vec3 pixel_delta_v;

	void initialize()
	{
		image_height = int(image_width/aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;
	
		rayTracePixels.resize(image_width * (image_width/aspect_ratio) * 3);
		
		center = glm::vec3(0.0);

		auto focal_length = 1.0;
		auto viewport_height = 2.;
		auto viewport_width = viewport_height * ((double(image_width)/image_height));
		auto camera_center = glm::vec3(0.);
		
		auto viewport_u = glm::vec3(viewport_width, 0., 0.);
		auto viewport_v = glm::vec3(0., -viewport_height, 0.);

		pixel_delta_u = viewport_u/image_width;
		pixel_delta_v = viewport_v/image_height;
	
		auto viewport_upper_left = camera_center - glm::vec3(0,0, focal_length) - viewport_u/2 - viewport_v/2;;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
	};
	
	glm::vec3 ray_color(const ray& r, const hittable& world) const
	{
		hit_record rec;
		if (world.hit(r, interval(0, infinity), rec))
		{
			return 0.5 * (rec.normal + glm::vec3(1.));
		};
	
		glm::vec3 unit_direction(glm::normalize(r.direction()));
		auto a = 0.5*(unit_direction.y + 1.0);
		return (1.0 - a) * glm::vec3(1.) + a * glm::vec3(0.5, 0.7, 1.);
	};

	void write_color(uint32_t& index, const glm::vec3 pixel_color)
	{
		auto r = pixel_color.r; 
		auto g = pixel_color.g;
		auto b = pixel_color.b;
		
		int ir = int(255.999 * r);
		int ig = int(255.999 * g);
		int ib = int(255.999 * b);
		
		rayTracePixels[++index] = ir;
		rayTracePixels[++index] = ig;
		rayTracePixels[++index] = ib;
	};

};
