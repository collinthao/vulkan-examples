#pragma once

#include "hittable.h"

class material
{
	public:
	virtual ~material() = default;
	
	virtual bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const
	{
		return false;
	};
};

class lambertian : public material
{
	public:
	lambertian(const glm::vec3& albedo)
	: albedo(albedo) {};

	bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		auto scatter_direction = rec.normal + glm_ext::random_unit_vector();
		
		if (glm_ext::near_zero(scatter_direction))
		{
			scatter_direction = rec.normal;
		};

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	};
	
	private:
	glm::vec3 albedo;
};

class metal : public material
{
	public:
	metal(const glm::vec3& albedo)
	: albedo(albedo) {};

	bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 reflected = reflect(r_in.direction(), rec.normal);
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return true;
	};
	
	private:
	glm::vec3  albedo;
};

