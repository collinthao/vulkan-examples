#pragma once

namespace glm_ext
{
	bool near_zero(glm::vec3& e)
	{
		auto s = 1e-8;
		
		return (std::fabs(e[0]) < s && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s));
	};
		
	float length_squared(glm::vec3& e)
	{
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	};

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

	glm::vec3 random_in_unit_disk()
	{
		while(true)
		{
			auto p = glm::vec3(random_double(-1, 1), random_double(-1, 1), 0);
			if (glm_ext::length_squared(p) < 1)
			{
				return p;
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
	
	glm::vec3 reflect(const glm::vec3& v, const glm::vec3& n)
	{
		return v - 2*(glm::dot(v,n)*n);
	};

	glm::vec3 refract(const glm::vec3& uv, const glm::vec3& n, double etai_over_etat)
	{
		auto cos_theta = std::fmin(glm::dot(-uv, n), 1.0);
		glm::vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
		glm::vec3 r_out_parallel = -sqrt(std::fabs(1. - glm_ext::length_squared(r_out_perp))) * n;
		return r_out_perp + r_out_parallel;
	};

};

