#pragma once

namespace glm_ext
{
	glm::vec3 random()
	{
		return glm::vec3(random_double(), random_double(), random_double());
	};

	glm::vec3 random(double min, double max)
	{
		return glm::vec3(random_double(min, max), random_double(min, max), random_double(min, max));
	};
	
	glm::vec3 random_unit_vector()
	{
		while(true)
		{
			auto p = glm_ext::random(-1, 1);
			auto lensq = p[0] * p[0] + p[1] * p[1] + p[2] * p[2];
			bool b = 1e-160 < lensq && lensq <= 1;
			//std::cout << lensq << '\n';
			if (1e-160 < lensq && lensq <= 1)
			{
				return p/double(sqrt(lensq));
			};
		};
	};
	
	glm::vec3 random_on_hemisphere(const glm::vec3& normal)
	{
		glm::vec3 on_unit_sphere = random_unit_vector();
		if (glm::dot(on_unit_sphere, normal) > 0.0)
		{
			return on_unit_sphere;
		}
		else
		{
			return -on_unit_sphere;
		};
	};
};
